#pragma once

class SockUtil
{
public:
	static bool StaticInit();
	static void CleanUp();
	static UDPSocketPtr CreateUDPSocket(int family);
};


enum LogLevel
{
	LL_Verbose = 0,
	LL_Debug,
	LL_Info,
	LL_Warning,
	LL_Error,
	LL_Fatal,
};

class StringUtil
{
public:
	static const char* GetLevelString(LogLevel level);
	static string Format(const char* format, ...);

	static void Log(LogLevel level, const char* format, ...);

	static void SetShowLogLevel(LogLevel level)
	{
		sShowLogLevel = level;
	}
private:
	static LogLevel sShowLogLevel;
};

#define FATAL(...) StringUtil::Log(LL_Fatal, __VA_ARGS__);
#define ERR(...) StringUtil::Log(LL_Error, __VA_ARGS__);
#define WARN(...) StringUtil::Log(LL_Warning, __VA_ARGS__);
#define INFO(...) StringUtil::Log(LL_Info, __VA_ARGS__);
#define DEBUG(...) StringUtil::Log(LL_Debug, __VA_ARGS__);
#define VERBO(...) StringUtil::Log(LL_Verbose, __VA_ARGS__);

// random float number function is provided
class MathUtil
{
public:
	static float GetRandomFloat()
	{
		//return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

		// the old rand() function is inaccurate, use the new standard one;
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_real_distribution< float > dis(0.f, 1.f);
		return dis(gen);
	}
};

// Time utility functions are important in network programs;
class TimeUtil
{
public:
	// TimeUtil singleton
	static TimeUtil Instance()
	{
		return s_Instance;
	}

	void Update();
	float GetDeltaTime() const
	{
		return static_cast<float>(m_DeltaTime);
	}

	double GetTime() const;

	float GetTimef() const
	{
		return static_cast<float>(GetTime());
	}

	float GetFrameStartTime() const
	{
		return static_cast<float>(m_FrameStartTime);
	}

private:
	TimeUtil();
	static TimeUtil s_Instance;

	static LARGE_INTEGER s_StartTime;

	uint64_t m_DeltaTick;
	double m_DeltaTime;

	double m_LastFrameStartTime;
	double m_FrameStartTime;
	double m_PerfCountDuration;
};