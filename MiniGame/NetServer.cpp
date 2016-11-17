#include "stdafx.h"

NetServer::NetServer(const string& service, int family)
	: NetManager(service, family)
	, m_NewPlayerId(0)
	, m_LastSentHeartbeatTime(0.f)
{
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

void NetServer::HandlePacketFromNewClient(InputBitStream& is, const SockAddrIn& addr)
{
	int8_t msgType;
	is.Read(msgType);

	if (msgType == Msg_Hello)
	{
		string name;

		// connection has not set up yet, HELLO msg can't be reliable,
		// it relies on repeating sending from the client;
		// read and discard the reliable bit;
		bool bReliable;
		is.Read(bReliable);

		HelloMsg::Read(is, name);

		SockUtil::LogMessage(SockUtil::LL_Info, string("Received HELLO from ") + name);

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
	connPtr->SendWelcomeMsg(connPtr->GetPlayerId());
}

void NetServer::SetReady(ClientProxyPtr clientPtr, InputBitStream& is)
{
	bool bReady;
	ReadyMsg::Read(is, bReady);
	clientPtr->SetReady(bReady);

	SockUtil::LogMessage(SockUtil::LL_Info, clientPtr->GetConnection()->GetPlayerName() + string(" is ready"));
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
	}
}

void NetServer::HandleConnectionError(const SockAddrIn& sockAddr)
{

}

void NetServer::SendOutgoingPackets()
{
	SendHeartbeat();
}

void NetServer::SendHeartbeat()
{
	for (auto pair : m_IdToClientMap)
	{
		ClientProxyPtr clientPtr = pair.second;

		if (clientPtr->GetReady())
		{
			float time = TimeUtil::Instance().GetTimef();

			if (time > m_LastSentHeartbeatTime + cHeartbeatTimeout)
			{
				clientPtr->GetConnection()->SendHeartbeatMsg(time);

				m_LastSentHeartbeatTime = time;
			}
		}
	}
}
