#include "stdafx.h"
#include "S6010StreamTester.h"
#include "DlgMainControl.h"

#include "DlgPlayerBase.h"
#include "DlgAVISave.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDlgMainControl *glPDlgMC;


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgMainControl dialog

CDlgMainControl::CDlgMainControl(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgMainControl::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgMainControl)
	m_edStrStreamFilePath = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32

	m_idxCurSelChBut = IDX_CBUT_CH_1;
	m_idxCurZoom[IDX_ZOOM_HOR] = m_idxCurZoom[IDX_ZOOM_VER] = IDX_ZOOM_FIT;
}

void CDlgMainControl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgMainControl)
	DDX_Control(pDX, IDC_SLI_PLAYER_ALL, m_sliPlayerAll);
	DDX_Control(pDX, IDC_SLI_PLAYER, m_sliPlayer);
	DDX_Text(pDX, IDC_ED_STREAM_FILE_PATH, m_edStrStreamFilePath);
	//}}AFX_DATA_MAP
	
	DDX_Control(pDX, IDC_BUT_OPEN_STREAM_FILE, m_butAll[IDX_BUT_OPEN_STREAM_FILE]);

	DDX_Control(pDX, IDC_BUT_DISP_1, m_butAll[IDX_BUT_DISP_1]);
	DDX_Control(pDX, IDC_BUT_DISP_4, m_butAll[IDX_BUT_DISP_4]);
	DDX_Control(pDX, IDC_BUT_DISP_9, m_butAll[IDX_BUT_DISP_9]);
	DDX_Control(pDX, IDC_BUT_DISP_16, m_butAll[IDX_BUT_DISP_16]);
	DDX_Control(pDX, IDC_BUT_DISP_1V, m_butAll[IDX_BUT_DISP_V1]);
	DDX_Control(pDX, IDC_BUT_DISP_4V, m_butAll[IDX_BUT_DISP_V4]);
	DDX_Control(pDX, IDC_BUT_DISP_9V, m_butAll[IDX_BUT_DISP_V9]);
	DDX_Control(pDX, IDC_BUT_DISP_16V, m_butAll[IDX_BUT_DISP_V16]);
	DDX_Control(pDX, IDC_BUT_DISP_32, m_butAll[IDX_BUT_DISP_32]);
	DDX_Control(pDX, IDC_BUT_DISP_36, m_butAll[IDX_BUT_DISP_36]);
	
	DDX_Control(pDX, IDC_CH_CHANNEL_1, m_butAll[IDX_CBUT_CH_1]);
	DDX_Control(pDX, IDC_CH_CHANNEL_2, m_butAll[IDX_CBUT_CH_2]);
	DDX_Control(pDX, IDC_CH_CHANNEL_3, m_butAll[IDX_CBUT_CH_3]);
	DDX_Control(pDX, IDC_CH_CHANNEL_4, m_butAll[IDX_CBUT_CH_4]);
	DDX_Control(pDX, IDC_CH_CHANNEL_5, m_butAll[IDX_CBUT_CH_5]);
	DDX_Control(pDX, IDC_CH_CHANNEL_6, m_butAll[IDX_CBUT_CH_6]);
	DDX_Control(pDX, IDC_CH_CHANNEL_7, m_butAll[IDX_CBUT_CH_7]);
	DDX_Control(pDX, IDC_CH_CHANNEL_8, m_butAll[IDX_CBUT_CH_8]);
	DDX_Control(pDX, IDC_CH_CHANNEL_9, m_butAll[IDX_CBUT_CH_9]);
	DDX_Control(pDX, IDC_CH_CHANNEL_10, m_butAll[IDX_CBUT_CH_10]);
	DDX_Control(pDX, IDC_CH_CHANNEL_11, m_butAll[IDX_CBUT_CH_11]);
	DDX_Control(pDX, IDC_CH_CHANNEL_12, m_butAll[IDX_CBUT_CH_12]);
	DDX_Control(pDX, IDC_CH_CHANNEL_13, m_butAll[IDX_CBUT_CH_13]);
	DDX_Control(pDX, IDC_CH_CHANNEL_14, m_butAll[IDX_CBUT_CH_14]);
	DDX_Control(pDX, IDC_CH_CHANNEL_15, m_butAll[IDX_CBUT_CH_15]);
	DDX_Control(pDX, IDC_CH_CHANNEL_16, m_butAll[IDX_CBUT_CH_16]);

	DDX_Control(pDX, IDC_CH_CHANNEL_17, m_butAll[IDX_CBUT_CH_V1]);
	DDX_Control(pDX, IDC_CH_CHANNEL_18, m_butAll[IDX_CBUT_CH_V2]);
	DDX_Control(pDX, IDC_CH_CHANNEL_19, m_butAll[IDX_CBUT_CH_V3]);
	DDX_Control(pDX, IDC_CH_CHANNEL_20, m_butAll[IDX_CBUT_CH_V4]);
	DDX_Control(pDX, IDC_CH_CHANNEL_21, m_butAll[IDX_CBUT_CH_V5]);
	DDX_Control(pDX, IDC_CH_CHANNEL_22, m_butAll[IDX_CBUT_CH_V6]);
	DDX_Control(pDX, IDC_CH_CHANNEL_23, m_butAll[IDX_CBUT_CH_V7]);
	DDX_Control(pDX, IDC_CH_CHANNEL_24, m_butAll[IDX_CBUT_CH_V8]);
	DDX_Control(pDX, IDC_CH_CHANNEL_25, m_butAll[IDX_CBUT_CH_V9]);
	DDX_Control(pDX, IDC_CH_CHANNEL_26, m_butAll[IDX_CBUT_CH_V10]);
	DDX_Control(pDX, IDC_CH_CHANNEL_27, m_butAll[IDX_CBUT_CH_V11]);
	DDX_Control(pDX, IDC_CH_CHANNEL_28, m_butAll[IDX_CBUT_CH_V12]);
	DDX_Control(pDX, IDC_CH_CHANNEL_29, m_butAll[IDX_CBUT_CH_V13]);
	DDX_Control(pDX, IDC_CH_CHANNEL_30, m_butAll[IDX_CBUT_CH_V14]);
	DDX_Control(pDX, IDC_CH_CHANNEL_31, m_butAll[IDX_CBUT_CH_V15]);
	DDX_Control(pDX, IDC_CH_CHANNEL_32, m_butAll[IDX_CBUT_CH_V16]);

	DDX_Control(pDX, IDC_BUT_SETTING_GENERAL, m_butAll[IDX_BUT_SETTING_GENERAL]);
	DDX_Control(pDX, IDC_BUT_MINIMIZE, m_butAll[IDX_BUT_MINIMIZE]);

	DDX_Control(pDX, IDC_BUT_PLAY, m_butAll[IDX_BUT_PLAYER_PLAY]);
	DDX_Control(pDX, IDC_BUT_BACK_GOP, m_butAll[IDX_BUT_PLAYER_BACK_GOP]);
	DDX_Control(pDX, IDC_BUT_PAUSE, m_butAll[IDX_BUT_PLAYER_PAUSE]);
	DDX_Control(pDX, IDC_BUT_FW_ONE_FRM, m_butAll[IDX_BUT_PLAYER_FW_ONE_FRM]);
	DDX_Control(pDX, IDC_BUT_FW_GOP, m_butAll[IDX_BUT_PLAYER_FW_GOP]);
	DDX_Control(pDX, IDC_BUT_STOP, m_butAll[IDX_BUT_PLAYER_STOP]);

	DDX_Control(pDX, IDC_BUT_PLAY_ALL, m_butAll[IDX_BUT_PLAYER_PLAY_ALL]);
	DDX_Control(pDX, IDC_BUT_BACK_GOP_ALL, m_butAll[IDX_BUT_PLAYER_BACK_GOP_ALL]);
	DDX_Control(pDX, IDC_BUT_PAUSE_ALL, m_butAll[IDX_BUT_PLAYER_PAUSE_ALL]);
	DDX_Control(pDX, IDC_BUT_FW_ONE_FRM_ALL, m_butAll[IDX_BUT_PLAYER_FW_ONE_FRM_ALL]);
	DDX_Control(pDX, IDC_BUT_FW_GOP_ALL, m_butAll[IDX_BUT_PLAYER_FW_GOP_ALL]);
	DDX_Control(pDX, IDC_BUT_STOP_ALL, m_butAll[IDX_BUT_PLAYER_STOP_ALL]);

	DDX_Control(pDX, IDC_CH_DEBLOCKING, m_butAll[IDX_CBUT_MISC_DEBLOCKING]);
	DDX_Control(pDX, IDC_CH_DERINGING, m_butAll[IDX_CBUT_MISC_DERINGING]);
	DDX_Control(pDX, IDC_CH_DEINTERLACE, m_butAll[IDX_CBUT_MISC_DEINTERLACE]);
	DDX_Control(pDX, IDC_CH_MOTION_INFO, m_butAll[IDX_CBUT_MISC_MOTION_INFO]);
	DDX_Control(pDX, IDC_CH_CHANNEL_INFO, m_butAll[IDX_CBUT_MISC_CHANNEL_INFO]);
	DDX_Control(pDX, IDC_CH_WATERMARK_INFO, m_butAll[IDX_CBUT_MISC_WM_INFO]);
	DDX_Control(pDX, IDC_BUT_AVI_SAVE, m_butAll[IDX_BUT_MISC_AVI_SAVE]);
	DDX_Control(pDX, IDC_BUT_SNAPSHOT, m_butAll[IDX_BUT_MISC_SNAPSHOT]);
	DDX_Control(pDX, IDC_CH_I_BASED_PRED, m_butAll[IDX_CBUT_MISC_I_BASED_PRED]);
	

	DDX_Control(pDX, IDC_CH_ZOOM_H_X1, m_butAll[IDX_CBUT_ZOOM_H_X1]);
	DDX_Control(pDX, IDC_CH_ZOOM_H_X2, m_butAll[IDX_CBUT_ZOOM_H_X2]);
	DDX_Control(pDX, IDC_CH_ZOOM_H_X3, m_butAll[IDX_CBUT_ZOOM_H_X3]);
	DDX_Control(pDX, IDC_CH_ZOOM_H_FIT, m_butAll[IDX_CBUT_ZOOM_H_FIT]);
	DDX_Control(pDX, IDC_CH_ZOOM_V_X1, m_butAll[IDX_CBUT_ZOOM_V_X1]);
	DDX_Control(pDX, IDC_CH_ZOOM_V_X2, m_butAll[IDX_CBUT_ZOOM_V_X2]);
	DDX_Control(pDX, IDC_CH_ZOOM_V_X3, m_butAll[IDX_CBUT_ZOOM_V_X3]);
	DDX_Control(pDX, IDC_CH_ZOOM_V_FIT, m_butAll[IDX_CBUT_ZOOM_V_FIT]);

	DDX_Control(pDX, IDC_BUT_SZ_DISP_X1, m_butAll[IDX_BUT_SZ_DISP_X1]);
	DDX_Control(pDX, IDC_BUT_SZ_DISP_X2, m_butAll[IDX_BUT_SZ_DISP_X2]);
	DDX_Control(pDX, IDC_BUT_SZ_DISP_X3, m_butAll[IDX_BUT_SZ_DISP_X3]);
	DDX_Control(pDX, IDC_BUT_SZ_DISP_FULL, m_butAll[IDX_BUT_SZ_DISP_FULL]);
}

