#include "stdafx.h"

bool GameServer::StaticInit()
{
	GameServer* server = new GameServer();
	if (server->InitNetManager())
	{
		s_Instance.reset(server);
		NetServer::Instance()->EnableDeliveryStats(false);
		StringUtil::SetConsoleLogLevel(LL_Debug);
		StringUtil::SetDebugWindowLogLevel(LL_Debug);

		INFO("Server is ready, waiting for players to join...");
		return true;
	}

	return false;
}

GameServer::GameServer()
	: m_NewMatchId(0)
	, m_PlayerNumberLimit(CardDeck::cPlayerNumLimit)
{
}

bool GameServer::InitNetManager()
{
	// enable/disable the server heartbeats and showing delivery stats
	return NetServer::StaticInit(m_Service, m_NetFamily, true, false);
}

void GameServer::DoFrame()
{
	NetServer::Instance()->ProcessIncomingPackets();
	Game::DoFrame();
	NetServer::Instance()->SendOutgoingPackets();
	NetServer::Instance()->CheckForDisconnects();

	UpdateMatches();
}

int GameServer::Run()
{
	// init game logic here;

	// run game loop;
	return Game::Run();
}

void GameServer::ProcessPacket(uint8_t msgType, NetPlayerPtr playerPtr, InputBitStream& is)
{
	switch (msgType)
	{
	case Msg_Match_FindMatch:
		FindMatch(playerPtr, is);
		break;

	case Msg_Match_PlayTurn:
		PlayTurn(playerPtr, is);
		break;

	case Msg_Match_LeaveGame:
		LeaveGame(playerPtr, is);
		break;

	case Msg_Match_Ready:
		MatchReady(playerPtr, is);
		break;
	}
}

bool GameServer::IsMatchReady(MatchPtr matchPtr) const
{
	if (matchPtr->GetPlayerNum() < m_PlayerNumberLimit)
		return false;

	for (auto& playerPtr : matchPtr->GetPlayerList())
	{
		NetPlayerPtr netPlayerPtr = NetServer::Instance()->FindPlayer(playerPtr->GetId());
		assert(netPlayerPtr);
		if (netPlayerPtr->GetState() != Net_ReadyForPlay)
			return false;
	}

	return true;
}

MatchPtr GameServer::FindAvailableMatch() const
{
	for (auto& pair : m_MatchIdToMatchMap)
	{
		if (!IsMatchFull(pair.second))
			return pair.second;
	}

	return nullptr;
}

void GameServer::FindMatch(NetPlayerPtr newPlayerPtr, InputBitStream& is)
{
	// we need to consume the packet even we there is no more params than the reliability data;
	if (!FindMatchMsg::Receive(*newPlayerPtr->GetConnection(), is))
	{
		return;
	}

	if (newPlayerPtr->GetState() != Net_Connected)
	{
		ERR("%s: %s is in wrong state[%d], ignore the request", __FUNCTION__, newPlayerPtr->GetName().c_str(), newPlayerPtr->GetState());
		FindMatchReplyMsg::Send(*newPlayerPtr->GetConnection(), Match::cInvalidMatchId);

		return;
	}

	MatchPtr matchPtr = FindAvailableMatch();

	if (matchPtr == nullptr)
	{
		matchPtr = make_shared<Match>(m_NewMatchId);
		m_MatchIdToMatchMap[m_NewMatchId++] = matchPtr;
	}

	matchPtr->AddPlayer(newPlayerPtr);
	newPlayerPtr->SetMatch(matchPtr);
	newPlayerPtr->SetState(Net_InMatch);

	if (matchPtr->GetState() == Match_None)
	{
		matchPtr->SetState(Match_Waiting);
	}

	MatchId matchId = matchPtr->GetId();
	FindMatchReplyMsg::Send(*newPlayerPtr->GetConnection(), matchId);

	INFO("%s: Find Match[%d] for player[%s]", __FUNCTION__, matchId, newPlayerPtr->GetName().c_str());

	// send JoinMatchMsg to players in match;
	PlayerList players = matchPtr->GetPlayerList();
	for (auto& playerPtr : players)
	{
		if (playerPtr != newPlayerPtr)
		{
			// notify the new player about existing players;
			JoinMatchMsg::Send(*newPlayerPtr->GetConnection(), matchId, playerPtr->GetId(), playerPtr->GetName());

			INFO("%s: Send player[%d] about player [%d]", __FUNCTION__, newPlayerPtr->GetId(), playerPtr->GetId());

			// notify existing players about the new player;
			NetPlayerPtr netPlayerPtr = NetServer::Instance()->FindPlayer(playerPtr->GetId());
			if (netPlayerPtr)
			{
				JoinMatchMsg::Send(*netPlayerPtr->GetConnection(), matchId, newPlayerPtr->GetId(), newPlayerPtr->GetName());

				INFO("%s: Send player[%d] about player [%d]", __FUNCTION__, netPlayerPtr->GetId(), newPlayerPtr->GetId());
			}
			else
			{
				ERR("%s: player[%d] in match[%d] doesn't exist", __FUNCTION__, newPlayerPtr->GetId(), matchId);
			}
		}			
	}
}

