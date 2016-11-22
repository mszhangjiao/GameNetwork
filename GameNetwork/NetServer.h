#pragma once

// defines the server networking features:
// - manages the client proxies/connections on the server;
// - send packets to connected clients;
// - handles received packets;
class NetServer : public NetManager
{
public:
	static bool StaticInit(const string& service, int family, bool enableHeartbeat);
	
	static NetServer* Instance()
	{
		return dynamic_cast<NetServer *>(NetManager::Instance().get());
	}

	virtual bool IsServer() override
	{
		return true;
	}

	virtual void ProcessPacket(InputBitStream& is, const SockAddrIn& addr) override;
	virtual void HandleConnectionError(const SockAddrIn& sockAddr) override;

	void SendOutgoingPackets();
	void HandlePacketFromNewClient(InputBitStream& is, const SockAddrIn& addr);
	void ProcessPacket(NetPlayerPtr playerPtr, InputBitStream& is);

	virtual void ShowDroppedPacket(InputBitStream& is, const SockAddrIn& addr) override;
	void CheckForDisconnects();

private:
	NetServer(const string& service, int family, bool enableHeartbeat = false);

	void SendWelcomePacket(NetPlayerPtr playerPtr);

	typedef unordered_map<int, NetPlayerPtr> IdToPlayerMap;
	typedef unordered_map<SockAddrIn, NetPlayerPtr> AddrToPlayerMap;

	AddrToPlayerMap m_AddrToPlayerMap;
	IdToPlayerMap m_IdToPlayerMap;

	uint8_t m_NewPlayerId;
	bool m_SendHeartbeats;
};