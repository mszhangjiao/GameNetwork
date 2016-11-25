#include "stdafx.h"

const char* GameClient::cCommandList[] =
{
	"replay",
	"leave"
};

bool GameClient::StaticInit(const string& serverIP, const string& playerName)
{
	GameClient* client = new GameClient(serverIP, playerName);
	if (client->InitNetManager())
	{
		s_Instance.reset(client);
		StringUtil::SetConsoleLogLevel(LL_Info);
		StringUtil::SetDebugWindowLogLevel(LL_Debug);

		INFO("Client %s is ready, connecting to server %s ...", playerName.c_str(), serverIP.c_str());
		return true;
	}

	return false;
}

GameClient::GameClient(const string& serverIP, const string& playerName)
	: m_ServerIP(serverIP)
	, m_PlayerName(playerName)
	, m_CurrentMatchPtr(nullptr)
{
	m_Thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)GameClient::ConsoleCommandProc, this, 0, 0);
}

bool GameClient::InitNetManager()
{
	return NetClient::StaticInit(m_ServerIP, m_Service, m_NetFamily, m_PlayerName);
}

void GameClient::DoFrame()
{
	NetClient::Instance()->CheckForDisconnects();
	
	if (!NetClient::Instance()->GetLocalPlayerPtr()->IsDisconnected())
	{
		NetClient::Instance()->ProcessIncomingPackets();
		NetClient::Instance()->SendOutgoingPackets();

		Game::DoFrame();
	}

	UpdateLocalPlayer();

	ExecConsoleCommand();
}

void GameClient::FindMatch()
{
	NetPlayerPtr playerPtr = NetClient::Instance()->GetLocalPlayerPtr();

	FindMatchMsg::Send(*playerPtr->GetConnection());
	playerPtr->SetState(Net_FindingMatch);
}

void GameClient::PlayTurn(TurnId turnId)
{
	NetPlayerPtr playerPtr = NetClient::Instance()->GetLocalPlayerPtr();
	
	Card card = m_CardList.front();
	PlayTurnMsg::Send(*playerPtr->GetConnection(), turnId, card);

	INFO("%s: turnId[%d], current card list [%s], played card[%d]", __FUNCTION__, turnId, CardsToString().c_str(), card);

	m_CardList.pop_front();
}

void GameClient::UpdateLocalPlayer()
{
	switch (NetClient::Instance()->GetLocalPlayerPtr()->GetState())
	{
	case Net_Connected:
		FindMatch();
		break;
		//case Net_FindingMatch:
		//	break;
	}
}

// called from NetClient
void GameClient::ProcessPacket(uint8_t msgType, NetPlayerPtr playerPtr, InputBitStream& is)
{
	switch (msgType)
	{
	case Msg_Match_FindMatch_Reply:
		HandleFindMatchReply(is);
		break;

	case Msg_Match_JoinMatch:
		HandleJoinMatch(is);
		break;

	case Msg_Match_StartMatch:
		HandleStartMatch(is);
		break;

	case Msg_Match_StartTurn:
		HandleStartTurn(is);
		break;

	case Msg_Match_EndTurn:
		HandleEndTurn(is);
		break;

	case Msg_Match_EndMatch:
		HandleEndMatch(is);
		break;

	default:
		ERR("%s: Received unknow msgType[%d]", __FUNCTION__, msgType);
		break;
	}
}

void GameClient::HandleFindMatchReply(InputBitStream& is)
{
	NetPlayerPtr playerPtr = NetClient::Instance()->GetLocalPlayerPtr();

	MatchId id;
	if (!FindMatchReplyMsg::Receive(*playerPtr->GetConnection(), is, id))
	{
		return;
	}

	if (playerPtr->GetState() != Net_FindingMatch)
	{
		ERR("%s: Get FindMatchReply in wrong state[%d]", __FUNCTION__, playerPtr->GetState());
		return;
	}

	if (id == Match::cInvalidMatchId)
	{
		ERR("%s: Failed to find a match, maybe already in a match", __FUNCTION__);
		return;
	}
	else
	{
		m_CurrentMatchPtr = make_shared<Match>(id);
		m_CurrentMatchPtr->AddPlayer(playerPtr);
		playerPtr->SetMatch(m_CurrentMatchPtr);
		playerPtr->SetState(Net_InMatch);
		m_CurrentMatchPtr->SetState(Match_Waiting);

		INFO("%s: Find match[%d], set player ready", __FUNCTION__, id);

		playerPtr->SetState(Net_ReadyForPlay);
		MatchReadyMsg::Send(*playerPtr->GetConnection(), true);
	}
}

