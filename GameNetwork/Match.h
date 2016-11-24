#pragma once

enum MatchState
{
	Match_None,
	Match_Waiting,
	Match_Started,
	Match_TurnStarted,
	Match_Ended,
};

typedef uint16_t MatchId;
typedef uint8_t TurnId;

// forward declarations;
class Player;
typedef shared_ptr<Player> PlayerPtr;
typedef vector<PlayerPtr> PlayerList;

class Match
{
public:
	static const MatchId cInvalidMatchId = 0xffff;

	Match(MatchId id)
		: m_Id(id)
		, m_State(Match_None)
		, m_MaxTurnId(0)
		, m_CurrentTurnId(0)
	{}

	MatchId GetId()
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
		if (m_Players.find(pPlayer->GetId()) != m_Players.end())
		{
			ERR("%s: %d is already in the match", __FUNCTION__, pPlayer->GetId());
			return false;
		}

		m_Players[pPlayer->GetId()] = pPlayer;
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

	PlayerPtr GetPlayerById(PlayerId id)
	{
		auto it = m_Players.find(id);
		if (it != m_Players.end())
			return it->second;

		return nullptr;
	}

	int GetPlayerNum() const
	{
		return m_Players.size();
	}

	PlayerList GetPlayerList()
	{
		PlayerList players;

		for (auto& pair : m_Players)
		{
			players.push_back(pair.second);
		}

		return players;
	}

	void SetMaxTurnId(TurnId id)
	{
		m_MaxTurnId = id;
	}

	TurnId GetMaxTurnId() const
	{
		return m_MaxTurnId;
	}

	void SeteCurrentTurnId(TurnId id)
	{
		m_CurrentTurnId = id;
	}

	TurnId GetCurrentTurnId() const
	{
		return m_CurrentTurnId;
	}

	void MoveToNextTurn()
	{
		++m_CurrentTurnId;
	}

private:
	typedef unordered_map<PlayerId, PlayerPtr> IdToPlayerPtrMap;

	MatchId m_Id;
	MatchState m_State;
	IdToPlayerPtrMap m_Players;
	TurnId m_MaxTurnId;
	TurnId m_CurrentTurnId;
};

typedef shared_ptr<Match> MatchPtr;