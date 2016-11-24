#include "stdafx.h"

NetServer::NetServer(const string& service, int family, bool enableHeartbeat, bool showDeliveryStats)
	: NetManager(service, family)
	, m_NewPlayerId(0)
	, m_SendHeartbeats(enableHeartbeat)
	, m_ShowDeliveryStats(showDeliveryStats)
{
	SetDropPacketChance(0.10f);
	SetSimulatedLatency(0.10f);
}

bool NetServer::StaticInit(const string& service, int family, bool enableHeartbeat, bool showDeliveryStats)
{
	s_Instance = shared_ptr<NetServer>(new NetServer(service, family, enableHeartbeat, showDeliveryStats));
	return s_Instance->Init();	
}

void NetServer::ProcessPacket(InputBitStream& is, const SockAddrIn& addr)
{
	auto it = m_AddrToPlayerMap.find(addr);

	if (it == m_AddrToPlayerMap.end())
	{
		HandlePacketFromNewClient(is, addr);
	}
	else
	{
		ProcessPacket(it->second, is);
	}
}

void NetServer::ShowDroppedPacket(InputBitStream& is, const SockAddrIn& addr)
{
	auto it = m_AddrToPlayerMap.find(addr);

	if (it != m_AddrToPlayerMap.end())
	{
		it->second->GetConnection()->ShowDroppedPacket(is);
	}
}


void NetServer::HandlePacketFromNewClient(InputBitStream& is, const SockAddrIn& addr)
{
	int8_t msgType;
	is.Read(msgType);

	if (msgType == Msg_Net_Hello)
	{
		string name;

		// connection has not set up yet, HELLO msg can't be reliable,
		// it relies on repeating sending from the client;
		// but the messge format is same with other messages, so skip two bits for reliable and ack flags;
		// read and discard the reliable bit;
		bool bSkip;
		is.Read(bSkip);
		is.Read(bSkip);

		HelloMsg::Read(is, name);

		INFO("Received HELLO from %s", name.c_str());

		ConnectionPtr newConnPtr = make_shared<Connection>(addr);
		NetPlayerPtr newPlayerPtr = make_shared<NetPlayer>(newConnPtr, name, m_NewPlayerId++);
		m_AddrToPlayerMap.emplace(addr, newPlayerPtr);
		m_IdToPlayerMap.emplace(newPlayerPtr->GetId(), newPlayerPtr);

		newConnPtr->InitHeartbeat(newPlayerPtr->GetId() * 1000);
		SendWelcomePacket(newPlayerPtr);
	}
}

void NetServer::SendWelcomePacket(NetPlayerPtr playerPtr)
{
	ConnectionPtr connPtr = playerPtr->GetConnection();
	playerPtr->SetState(Net_Connected);
	connPtr->SetLastReceivedPacketTime(TimeUtil::Instance().GetTimef());
	WelcomeMsg::Send(*connPtr, playerPtr->GetId());
}

void NetServer::ProcessPacket(NetPlayerPtr playerPtr, InputBitStream& is)
{
	uint8_t msgType;
	is.Read(msgType);

	switch (msgType)
	{
	case Msg_Net_Hello:
		SendWelcomePacket(playerPtr);
		break;

	case Msg_Net_Ack:
		AckMsg::Receive(*playerPtr->GetConnection(), is);
		break;

	// process the rest messages in the game layer;
	default:
		Game::Instance()->ProcessPacket(msgType, playerPtr, is);
		break;
	}

	playerPtr->GetConnection()->SetLastReceivedPacketTime(TimeUtil::Instance().GetTimef());
}

void NetServer::DisconnectPlayer(PlayerId playerId)
{
	INFO("%s: player [%d]", __FUNCTION__, playerId);

	auto it = m_IdToPlayerMap.find(playerId);
	if (it != m_IdToPlayerMap.end())
	{
		NetPlayerPtr playerPtr = it->second;
		MatchPtr matchPtr = playerPtr->GetMatch().lock();
		if (matchPtr)
		{
			matchPtr->RemovePlayer(playerId);
		}
	}	
	m_IdToPlayerMap.erase(playerId);

	for (auto pair : m_AddrToPlayerMap)
	{
		NetPlayerPtr playerPtr = pair.second;

		if (playerPtr->GetId() == playerId)
		{
			m_AddrToPlayerMap.erase(pair.first);
			break;
		}
	}
}

void NetServer::CheckForDisconnects()
{
	for (auto pair : m_IdToPlayerMap)
	{
		NetPlayerPtr playerPtr = pair.second;

		if (playerPtr->GetConnection()->GetLastReceivedPacketTime() < TimeUtil::Instance().GetTimef() - cDisconnectTimeoutValue)
		{
			INFO("Disconnected: player id[%d], player name[%s]", playerPtr->GetId(), playerPtr->GetName().c_str());
			DisconnectPlayer(playerPtr->GetId());
			
			break;
		}
	}
}

void NetServer::SendOutgoingPackets()
{
	for (auto pair : m_IdToPlayerMap)
	{
		NetPlayerPtr playerPtr = pair.second;

		// don't resend message if the recipiant doesn't respond, it may be just disconnected...
		playerPtr->GetConnection()->ProcessTimedOutPackets();
		playerPtr->GetConnection()->ProcessTimedoutAcks();
		if (m_SendHeartbeats)
		{
			playerPtr->GetConnection()->SendHeartbeat();
		}

		if (m_ShowDeliveryStats)
		{
			playerPtr->GetConnection()->ShowDeliveryStats(playerPtr->GetName());
		}
	}
}

// To-do...
void NetServer::HandleConnectionError(const SockAddrIn& sockAddr)
{

}