#pragma once

// it defines server specific features, again the "game" is supper simple now:
// the server waits for clients, and simply sends each client heart beats;
class GameServer : public MiniGame
{
public:
	static bool StaticInit();
	virtual void ProcessPacket(uint8_t msgType, NetPlayerPtr playerPtr, InputBitStream& is) override;

	void SetGamePlayerNumLimit(int num)
	{
		m_PlayerNumberLimit = num;
	}

private:
	GameServer();
	bool InitNetManager();
	MatchPtr FindAvailableMatch() const;

	inline bool GameServer::IsMatchFull(MatchPtr matchPtr) const
	{
		return (matchPtr->GetPlayerNum() == m_PlayerNumberLimit);
	}

	bool IsMatchReady(MatchPtr matchPtr) const;
	
	void FindMatch(NetPlayerPtr playerPtr, InputBitStream& is);
	void PlayTurn(NetPlayerPtr playerPtr, InputBitStream& is);
	void LeaveGame(NetPlayerPtr playerPtr, InputBitStream& is);
	void MatchReady(NetPlayerPtr playerPtr, InputBitStream& is);

	// run each frame;
	virtual void DoFrame() override;
	// run one time;
	virtual int Run() override;

	void UpdateMatches();
	void StartMatch(MatchPtr matchPtr);
	void RunMatch(MatchPtr matchPtr);
	void UpdateTurn(MatchPtr matchPtr);
	void EndMatch(MatchPtr matchPtr);

	MatchId m_NewMatchId;
	int m_PlayerNumberLimit;

	typedef unordered_map<MatchId, MatchPtr> MatchIdToMatchMap;
	MatchIdToMatchMap m_MatchIdToMatchMap;

	// server stores initial dealt cards for each match;
	// an element is added when starting a match; and it is removed when ending a match;
	typedef unordered_map<MatchId, PlayerDealtCards> MatchIdToDealtCardsMap;
	MatchIdToDealtCardsMap m_MatchIdToDealtCardsMap;

	// server stores cards that each player played in current turn;
	// the map is cleaned at the beginning of each turn;
	// an element is added when getting a PlayCardMsg from a player;
	// when the server gets PlayCardMsg from all players in the match,
	// the server will calculate the score for this turn and move to the next turn;
	typedef unordered_map<MatchId, PlayerPlayedCards> MatchIdToPlayedCardsMap;
	MatchIdToPlayedCardsMap m_MatchIdToPlayedCardsMap;
};