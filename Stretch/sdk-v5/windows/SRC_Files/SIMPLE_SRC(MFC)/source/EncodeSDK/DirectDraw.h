#pragma once


#include "memcpy.h"
#include "vfw.h"
#include "DDraw.h"
// CDirectDraw

class CDirectDraw : public CWnd
{
	DECLARE_DYNAMIC(CDirectDraw)

public:
	CDirectDraw();
	virtual ~CDirectDraw();
	LPDIRECTDRAW7				m_lpDD;				
	LPDIRECTDRAWSURFACE7		m_lpDDSPrimary;		
	LPDIRECTDRAWSURFACE7		m_lpDDSOffScr;		
	LPDIRECTDRAWCLIPPER			m_lpClipper;
	DDSURFACEDESC2				m_ddsd;				
	DDSURFACEDESC2				m_ddsds;					
	
	CWnd						*m_pDrawWin;

	int							m_iVdo_YBufSize;
	int							m_iVdo_UBufSize;
	int							m_iVdo_VBufSize;

	BOOL InitDirectDraw(USHORT usImgWidth, USHORT usImgHeight, CWnd* pcWnd);
	//BOOL DrawYV12(LPBYTE lpYV12, USHORT usImgWidth, USHORT usImgHeight, CWnd* pcWnd);
	BOOL CDirectDraw::DrawYV12(LPBYTE lpYV12, USHORT usImgWidth, USHORT usImgHeight, CWnd* pcWnd, HWND hWnd, RECT rcdraw);
	BOOL DrawYUV(char * y, char * u, char *v, USHORT usImgWidth, USHORT usImgHeight, CWnd* pcWnd);
	BOOL CloseDirectDraw();	
	void memcpyMMX(void *Dest, void *Src, size_t nBytes);
protected:
	DECLARE_MESSAGE_MAP()
};


