#include "stdafx.h"

void testBitStreams()
{
	cout << __FUNCTION__ << endl;

	OutputBitStream os;
	os.WriteBits(17, 5);
	os.WriteBits(33, 6);
	os.WriteBits(5, 3);

	InputBitStream is(os.GetBuffer(), os.GetBitLength());
	uint8_t a;
	is.ReadBits(a, 5);
	cout << static_cast<int>(a) << endl;
	is.ReadBits(a, 6);
	cout << static_cast<int>(a) << endl;
	is.ReadBits(a, 3);
	cout << static_cast<int>(a) << endl;
}

void testMap()
{
	map<int, string> coll;

	if (coll.find(1) == coll.end())
	{
		coll[1] = "one";
	}
}

void testMessage()
{
	OutputBitStream os = HelloMsg::Write(string("Nancy"));

	OutputBitStream os2 = WelcomeMsg::Write(static_cast<uint8_t>(1));

	InputBitStream is(os.GetBuffer(), os.GetBitLength());
	uint8_t type;
	is.Read(type);

	switch (static_cast<MsgType>(type))
	{
		case Msg_Hello:
		{
			string name;
			HelloMsg::Read(is, name);
			cout << "Hello from: " << name << endl;
		}
			break;

		case Msg_Welcome:
		{
			int8_t playerID;
			WelcomeMsg::Read(is, playerID);
			cout << "Welcome " << ", playerID: " << static_cast<int>(playerID) << endl;
		}
		break;
	
		default:
			cout << "Unknown message type: " << type << endl;
			break;
	}
}

void UnitTests()
{
	testMessage();
}