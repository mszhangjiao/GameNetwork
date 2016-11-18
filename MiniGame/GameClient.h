#pragma once

#include <string>

#include "Game.h"

using namespace std;

// it defines client specific features, again the "game" is supper simple now:
// after connecting to a server, the client simply receives the heart beats from server and show them;
class GameClient : public Game
{
public:
	static bool StaticInit(const string& serverIP, const string& playerName);

	bool InitNetManager();
private:
	GameClient(const string& serverIP, const string& playerName);
	virtual void DoFrame() override;

	string m_ServerIP;
	string m_PlayerName;
};