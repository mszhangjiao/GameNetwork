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
		int error = GetLastError();
		FATAL("%d, Failed to create UDP socket", error);

		return nullptr;
	}
}

LogLevel StringUtil::sConsoleLogLevel = LL_Info;
LogLevel StringUtil::sDebugWindowLogLevel = LL_Debug;

const char* StringUtil::GetLevelString(LogLevel level)
{
	switch (level)
	{
	case LL_Verbose:
		return "VERBOSE";
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
	case LL_Prompt:
		return "PROMPT";
	default:
		return "UNKNOWN";
	}
}

void StringUtil::Log(LogLevel level, const char* format, ...)
{
	char info[4096];

	sprintf_s(info, "%6.2f %s: ", TimeUtil::Instance().GetTimef(), GetLevelString(level));

	string msg(info);

	va_list args;
	va_start(args, format);
	vsnprintf_s(info, 4096, format, args);
	va_end(args);

	msg += info;
	msg += "\n";

	if (level >= sDebugWindowLogLevel)
		OutputDebugStringA(msg.c_str());

	if (level >= sConsoleLogLevel)
		cout << msg;
}

string StringUtil::Format(const char* format, ...)
{
	char info[4096];

	va_list args;
	va_start(args, format);
	vsnprintf_s(info, 4096, format, args);
	va_end(args);

	return string(info);
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