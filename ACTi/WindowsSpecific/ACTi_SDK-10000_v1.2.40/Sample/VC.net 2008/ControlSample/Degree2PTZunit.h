#pragma once

class CDegree2PTZunit
{
public:
	CDegree2PTZunit(void);
	virtual ~CDegree2PTZunit(void);


private:
	float m_fMaxPanUnit;
	float m_fMinPanUnit;

	float m_fMaxTiltUnit;
	float m_fMinTiltUnit;

	float m_fMaxZoomUnit;
	float m_fMinZoomUnit;

	float m_fMaxPanDegree;
	float m_fMaxTiltDegree;


public:
	bool IsInit();
	void SetMaxPanDegreeAnsMaxTiltDegree(float fMaxPanDegree, float fMaxTiltDegree);
	void SetPanUnit(float fMaxPan, float fMinPan);
	void SetTiltUnit(float fMaxTilt, float fMinTilt);
	void SetZoomUnit(float fMaxZoom, float fMinZoom);
	/*
			
			^  up is y axis  
			|
			|
			 ------>  right is x axis

			 and the front is z axis
	*/


	// rotate Y-axis (0 ~ 360 ) mapping to ( fMaxPan ~ fMinPan )
	float GetPanUnitFromYAXISRotateDegree(float fYRotation);
	// rotate X-axis ( 0 ~ 90 ) mapping to ( fMaxTilt ~ fMinTilt )
	float GetTiltUnitFromXAXISRotateDegree(float fXRotation);
	// get zoom degree (0 ~ 100 ) mapping to ( fMinZoom ~ fMaxZoom )
	float GetZoomUnitFromZoomPercent(float fZoomPercent);

	// ( fMaxPan ~ fMinPan ) mapping to rotate Y-axis (0 ~ 360 ) 
	float GetYAXISRotateDegreeFromPanUnit(float fPanUnit);
	// ( fMaxTilt ~ fMinTilt ) mapping to rotate X-axis ( 0 ~ 90 ) 
	float GetXAXISRotateDegreeFromTiltUnit(float fTiltUnit);
	// ( fMinZoom ~ fMaxZoom ) mapping to get zoom degree (0 ~ 100 ) 
	float GetZoomPercentFromZoomUnit(float fZoonUnit);

};
