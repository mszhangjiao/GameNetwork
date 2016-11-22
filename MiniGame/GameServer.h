#pragma once

// it defines server specific features, again the "game" is supper simple now:
// the server waits for clients, and simply sends each client heart beats;
class GameServer : public MiniGame
{
public:
	static bool StaticInit();

private:
	GameServer();
	bool InitNetManager();

	// run each frame;
	virtual void DoFrame() override;
	// run one time;
	virtual int Run() override;

	void PrepareMatch();
	void StartMatch();
	void EndMatch();

	CardDeck m_CardDeck;
	int m_MinPlayerNumber;
	int m_MaxPlayerNumber;
};