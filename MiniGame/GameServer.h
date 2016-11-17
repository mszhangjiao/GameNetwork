#pragma once

class GameServer : public Game
{
public:
	static bool StaticInit();
	virtual void DoFrame() override;
	virtual int Run() override;

	void HandleNewClient();
	void HandleDisconnectedClient();
public:

private:
	GameServer();
	bool InitNetManager();
};