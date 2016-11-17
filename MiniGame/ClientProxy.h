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
		m_Ready = m_ConnectionPtr->HandleReadyPacket(is);		
	}


private:
	ConnectionPtr m_ConnectionPtr;
	bool m_Ready;
};

typedef shared_ptr<ClientProxy> ClientProxyPtr;