/************************************************************************
 *
 * Copyright 2008 Stretch, Inc. All rights reserved.
 *
 * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE SECRETS OF
 * STRETCH, INC. USE, DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT
 * THE PRIOR EXPRESS WRITTEN PERMISSION OF STRETCH, INC.
 *
 ************************************************************************/

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "splayer.h"


class CChildFrame;

// CTestPlayersApp:
// See TestPlayers.cpp for the implementation of this class
//
class CTestPlayersApp : public CWinApp
{
public:
	CTestPlayersApp();

// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	void Close();
	void HandleDecodedFrame(sx_int32 port, splayer_frame_t *frame);

// Implementation
protected:
	HMENU m_hMDIMenu;
	HACCEL m_hMDIAccel;

public:
	afx_msg void OnAppAbout();
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenStream();
	afx_msg void OnActionPlay();
	afx_msg void OnActionStop();
	afx_msg void OnActionPause();
	afx_msg void OnActionFastFwd();
	afx_msg void OnActionFastRew();
	afx_msg void OnActionStepFwd();
	afx_msg void OnActionStepBwd();
	afx_msg void OnActionCapture();
	afx_msg void OnActionDecode();
	afx_msg void OnActionZoom();
	DECLARE_MESSAGE_MAP()

private:
	bool	m_bInitialized;		// flag of initialization
	int		m_iNumPlayers;		// number of players
	CChildFrame **m_ppChilds;
};

class CNewDialog : public CDialog
{
public:
	CNewDialog();

	int GetNumChannles();

// Dialog Data
	enum { IDD = IDD_DIALOG_NEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	int		m_iIndex;
};

class CZoomDialog : public CDialog
{
public:
	CZoomDialog();

	int GetWindowId() {return m_iWindow;}
	int GetLeft() {return m_iLeft;}
	int GetTop() {return m_iTop;}
	int GetWidth() {return m_iWidth;}
	int GetHeight() {return m_iHeight;}
	void SetLeft(int iVal) {m_iLeft = iVal;}
	void SetTop(int iVal) {m_iTop = iVal;}
	void SetWidth(int iVal) {m_iWidth = iVal;}
	void SetHeight(int iVal) {m_iHeight = iVal;}

// Dialog Data
	enum { IDD = IDD_DIALOG_ZOOM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	int		m_iWindow;
	int		m_iLeft;
	int		m_iTop;
	int		m_iWidth;
	int		m_iHeight;
};

extern CTestPlayersApp theApp;