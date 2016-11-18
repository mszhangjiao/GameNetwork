#include "stdafx.h"

NetClient::NetClient(const string& serverIP, const string& service, int family, const string& playerName)
	: NetManager(service, family)
	, m_ServerIP(serverIP)
	, m_PlayerName(playerName)
	, m_ServerConnectionPtr(nullptr)
{
	SetDropPacketChance(0.1f);
	SetSimulatedLatency(0.1f);
}

bool NetClient::StaticInit(const string& serverIP, const string& service, int family, const string& playerName)
{
	s_Instance.reset(new NetClient(serverIP, service, family, playerName));
	return Instance()->Init();
}

bool NetClient::Init()
{
	bool result = NetManager::Init();
	if (result)
	{
		result = m_ServerSockAddr.CreateFrom(m_ServerIP.c_str(), m_Service.c_str());
	}

	if (result)
	{
		m_ServerConnectionPtr.reset(new Connection(m_ServerSockAddr, m_PlayerName, 0));
		m_NetState = Net_Hello;
		m_TimeOfLastHello = 0.f;
	}

	return result;
}

void NetClient::ShutdownAndClose()
{
	NetManager::ShutdownAndClose();

	m_ServerSockAddr = SockAddrIn::NullAddr;
	m_ServerConnectionPtr = nullptr;
}


void NetClient::ProcessPacket(InputBitStream& is, const SockAddrIn& addr)
{
	// ignore msgs from unknown hosts;
	if (addr != m_ServerConnectionPtr->GetRemoteAddr())
	{
		return;
	}

	m_ServerConnectionPtr->SetLastReceivedPacketTime(TimeUtil::Instance().GetTimef());

	uint8_t msgType;
	is.Read(msgType);

	switch (msgType)
	{
	case Msg_Welcome:
		if (m_NetState == Net_Hello)
		{
			HandleWelcomePacket(is);
		}
		break;

	case Msg_Heartbeat:
		if (m_NetState == Net_Welcomed)
		{
			uint32_t heartbeat;
			if (HeartbeatMsg::Receive(*m_ServerConnectionPtr, is, heartbeat))
			{
				char info[64];
				sprintf_s(info, "Server heartbeat: [%4d]", heartbeat);
				LogUtil::LogMessage(LL_Info, string(info));
			}
		}
		break;

	case Msg_Ack:
		AckMsg::Receive(*m_ServerConnectionPtr, is);
	}
}

void NetClient::HandleConnectionError(const SockAddrIn& sockAddr)
{

}

void NetClient::SendOutgoingPackets()
{
	switch (m_NetState)
	{
	case Net_Hello:
		UpdateSendingHello();
		break;

	case Net_Welcomed:
		UpdateSendingInput();
		// don't resend message if the recipiant doesn't respond, it may be just disconnected...
		m_ServerConnectionPtr->ProcessTimedOutPackets();
		m_ServerConnectionPtr->ProcessTimedoutAcks();
		break;
	}
}

// shutdown the network with 
void NetClient::CheckForDisconnects()
{	
	if (m_NetState == Net_Welcomed && m_ServerConnectionPtr->GetLastReceivedPacketTime() < TimeUtil::Instance().GetTimef() - cDisconnectTimeoutValue)
	{
		char info[64];
		sprintf_s(info, "Disconnected: player id[%d], player name[%s]", m_ServerConnectionPtr->GetPlayerId(), m_ServerConnectionPtr->GetPlayerName().c_str());
		LogUtil::LogMessage(LL_Info, info);

		ShutdownAndClose();
		m_NetState = Net_Disconnected;
	}
}

void NetClient::SendHelloPacket()
{
	//m_ServerConnectionPtr->SendHelloMsg(m_PlayerName);
	HelloMsg::Send(*m_ServerConnectionPtr, m_PlayerName);
}

void NetClient::HandleWelcomePacket(InputBitStream& is)
{
	int8_t playerId;
	if (WelcomeMsg::Receive(*m_ServerConnectionPtr, is, playerId))
	{
		m_ServerConnectionPtr->SetPlayerId(playerId);
		m_NetState = Net_Welcomed;

		LogUtil::LogMessage(LL_Info, string("Welcomed by the server, playerId: ") + to_string(playerId));

		ReadyMsg::Send(*m_ServerConnectionPtr, true);
	}
}

void NetClient::UpdateSendingHello()
{
	float time = TimeUtil::Instance().GetTimef();
	if (time > m_TimeOfLastHello + cHelloTimeout)
	{
		SendHelloPacket();
		m_TimeOfLastHello = time;
	}
}

void NetClient::ShowDroppedPacket(InputBitStream& is, const SockAddrIn& addr)
{
	m_ServerConnectionPtr->ShowDroppedPacket(is);
}

// To-do...
void NetClient::UpdateSendingInput()
{

}

void NetClient::SendInputPacket()
{

}
