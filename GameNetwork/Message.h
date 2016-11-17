#pragma once

// implement Message using variadic template;
template<uint8_t MessageType, bool Reliabile, typename ...Types>
class Message
{
public:
	// using the move semantics introduced in C++11, return the local output bit steam;
	static OutputBitStream Write(const Types& ...args)
	{
		static_assert(MessageType < 256, "Control channel message must be a byte.");

		OutputBitStream os;

		os.Write(args...);

		return os;
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
		return Reliabile;
	}
};

enum MsgType
{
	Msg_Hello = 0,
	Msg_Welcome,
	Msg_Ready,
	Msg_Heartbeat,
	Msg_Max,
};

typedef Message<Msg_Hello, false, string> HelloMsg;
typedef Message<Msg_Welcome, false, int8_t> WelcomeMsg;
typedef Message<Msg_Ready, false, bool> ReadyMsg;
typedef Message<Msg_Heartbeat, true, float> HeartbeatMsg;
