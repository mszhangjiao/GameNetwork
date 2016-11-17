#include "stdafx.h"

bool GameServer::StaticInit()
{
	GameServer* server = new GameServer();
	if (server->InitNetManager())
	{
		s_Instance.reset(server);
		Utility::LogMessage(LL_Info, "Server is ready, waiting for players to join...");
		return true;
	}

	return false;
}

GameServer::GameServer()
{
}

bool GameServer::InitNetManager()
{
	return NetServer::StaticInit(m_Service, m_NetFamily);
}

// run each frame;
void GameServer::DoFrame()
{
	NetServer::Instance()->ProcessIncomingPackets();
	Game::DoFrame();
	NetServer::Instance()->SendOutgoingPackets();
}

// run one time;
int GameServer::Run()
{
	// init game logic here;

	// run game loop;
	return Game::Run();
}