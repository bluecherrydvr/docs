#include "stdafx.h"
#include ".\degree2ptzunit.h"
//#using <mscorlib.dll>

CDegree2PTZunit::CDegree2PTZunit(void)
{
	m_fMaxPanUnit = 0.0f;
	m_fMinPanUnit = 0.0f;

	m_fMaxTiltUnit = 0.0f;
	m_fMinTiltUnit = 0.0f;

	m_fMaxZoomUnit = 0.0f;
	m_fMinZoomUnit = 0.0f;

	m_fMaxPanDegree = 360.0f;
	m_fMaxTiltDegree = 90.0f;
}

CDegree2PTZunit::~CDegree2PTZunit(void)
{

}

bool CDegree2PTZunit::IsInit()
{
	if((m_fMaxPanUnit == m_fMinPanUnit) && (m_fMaxTiltUnit == m_fMinTiltUnit))
	{
		return false;
	}
	return true;
}

void CDegree2PTZunit::SetMaxPanDegreeAnsMaxTiltDegree(float fMaxPanDegree, float fMaxTiltDegree)
{
	m_fMaxPanDegree = fMaxPanDegree;
	m_fMaxTiltDegree = fMaxTiltDegree;
}

void CDegree2PTZunit::SetPanUnit(float fMaxPan, float fMinPan)
{
	m_fMaxPanUnit = fMaxPan;
	m_fMinPanUnit = fMinPan;
}
void CDegree2PTZunit::SetTiltUnit(float fMaxTilt, float fMinTilt)
{
	m_fMaxTiltUnit = fMaxTilt;
	m_fMinTiltUnit = fMinTilt;
}
void CDegree2PTZunit::SetZoomUnit(float fMaxZoom, float fMinZoom)
{
	m_fMaxZoomUnit = fMaxZoom;
	m_fMinZoomUnit = fMinZoom;
}
// rotate Y-axis (0 ~ 360 ) mapping to ( fMaxPan ~ fMinPan )
float CDegree2PTZunit::GetPanUnitFromYAXISRotateDegree(float fYRotation)
{
	return (fYRotation/m_fMaxPanDegree) * ( m_fMaxPanUnit - m_fMinPanUnit ) + m_fMinPanUnit;
}
// rotate X-axis ( 0 ~ 90 ) mapping to ( fMaxTilt ~ fMinTilt )
float CDegree2PTZunit::GetTiltUnitFromXAXISRotateDegree(float fXRotation)
{
	return (fXRotation/m_fMaxTiltDegree) * ( m_fMaxTiltUnit - m_fMinTiltUnit ) + m_fMinTiltUnit;
}
// get zoom degree (0 ~ 100) mapping to ( fMinZoom ~ fMaxZoom )
float CDegree2PTZunit::GetZoomUnitFromZoomPercent(float fZoomPercent)
{
	return (fZoomPercent/100.0f) * ( m_fMaxZoomUnit - m_fMinZoomUnit ) + m_fMinZoomUnit;
}

// ( fMaxPan ~ fMinPan ) mapping to rotate Y-axis (0 ~ 360 ) 
float CDegree2PTZunit::GetYAXISRotateDegreeFromPanUnit(float fPanUnit)
{
	return (fPanUnit - m_fMinPanUnit)/(m_fMaxPanUnit - m_fMinPanUnit) * m_fMaxPanDegree;
}
// ( fMaxTilt ~ fMinTilt ) mapping to rotate X-axis ( 0 ~ 90 ) 
float CDegree2PTZunit::GetXAXISRotateDegreeFromTiltUnit(float fTiltUnit)
{
	return (fTiltUnit - m_fMinTiltUnit)/(m_fMaxTiltUnit - m_fMinTiltUnit) * m_fMaxTiltDegree;
}
// ( fMinZoom ~ fMaxZoom ) mapping to get zoom degree (0 ~ 100 ) 
float CDegree2PTZunit::GetZoomPercentFromZoomUnit(float fZoonUnit)
{
	return (fZoonUnit - m_fMinZoomUnit)/(m_fMaxZoomUnit - m_fMinZoomUnit) * 100.0f;
}
