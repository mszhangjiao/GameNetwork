#include "stdafx.h"

bool InFlightPacketLessThan(const InFlightPacket& a, const InFlightPacket& b)
{
	return a.GetDispatchTime() < b.GetDispatchTime();
}

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

// return whether we need to drop this packet;
// true: keep it
// false: drop it
bool Connection::ReadAndProcessReliability(InputBitStream& is)
{
	bool reliable;
	is.Read(reliable);

	bool isValid = true;

	if (reliable)
	{
		isValid = ProcessSequence(is);
	}

	ProcessAcks(is);

	return isValid;
}

SequenceNumber Connection::WriteSequence(OutputBitStream& os, bool reliable)
{
	SequenceNumber seq = 0;

	os.Write(reliable);
	if (reliable)
	{
		seq = m_NextOutgoingSequence++;
		os.Write(seq);

		++m_DispatchedPackets;

		m_InFlightPackets.emplace_back(this, seq);
	}

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
// false: skip it
bool Connection::ProcessSequence(InputBitStream& is)
{
	SequenceNumber seq;
	is.Read(seq);

	char info[256];
	sprintf_s(info, "Received seq: (%d), Expecting: (%d)", seq, m_NextExpectedSequence);
	LogUtil::LogMessage(LL_Debug, string(info));

	if (seq == m_NextExpectedSequence)
	{
		m_NextExpectedSequence++;

		AddPendingAck(seq);

		return true;
	}

	// ack seq will automatically nak the missing packets, 
	// we also ack the packet with smaller sequence number,
	// because each packet will be resent before it's acked;
	AddPendingAck(seq);

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
		vector<InFlightPacket> packetsToAdd;

		char info[256];
		sprintf_s(info, "Receive Acks: start(%d), end(%d)", ackStart, ackEnd);
		LogUtil::LogMessage(LL_Debug, string(info));

		// handle the sequence number equal or smaller than the first packet in the queue;
		while (ackStart < ackEnd && !m_InFlightPackets.empty())
		{
			auto& packet = m_InFlightPackets.front();
			SequenceNumber packetSeq = packet.GetSequenceNumber();
			if (packetSeq == ackStart)
			{
				packet.HandleDeliveryResult(true);
				m_InFlightPackets.pop_front();
				++ackStart;
			}
			else if (packetSeq > ackStart)
			{
				++ackStart;
			}
			else
			{
				// handle this case in next loop;
				break;
			}
		}

		for (auto& packet : m_InFlightPackets)
		{
			if (packet.GetSequenceNumber() < ackStart)
			{
				if (packet.GetDispatchTime() < TimeUtil::Instance().GetTimef() - cPacketAckTimeout)
				{
					packet.HandleDeliveryResult(false);

					// update the timestamp;
					packet.UpdateDispatchTime(TimeUtil::Instance().GetTimef());
				}
			}
		}		
	}
}

// resending the in-flight packets when they are old enough
// even if there is no nack
void Connection::ProcessTimedOutPackets()
{
	for (auto& packet : m_InFlightPackets)
	{
		if (packet.GetDispatchTime() < TimeUtil::Instance().GetTimef() - cPacketAckTimeout)
		{
			packet.HandleDeliveryResult(false);

			// update the timestamp;
			packet.UpdateDispatchTime(TimeUtil::Instance().GetTimef());
		}
	}
}

// we should send the pending acks in time;
// normally we pigback the Acks in the outgoing packets,
// this is the only case that we send a sole AckMsg, 
// just in case the pending acks are not sent in time;
void Connection::ProcessTimedoutAcks()
{
	float time = TimeUtil::Instance().GetTimef();

	if (m_PendingAcks.size() > 0
		&& time > m_LastSentAckTime + cPacketAckTimeout / 2.f)
	{
		AckMsg::Send(*this);

		m_LastSentAckTime = time;
	}
}

void Connection::ShowDeliveryStats()
{
	float time = TimeUtil::Instance().GetTimef();

	if (time > m_LastShowStatsTime + cShowStatsTimeout)
	{
		char stats[256];
		sprintf_s(stats, "Stats for [%6s]: heartbeat[%4d], resent rate[%6.2f%s], dispatched[%4d], acked[%4d], resent[%4d]", 
			m_PlayerName.c_str(), m_Heartbeat,
			static_cast<float>(m_ResentPackets) / m_DispatchedPackets * 100.f, "%",
			m_DispatchedPackets, m_AckedPackets, m_ResentPackets);

		LogUtil::LogMessage(LL_Info, string(stats));

		m_LastShowStatsTime = time;
	}
}

void Connection::ShowDroppedPacket(InputBitStream& is) const
{
	uint8_t msgType;
	bool reliable;
	bool hasAck;
	SequenceNumber seq;

	char info[1024];
	string msg;

	is.Read(msgType);
	is.Read(reliable);

	sprintf_s(info, "Dropped Packet: msgType(%d), reliable(%d), ", msgType, reliable);
	msg = string(info);

	if (reliable)
	{
		is.Read(seq);
		sprintf_s(info, "seq(%d), ", seq);
		msg += string(info);
	}

	is.Read(hasAck);

	sprintf_s(info, "hasAck(%d), ", hasAck);
	msg += string(info);

	if (hasAck)
	{
		AckRange range;
		range.Read(is);

		sprintf_s(info, "AckRange(%d, %d), ", range.GetStart(), range.GetCount());
		msg += string(info);
	}

	LogUtil::LogMessage(LL_Debug, msg);
}


// simply resend the packet if the packet is dropped;
void Connection::onDelivery(int key, bool bSuccessful)
{
	char info[64];
	sprintf_s(info, "Delivery: seq(%d), success(%d)", key, bSuccessful);
	LogUtil::LogMessage(LL_Debug, string(info));

	if (bSuccessful)
		++m_AckedPackets;
	else
		++m_ResentPackets;

	auto& pair = m_OutgoingPackets.find(key);
	if (pair != m_OutgoingPackets.end())
	{
		if (!bSuccessful)
		{
			char msg[256];
			sprintf_s(msg, "Resending packet: %d", key);
			LogUtil::LogMessage(LL_Debug, string(msg));

			auto& os = pair->second;
			SendPacket(os);
		}
		else
		{
			m_OutgoingPackets.erase(key);
		}
	}
}

void Connection::SendHeartbeat()
{
	float time = TimeUtil::Instance().GetTimef();

	if (time > m_LastSentHeartbeatTime + cHeartbeatTimeout)
	{
		HeartbeatMsg::Send(*this, m_Heartbeat++);

		m_LastSentHeartbeatTime = time;
	}
}
