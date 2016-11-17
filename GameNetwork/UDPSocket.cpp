#include "stdafx.h"
#include <string>
#include <cassert>
#include "UDPSocket.h"
#include "SockUtil.h"

using namespace std;

int UDPSocket::Bind(const char* host, const char* service)
{
	SockAddrIn sockAddr;

	// Associate a local address with the socket but let provider assign a port number
	if (sockAddr.CreateFrom(host, service, m_Family) == false)
	{
		string msg = "Failed in bind: not able to create SockAddrIn";
		int error = GetLastError();

		SockUtil::LogMessage(SockUtil::LL_Fatal, error, msg);
		return error;
	}

	int result = bind(m_Socket, sockAddr, sizeof(sockaddr_storage));
	if (result != 0)
	{
		string msg = "Failed to bind addr";
		int error = GetLastError();

		SockUtil::LogMessage(SockUtil::LL_Fatal, error, msg);
		return error;
	}
	else
		return NO_ERROR;
}

int UDPSocket::SetNonBlockingMode(bool bNonBlock)
{
#ifdef WIN32
	u_long arg = bNonBlock ? 1 : 0;
	int result = ioctlsocket(m_Socket, FIONBIO, &arg);
#else
	int flags = fcntl(m_hSocket, F_GETFL, 0);
	flags = bNonBlock ?
		(flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
	fcntl(m_hSocket, F_SETFL, flags);
#endif

	if (result == SOCKET_ERROR)
	{
		string msg = "Failed to set non-block socket";
		int error = GetLastError();

		SockUtil::LogMessage(SockUtil::LL_Fatal, error, msg);
		return error;
	}
	else
		return NO_ERROR;
}

int UDPSocket::SendTo(const char *buffer, DWORD size, const SockAddrIn& toAddr)
{
	assert(IsOpen());
	assert(buffer != NULL);

	if (!IsOpen() || buffer == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return -1;
	}

	int result = 1;
	DWORD bytesWrite = 0;

	result = sendto(m_Socket, buffer, size, 0, LPSOCKADDR(&toAddr), sizeof(sockaddr_storage));

	if (result == SOCKET_ERROR)
	{
		SetLastError(WSAGetLastError());
	}

	bytesWrite = (result > 0) ? result : -1;

	return bytesWrite;
}

int UDPSocket::ReceiveFrom(char* buffer, int size, SockAddrIn& fromAddr)
{
	assert(buffer != NULL);

	if (!IsOpen() || buffer == NULL)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return -1;
	}

	DWORD bytesRead = 0L;
	int result = 1;
	
	int addrLen;
	addrLen = sizeof(sockaddr_storage);
	result = recvfrom(m_Socket, buffer, size, 0, LPSOCKADDR(&fromAddr), &addrLen);

	if (result < 0)
	{
		SetLastError(WSAGetLastError());
	}

	bytesRead = (DWORD)((result >= 0) ? result : -1);

	return bytesRead;
}
