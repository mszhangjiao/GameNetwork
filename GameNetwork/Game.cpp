#include "stdafx.h"

GamePtr Game::s_Instance = nullptr;

Game::Game()
	: m_IsRunning(true)
{
	SockUtil::StaticInit();
}

Game::~Game()
{
	SockUtil::CleanUp();
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