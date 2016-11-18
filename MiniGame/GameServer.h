#pragma once

// it defines server specific features, again the "game" is supper simple now:
// the server waits for clients, and simply sends each client heart beats;
class GameServer : public Game
{
public:
	static bool StaticInit();
	virtual void DoFrame() override;
	virtual int Run() override;

private:
	GameServer();
	bool InitNetManager();
};