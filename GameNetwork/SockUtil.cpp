#include "stdafx.h"

#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

bool Utility::StaticInit()
{
#ifdef WIN32
	WSADATA data = { 0 };
	WORD version = MAKEWORD(2, 2);
	return (0 == WSAStartup(version, &data));
#else
	return true;
#endif
}

void Utility::CleanUp()
{
#ifdef WIN32
	WSACleanup();
#endif
}

UDPSocketPtr Utility::CreateUDPSocket(int family)
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

		Utility::LogMessage(LL_Fatal, error, msg);

		return nullptr;
	}
}

string Utility::GetLevelString(LogLevel level)
{
	switch (level)
	{
	case LL_Debug:
		return "DEBUG";
	case LL_Info:
		return "INFO";
	case LL_Warning:
		return "WARNING";
	case LL_Error:
		return "ERROR";
	case LL_Fatal:
		return "FATAL";
	default:
		return "NOT DEFINED";
	}
}

void Utility::LogMessage(LogLevel level, string s)
{
	char info[16];
	sprintf_s(info, "%6.2f: ", TimeUtil::Instance().GetTimef());

	string msg(info);

	msg += GetLevelString(level);
	msg += ": ";
	msg += (s);
	msg += "\n";

	OutputDebugStringA(msg.c_str());
	
	if (level != LL_Debug)
		cout << msg;
}

void Utility::LogMessage(LogLevel level, int errorCode, string s)
{
	char info[16];
	sprintf_s(info, "%6.2f: ", TimeUtil::Instance().GetTimef());

	string msg(info);
	
	msg += GetLevelString(level);
	msg += ": ";
	msg += to_string(errorCode);
	msg += ", ";
	msg += (s);
	msg += "\n";

	OutputDebugStringA(msg.c_str());

	if (level != LL_Debug)
		cout << msg;
}
