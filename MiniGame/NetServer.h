#pragma once

class NetServer : public NetManager
{
public:
	static bool StaticInit(const string& service, int family);
	
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
	void ProcessPacket(ClientProxyPtr clientPtr, InputBitStream& is);

	virtual void ShowDroppedPacket(InputBitStream& is, const SockAddrIn& addr) override;

	//void CheckForDisconnects();

private:
	NetServer(const string& service, int family);

	void SendWelcomePacket(ClientProxyPtr clientPtr);

	typedef unordered_map<int, ClientProxyPtr> IdToClientMap;
	typedef unordered_map<SockAddrIn, ClientProxyPtr> AddrToClientMap;

	AddrToClientMap m_AddrToClientMap;
	IdToClientMap m_IdToClientMap;

	uint8_t m_NewPlayerId;
};