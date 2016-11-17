#include "stdafx.h"

GamePtr Game::s_Instance = nullptr;

Game::Game()
	: m_Service(cMiniGameSockPort)
	, m_NetFamily(cNetFamily)
	, m_IsRunning(true)
{
	Utility::StaticInit();
}

Game::~Game()
{
	Utility::CleanUp();
}

int Game::Run()
{
	while (m_IsRunning)
	{
		TimeUtil::Instance().Update();
		DoFrame();
	}

	return 0;
}

void Game::DoFrame()
{
	// update the game logic;
}