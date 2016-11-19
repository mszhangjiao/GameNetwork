#include "stdafx.h"

NetServer::NetServer(const string& service, int family)
	: NetManager(service, family)
	, m_NewPlayerId(0)
{
	SetDropPacketChance(0.10f);
	SetSimulatedLatency(0.10f);
}

bool NetServer::StaticInit(const string& service, int family)
{
	s_Instance = shared_ptr<NetServer>(new NetServer(service, family));
	return s_Instance->Init();	
}

void NetServer::ProcessPacket(InputBitStream& is, const SockAddrIn& addr)
{
	auto it = m_AddrToClientMap.find(addr);

	if (it == m_AddrToClientMap.end())
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
	auto it = m_AddrToClientMap.find(addr);

	if (it != m_AddrToClientMap.end())
	{
		it->second->GetConnection()->ShowDroppedPacket(is);
	}
}


void NetServer::HandlePacketFromNewClient(InputBitStream& is, const SockAddrIn& addr)
{
	int8_t msgType;
	is.Read(msgType);

	if (msgType == Msg_Hello)
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

		ConnectionPtr newConnPtr = make_shared<Connection>(addr, name, m_NewPlayerId++);
		ClientProxyPtr newClientPtr = make_shared<ClientProxy>(newConnPtr);
		m_AddrToClientMap[addr] = newClientPtr;
		m_IdToClientMap[newConnPtr->GetPlayerId()] = newClientPtr;

		SendWelcomePacket(newClientPtr);
	}
}

void NetServer::SendWelcomePacket(ClientProxyPtr clientPtr)
{
	ConnectionPtr connPtr = clientPtr->GetConnection();
	WelcomeMsg::Send(*connPtr, connPtr->GetPlayerId());
}

void NetServer::ProcessPacket(ClientProxyPtr clientPtr, InputBitStream& is)
{
	int8_t msgType;
	is.Read(msgType);

	switch (msgType)
	{
	case Msg_Hello:
		SendWelcomePacket(clientPtr);
		break;

	case Msg_Ready:
		clientPtr->HandleReadyPacket(is);
		break;

	case Msg_Ack:
		AckMsg::Receive(*clientPtr->GetConnection(), is);
		break;
	}

	clientPtr->GetConnection()->SetLastReceivedPacketTime(TimeUtil::Instance().GetTimef());
}

void NetServer::CheckForDisconnects()
{
	for (auto pair : m_IdToClientMap)
	{
		ClientProxyPtr clientPtr = pair.second;

		if (clientPtr->GetReady() && clientPtr->GetConnection()->GetLastReceivedPacketTime() < TimeUtil::Instance().GetTimef() - cDisconnectTimeoutValue)
		{
			INFO("Disconnected: player id[%d], player name[%s]", clientPtr->GetConnection()->GetPlayerId(), clientPtr->GetConnection()->GetPlayerName().c_str());

			m_IdToClientMap.erase(pair.first);
			break;
		}
	}
}

void NetServer::SendOutgoingPackets()
{
	for (auto pair : m_IdToClientMap)
	{
		ClientProxyPtr clientPtr = pair.second;

		if (clientPtr->GetReady())
		{
			// don't resend message if the recipiant doesn't respond, it may be just disconnected...
			clientPtr->GetConnection()->ProcessTimedOutPackets();
			clientPtr->GetConnection()->ProcessTimedoutAcks();
			clientPtr->GetConnection()->SendHeartbeat();
			clientPtr->GetConnection()->ShowDeliveryStats();
		}
	}
}

// To-do...
void NetServer::HandleConnectionError(const SockAddrIn& sockAddr)
{

}

