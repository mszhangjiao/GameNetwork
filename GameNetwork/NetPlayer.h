#pragma once

#pragma once

// definition for "remote" player;
// contains no connection info;
class Player
{
public:
	const uint32_t cInvalidPlayerId = 0xffffffff;

	Player(const string& name)
		: m_Id(cInvalidPlayerId)
		, m_PlayerName(name)
	{}

	Player(const string& name, uint32_t id)
		: m_Id(id)
		, m_PlayerName(name)
	{}

	uint32_t GetPlayerId() const
	{
		return m_Id;
	}

	void SetPlayerId(uint32_t id)
	{
		m_Id = id;
	}

	string GetPlayerName() const
	{
		return m_PlayerName;
	}

	uint32_t GetScore() const
	{
		return m_Score;
	}

	void SetScore(uint32_t score)
	{
		m_Score = score;
	}

private:
	uint32_t m_Id;
	string m_PlayerName;
	uint32_t m_Score;
};

typedef shared_ptr<Player> PlayerPtr;

enum NetState
{
	Net_None,
	Net_Hello,
	Net_Connected,
	Net_InMatch,
	Net_Disconnected,
};

// definition for "local player" or players on server;
// contains a connection pointer;
class NetPlayer : public Player
{
public:
	NetPlayer(const string& name)
		: Player(name)
		, m_ConnectionPtr(nullptr)
	{}

	NetPlayer(ConnectionPtr connPtr, const string& name, uint32_t id)
		: Player(name, id)
		, m_ConnectionPtr(connPtr)
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

	bool IsClientDisconnected()
	{
		return m_State == Net_Disconnected;
	}

private:
	NetState m_State;
	ConnectionPtr m_ConnectionPtr;
};

typedef shared_ptr<NetPlayer> NetPlayerPtr;