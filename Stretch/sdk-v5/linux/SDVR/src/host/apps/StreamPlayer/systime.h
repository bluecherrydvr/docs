// systime.h
#ifndef __SYSTIME_H__
#define __SYSTIME_H__

class CSystemTime
{
public:
	~CSystemTime();
	CSystemTime();

	void Reset();
	double GetTime();
	void Calibrate(double dRefTime);
	double GetTimeFromTimestamp(double dTime);

private:
	LARGE_INTEGER	m_liPerfFreq;	// performance frequency
	LARGE_INTEGER	m_liInitCount;	// initial count
	double			m_dTimeDiff;	// time diff between system time and ref time
	sx_bool			m_bCalibrated;	// if calibrated
};

#endif
