#pragma once

#pragma once

typedef uint16_t PlayerId;

// definition for "remote" player;
// contains no connection info;
class Player
{
public:
	const PlayerId cInvalidPlayerId = 0xffff;

	Player(const string& name)
		: m_Id(cInvalidPlayerId)
		, m_Name(name)
		, m_Score(0)
	{}

	Player(PlayerId id, const string& name)
		: m_Id(id)
		, m_Name(name)
		, m_Score(0)
	{}

	PlayerId GetId() const
	{
		return m_Id;
	}

	void SetId(PlayerId id)
	{
		m_Id = id;
	}

	string GetName() const
	{
		return m_Name;
	}

	uint32_t GetScore() const
	{
		return m_Score;
	}

	void SetScore(uint32_t score)
	{
		m_Score = score;
	}

	void IncreaseScore(uint32_t score)
	{
		m_Score += score;
	}

	bool operator==(const Player& other) const
	{
		return (m_Id == other.m_Id) && (m_Name == other.m_Name);
	}

private:
	PlayerId m_Id;
	string m_Name;
	uint32_t m_Score;
};

typedef shared_ptr<Player> PlayerPtr;

enum NetState
{
	Net_None = 0,
	Net_Hello,
	Net_Connected,
	Net_FindingMatch,
	Net_InMatch,
	Net_ReadyForPlay,
	Net_Playing,
	Net_Disconnected,
};

class Match;

// use weak_ptr to avoid cyclic references;
typedef weak_ptr<Match> MatchWeakPtr;
typedef shared_ptr<Match> MatchPtr;

// definition for "local player" or players on server;
// contains a connection pointer;
class NetPlayer : public Player
{
public:
	NetPlayer(const string& name)
		: Player(name)
		, m_ConnectionPtr(nullptr)
		, m_State(Net_None)
	{}

	NetPlayer(ConnectionPtr connPtr, const string& name, uint32_t id)
		: Player(id, name)
		, m_ConnectionPtr(connPtr)
		, m_State(Net_None)
	{}

	ConnectionPtr GetConnection()
	{
		return m_ConnectionPtr;
	}

	void SetConnection(ConnectionPtr connPtr)
	{
		m_ConnectionPtr = connPtr;
	}

	NetState GetState() const
	{
		return m_State;
	}

	void SetState(NetState state)
	{
		m_State = state;
	}

	// this includes all the states from Net_Connected and before Net_Disconnected;
	bool IsConnected()
	{
		return m_State >= Net_Connected && m_State < Net_Disconnected;
	}

	// this doesn't include states before Net_Connected (Net_None or Net_Hello).
	// so IsDisconnected() is not equal to !IsConnected() and vice versa;
	bool IsDisconnected()
	{
		return m_State == Net_Disconnected;
	}

	MatchWeakPtr GetMatch() const
	{
		return m_MatchPtr;
	}

	void SetMatch(MatchPtr matchPtr)
	{
		m_MatchPtr = matchPtr;
	}

private:
	NetState m_State;
	ConnectionPtr m_ConnectionPtr;
	MatchWeakPtr m_MatchPtr;
};

typedef shared_ptr<NetPlayer> NetPlayerPtr;