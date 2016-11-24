#pragma once

// defines the server networking features:
// - manages the client proxies/connections on the server;
// - send packets to connected clients;
// - handles received packets;
class NetServer : public NetManager
{
public:
	static bool StaticInit(const string& service, int family, bool enableHeartbeat = true, bool showDeliveryStats = false);
	
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
	void DisconnectPlayer(PlayerId playerId);

	void EnableDeliveryStats(bool enable)
	{
		m_ShowDeliveryStats = enable;
	}

	NetPlayerPtr FindPlayer(PlayerId id)
	{
		if (m_IdToPlayerMap.find(id) != m_IdToPlayerMap.end())
			return m_IdToPlayerMap[id];

		return nullptr;
	}

private:
	NetServer(const string& service, int family, bool enableHeartbeat = true, bool showDeliveryStats = false);

	void SendWelcomePacket(NetPlayerPtr playerPtr);

	typedef unordered_map<PlayerId, NetPlayerPtr> IdToPlayerMap;
	typedef unordered_map<SockAddrIn, NetPlayerPtr> AddrToPlayerMap;

	AddrToPlayerMap m_AddrToPlayerMap;
	IdToPlayerMap m_IdToPlayerMap;

	PlayerId m_NewPlayerId;

	bool m_SendHeartbeats;
	bool m_ShowDeliveryStats;
};