void GameClient::HandleJoinMatch(InputBitStream& is)
{
	NetPlayerPtr localPlayerPtr = NetClient::Instance()->GetLocalPlayerPtr();

	MatchId matchId;
	PlayerId playerId;
	string name;
	if (!JoinMatchMsg::Receive(*localPlayerPtr->GetConnection(), is, matchId, playerId, name))
	{
		return;
	}

	if (localPlayerPtr->GetState() < Net_InMatch || m_CurrentMatchPtr == nullptr)
	{
		ERR("%s: Get FindMatchReply in wrong state[%d]", __FUNCTION__, localPlayerPtr->GetState());
		return;
	}

	INFO("%s: player[%d] joins match[%d]", __FUNCTION__, playerId, matchId);

	if (matchId != m_CurrentMatchPtr->GetId())
	{
		ERR("%s: mismatched match, my match[%d], this match[%d]", __FUNCTION__, m_CurrentMatchPtr->GetId(), matchId);
		return;
	}

	PlayerPtr playerPtr = make_shared<Player>(playerId, name);
	m_CurrentMatchPtr->AddPlayer(playerPtr);
}

void GameClient::HandleStartMatch(InputBitStream& is)
{
	NetPlayerPtr localPlayerPtr = NetClient::Instance()->GetLocalPlayerPtr();

	MatchId matchId;
	if (!StartMatchMsg::Receive(*localPlayerPtr->GetConnection(), is, matchId, m_CardList))
	{
		return;
	}

	if (localPlayerPtr->GetState() != Net_ReadyForPlay || m_CurrentMatchPtr == nullptr)
	{
		ERR("%s: in wrong state[%d]", __FUNCTION__, localPlayerPtr->GetState());
		return;
	}

	if (m_CurrentMatchPtr->GetId() != matchId)
	{
		ERR("%s: mismatched maches: my match[%d], their match[%d]", __FUNCTION__, m_CurrentMatchPtr->GetId(), matchId);
		return;
	}

	localPlayerPtr->SetState(Net_Playing);
	m_InitialCardsNum = m_CardList.size();

	m_CurrentMatchPtr->SetMaxTurnId(static_cast<TurnId>(m_CardList.size()));
	m_CurrentMatchPtr->SeteCurrentTurnId(0);
	m_CurrentMatchPtr->SetState(Match_Started);

	string msg = StringUtil::Format("%s: get dealt cards from match[%d]:", __FUNCTION__, matchId);
	for (auto card : m_CardList)
	{
		msg += " ";
		msg += to_string(card);
	}

	INFO("%s", msg.c_str());
}

void GameClient::HandleStartTurn(InputBitStream& is)
{
	NetPlayerPtr localPlayerPtr = NetClient::Instance()->GetLocalPlayerPtr();

	MatchId matchId;
	TurnId turnId;

	if (!StartTurnMsg::Receive(*localPlayerPtr->GetConnection(), is, matchId, turnId))
	{
		return;
	}

	if (localPlayerPtr->GetState() != Net_Playing || m_CurrentMatchPtr == nullptr)
	{
		ERR("%s: in wrong state[%d]", __FUNCTION__, localPlayerPtr->GetState());
		return;
	}

	if (m_CurrentMatchPtr->GetId() != matchId)
	{
		ERR("%s: mismatched maches: my match[%d], their match[%d]", __FUNCTION__, m_CurrentMatchPtr->GetId(), matchId);
		return;
	}

	if (turnId != m_CurrentMatchPtr->GetCurrentTurnId())
	{
		ERR("%s: wrong turnId [%d], my current turnId [%d], max turnId [%d]", __FUNCTION__, turnId, m_CurrentMatchPtr->GetCurrentTurnId(), m_CurrentMatchPtr->GetMaxTurnId());
		return;
	}

	DEBUG("%s: turn [%d]", __FUNCTION__, turnId);

	PlayTurn(turnId);
}

