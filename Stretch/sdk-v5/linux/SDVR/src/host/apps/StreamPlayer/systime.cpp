// systime.cpp

#include "sdvr_player.h"
#include "systime.h"

CSystemTime::~CSystemTime()
{
}

CSystemTime::CSystemTime()
{
	m_bCalibrated = false;
	m_dTimeDiff = 0.0;
	QueryPerformanceFrequency(&m_liPerfFreq);
}

void CSystemTime::Reset()
{
	m_bCalibrated = false;
	m_dTimeDiff = 0.0;
	QueryPerformanceCounter(&m_liInitCount);
}

double CSystemTime::GetTime()
{
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	return (double)(count.QuadPart - m_liInitCount.QuadPart) / (double)m_liPerfFreq.QuadPart;
}

void CSystemTime::Calibrate(double dRefTime)
{
	double alpha = 0.9;
	m_dTimeDiff = m_dTimeDiff * (1.0 - alpha) + (dRefTime - GetTime()) * alpha;
	m_bCalibrated = true;
}

double CSystemTime::GetTimeFromTimestamp(double dTime)
{
	if (m_bCalibrated)
		return dTime - m_dTimeDiff;
	else
		return GetTime() + 0.01;
}
