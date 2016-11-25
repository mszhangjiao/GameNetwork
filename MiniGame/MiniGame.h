#pragma once

// game messages

enum MsgGameType
{
	Msg_Match_FindMatch = Msg_Match_Start,
	Msg_Match_FindMatch_Reply,
	Msg_Match_JoinMatch,
	Msg_Match_StartMatch,
	Msg_Match_StartTurn,
	Msg_Match_EndTurn,
	Msg_Match_PlayTurn,
	Msg_Match_EndMatch,
	Msg_Match_LeaveGame,
	Msg_Match_Ready,
};

// IMPORTANT NOTE!!! 
// we must check the return value from Msg::Receive(), otherwise, we may get undefined data;

// no param;
typedef Message<Msg_Match_FindMatch, true> FindMatchMsg;

typedef Message<Msg_Match_FindMatch_Reply, true, MatchId> FindMatchReplyMsg;

// string: player name;
typedef Message<Msg_Match_JoinMatch, true, MatchId, PlayerId, string> JoinMatchMsg;

typedef Message<Msg_Match_StartMatch, true, MatchId, CardList> StartMatchMsg;

typedef Message<Msg_Match_StartTurn, true, MatchId, TurnId> StartTurnMsg;

typedef Message<Msg_Match_EndTurn, true, MatchId, TurnId, vector<PlayerId>, vector<Card>, vector<Score>> EndTurnMsg;

typedef Message<Msg_Match_PlayTurn, true, TurnId, Card> PlayTurnMsg;

typedef Message<Msg_Match_EndMatch, true, MatchId, vector<PlayerId>, vector<Score>> EndMatchMsg;

typedef Message<Msg_Match_LeaveGame, true> LeaveGameMsg;

// bool: bReady;
typedef Message<Msg_Match_Ready, true, bool> MatchReadyMsg;

// Common definitions for both server and client
class MiniGame : public Game
{
public:
	// To simplify the commandline, use static port number and network family;
	const char* cMiniGameSockPort = "50001";
	const int cNetFamily = AF_INET;

protected:
	MiniGame()
	{
		SetService(cMiniGameSockPort);
		SetFamily(cNetFamily);
	}
};