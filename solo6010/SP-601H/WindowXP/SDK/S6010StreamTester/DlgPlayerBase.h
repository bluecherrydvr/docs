#if !defined(AFX_DLGPLAYERBASE_H__B90912B0_CD02_445C_8460_F73711AA1839__INCLUDED_)
#define AFX_DLGPLAYERBASE_H__B90912B0_CD02_445C_8460_F73711AA1839__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgPlayerBase.h : header file
//

#include "WndS6010Disp.h"
#include "InfoS6010StreamIdx.h"
#include "SnapshotManager.h"
#include "SoundOut.h"

enum
{
	IDX_TH_MP4_PLAYER,
	IDX_TH_G723_PLAYER,
	NUM_THREAD
};

enum
{
	CTRL_DLG_MP4,
	CTRL_DLG_G723,
	NUM_CTRL_DLG
};

enum
{
	EVT_MP4_PLAYER,
	NUM_EVENT
};

enum
{
	IDX_PLAYER_REQ_INIT,	// File Open & First Image Display & Pause
	IDX_PLAYER_REQ_PLAY,	// Play Request -> Time Init & Play
	IDX_PLAYER_REQ_STOP,	// Stop Request -> Show First Frame & Init File Pointer & Pause
	IDX_PLAYER_PLAYING,
	IDX_PLAYER_PAUSE,
	IDX_PLAYER_BACK_GOP,
	IDX_PLAYER_FW_ONE_FRM,
	IDX_PLAYER_FW_GOP,
	IDX_PLAYER_SET_POS,
};

enum
{
	G723_PLAYER_REQ_PLAY,
	G723_PLAYER_REQ_STOP,
	G723_PLAYER_PLAYING,
	G723_PLAYER_PAUSE,
	G723_PLAYER_SET_POS,
};

class CDlgMainControl;
class CDlgG723PlayCtrl;
extern CDlgMainControl glDlgMC;
extern CDlgG723PlayCtrl glDlgG723PC;

/////////////////////////////////////////////////////////////////////////////
// CDlgPlayerBase dialog

class CDlgPlayerBase : public CDialog
{
// Construction
public:
	CDlgPlayerBase(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgPlayerBase)
	enum { IDD = IDD_DLG_WND_PLAYER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	CWndS6010Disp m_wndS6010Disp;
	CInfoS6010StreamIdx m_infoStreamIdx;
	CSnapshotManager m_manSnapshot;

	BOOL m_bFileOpen;
	CString m_strStreamFilePath;
	SIZE m_szDispWnd;
	BOOL m_bUpdateDispWndPt[MAX_DISP_WND];
	BOOL m_bFillDispWndBlack[MAX_DISP_WND];	// Fill Disp Pos to Black (Channel Disp Base)...
		// use glPWndDisp->FillBackBlack () to fill entire area.

	int m_idxCurDispMode;
	int m_idxCurVideoMode;

	int m_idxCurPlayerMode[MAX_DISP_WND];
	HANDLE m_hXvidDec[MAX_DISP_WND];
	UINT m_nPlayerTimerRes;	// Player Timer Resolution
	UINT m_nPlayerTimerID;	// Player Timer ID
	CEvent m_bufEvent[NUM_EVENT];	// Player Thread Event
	CCriticalSection m_crtPlayer;	// m_idxModePlayer access lock

	BOOL m_bDeblocking;
	BOOL m_bDeringing;
	BOOL m_bDeinterlace;
	BOOL m_bMotionInfo;
	BOOL m_bChannelInfo;
	BOOL m_bWMInfo;
	BOOL m_bIBasedPred;

	BOOL m_bMotionEx;
	int m_nAlign;

	volatile BOOL m_bEndThread[NUM_THREAD];

	int m_idxCurDispCh[MAX_DISP_WND];	// Channel index that displayed each position...
	int m_idxCurSelDisp;
	int m_idxCurZoom[NUM_ZOOM];

	// For Snapshot[S]...
	BOOL m_bReqSnapshot;	// Snapshot Button Down...
	CString m_strSnapshotFilePath;	// Snapshot File Path...
	// For Snapshot[E]...

	int m_posSetFrm[MAX_DISP_WND];

	// For G723 Player[S]
	CString m_strG723FilePath;
	CSoundOut m_soundOut;
	DWORD m_numG723Frm;
	int m_posCurG723Frm;
	volatile int m_curG723PlayerMode;
	int m_curG723PlayCh;
	CCriticalSection m_crtG723PlayerMode;	// m_curG723PlayerMode access lock

	BOOL OpenG723File (CString strFilePath);
	void ChangeCurG723PlayerMode (int idxMode);
	void ChangeCurG723PlayCh (int idxCh);
	__inline DWORD GetNumG723Frm () { return m_numG723Frm; }
	__inline int GetCurPosG723Frm () { return m_posCurG723Frm; }
	BOOL WriteG723RAW (int idxCh, const char *strFilePath);
	// For G723 Player[E]

	BOOL OpenStreamFile (CString strStreamFilePath);
	__inline BOOL IsFileOpened () { return m_bFileOpen; }

	void StartMP4Player ();
	void EndMP4Player ();
	void StartG723Player ();
	void EndG723Player ();
	void ChangeCurPlayerMode (int idxMode);
	void ChangeCurPlayerMode (int idxDisp, int idxMode);
	__inline int GetCurPlayerMode (int idxDisp) { return m_idxCurPlayerMode[idxDisp]; }
	__inline int GetCurPlayerMode () { return m_idxCurPlayerMode[m_idxCurSelDisp]; }
	__inline int GetCurDispMode () { return m_idxCurDispMode; }
	void UpdateDisplay ();
	void UpdatePlayerWndPt (BOOL bUpdate);
	void FillDispWndBlack ();
		
	void ChangeDispMode (int idxDispMode);
	void ChangeChannel (int idxCh);
	void ChangeZoom (int idxHV, int idxZoom);
	void ChangeDispSize (int idxDispSz);

	void TakeSnapshot (CString strSnapshotFilePath);
	void MakeAVIFile (CString strAVIFilePath, int idxCh);
	void MakeRAWStream (CString strFilePath, int idxCh, BOOL bInsertVOLH);

	void S6010StreamPlayerQuit ();
	void OnKeyDownFromWndDisp (UINT nChar, UINT nRepCnt, UINT nFlags);

	int GetFrameNum (int idxDisp);
	void SetFramePos (int idxDisp, int idxPos);
	void SetFramePosInPercent (int idxDisp, int idxPosPercent);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgPlayerBase)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	afx_msg LRESULT OnCtrlDlgChange (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDispWndLButClick (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDispWndLButDblClk (WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDispWndMouseDrag (WPARAM wParam, LPARAM lParam);
	// Generated message map functions
	//{{AFX_MSG(CDlgPlayerBase)
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGPLAYERBASE_H__B90912B0_CD02_445C_8460_F73711AA1839__INCLUDED_)
