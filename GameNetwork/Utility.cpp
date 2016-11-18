#include "stdafx.h"

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

		LogUtil::LogMessage(LL_Fatal, error, msg);

		return nullptr;
	}
}

string LogUtil::GetLevelString(LogLevel level)
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

void LogUtil::LogMessage(LogLevel level, string s)
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

void LogUtil::LogMessage(LogLevel level, int errorCode, string s)
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

TimeUtil TimeUtil::s_Instance;
LARGE_INTEGER TimeUtil::s_StartTime = { 0 };

TimeUtil::TimeUtil()
{
	LARGE_INTEGER perFreq;

	QueryPerformanceFrequency(&perFreq);
	m_PerfCountDuration = 1.0 / perFreq.QuadPart;

	QueryPerformanceCounter(&s_StartTime);
	m_LastFrameStartTime = GetTime();

	srand(static_cast<int>(GetTime()));
}

void TimeUtil::Update()
{
	double curTime = GetTime();
	m_DeltaTime = curTime - m_LastFrameStartTime;
	m_LastFrameStartTime = curTime;
	m_FrameStartTime = m_LastFrameStartTime;
}

double TimeUtil::GetTime() const
{
	LARGE_INTEGER curTime;
	LARGE_INTEGER timeSinceStart;

	QueryPerformanceCounter(&curTime);

	timeSinceStart.QuadPart = curTime.QuadPart - s_StartTime.QuadPart;
	return timeSinceStart.QuadPart * m_PerfCountDuration;
}