void GameServer::PlayTurn(NetPlayerPtr playerPtr, InputBitStream& is)
{
	TurnId turnId;
	Card card;
	if (!PlayTurnMsg::Receive(*playerPtr->GetConnection(), is, turnId, card))
	{
		return;
	}

	PlayerId playerId = playerPtr->GetId();

	INFO("%s: player[%d] played card [%d] in turn [%d]", __FUNCTION__, playerId, card, turnId);

	bool validated = false;
	MatchPtr matchPtr = playerPtr->GetMatch().lock();
	MatchId matchId = matchPtr->GetId();
	if (matchPtr && matchPtr->GetCurrentTurnId() == turnId)
	{
		// check if this player owns this card;
		auto it = m_MatchIdToDealtCardsMap[matchId].find(playerId);
		if ( it != m_MatchIdToDealtCardsMap[matchId].end())
		{
			auto itCard = find(it->second.cbegin(), it->second.cend(), card);
			if (itCard != it->second.cend())
			{
				m_MatchIdToPlayedCardsMap[matchId].emplace(playerId, card);

				it->second.erase(itCard);
				validated = true;
				INFO("%s: validated player[%d] card [%d]", __FUNCTION__, playerId, card);
			}
		}
	}

	if (!validated)
	{
		ERR("%s: validation failed with player [%d] card [%d]", __FUNCTION__, playerId, card);
	}
}

void GameServer::LeaveGame(NetPlayerPtr playerPtr, InputBitStream& is)
{
	if (!LeaveGameMsg::Receive(*playerPtr->GetConnection(), is))
	{
		return;
	}

	INFO("%s: player [%6s] leave game", __FUNCTION__, playerPtr->GetName().c_str());

	PlayerId playerId = playerPtr->GetId();
	MatchPtr matchPtr = playerPtr->GetMatch().lock();
	if (matchPtr)
	{
		matchPtr->RemovePlayer(playerId);
	}
	playerPtr->SetMatch(nullptr);
	playerPtr->SetState(Net_Connected);
	playerPtr->SetScore(0);

	// delete the match when the last player leaves;
	if (matchPtr->GetPlayerNum() == 0)
	{
		INFO("%s: delete match [%d] when the last plaeyr leaves", __FUNCTION__, matchPtr->GetId());
		m_MatchIdToMatchMap.erase(matchPtr->GetId());
	}

	NetServer::Instance()->DisconnectPlayer(playerPtr->GetId());
}

void GameServer::MatchReady(NetPlayerPtr playerPtr, InputBitStream& is)
{
	bool bReady;
	if (!MatchReadyMsg::Receive(*playerPtr->GetConnection(), is, bReady))
	{
		return;
	}

	PlayerId playerId = playerPtr->GetId();
	MatchPtr matchPtr = playerPtr->GetMatch().lock();
	if (!matchPtr)
	{
		ERR("%s: player [%6s] is not in match", __FUNCTION__, playerPtr->GetName().c_str());
		return;
	}

	INFO("%s: player [%6s] is ready for match [%d]", __FUNCTION__, playerPtr->GetName().c_str(), matchPtr->GetId());
	playerPtr->SetState(Net_ReadyForPlay);
	playerPtr->SetScore(0);
}

void GameServer::UpdateMatches()
{
	if (m_MatchIdToMatchMap.empty())
		return;

	for (auto& pair : m_MatchIdToMatchMap)
	{
		MatchPtr matchPtr = pair.second;

		if ((Match_Waiting == matchPtr->GetState()) && IsMatchReady(matchPtr))
		{
			StartMatch(matchPtr);
		}

		if (Match_Started == matchPtr->GetState())
		{
			RunMatch(matchPtr);
		}

		if (Match_TurnStarted == matchPtr->GetState())
		{
			UpdateTurn(matchPtr);
		}

		if (Match_Ended == matchPtr->GetState())
		{
			EndMatch(matchPtr);

			// when EndMatch is called, a match will be erased from m_MatchIdToMatchMap,
			// which will invalidate pair iterator. So break the for loop in this frame.
			break;
		}
	}
}

void GameServer::StartMatch(MatchPtr matchPtr)
{
	DealtCards dealtCards = CardDeck::DealCards();

	assert(dealtCards.size() == matchPtr->GetPlayerNum());
	PlayerList players = matchPtr->GetPlayerList();
	size_t num = players.size();

	for (size_t i = 0; i < num; ++i)
	{
		NetPlayerPtr netPlayerPtr = NetServer::Instance()->FindPlayer(players[i]->GetId());

		if (netPlayerPtr)
		{
			StartMatchMsg::Send(*netPlayerPtr->GetConnection(), matchPtr->GetId(), dealtCards[i]);

			INFO("%s: sending player[%d] dealt cards", __FUNCTION__, netPlayerPtr->GetId());

			m_MatchIdToDealtCardsMap[matchPtr->GetId()].emplace(netPlayerPtr->GetId(), dealtCards[i]);
			netPlayerPtr->SetState(Net_Playing);
		}
	}

	matchPtr->SetState(Match_Started);
	matchPtr->SetMaxTurnId(static_cast<TurnId>(dealtCards[0].size()));
	matchPtr->SeteCurrentTurnId(0);
}

