#pragma once

class SockUtil
{
public:
	static bool StaticInit();
	static void CleanUp();
	static UDPSocketPtr CreateUDPSocket(int family);

	enum LogLevel
	{
		LL_Info = 0,
		LL_Warning,
		LL_Error,
		LL_Fatal,
	};

	static void LogMessage(LogLevel level, string s);
	static void LogMessage(LogLevel level, int errorCode, string s);

private:
	static string GetLevelString(LogLevel level);
};