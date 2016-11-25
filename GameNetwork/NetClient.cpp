#include "stdafx.h"

NetClient::NetClient(const string& serverIP, const string& service, int family, const string& playerName, bool enableHeartbeat)
	: NetManager(service, family)
	, m_ServerIP(serverIP)
	, m_SendHeartbeats(enableHeartbeat)
{
	SetDropPacketChance(0.1f);
	SetSimulatedLatency(0.1f);

	m_LocalPlayerPtr = make_shared<NetPlayer>(playerName);
}

bool NetClient::StaticInit(const string& serverIP, const string& service, int family, const string& playerName, bool enableHeartbeat)
{
	s_Instance.reset(new NetClient(serverIP, service, family, playerName, enableHeartbeat));
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
		uint32_t heartbeat;
		if (HeartbeatMsg::Receive(*m_LocalPlayerPtr->GetConnection(), is, heartbeat))
		{
			VERBO("Server heartbeat: [%4d]", heartbeat);
		}
		break;

	case Msg_Net_Ack:
		AckMsg::Receive(*m_LocalPlayerPtr->GetConnection(), is);
		break;

	default:
		Game::Instance()->ProcessPacket(msgType, m_LocalPlayerPtr, is);
		break;
	}
}

void NetClient::HandleConnectionError(const SockAddrIn& sockAddr)
{

}

void NetClient::SendOutgoingPackets()
{
	NetState state = m_LocalPlayerPtr->GetState();
	if (state == Net_Hello)
	{
		UpdateSendingHello();
	}
	else if (state >= Net_Connected && state < Net_Disconnected)
	{
		m_LocalPlayerPtr->GetConnection()->ProcessTimedOutPackets();
		m_LocalPlayerPtr->GetConnection()->ProcessTimedoutAcks();

		if (m_SendHeartbeats)
		{
			m_LocalPlayerPtr->GetConnection()->SendHeartbeat();
		}
	}
}

// shutdown the network with 
void NetClient::CheckForDisconnects()
{	
	if (m_LocalPlayerPtr->IsConnected() && m_LocalPlayerPtr->GetConnection()->GetLastReceivedPacketTime() < TimeUtil::Instance().GetTimef() - cDisconnectTimeoutValue)
	{
		INFO("Disconnected: player id[%d], player name[%s]", m_LocalPlayerPtr->GetId(), m_LocalPlayerPtr->GetName().c_str());

		ShutdownAndClose();
		m_LocalPlayerPtr->SetState(Net_Disconnected);
	}
}

void NetClient::SendHelloPacket()
{
	HelloMsg::Send(*m_LocalPlayerPtr->GetConnection(), m_LocalPlayerPtr->GetName());
}

void NetClient::HandleWelcomePacket(InputBitStream& is)
{
	PlayerId playerId;
	if (WelcomeMsg::Receive(*m_LocalPlayerPtr->GetConnection(), is, playerId))
	{
		m_LocalPlayerPtr->SetId(playerId);
		m_LocalPlayerPtr->SetState(Net_Connected);

		INFO("Welcomed by the server, playerId: %d, request to find a match", playerId);
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
