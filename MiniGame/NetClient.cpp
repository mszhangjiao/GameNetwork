#include "stdafx.h"

NetClient::NetClient(const string& serverIP, const string& service, int family, const string& playerName)
	: NetManager(service, family)
	, m_ServerIP(serverIP)
	, m_PlayerName(playerName)
	, m_ServerConnectionPtr(nullptr)
{
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

void NetClient::ProcessPacket(InputBitStream& is, const SockAddrIn& addr)
{
	// ignore msgs from unknown hosts;
	if (addr != m_ServerConnectionPtr->GetRemoteAddr())
	{
		return;
	}

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
			m_ServerConnectionPtr->HandleHeartbeatPacket(is);
		}
		break;
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
		break;
	}
}

void NetClient::CheckForDisconnects()
{

}

void NetClient::SendHelloPacket()
{
	m_ServerConnectionPtr->SendHelloMsg(m_PlayerName);
}

void NetClient::SendInputPacket()
{

}

void NetClient::HandleWelcomePacket(InputBitStream& is)
{
	int8_t playerId;
	WelcomeMsg::Read(is, playerId);

	m_ServerConnectionPtr->SetPlayerId(playerId);
	m_NetState = Net_Welcomed;

	SockUtil::LogMessage(SockUtil::LL_Info, string("Welcomed by the server, playerId: ") + to_string(playerId));

	m_ServerConnectionPtr->SendReadyMsg(true);
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

void NetClient::UpdateSendingInput()
{

}
