// MiniGame.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

void PrintUsage()
{
	cout << "Usage: " << endl;
	cout << "\t" << "To run a game server: MiniGame -s" << endl;
	cout << "\t" << "To run a game client: MiniGame -c serverIP playerName" << endl;
	cout << "\t" << "To run unit test: MiniGame -t" << endl;
}

// To run a game server: MiniGame -s
// To run a game client: MiniGame -c serverIP
int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		PrintUsage();
		return -1;
	}

	string mode(argv[1]);

	if (mode == "-s")
	{
		if (GameServer::StaticInit())
			return GameServer::Instance()->Run();
	}
	else if (mode == "-c")
	{
		if (argc < 4)
		{
			PrintUsage();
			return -2;
		}

		string serverIP(argv[2]);
		string playerName(argv[3]);
		if (GameClient::StaticInit(serverIP, playerName))
			return GameClient::Instance()->Run();
	}
	else if (mode == "-t")
	{
		UnitTests();
	}

	system("pause");

    return -3;
}