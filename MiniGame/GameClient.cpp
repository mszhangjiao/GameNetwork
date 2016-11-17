#include "stdafx.h"

bool GameClient::StaticInit(const string& serverIP, const string& playerName)
{
	GameClient* client = new GameClient(serverIP, playerName);
	if (client->InitNetManager())
	{
		s_Instance.reset(client);
		return true;
	}

	return false;
}

GameClient::GameClient(const string& serverIP, const string& playerName)
	: m_ServerIP(serverIP)
	, m_PlayerName(playerName)
{
}

bool GameClient::InitNetManager()
{
	return NetClient::StaticInit(m_ServerIP, m_Service, m_NetFamily, m_PlayerName);
}

void GameClient::DoFrame()
{
	NetClient::Instance()->ProcessIncomingPackets();
	Game::DoFrame();
	NetClient::Instance()->SendOutgoingPackets();
}
