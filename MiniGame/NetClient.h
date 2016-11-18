#pragma once

enum NetClientState
{
	Net_None,
	Net_Hello,
	Net_Welcomed,
	Net_Disconnected,
};

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

	static bool StaticInit(const string& serverIP, const string& service, int family, const string& playerName);

	virtual bool IsServer() override
	{
		return false;
	}

	NetClientState GetClientState()
	{
		return m_NetState;
	}

	bool IsClientDisconnected()
	{
		return m_NetState == Net_Disconnected;
	}

	virtual void ProcessPacket(InputBitStream& is, const SockAddrIn& addr) override;
	virtual void HandleConnectionError(const SockAddrIn& sockAddr) override;

	void SendOutgoingPackets();
	void CheckForDisconnects();
	void ShutdownAndClose();

	virtual void ShowDroppedPacket(InputBitStream& is, const SockAddrIn& addr) override;

private:
	NetClient(const string& serverIP, const string& service, int family, const string& playerName);
	bool Init();

	// Packet sending functions;
	void SendHelloPacket();
	void SendInputPacket();

	// Packets handling functions;
	void HandleWelcomePacket(InputBitStream& is);

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