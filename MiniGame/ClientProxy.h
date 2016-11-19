#pragma once

// it's used on server side representing a connected client;
// it's on top of Connection layer, so we are able to add more features to it apart from the client connection;
// it doesn't have much more than a connection right now, because we don't have a game yet...
class ClientProxy
{
public:
	ClientProxy(ConnectionPtr connPtr)
		: m_ConnectionPtr(connPtr)
		, m_Ready(false)
	{}

	ConnectionPtr GetConnection()
	{
		return m_ConnectionPtr;
	}

	void SetReady(bool bReady)
	{
		m_Ready = bReady;
	}

	bool GetReady() const
	{
		return m_Ready;
	}

	void HandleReadyPacket(InputBitStream& is)
	{
		if (ReadyMsg::Receive(*m_ConnectionPtr, is, m_Ready))
		{
			INFO("%s is ready: %d", m_ConnectionPtr->GetPlayerName().c_str(), m_Ready);

			if (m_Ready)
			{
				m_ConnectionPtr->InitHeartbeat(m_ConnectionPtr->GetPlayerId() * 1000);
			}
		}
	}

private:
	ConnectionPtr m_ConnectionPtr;
	bool m_Ready;
};

typedef shared_ptr<ClientProxy> ClientProxyPtr;