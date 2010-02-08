#pragma once

class CVideoWindow : public CWnd
{
public:
	CVideoWindow(bool *pbFullScreen = NULL);
	virtual ~CVideoWindow();

	DWORD	m_XLEN;
	DWORD	m_YLEN;

private:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	bool *m_pbFullScreen;

protected:
	DECLARE_MESSAGE_MAP()
};
