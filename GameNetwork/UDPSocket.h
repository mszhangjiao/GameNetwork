#pragma once

// this is a simple class for UDP socket;
// currently it only contains the basic functions necessary for this project;
class UDPSocket
{
public:
	UDPSocket()
		: m_Socket(INVALID_SOCKET)
	{
	}

	~UDPSocket()
	{
		ShutdownAndClose();
	}

	bool UDPSocket::IsOpen() const
	{
		return (m_Socket != INVALID_SOCKET);
	}

	int Bind(const char* host, const char* service);
	int SetNonBlockingMode(bool nonBlocking);
	int SendTo(const char *buffer, DWORD size, const SockAddrIn& toAddr);
	int ReceiveFrom(char* buffer, int size, SockAddrIn& fromAddr);

	void ShutdownAndClose()
	{
		if (IsOpen())
		{
			ShutdownAndClose(static_cast<SOCKET>(InterlockedExchange((ULONG*)&m_Socket, INVALID_SOCKET)));
		}
	}

private:
	UDPSocket::UDPSocket(SOCKET sock, int family)
		: m_Socket(sock)
		, m_Family(family)
	{
	}

	void ShutdownAndClose(SOCKET sock)
	{
		shutdown(sock, SD_BOTH);
		closesocket(sock);
	}

	friend class SockUtil;
	SOCKET m_Socket;
	int m_Family;
};

typedef shared_ptr<UDPSocket> UDPSocketPtr;