BEGIN_MESSAGE_MAP(CDlgMainControl, CDialog)
	ON_MESSAGE (WM_DOO_INDEX_BUTTON_CLICK, OnIndexButtonClick)
	ON_MESSAGE (WM_DOO_UPDATE_FILE_PATH, OnUpdateFilePath)
	//{{AFX_MSG_MAP(CDlgMainControl)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_NCHITTEST()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgMainControl message handlers

BOOL CDlgMainControl::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	
	m_butAll[IDX_CBUT_MISC_CHANNEL_INFO].SetCheck (TRUE);
	m_butAll[m_idxCurZoom[IDX_ZOOM_HOR] +IDX_CBUT_ZOOM_H_X1].SetCheck (TRUE);
	m_butAll[m_idxCurZoom[IDX_ZOOM_VER] +IDX_CBUT_ZOOM_V_X1].SetCheck (TRUE);

	m_sliPlayer.SetRange (0, 0);
	m_sliPlayer.SetPos (0);
	m_sliPlayerAll.SetRange (0, 100);
	m_sliPlayerAll.SetPos (0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CDlgMainControl::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDlgMainControl::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CDialog::OnPaint() for painting messages
}

int CDlgMainControl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	int i;

	for (i=0; i<NUM_ALL_BUTTONS; i++)
	{
		m_butAll[i].SetID (i);
	}

	glPDlgMC = this;

	return 0;
}

