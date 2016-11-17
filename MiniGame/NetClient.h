#pragma once

class NetClient : public NetManager
{
public:
	const float cHelloTimeout = 1.f;

	static NetClient* Instance()
	{
		return dynamic_cast<NetClient *>(NetManager::Instance().get());
	}

	static bool StaticInit(const string& serverIP, const string& service, int family, const string& playerName);

	virtual bool IsServer() override
	{
		return false;
	}

	NetClientState GetClientState()
	{
		return m_NetState;
	}

	virtual void ProcessPacket(InputBitStream& is, const SockAddrIn& addr) override;
	virtual void HandleConnectionError(const SockAddrIn& sockAddr) override;

	void SendOutgoingPackets();
	void CheckForDisconnects();

private:
	NetClient(const string& serverIP, const string& service, int family, const string& playerName);
	bool Init();

	// Packet sending functions;
	void SendHelloPacket();
	void SendInputPacket();

	// Packets handling functions;
	void HandleWelcomePacket(InputBitStream& is);
	void HandleHeartbeatPacket(InputBitStream& is);
	void HandleStatePacket(InputBitStream& is);

	void HandleScoreState(InputBitStream& is);

	// Update functions;
	void UpdateSendingHello();
	void UpdateSendingInput();	

	SockAddrIn m_ServerSockAddr;
	string m_ServerIP;
	string m_PlayerName;
	NetClientState m_NetState;
	float m_TimeOfLastHello;

	ConnectionPtr m_ServerConnectionPtr;
};