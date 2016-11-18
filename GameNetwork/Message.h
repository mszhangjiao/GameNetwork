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

// message type definition
enum MsgType
{
	Msg_Hello = 0,
	Msg_Welcome,
	Msg_Ready,
	Msg_Heartbeat,
	Msg_Ack,
	Msg_Max,
};

// message class definition: how simple it is!
// params: MesgType, Reliable, variadic...
typedef Message<Msg_Hello, false, string> HelloMsg;
typedef Message<Msg_Welcome, false, int8_t> WelcomeMsg;
typedef Message<Msg_Ready, true, bool> ReadyMsg;
typedef Message<Msg_Heartbeat, true, uint32_t> HeartbeatMsg;
typedef Message<Msg_Ack, false> AckMsg;
