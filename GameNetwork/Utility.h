#pragma once

enum LogLevel
{
	LL_Debug = 0,
	LL_Info,
	LL_Warning,
	LL_Error,
	LL_Fatal,
};

class Utility
{
public:
	static bool StaticInit();
	static void CleanUp();
	static UDPSocketPtr CreateUDPSocket(int family);

	static void LogMessage(LogLevel level, string s);
	static void LogMessage(LogLevel level, int errorCode, string s);

	static float GetRandomFloat()
	{
		//return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

		// the old rand() function is inaccurate, use the new standard one;
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_real_distribution< float > dis(0.f, 1.f);
		return dis(gen);
	}

private:
	static string GetLevelString(LogLevel level);
};