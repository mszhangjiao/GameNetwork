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
	LL_Debug = 0,
	LL_Info,
	LL_Warning,
	LL_Error,
	LL_Fatal,
};

// Todo: the logUtil is not easy to use right now, need rewrite to support easier logging;
// the only good thing about it is it support log level, so we can define the different behavior accordingly;
class LogUtil
{
public:
	static void LogMessage(LogLevel level, string s);
	static void LogMessage(LogLevel level, int errorCode, string s);

private:
	static string GetLevelString(LogLevel level);
};

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