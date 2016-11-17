#pragma once

enum NetClientState
{
	Net_None,
	Net_Hello,
	Net_Welcomed
};

struct ReceivedPacket
{
	ReceivedPacket(const InputBitStream& is, const SockAddrIn& addr, float time)
		: m_InputBitStream(is)
		, m_FromAddress(addr)
		, m_ReceivedTime(time)
	{}

	InputBitStream m_InputBitStream;
	SockAddrIn m_FromAddress;
	float m_ReceivedTime;
};

class NetManager
{
public:
	static const int cMaxPacketsPerFrame = 1;
	static const int cPacketBufferSize = 1500;


	static shared_ptr<NetManager> Instance()
	{
		return s_Instance;
	}

	NetManager(const string& service, int family)
		: m_Service(service)
		, m_Family(family)
	{}

	virtual ~NetManager() {}

	virtual bool IsServer() = 0;

	virtual void ProcessPacket(InputBitStream& is, const SockAddrIn& addr) = 0;
	virtual void HandleConnectionError(const SockAddrIn& sockAddr) = 0;

	bool IsOpen()
	{
		return (m_UDPSockPtr != nullptr && m_UDPSockPtr->IsOpen());
	}

	bool Init();
	void ProcessIncomingPackets();
	void SendPacket(const OutputBitStream& os, const SockAddrIn& addr);

protected:
	string m_Service;
	int m_Family;

	// NetManager singleton;
	static shared_ptr<NetManager> s_Instance;

private:
	void ReadIncomingPackets();
	void ProcessQueuedPackets();
	void UpdateBytesSentLastFrame();

	UDPSocketPtr m_UDPSockPtr;
	queue<ReceivedPacket> m_ReceivedPackets;
};

typedef shared_ptr<NetManager> NetManagerPtr;