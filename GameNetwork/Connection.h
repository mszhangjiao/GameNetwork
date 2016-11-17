#pragma once

typedef uint16_t SequenceNumber;

class DeliveryHandler
{
public:
	virtual void onDelivery(int key, bool bSuccessful) = 0;
};

typedef shared_ptr<DeliveryHandler> DeliveryHandlerPtr;

class InFlightPacket
{
public:
	InFlightPacket(DeliveryHandler* handler, SequenceNumber sn)
		: m_DeliveryHandler(handler)
		, m_SequenceNumber(sn)
	{
		m_DispatchTime = TimeUtil::Instance().GetTimef();
	}

	SequenceNumber GetSequenceNumber() const
	{
		return m_SequenceNumber;
	}

	float GetDispatchTime() const
	{
		return m_DispatchTime;
	}

	void UpdateDispatchTime(float time)
	{
		m_DispatchTime = time;
	}

	void HandleDeliveryResult(bool bSuccessful) const
	{
		if (m_DeliveryHandler)
			m_DeliveryHandler->onDelivery(m_SequenceNumber, bSuccessful);
	}

private:
	SequenceNumber m_SequenceNumber;
	float m_DispatchTime;
	DeliveryHandler* m_DeliveryHandler;
};

struct AckRange
{
private:
	SequenceNumber m_Start;
	uint32_t m_Count;

public:
	AckRange()
		: m_Start(0)
		, m_Count(0)
	{}

	AckRange(SequenceNumber start)
		: m_Start(start)
		, m_Count(1)
	{}

	SequenceNumber GetStart() const
	{
		return m_Start;
	}

	uint32_t GetCount() const
	{
		return m_Count;
	}

	bool Extend(SequenceNumber seq)
	{
		if (seq == m_Start + m_Count)
		{
			m_Count++;
			return true;
		}
		else
		{
			return false;
		}
	}

	void Write(OutputBitStream& os);
	void Read(InputBitStream& is);
};

// Connection class guanrantees the packets to be delivered and in order;
// the out-of-order packets will be kept in the queue until the previous packets are received;
// Packet format:
// MessageType: uint8_t
// Reliable: bool [+ SequenceNumber]
// HasAck: bool [+ AckRange]
class Connection : public DeliveryHandler
{
public:
	const float cPacketAckTimeout = 0.4f;
	const float cShowStatsTimeout = 3.f;
	const float cHeartbeatTimeout = 2.f;
	const SequenceNumber cStartSequenceNumber = 0;

	Connection(const SockAddrIn& remoteAddr, const string& playerName, int playerId)
		: m_RemoteAddr(remoteAddr)
		, m_PlayerName(playerName)
		, m_PlayerId(playerId)
		, m_NextOutgoingSequence(cStartSequenceNumber)
		, m_NextExpectedSequence(cStartSequenceNumber)
		, m_DispatchedPackets(0)
		, m_ResentPackets(0)
		, m_AckedPackets(0)
		, m_LastSentAckTime(0.f)
		, m_LastShowStatsTime(0.f)
		, m_LastSentHeartbeatTime(0.f)
		, m_Heartbeat(0)
	{
	}

	const SockAddrIn& GetRemoteAddr() const
	{
		return m_RemoteAddr;
	}

	const string& GetPlayerName() const
	{
		return m_PlayerName;
	}

	uint8_t GetPlayerId() const
	{
		return m_PlayerId;
	}

	void SetPlayerId(int8_t playerId)
	{
		m_PlayerId = playerId;
	}

	void InitHeartbeat(int32_t start)
	{
		m_Heartbeat = start;
	}

	bool IsOpen() const
	{
		return NetManager::Instance()->IsOpen();
	}

	void SendPacket(const OutputBitStream& os)
	{
		NetManager::Instance()->SendPacket(os, m_RemoteAddr);
	}

	// implement DeliveryHandler interface;
	virtual void onDelivery(int key, bool bSuccessful) override;

	SequenceNumber WriteSequence(OutputBitStream& os, bool reliable);
	SequenceNumber WriteReliability(OutputBitStream& os, bool reliable)
	{
		SequenceNumber seq = WriteSequence(os, reliable);
		WriteAckData(os);

		return seq;
	}

	bool ReadAndProcessReliability(InputBitStream& is);
	void ProcessTimedOutPackets();
	void ProcessTimedoutAcks();
	void SaveOutgoingPacket(SequenceNumber seq, const OutputBitStream& os)
	{
		m_OutgoingPackets.emplace(seq, os);
	}

	void SendHeartbeat();
	void ShowDeliveryStats();

	void ShowDroppedPacket(InputBitStream& is) const;
private:
	void WriteAckData(OutputBitStream& os);

	bool ProcessSequence(InputBitStream& is);
	void ProcessAcks(InputBitStream& is);
	void AddPendingAck(SequenceNumber seq);

	typedef map<SequenceNumber, OutputBitStream> SeqToOutputBitStreamMap;
	SeqToOutputBitStreamMap m_OutgoingPackets;

	deque<InFlightPacket> m_InFlightPackets;
	deque<AckRange> m_PendingAcks;

	SockAddrIn m_RemoteAddr;
	string m_PlayerName;
	uint8_t m_PlayerId;

	SequenceNumber m_NextOutgoingSequence;
	SequenceNumber m_NextExpectedSequence;

	int m_DispatchedPackets;
	int m_ResentPackets;
	int m_AckedPackets;

	float m_LastSentAckTime;
	float m_LastShowStatsTime;
	float m_LastSentHeartbeatTime;

	uint32_t m_Heartbeat;
};

typedef shared_ptr<Connection> ConnectionPtr;