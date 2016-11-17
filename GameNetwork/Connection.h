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
	InFlightPacket(SequenceNumber sn)
		: m_SequenceNumber(sn)
	{}

	SequenceNumber GetSequenceNumber() const
	{
		return m_SequenceNumber;
	}

	float GetDispatchTime() const
	{
		return m_DispatchTime;
	}

	void SetDeliveryHandler(DeliveryHandlerPtr handler)
	{
		m_DeliveryHandler = handler;
	}

	DeliveryHandlerPtr GetDeliveryHandler()
	{
		return m_DeliveryHandler;
	}

	void HandleDeliveryResult(bool bSuccessful) const
	{
		if (m_DeliveryHandler)
			m_DeliveryHandler->onDelivery(m_SequenceNumber, bSuccessful);
	}

private:
	SequenceNumber m_SequenceNumber;
	float m_DispatchTime;
	DeliveryHandlerPtr m_DeliveryHandler;
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
class Connection : public DeliveryHandler
{
public:
	const float cPacketAckTimeout = 0.5f;

	Connection(const SockAddrIn& remoteAddr, const string& playerName, int playerId)
		: m_RemoteAddr(remoteAddr)
		, m_PlayerName(playerName)
		, m_PlayerId(playerId)
		, m_NextOutgoingSequence(0)
		, m_NextExpectedSequence(0)
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

	bool IsOpen() const
	{
		return NetManager::Instance()->IsOpen();
	}

	void SendPacket(const OutputBitStream& os)
	{
		NetManager::Instance()->SendPacket(os, m_RemoteAddr);
	}

	void SendHelloMsg(const string& playerName)
	{
		OutputBitStream os;

		os.Write(HelloMsg::GetMessageType());
		os.Write(HelloMsg::IsReliable());

		if (HelloMsg::IsReliable())
		{
			WriteReliablility(os);
		}

		os.Append(HelloMsg::Write(playerName));

		SendPacket(os);
	}

	void SendWelcomeMsg(uint8_t playerId)
	{
		OutputBitStream os;

		os.Write(WelcomeMsg::GetMessageType());
		os.Write(WelcomeMsg::IsReliable());

		if (WelcomeMsg::IsReliable())
		{
			WriteReliablility(os);
		}

		os.Append(WelcomeMsg::Write(playerId));

		SendPacket(os);
	}

	void SendReadyMsg(bool bReady)
	{
		OutputBitStream os;

		os.Write(ReadyMsg::GetMessageType());
		os.Write(ReadyMsg::IsReliable());

		if (ReadyMsg::IsReliable())
		{
			WriteReliablility(os);
		}

		os.Append(ReadyMsg::Write(bReady));

		SendPacket(os);
	}

	void SendHeartbeatMsg(float time)
	{
		OutputBitStream os;

		os.Write(HeartbeatMsg::GetMessageType());
		os.Write(HeartbeatMsg::IsReliable());

		if (HeartbeatMsg::IsReliable())
		{
			WriteReliablility(os);
		}

		os.Append(HeartbeatMsg::Write(time));

		SendPacket(os);
	}

	// implement DeliveryHandler interface;
	virtual void onDelivery(int key, bool bSuccessful) override;

	SequenceNumber WriteReliablility(OutputBitStream& os);
	bool ReadAndProcessReliability(InputBitStream& is);
	void ProcessTimedOutPackets();
	void SaveOutgoingPacket(SequenceNumber seq, const OutputBitStream& os)
	{
		m_OutgoingPackets.emplace(seq, os);
	}

	void HandleHeartbeatPacket(InputBitStream& is);
	void HandleScoreState(InputBitStream& is);
	bool HandleReadyPacket(InputBitStream& is);

private:
	SequenceNumber WriteSequence(OutputBitStream& os);
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
	int m_DroppedPackets;
	int m_DeliveredPackets;
};

typedef shared_ptr<Connection> ConnectionPtr;