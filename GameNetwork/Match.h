#pragma once

enum MatchState
{
	Match_None,
	Match_Waiting,
	Match_Started,
	Match_Ended,
};

class Match
{
public:
	const uint32_t cInvalidMatchId = 0xffffffff;

	Match()
		: m_Id(cInvalidMatchId)
		, m_State(Match_None)
	{}

	Match(uint32_t id)
		: m_Id(id)
		, m_State(Match_None)
	{}

	uint32_t GetId()
	{
		return m_Id;
	}

	MatchState GetState()
	{
		return m_State;
	}

	void SetState(MatchState state)
	{
		m_State = state;
	}

	bool AddPlayer(PlayerPtr pPlayer)
	{
		if (m_Players.find(pPlayer->GetPlayerId()) != m_Players.end())
		{
			ERR("%s: %d is already in the match", __FUNCTION__, pPlayer->GetPlayerId());
			return false;
		}

		m_Players[pPlayer->GetPlayerId()] = pPlayer;
		return true;
	}

	bool RemovePlayer(uint32_t playerId)
	{
		if (m_Players.find(playerId) == m_Players.end())
		{
			ERR("%s: %d doesn't exist in the match", __FUNCTION__, playerId);
			return false;
		}

		m_Players.erase(playerId);
		return true;
	}

private:
	typedef unordered_map<uint32_t, PlayerPtr> IdToPlayerPtrMap;

	uint32_t m_Id;
	MatchState m_State;
	IdToPlayerPtrMap m_Players;
};