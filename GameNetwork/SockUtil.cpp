#include "stdafx.h"
#include <iostream>
#include <winsock2.h>
#include "SockUtil.h"

#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

bool SockUtil::StaticInit()
{
#ifdef WIN32
	WSADATA data = { 0 };
	WORD version = MAKEWORD(2, 2);
	return (0 == WSAStartup(version, &data));
#else
	return true;
#endif
}

void SockUtil::CleanUp()
{
#ifdef WIN32
	WSACleanup();
#endif
}

UDPSocketPtr SockUtil::CreateUDPSocket(int family)
{
	SOCKET sock;
	int type = SOCK_DGRAM;
	sock = socket(family, type, IPPROTO_UDP);

	if (sock != INVALID_SOCKET)
	{
		return shared_ptr<UDPSocket>(new UDPSocket(sock, family));
	}
	else
	{
		string msg = "Failed to create UDP socket";
		int error = GetLastError();

		SockUtil::LogMessage(SockUtil::LL_Fatal, error, msg);

		return nullptr;
	}
}

string SockUtil::GetLevelString(LogLevel level)
{
	switch (level)
	{
	case SockUtil::LL_Info:
		return "INFO";
	case SockUtil::LL_Warning:
		return "WARNING";
	case SockUtil::LL_Error:
		return "ERROR";
	case SockUtil::LL_Fatal:
		return "FATAL";
	default:
		return "NOT DEFINED";
	}
}

void SockUtil::LogMessage(LogLevel level, string s)
{
	string msg = GetLevelString(level);
	msg += ": ";
	msg += (s);

	cout << msg << endl;
}

void SockUtil::LogMessage(LogLevel level, int errorCode, string s)
{
	string msg = GetLevelString(level);
	msg += ": ";
	msg += to_string(errorCode);
	msg += ", ";
	msg += (s);

	cout << msg << endl;
}