void CDlgMainControl::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
}

LRESULT CDlgMainControl::OnIndexButtonClick (WPARAM wParam, LPARAM lParam)
{
	if (wParam <= IDX_BUT_OPEN_STREAM_FILE)	// File Open...
	{
		CFileDialog dlg (TRUE);
		CString strTmp;

		if (dlg.DoModal () == IDOK)
		{
			strTmp = dlg.GetPathName ();

			if (strTmp.Find ("RG7_") != -1)
			{	// G723 File Open
				GetParent ()->SendMessage (WM_DOO_CTRL_DLG_CHANGE, CTRL_DLG_G723, (LONG)&strTmp);
				return 0;
			}
			m_edStrStreamFilePath = strTmp;
			UpdateData (FALSE);

			if (glPDlgPB->OpenStreamFile (m_edStrStreamFilePath) == FALSE)
			{
				AfxMessageBox ("ERROR: Invalid Stream File");
			}
			else
			{
				if (glPDlgPB->m_idxCurDispMode == IDX_DISP_MODE_1)
				{
					int i;
					for (i=0; i<MAX_ENC_CHANNEL; i++)
					{
						if (glPDlgPB->m_infoStreamIdx.GetMaxFrameNum (i) > 0)
						{
							OnIndexButtonClick (IDX_CBUT_CH_1 +i, 0);
							break;
						}
					}
				}
				ChangeSliderState ();
			}
		}
	}
	else if (wParam <= IDX_BUT_DISP_36)	// Display...
	{
		glPDlgPB->ChangeDispMode (wParam -IDX_BUT_DISP_1 +IDX_DISP_MODE_1);
		ChangeSliderState ();
	}
	else if (wParam <= IDX_CBUT_CH_V16)	// Channel...
	{
		if (m_idxCurSelChBut == int(wParam))
		{
			m_butAll[wParam].SetCheck (TRUE);
		}
		else
		{
			m_butAll[m_idxCurSelChBut].SetCheck (FALSE);

			glPDlgPB->ChangeChannel (wParam -IDX_CBUT_CH_1);
			ChangeSliderState ();
			
			m_idxCurSelChBut = wParam;
			m_butAll[m_idxCurSelChBut].SetCheck (TRUE);
		}
	}
	else if (wParam == IDX_BUT_SETTING_GENERAL)	// Settings...
	{
	}
	else if (wParam == IDX_BUT_MINIMIZE)	// Minimize...
	{
		ShowWindow (SW_HIDE);
	}
	else if (wParam <= IDX_BUT_PLAYER_STOP)	// Player (Selected Channel)...
	{
		switch (wParam)
		{
		case IDX_BUT_PLAYER_PLAY:
			glPDlgPB->ChangeCurPlayerMode (IDX_PLAYER_REQ_PLAY);
			break;
		case IDX_BUT_PLAYER_PAUSE:
			glPDlgPB->ChangeCurPlayerMode (IDX_PLAYER_PAUSE);
			break;
		case IDX_BUT_PLAYER_STOP:
			glPDlgPB->ChangeCurPlayerMode (IDX_PLAYER_REQ_STOP);
			break;
		case IDX_BUT_PLAYER_BACK_GOP:
		case IDX_BUT_PLAYER_FW_ONE_FRM:
		case IDX_BUT_PLAYER_FW_GOP:
			if (glPDlgPB->GetCurPlayerMode () == IDX_PLAYER_PAUSE)
			{
				switch (wParam)
				{
				case IDX_BUT_PLAYER_BACK_GOP:
					glPDlgPB->ChangeCurPlayerMode (IDX_PLAYER_BACK_GOP);
					break;
				case IDX_BUT_PLAYER_FW_ONE_FRM:
					glPDlgPB->ChangeCurPlayerMode (IDX_PLAYER_FW_ONE_FRM);
					break;
				case IDX_BUT_PLAYER_FW_GOP:
					glPDlgPB->ChangeCurPlayerMode (IDX_PLAYER_FW_GOP);
					break;
				}
			}
		}
	}
	else if (wParam <= IDX_BUT_PLAYER_STOP_ALL)	// Player (All Channel)...
	{
		int i, nNumCurDisp;
		nNumCurDisp = GL_DISP_MODE_TO_NUM_DISP[glPDlgPB->GetCurDispMode ()];

		switch (wParam)
		{
		case IDX_BUT_PLAYER_PLAY_ALL:
			for (i=0; i<nNumCurDisp; i++)
			{
				glPDlgPB->ChangeCurPlayerMode (i, IDX_PLAYER_REQ_PLAY);
			}
			break;
		case IDX_BUT_PLAYER_PAUSE_ALL:
			for (i=0; i<nNumCurDisp; i++)
			{
				glPDlgPB->ChangeCurPlayerMode (i, IDX_PLAYER_PAUSE);
			}
			break;
		case IDX_BUT_PLAYER_STOP_ALL:
			for (i=0; i<nNumCurDisp; i++)
			{
				glPDlgPB->ChangeCurPlayerMode (i, IDX_PLAYER_REQ_STOP);
			}
			break;
		case IDX_BUT_PLAYER_BACK_GOP_ALL:
		case IDX_BUT_PLAYER_FW_ONE_FRM_ALL:
		case IDX_BUT_PLAYER_FW_GOP_ALL:
			for (i=0; i<nNumCurDisp; i++)
			{
				if (glPDlgPB->GetCurPlayerMode (i) == IDX_PLAYER_PAUSE)
				{
					switch (wParam)
					{
					case IDX_BUT_PLAYER_BACK_GOP_ALL:
						glPDlgPB->ChangeCurPlayerMode (i, IDX_PLAYER_BACK_GOP);
						break;
					case IDX_BUT_PLAYER_FW_ONE_FRM_ALL:
						glPDlgPB->ChangeCurPlayerMode (i, IDX_PLAYER_FW_ONE_FRM);
						break;
					case IDX_BUT_PLAYER_FW_GOP_ALL:
						glPDlgPB->ChangeCurPlayerMode (i, IDX_PLAYER_FW_GOP);
						break;
					}
				}
			}
		}
	}
	else if (wParam <= IDX_CBUT_MISC_I_BASED_PRED)	// Misc...
	{
		switch (wParam)
		{
		case IDX_CBUT_MISC_DEBLOCKING:
			glPDlgPB->m_bDeblocking = 1 -glPDlgPB->m_bDeblocking;
			break;
		case IDX_CBUT_MISC_DERINGING:
			glPDlgPB->m_bDeringing = 1 -glPDlgPB->m_bDeringing;
			break;
		case IDX_CBUT_MISC_DEINTERLACE:
			glPDlgPB->m_bDeinterlace = 1 -glPDlgPB->m_bDeinterlace;
			break;
		case IDX_CBUT_MISC_MOTION_INFO:
			glPDlgPB->m_bMotionInfo = 1 -glPDlgPB->m_bMotionInfo;

			glPWndDisp->Invalidate (FALSE);
			break;
		case IDX_CBUT_MISC_CHANNEL_INFO:
			glPDlgPB->m_bChannelInfo = 1 -glPDlgPB->m_bChannelInfo;

			glPWndDisp->Invalidate (FALSE);
			break;
		case IDX_CBUT_MISC_WM_INFO:
			glPDlgPB->m_bWMInfo = 1 -glPDlgPB->m_bWMInfo;
			break;
		case IDX_BUT_MISC_AVI_SAVE:
			OnAVISaveButDn ();
			break;
		case IDX_BUT_MISC_SNAPSHOT:
			OnSnapshotButDn ();
			break;
		case IDX_CBUT_MISC_I_BASED_PRED:
			glPDlgPB->m_bIBasedPred = 1 -glPDlgPB->m_bIBasedPred;
			break;
		}
	}
	else if (wParam <= IDX_CBUT_ZOOM_V_FIT)	// MiscZoom
	{
		ZoomButtonDn (wParam);
	}
	else if (wParam <= IDX_BUT_SZ_DISP_FULL)	// Disp Size
	{
		glPDlgPB->ChangeDispSize (wParam -IDX_BUT_SZ_DISP_X1 +IDX_DISP_SZ_X1);
	}

	return 0;
}

