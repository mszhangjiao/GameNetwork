#pragma once

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