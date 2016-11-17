#include "stdafx.h"

void AckRange::Write(OutputBitStream& os)
{
	os.Write(m_Start);
	bool hasCount = m_Count > 1;
	os.Write(hasCount);
	if (hasCount)
	{
		// write m_Count - 1, because the count is 1 without count field;
		uint8_t writeCount = (m_Count - 1 > 255) ? 255 : m_Count - 1;
		os.Write(writeCount);
	}
}

void AckRange::Read(InputBitStream& is)
{
	is.Read(m_Start);
	bool hasCount;
	is.Read(hasCount);

	if (hasCount)
	{
		uint8_t readCount;
		is.Read(readCount);
		m_Count = readCount + 1;
	}
	else
	{
		m_Count = 1;
	}
}

SequenceNumber Connection::WriteReliablility(OutputBitStream& os)
{
	SequenceNumber seq = WriteSequence(os);
	WriteAckData(os);

	return seq;
}

// return whether we need to drop this packet;
// true: keep it
// false: drop it
bool Connection::ReadAndProcessReliability(InputBitStream& is)
{
	bool result = ProcessSequence(is);
	ProcessAcks(is);
	return result;
}

SequenceNumber Connection::WriteSequence(OutputBitStream& os)
{
	SequenceNumber seq = m_NextOutgoingSequence++;
	os.Write(seq);

	++m_DispatchedPackets;

	m_InFlightPackets.emplace_back(seq);
	return seq;
}

void Connection::WriteAckData(OutputBitStream& os)
{
	// if we have pending acks, we will write the first range;
	// otherwise, write a 0 bit;
	bool hasAcks = (m_PendingAcks.size() > 0);

	os.Write(hasAcks);

	if (hasAcks)
	{
		m_PendingAcks.front().Write(os);
		m_PendingAcks.pop_front();
	}
}

// return whether we need to drop this packet;
// true: keep it
// false: drop it
bool Connection::ProcessSequence(InputBitStream& is)
{
	SequenceNumber seq;
	is.Read(seq);

	if (seq == m_NextExpectedSequence)
	{
		m_NextExpectedSequence++;

		AddPendingAck(seq);

		return true;
	}
	else if (seq > m_NextExpectedSequence)
	{
		// ack seq will automatically nak the missing packets, 
		// the sender will process it;
		AddPendingAck(seq);
	}

	return false;
}

void Connection::AddPendingAck(SequenceNumber seq)
{
	if (m_PendingAcks.size() == 0)
	{
		m_PendingAcks.emplace_back(seq);
	}
	else if (!m_PendingAcks.back().Extend(seq))
	{
		m_PendingAcks.emplace_back(seq);
	}
}

void Connection::ProcessAcks(InputBitStream& is)
{
	bool hasAcks;
	is.Read(hasAcks);

	if (hasAcks)
	{
		AckRange ackRange;
		ackRange.Read(is);

		SequenceNumber ackStart = ackRange.GetStart();
		uint32_t ackEnd = ackStart + ackRange.GetCount();
		while (ackStart < ackEnd && !m_InFlightPackets.empty())
		{
			const auto& packet = m_InFlightPackets.front();
			SequenceNumber packetSeq = packet.GetSequenceNumber();
			if (packetSeq < ackStart)
			{
				packet.HandleDeliveryResult(false);
				m_InFlightPackets.pop_front();
				++m_DroppedPackets;
			}
			else if (packetSeq == ackStart)
			{
				packet.HandleDeliveryResult(true);
				m_InFlightPackets.pop_front();
				++m_DeliveredPackets;
				++ackStart;
			}
			else
			{
				++ackStart;
			}
		}
	}
}

void Connection::ProcessTimedOutPackets()
{
	float timeoutTime = TimeUtil::Instance().GetTimef() - cPacketAckTimeout;

	while (!m_InFlightPackets.empty())
	{
		const auto& packet = m_InFlightPackets.front();

		if (packet.GetDispatchTime() < timeoutTime)
		{
			packet.HandleDeliveryResult(false);
			m_InFlightPackets.pop_front();
		}
		else
		{
			// we can skip the rest because the in-flight packets are ordered in time,
			break;
		}
	}
}

// simply resend the packet if the packet is dropped;
void Connection::onDelivery(int key, bool bSuccessful)
{
	auto& pair = m_OutgoingPackets.find(key);
	if (pair != m_OutgoingPackets.end())
	{
		if (!bSuccessful)
		{
			auto& os = pair->second;
			//SendPacket(os);
		}

		m_OutgoingPackets.erase(key);
	}
}

void Connection::HandleHeartbeatPacket(InputBitStream& is)
{
	bool reliable;
	is.Read(reliable);

	if (reliable)
	{
		ReadAndProcessReliability(is);
	}

	float time;
	HeartbeatMsg::Read(is, time);
	SockUtil::LogMessage(SockUtil::LL_Info, string("Server heartbeat: ") + to_string(static_cast<int>(time)));
}

bool Connection::HandleReadyPacket(InputBitStream& is)
{
	bool reliable;
	is.Read(reliable);

	if (reliable)
	{
		ReadAndProcessReliability(is);
	}

	bool ready;
	ReadyMsg::Read(is, ready);

	SockUtil::LogMessage(SockUtil::LL_Info, m_PlayerName + " is ready: " + to_string(ready));

	return ready;
}

void Connection::HandleScoreState(InputBitStream& is)
{

}