void GameClient::HandleEndTurn(InputBitStream& is)
{
	NetPlayerPtr localPlayerPtr = NetClient::Instance()->GetLocalPlayerPtr();

	MatchId matchId;
	TurnId turnId;
	vector<PlayerId> players;
	vector<Card> cards;
	vector<Score> scores;
	if (!EndTurnMsg::Receive(*localPlayerPtr->GetConnection(), is, matchId, turnId, players, cards, scores))
	{
		return;
	}

	if (localPlayerPtr->GetState() != Net_Playing || m_CurrentMatchPtr == nullptr)
	{
		ERR("%s: in wrong state[%d]", __FUNCTION__, localPlayerPtr->GetState());
		return;
	}

	if (m_CurrentMatchPtr->GetId() != matchId)
	{
		ERR("%s: mismatched maches: my match[%d], their match[%d]", __FUNCTION__, m_CurrentMatchPtr->GetId(), matchId);
		return;
	}

	if (turnId != m_CurrentMatchPtr->GetCurrentTurnId())
	{
		ERR("%s: wrong turnId [%d], my current turnId [%d], max turnId [%d]", __FUNCTION__, turnId, m_CurrentMatchPtr->GetCurrentTurnId(), m_CurrentMatchPtr->GetMaxTurnId());
		return;
	}

	// display turn result
	for (size_t i = 0; i < players.size(); ++i)
	{
		string name("unknow player");
		PlayerPtr playerPtr = m_CurrentMatchPtr->GetPlayerById(players[i]);
		if (playerPtr)
			name = playerPtr->GetName();

		INFO("Turn [%2d] result: player [%6s], played card [%2d], turn score [%2d]", turnId, name.c_str(), cards[i], scores[i]);
	}

	m_CurrentMatchPtr->MoveToNextTurn();
}

void GameClient::HandleEndMatch(InputBitStream& is)
{
	NetPlayerPtr localPlayerPtr = NetClient::Instance()->GetLocalPlayerPtr();

	MatchId matchId;
	vector<PlayerId> players;
	vector<Score> scores;
	if (!EndMatchMsg::Receive(*localPlayerPtr->GetConnection(), is, matchId, players, scores))
	{
		return;
	}

	// display last turn result
	for (size_t i = 0; i < players.size(); ++i)
	{
		string name("unknow player");
		PlayerPtr playerPtr = m_CurrentMatchPtr->GetPlayerById(players[i]);
		if (playerPtr)
			name = playerPtr->GetName();

		INFO("Match [%2d] result: player [%6s], total score [%2d]", matchId, name.c_str(), scores[i]);
	}

	PromptUser("What do you want next? \"replay\" or \"leave\"");
}

void GameClient::MatchReady()
{
	NetPlayerPtr playerPtr = NetClient::Instance()->GetLocalPlayerPtr();

	playerPtr->SetScore(0);
	m_CardList.clear();
	m_InitialCardsNum = 0;

	playerPtr->SetState(Net_ReadyForPlay);
	MatchReadyMsg::Send(*playerPtr->GetConnection(), true);
}

void GameClient::LeaveGame()
{
	NetPlayerPtr playerPtr = NetClient::Instance()->GetLocalPlayerPtr();
	playerPtr->SetState(Net_Connected);
	playerPtr->SetScore(0);
	playerPtr->SetMatch(nullptr);
	m_CurrentMatchPtr.reset();
	m_CardList.clear();
	m_InitialCardsNum = 0;

	INFO("%s: player [%6s] leave match", __FUNCTION__, playerPtr->GetName().c_str());
	LeaveGameMsg::Send(*playerPtr->GetConnection());
}

void GameClient::PromptUser(const string& message)
{
	PROMPT(message.c_str());
}

void GameClient::PushCommand(const char* command)
{
	AutoCriticalSection aSection(&m_CSection);
	m_Commands.push(command);
}

bool GameClient::PopCommand(string& command)
{
	AutoCriticalSection aSection(&m_CSection);
	if (!m_Commands.empty())
	{
		command = m_Commands.front();
		m_Commands.pop();
		return true;
	}

	return false;
}

void GameClient::RunCommand()
{
	char command[cCommandLength];

	while (cin.getline(command, cCommandLength))
	{
		PushCommand(command);
		Sleep(500);
	}
}

void GameClient::ConsoleCommandProc(GameClient *game)
{
	game->RunCommand();
}

void GameClient::ExecConsoleCommand()
{
	struct convert
	{
		void operator()(char& c)
		{
			c = tolower(c);
		}
	};

	string command;
	for_each(command.begin(), command.end(), convert());

	bool result = PopCommand(command);

	if (!result)
		return;

	if (command == cCommandList[Com_Replay])
	{
		MatchReady();
	}
	else if (command == cCommandList[Com_Leave])
	{
		LeaveGame();
		exit(0);
	}
	else
	{
		INFO("%s: unknow command [%s], supported commands [%s]", __FUNCTION__, command.c_str(), GetSupportedCommands().c_str());
	}
}

string GameClient::GetSupportedCommands() const
{
	string s;

	for (auto i = Com_Start + 1; i < Com_End; ++i)
	{
		s += cCommandList[i];
		if (i != Com_End - 1)
		{
			s += ", ";
		}
	}

	return s;
}
