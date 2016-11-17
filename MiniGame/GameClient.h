#pragma once

#include <string>

#include "Game.h"

using namespace std;

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