void CDlgMainControl::OnAVISaveButDn ()
{
	if (glPDlgPB->IsFileOpened () == FALSE)
	{
		AfxMessageBox ("Open stream file and try again.");
		return;
	}

	CDlgAVISave dlg;
	dlg.DoModal ();
}

void CDlgMainControl::ZoomButtonDn (int wParam)
{
	if (wParam <= IDX_CBUT_ZOOM_H_FIT)
	{
		if (m_idxCurZoom[IDX_ZOOM_HOR] != wParam -IDX_CBUT_ZOOM_H_X1)
		{
			m_butAll[m_idxCurZoom[IDX_ZOOM_HOR] +IDX_CBUT_ZOOM_H_X1].SetCheck (FALSE);
		}

		glPDlgPB->ChangeZoom (IDX_ZOOM_HOR, wParam -IDX_CBUT_ZOOM_H_X1);
		m_idxCurZoom[IDX_ZOOM_HOR] = wParam -IDX_CBUT_ZOOM_H_X1;
		m_butAll[m_idxCurZoom[IDX_ZOOM_HOR] +IDX_CBUT_ZOOM_H_X1].SetCheck (TRUE);
	}
	else
	{
		if (m_idxCurZoom[IDX_ZOOM_VER] != wParam -IDX_CBUT_ZOOM_V_X1)
		{
			m_butAll[m_idxCurZoom[IDX_ZOOM_VER] +IDX_CBUT_ZOOM_V_X1].SetCheck (FALSE);
		}

		glPDlgPB->ChangeZoom (IDX_ZOOM_VER, wParam -IDX_CBUT_ZOOM_V_X1);
		m_idxCurZoom[IDX_ZOOM_VER] = wParam -IDX_CBUT_ZOOM_V_X1;
		m_butAll[m_idxCurZoom[IDX_ZOOM_VER] +IDX_CBUT_ZOOM_V_X1].SetCheck (TRUE);
	}
}

