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

enum MsgTypeStart
{
	Msg_Net_Start = 0,
	Msg_Game_Start = 128,
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

// message types in game layer
enum MsgGameType
{
	Msg_Game_FindMatch = Msg_Game_Start,
	Msg_Game_JoinMatch,
	Msg_Game_StartMatch,
	Msg_Game_StartTurn,
	Msg_Game_PlayTurn,
	Msg_Game_EndMatch,
	Msg_Game_Max,
};

// message class definition: how simple it is!
// params: MesgType, Reliable, variadic...

// string: PlayerName
typedef Message<Msg_Net_Hello, false, string> HelloMsg;

// int8_t: playerId: we may need to change it to int32_t
typedef Message<Msg_Net_Welcome, false, int8_t> WelcomeMsg;

// bool: bReady
typedef Message<Msg_Net_Ready, true, bool> ReadyMsg;

// uint32_t: heartbeat value
typedef Message<Msg_Net_Heartbeat, true, uint32_t> HeartbeatMsg;

// no param for ack msg;
typedef Message<Msg_Net_Ack, false> AckMsg;