void GameServer::RunMatch(MatchPtr matchPtr)
{
	TurnId turnId = matchPtr->GetCurrentTurnId();
	if ( turnId == matchPtr->GetMaxTurnId())
	{
		EndMatch(matchPtr);
		return;
	}

	if (Match_Started != matchPtr->GetState())
	{
		return;
	}

	PlayerList players = matchPtr->GetPlayerList();
	size_t num = players.size();

	for (size_t i = 0; i < num; ++i)
	{
		NetPlayerPtr netPlayerPtr = NetServer::Instance()->FindPlayer(players[i]->GetId());

		if (netPlayerPtr)
		{
			StartTurnMsg::Send(*netPlayerPtr->GetConnection(), matchPtr->GetId(), turnId);				
		}
	}

	INFO("Start turn: matchId[%d], turnId[%d]", matchPtr->GetId(), turnId);

	matchPtr->SetState(Match_TurnStarted);
}

void GameServer::UpdateTurn(MatchPtr matchPtr)
{
	TurnId turnId = matchPtr->GetCurrentTurnId();
	MatchId matchId = matchPtr->GetId();

	PlayerList players = matchPtr->GetPlayerList();
	size_t num = players.size();

	bool gotPlayedCardForAll = true;

	auto& playedCards = m_MatchIdToPlayedCardsMap[matchId];
	map<Card, PlayerPtr> sortedPlayerMap;

	if (playedCards.size() < num)
	{
		gotPlayedCardForAll = false;
	}
	else
	{
		for (size_t i = 0; i < num; ++i)
		{
			auto it = playedCards.find(players[i]->GetId());
			if (it == playedCards.end())
			{
				gotPlayedCardForAll = false;
				break;
			}

			sortedPlayerMap.emplace(it->second, players[i]);
		}
	}

	if (gotPlayedCardForAll)
	{
		// in each turn, the player will get scores from 0 to (player number - 1);
		uint32_t score = 0;
		auto it = sortedPlayerMap.begin();
		vector<PlayerId> playerIds;
		vector<Card> cards;
		vector<Score> turnScores;
		vector<Score> matchScores;
		for (size_t i = 0; i < num; ++i)
		{		
			it->second->IncreaseScore(score);

			playerIds.push_back(it->second->GetId());

			cards.push_back(it->first);
			turnScores.push_back(score);

			if (turnId == matchPtr->GetMaxTurnId() - 1)
			{
				matchScores.push_back(it->second->GetScore());
			}

			++score;
			++it;
		}

		for (size_t i = 0; i < num; ++i)
		{
			NetPlayerPtr netPlayerPtr = NetServer::Instance()->FindPlayer(players[i]->GetId());

			if (netPlayerPtr)
			{
				// send the result to the players;
				if (turnId > 0)
				{
					INFO("Turn [%2d] result: player [%6s], played card [%2d], turn score [%2d]", turnId, netPlayerPtr->GetName().c_str(), cards[i], turnScores[i]);
				}

				if (turnId < matchPtr->GetMaxTurnId() - 1)
				{
					EndTurnMsg::Send(*netPlayerPtr->GetConnection(), matchId, turnId, playerIds, cards, turnScores);
					playedCards.clear();
				}
				else
				{
					
					INFO("Match [%2d] result: player [%6s], total score [%2d]", matchId, matchPtr->GetPlayerById(playerIds[i])->GetName().c_str(), matchScores[i]);

					EndMatchMsg::Send(*netPlayerPtr->GetConnection(), matchId, playerIds, matchScores);
				}
			}
		}
	}

	if (gotPlayedCardForAll)
	{
		// Move to next turn or end the match;
		if (turnId < matchPtr->GetMaxTurnId() - 1)
		{
			matchPtr->SetState(Match_Started);
			matchPtr->MoveToNextTurn();
		}
		else
		{
			INFO("%s: match [%d] ended", __FUNCTION__, matchId);
			matchPtr->SetState(Match_Ended);
		}
	}
}

void GameServer::EndMatch(MatchPtr matchPtr)
{
	MatchId matchId = matchPtr->GetId();

	m_MatchIdToDealtCardsMap.erase(matchId);
	matchPtr->SetState(Match_Waiting);

	PlayerList players = matchPtr->GetPlayerList();
	size_t num = players.size();

	for (size_t i = 0; i < num; ++i)
	{
		NetPlayerPtr netPlayerPtr = NetServer::Instance()->FindPlayer(players[i]->GetId());

		if (netPlayerPtr)
		{
			netPlayerPtr->SetState(Net_InMatch);
		}
	}
}