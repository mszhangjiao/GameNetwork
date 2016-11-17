#pragma once

// Common definitions for both server and client
class Game
{
public:
	static shared_ptr<Game> Instance()
	{
		return s_Instance;
	}

	virtual ~Game();
	virtual int Run();

	void SetGameRunning(bool running)
	{
		m_IsRunning = running;
	}

protected:
	Game();
	virtual void DoFrame();

	// To simplify the commandline, use static port number and network family;
	const char* cMiniGameSockPort = "50001";
	const int cNetFamily = AF_INET;

	string m_Service;
	int m_NetFamily;

	static shared_ptr<Game> s_Instance;

private:
	bool m_IsRunning;
};

typedef shared_ptr<Game> GamePtr;