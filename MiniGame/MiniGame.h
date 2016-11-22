#pragma once

// Common definitions for both server and client
class MiniGame : public Game
{
public:
	// To simplify the commandline, use static port number and network family;
	const char* cMiniGameSockPort = "50001";
	const int cNetFamily = AF_INET;

	MiniGame()
	{
		SetService(cMiniGameSockPort);
		SetFamily(cNetFamily);
	}
};