#pragma once

class Connection;

// implement Message using variadic template;
// each message type is defined with variadic template parameters,
// it uses OutputBitStream and InputBitStream for output and input;
// it calls Connection functions to write and read reliability data inside a packet;

// Packet format:
// MessageType: uint8_t
// Reliable: bool [+ SequenceNumber]
// HasAck: bool [+ AckRange]
// Message data: variadic...

// IMPORTANT NOTE!!!
// we must check the return value from Msg::Receive(), otherwise, we may get undefined data;

template<uint8_t MessageType, bool Reliable, typename ...Types>
class Message
{
public:
	static void Send(Connection& conn, const Types& ...args)
	{
		static_assert(MessageType < 256, "Control channel message must be a byte.");

		OutputBitStream os;

		os.Write(MessageType);

		SequenceNumber seq = conn.WriteReliability(os, Reliable);

		os.Write(args...);

		if (Reliable)
			conn.SaveOutgoingPacket(seq, os);

		conn.SendPacket(os);
	}

	// NOTE!!! 
	// the data is only read when receive returns true;
	static bool Receive(Connection& conn, InputBitStream& is, Types& ...args)
	{
		static_assert(MessageType < 256, "Control channel message must be a byte.");

		bool isValid = conn.ReadAndProcessReliability(is);

		if (isValid)
		{
			is.Read(args...);
		}

		return isValid;
	}

	static void Write(OutputBitStream& os, const Types& ...args)
	{
		static_assert(MessageType < 256, "Control channel message must be a byte.");

		os.Write(args...);
	}

	static void Read(InputBitStream& is, Types& ...args)
	{
		static_assert(MessageType < 256, "Control channel message must be a byte.");

		is.Read(args...);
	}

	static uint8_t GetMessageType()
	{
		return MessageType;
	}

	static bool IsReliable()
	{
		return Reliable;
	}
};

enum MsgTypeStart
{
	Msg_Net_Start = 0,
	Msg_Match_Start = 128,
};

// message types in Networking layer
enum MsgNetType
{
	Msg_Net_Hello = Msg_Net_Start,
	Msg_Net_Welcome,
	Msg_Net_Ready,
	Msg_Net_Heartbeat,
	Msg_Net_Ack,
	Msg_Net_Max,
};

// message class definition: how simple it is!
// params: MesgType, Reliable, variadic...

// IMPORTANT NOTE!!! 
// we must check the return value from Msg::Receive(), otherwise, we may get undefined data;

// network messages

// string: PlayerName
typedef Message<Msg_Net_Hello, false, string> HelloMsg;

typedef Message<Msg_Net_Welcome, false, PlayerId> WelcomeMsg;

// uint32_t: heartbeat value
typedef Message<Msg_Net_Heartbeat, true, uint32_t> HeartbeatMsg;

// no param;
typedef Message<Msg_Net_Ack, false> AckMsg;