void CDlgMainControl::OnSnapshotButDn ()
{
	CFileDialog dlg(FALSE, "BMP");

	if (dlg.DoModal () == IDOK)
	{
		glPDlgPB->TakeSnapshot (dlg.GetPathName ());
	}
}

void CDlgMainControl::ChangeChannelButState (int idxSelCh)
{
	m_butAll[m_idxCurSelChBut].SetCheck (FALSE);
	m_idxCurSelChBut = idxSelCh +IDX_CBUT_CH_1;
	m_butAll[m_idxCurSelChBut].SetCheck (TRUE);
}

void CDlgMainControl::OnOK() 
{
	// TODO: Add extra validation here
	
	glPDlgPB->S6010StreamPlayerQuit ();

	CDialog::OnOK();
}

void CDlgMainControl::OnCancel() 
{
	// TODO: Add extra validation here
	
	glPDlgPB->S6010StreamPlayerQuit ();

	CDialog::OnCancel();
}

UINT CDlgMainControl::OnNcHitTest(CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	return HTCAPTION;
/*	int szCaption = GetSystemMetrics (SM_CYCAPTION) +GetSystemMetrics (SM_CYDLGFRAME);

	CRect rcWnd;
	GetWindowRect (&rcWnd);
	if (point.y >= rcWnd.top +szCaption)
	{
		return HTCAPTION;
	}*/
	return CDialog::OnNcHitTest(point);
}

