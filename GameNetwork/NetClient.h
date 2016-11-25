#pragma once

// defines the client networking features:
// - set up connection with server, 
// - maintains client network states;
// - handles received packets;
class NetClient : public NetManager
{
public:
	const float cHelloTimeout = 1.f;

	static NetClient* Instance()
	{
		return dynamic_cast<NetClient *>(NetManager::Instance().get());
	}

	static bool StaticInit(const string& serverIP, const string& service, int family, const string& playerName, bool enableHeartbeat = true);

	virtual bool IsServer() override
	{
		return false;
	}

	NetPlayerPtr GetLocalPlayerPtr() const
	{
		return m_LocalPlayerPtr;
	}

	virtual void ProcessPacket(InputBitStream& is, const SockAddrIn& addr) override;
	virtual void HandleConnectionError(const SockAddrIn& sockAddr) override;

	void SendOutgoingPackets();
	void CheckForDisconnects();
	void ShutdownAndClose();

	virtual void ShowDroppedPacket(InputBitStream& is, const SockAddrIn& addr) override;

private:
	NetClient(const string& serverIP, const string& service, int family, const string& playerName, bool enableHeartbeat = true);
	bool Init();

	// Packet sending functions;
	void SendHelloPacket();
	void SendInputPacket();

	// Packets handling functions;
	void HandleWelcomePacket(InputBitStream& is);

	// Update functions;
	void UpdateSendingHello();
	void UpdateSendingInput();	

	string m_ServerIP;
	SockAddrIn m_ServerSockAddr;
	NetPlayerPtr m_LocalPlayerPtr;

	float m_TimeOfLastHello;
	bool m_SendHeartbeats;
};