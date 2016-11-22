#include "stdafx.h"

NetClient::NetClient(const string& serverIP, const string& service, int family, const string& playerName)
	: NetManager(service, family)
	, m_ServerIP(serverIP)
{
	SetDropPacketChance(0.1f);
	SetSimulatedLatency(0.1f);

	m_LocalPlayerPtr = make_shared<NetPlayer>(playerName);
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
		ConnectionPtr connPtr = make_shared<Connection>(m_ServerSockAddr);
		m_LocalPlayerPtr->SetConnection(connPtr);

		m_LocalPlayerPtr->SetState(Net_Hello);
		m_TimeOfLastHello = 0.f;
	}

	return result;
}

void NetClient::ShutdownAndClose()
{
	NetManager::ShutdownAndClose();

	m_ServerSockAddr = SockAddrIn::NullAddr;
	m_LocalPlayerPtr->SetConnection(nullptr);
}


void NetClient::ProcessPacket(InputBitStream& is, const SockAddrIn& addr)
{
	// ignore msgs from unknown hosts;
	if (addr != m_LocalPlayerPtr->GetConnection()->GetRemoteAddr())
	{
		return;
	}

	m_LocalPlayerPtr->GetConnection()->SetLastReceivedPacketTime(TimeUtil::Instance().GetTimef());

	uint8_t msgType;
	is.Read(msgType);

	switch (msgType)
	{
	case Msg_Net_Welcome:
		if (m_LocalPlayerPtr->GetState() == Net_Hello)
		{
			HandleWelcomePacket(is);
		}
		break;

	case Msg_Net_Heartbeat:
		if (m_LocalPlayerPtr->GetState() == Net_Connected)
		{
			uint32_t heartbeat;
			if (HeartbeatMsg::Receive(*m_LocalPlayerPtr->GetConnection(), is, heartbeat))
			{
				INFO("Server heartbeat: [%4d]", heartbeat);
			}
		}
		break;

	case Msg_Net_Ack:
		AckMsg::Receive(*m_LocalPlayerPtr->GetConnection(), is);
	}
}

void NetClient::HandleConnectionError(const SockAddrIn& sockAddr)
{

}

void NetClient::SendOutgoingPackets()
{
	switch (m_LocalPlayerPtr->GetState())
	{
	case Net_Hello:
		UpdateSendingHello();
		break;

	case Net_Connected:
		UpdateSendingInput();
		// don't resend message if the recipiant doesn't respond, it may be just disconnected...
		m_LocalPlayerPtr->GetConnection()->ProcessTimedOutPackets();
		m_LocalPlayerPtr->GetConnection()->ProcessTimedoutAcks();
		break;
	}
}

// shutdown the network with 
void NetClient::CheckForDisconnects()
{	
	if (m_LocalPlayerPtr->GetState() == Net_Connected && m_LocalPlayerPtr->GetConnection()->GetLastReceivedPacketTime() < TimeUtil::Instance().GetTimef() - cDisconnectTimeoutValue)
	{
		INFO("Disconnected: player id[%d], player name[%s]", m_LocalPlayerPtr->GetPlayerId(), m_LocalPlayerPtr->GetPlayerName().c_str());

		ShutdownAndClose();
		m_LocalPlayerPtr->SetState(Net_Disconnected);
	}
}

void NetClient::SendHelloPacket()
{
	HelloMsg::Send(*m_LocalPlayerPtr->GetConnection(), m_LocalPlayerPtr->GetPlayerName());
}

void NetClient::HandleWelcomePacket(InputBitStream& is)
{
	int8_t playerId;
	if (WelcomeMsg::Receive(*m_LocalPlayerPtr->GetConnection(), is, playerId))
	{
		m_LocalPlayerPtr->SetPlayerId(playerId);
		m_LocalPlayerPtr->SetState(Net_Connected);

		INFO("Welcomed by the server, playerId: %d", playerId);
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
	m_LocalPlayerPtr->GetConnection()->ShowDroppedPacket(is);
}

// To-do...
void NetClient::UpdateSendingInput()
{

}

void NetClient::SendInputPacket()
{

}
