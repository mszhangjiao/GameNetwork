#include "stdafx.h"

NetServer::NetServer(const string& service, int family, bool enableHeartbeat)
	: NetManager(service, family)
	, m_NewPlayerId(0)
	, m_SendHeartbeats(enableHeartbeat)
{
	SetDropPacketChance(0.10f);
	SetSimulatedLatency(0.10f);
}

bool NetServer::StaticInit(const string& service, int family, bool enableHeartbeat)
{
	s_Instance = shared_ptr<NetServer>(new NetServer(service, family, enableHeartbeat));
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
		m_AddrToPlayerMap[addr] = newPlayerPtr;
		m_IdToPlayerMap[newPlayerPtr->GetPlayerId()] = newPlayerPtr;

		newConnPtr->InitHeartbeat(newPlayerPtr->GetPlayerId() * 1000);
		SendWelcomePacket(newPlayerPtr);
	}
}

void NetServer::SendWelcomePacket(NetPlayerPtr playerPtr)
{
	ConnectionPtr connPtr = playerPtr->GetConnection();
	connPtr->SetLastReceivedPacketTime(TimeUtil::Instance().GetTimef());
	WelcomeMsg::Send(*connPtr, playerPtr->GetPlayerId());
}

void NetServer::ProcessPacket(NetPlayerPtr playerPtr, InputBitStream& is)
{
	int8_t msgType;
	is.Read(msgType);

	switch (msgType)
	{
	case Msg_Net_Hello:
		SendWelcomePacket(playerPtr);
		break;

	case Msg_Net_Ack:
		AckMsg::Receive(*playerPtr->GetConnection(), is);
		break;
	}

	playerPtr->GetConnection()->SetLastReceivedPacketTime(TimeUtil::Instance().GetTimef());
}

void NetServer::CheckForDisconnects()
{
	for (auto pair : m_IdToPlayerMap)
	{
		NetPlayerPtr playerPtr = pair.second;

		if (playerPtr->GetConnection()->GetLastReceivedPacketTime() < TimeUtil::Instance().GetTimef() - cDisconnectTimeoutValue)
		{
			INFO("Disconnected: player id[%d], player name[%s]", playerPtr->GetPlayerId(), playerPtr->GetPlayerName().c_str());

			m_IdToPlayerMap.erase(pair.first);
			break;
		}
	}
}

void NetServer::SendOutgoingPackets()
{
	if (m_SendHeartbeats)
	{
		for (auto pair : m_IdToPlayerMap)
		{
			NetPlayerPtr playerPtr = pair.second;

			// don't resend message if the recipiant doesn't respond, it may be just disconnected...
			playerPtr->GetConnection()->ProcessTimedOutPackets();
			playerPtr->GetConnection()->ProcessTimedoutAcks();
			playerPtr->GetConnection()->SendHeartbeat();
			playerPtr->GetConnection()->ShowDeliveryStats(playerPtr->GetPlayerName());
		}
	}
}

// To-do...
void NetServer::HandleConnectionError(const SockAddrIn& sockAddr)
{

}

