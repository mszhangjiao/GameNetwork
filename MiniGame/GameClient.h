#pragma once

#include <string>

using namespace std;

// it defines client specific features, again the "game" is supper simple now:
// after connecting to a server, the client simply receives the heart beats from server and show them;
class GameClient : public MiniGame
{
public:
	static bool StaticInit(const string& serverIP, const string& playerName);

private:
	GameClient(const string& serverIP, const string& playerName);
	virtual void DoFrame() override;

	bool InitNetManager();

	string m_ServerIP;
	string m_PlayerName;
};