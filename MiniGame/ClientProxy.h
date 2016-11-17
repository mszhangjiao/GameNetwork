#pragma once

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
			Utility::LogMessage(LL_Info, m_ConnectionPtr->GetPlayerName() + " is ready: " + to_string(m_Ready));
	}

private:
	ConnectionPtr m_ConnectionPtr;
	bool m_Ready;
};

typedef shared_ptr<ClientProxy> ClientProxyPtr;