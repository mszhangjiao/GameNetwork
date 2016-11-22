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
	
	void SetService(const string& service)
	{
		m_Service = service;
	}

	void SetFamily(int family)
	{
		m_NetFamily = family;
	}

	virtual void DoFrame();

	string m_Service;
	int m_NetFamily;

	static shared_ptr<Game> s_Instance;

private:
	bool m_IsRunning;
	Match m_Match;
};

typedef shared_ptr<Game> GamePtr;