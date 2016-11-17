#include "stdafx.h"
#include "SockAddrIn.h"
#include <stdlib.h>

class SockHandle;
SockAddrIn SockAddrIn::NullAddr;

// Get an IPv4 or IPv6 address from SockAddrIn structure;
bool SockAddrIn::GetIPAddr(char *ipaddr, UINT size)
{
	if (ipaddr && size > 8)
	{
		void *addr;
		char szIPAddr[HOSTNAME_SIZE] = { 0 };
		if (this->ss_family == AF_INET)
		{
			addr = &((sockaddr_in *)this)->sin_addr;
		}
		else
		{
			addr = &((sockaddr_in6 *)this)->sin6_addr;
		}

		if (inet_ntop(this->ss_family, addr, szIPAddr, HOSTNAME_SIZE) != NULL)
		{
			strncpy_s(ipaddr, size, szIPAddr, HOSTNAME_SIZE);
			return true;
		}
	}

	SetLastError(ERROR_INSUFFICIENT_BUFFER);
	return false;
}

// Get port number from service name;
USHORT SockAddrIn::GetPortNumber(const char *service)
{
	LPSERVENT servent;
	USHORT port = 0;

	if (isdigit(service[0]) != 0)
	{
		port = (USHORT)atoi(service);
	}
	else
	{
		servent = getservbyname(service, NULL);
		if (servent != NULL)
		{
			port = ntohs(servent->s_port);
		}
	}

	return port;
}

bool SockAddrIn::CreateFrom(const char *host, const char *service, int family /*= AF_INET*/)
{
	Clear();

	const char szZero[] = "0";
	ADDRINFO aiHints;
	ADDRINFO *aiList = NULL;

	memset(&aiHints, 0, sizeof(ADDRINFO));
	aiHints.ai_flags = AI_ADDRCONFIG;
	aiHints.ai_family = static_cast<short>(family);

	int result = getaddrinfo(host, service, &aiHints, &aiList);
	if (result != SOCKET_ERROR && aiList != NULL)
	{
		memcpy(this, aiList->ai_addr, aiList->ai_addrlen);
		freeaddrinfo(aiList);
	}
	else
	{
		SetLastError(WSAGetLastError());
	}

	return !IsNull();
}

bool SockAddrIn::CreateFrom(ULONG ipAddr, USHORT port, bool bFmtHost /*= true*/)
{
	Clear();

	SOCKADDR_IN *sa = reinterpret_cast<SOCKADDR_IN *>(this);
	sa->sin_family = AF_INET;
	if (bFmtHost)
	{
		sa->sin_addr.s_addr = htonl(ipAddr);
		sa->sin_port = htons(port);
	}
	else
	{
		sa->sin_addr.s_addr = ipAddr;
		sa->sin_port = port;
	}

	return !IsNull();
}