void CDlgMainControl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	
	CSliderCtrl *pSlider = (CSliderCtrl *)pScrollBar;
	int i;

	switch (pScrollBar->GetDlgCtrlID ())
	{
	case IDC_SLI_PLAYER:
		glPDlgPB->SetFramePos (glPDlgPB->m_idxCurSelDisp, pSlider->GetPos ());
		break;
	case IDC_SLI_PLAYER_ALL:
		for (i=0; i<GL_DISP_MODE_TO_NUM_DISP[glPDlgPB->GetCurDispMode ()]; i++)
		{
			glPDlgPB->SetFramePosInPercent (i, pSlider->GetPos ());
		}
		break;
	}
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CDlgMainControl::ChangeSliderState ()
{
	m_sliPlayer.SetRange (0, glPDlgPB->GetFrameNum (glPDlgPB->m_idxCurSelDisp) -1);
}

LRESULT CDlgMainControl::OnUpdateFilePath (WPARAM wParam, LPARAM lParam)
{
	m_edStrStreamFilePath = *((CString *)lParam);


	if (glPDlgPB->OpenStreamFile (m_edStrStreamFilePath) == FALSE)
	{
		AfxMessageBox ("ERROR: Invalid Stream File");
	UpdateData (FALSE);
	}
	else
	{
		ChangeSliderState ();
	}
	
	return 0;
}
