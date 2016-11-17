#include "stdafx.h"

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
