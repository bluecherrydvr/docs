// DlgG723PlayCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "S6010StreamTester.h"
#include "DlgG723PlayCtrl.h"
#include "DlgPlayerBase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgG723PlayCtrl dialog


CDlgG723PlayCtrl::CDlgG723PlayCtrl(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgG723PlayCtrl::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgG723PlayCtrl)
	m_edStrG723FilePath = _T("");
	//}}AFX_DATA_INIT

	m_idxSelCh = 0;
}


void CDlgG723PlayCtrl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgG723PlayCtrl)
	DDX_Control(pDX, IDC_SLI_PLAYER, m_sliPlayer);
	DDX_Text(pDX, IDC_ED_G723_FILE_PATH, m_edStrG723FilePath);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_CH_CHANNEL_1, m_bufCButCh[0]);
	DDX_Control(pDX, IDC_CH_CHANNEL_2, m_bufCButCh[1]);
	DDX_Control(pDX, IDC_CH_CHANNEL_3, m_bufCButCh[2]);
	DDX_Control(pDX, IDC_CH_CHANNEL_4, m_bufCButCh[3]);
	DDX_Control(pDX, IDC_CH_CHANNEL_5, m_bufCButCh[4]);
	DDX_Control(pDX, IDC_CH_CHANNEL_6, m_bufCButCh[5]);
	DDX_Control(pDX, IDC_CH_CHANNEL_7, m_bufCButCh[6]);
	DDX_Control(pDX, IDC_CH_CHANNEL_8, m_bufCButCh[7]);
	DDX_Control(pDX, IDC_CH_CHANNEL_9, m_bufCButCh[8]);
	DDX_Control(pDX, IDC_CH_CHANNEL_10, m_bufCButCh[9]);
	DDX_Control(pDX, IDC_CH_CHANNEL_11, m_bufCButCh[10]);
	DDX_Control(pDX, IDC_CH_CHANNEL_12, m_bufCButCh[11]);
	DDX_Control(pDX, IDC_CH_CHANNEL_13, m_bufCButCh[12]);
	DDX_Control(pDX, IDC_CH_CHANNEL_14, m_bufCButCh[13]);
	DDX_Control(pDX, IDC_CH_CHANNEL_15, m_bufCButCh[14]);
	DDX_Control(pDX, IDC_CH_CHANNEL_16, m_bufCButCh[15]);
}


BEGIN_MESSAGE_MAP(CDlgG723PlayCtrl, CDialog)
	ON_MESSAGE (WM_DOO_INDEX_BUTTON_CLICK, OnIndexButtonClick)
	ON_MESSAGE (WM_DOO_UPDATE_FILE_PATH, OnUpdateFilePath)
	ON_MESSAGE (WM_DOO_UPDATE_G723_SLIDER_POS, OnUpdateSliderPos)
	//{{AFX_MSG_MAP(CDlgG723PlayCtrl)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(IDC_BUT_OPEN_G723_FILE, OnButOpenG723File)
	ON_BN_CLICKED(IDC_BUT_PLAY, OnButPlay)
	ON_BN_CLICKED(IDC_BUT_PAUSE, OnButPause)
	ON_BN_CLICKED(IDC_BUT_STOP, OnButStop)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUT_RAW_SAVE, OnButRawSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgG723PlayCtrl message handlers

BOOL CDlgG723PlayCtrl::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	int i;

	for (i=0; i<NUM_CHANNEL; i++)
	{
		m_bufCButCh[i].SetID (i);
	}

	m_bufCButCh[m_idxSelCh].SetCheck (TRUE);

	m_sliPlayer.SetRange (0, 0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgG723PlayCtrl::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

void CDlgG723PlayCtrl::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here

}

UINT CDlgG723PlayCtrl::OnNcHitTest(CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	return HTCAPTION;
	return CDialog::OnNcHitTest(point);
}

void CDlgG723PlayCtrl::OnButOpenG723File() 
{
	// TODO: Add your control notification handler code here
	
	CFileDialog dlg(TRUE);
	if (dlg.DoModal () == IDOK)
	{
		CString strTmp = dlg.GetPathName ();

		if (strTmp.Find ("RG7_") == -1)
		{	// MP4 File Open
			GetParent ()->SendMessage (WM_DOO_CTRL_DLG_CHANGE, CTRL_DLG_MP4, (LONG)&strTmp);
			return;
		}
		m_edStrG723FilePath = strTmp;
		UpdateData (FALSE);

		if (glPDlgPB->OpenG723File (m_edStrG723FilePath) == FALSE)
		{
			AfxMessageBox ("ERROR: Invalid Stream File");
		}
		else
		{
			ChangeSliderState ();
		}
	}
}

void CDlgG723PlayCtrl::OnButPlay() 
{
	// TODO: Add your control notification handler code here
	
	glPDlgPB->ChangeCurG723PlayerMode (G723_PLAYER_REQ_PLAY);
}

void CDlgG723PlayCtrl::OnButPause() 
{
	// TODO: Add your control notification handler code here
	
	glPDlgPB->ChangeCurG723PlayerMode (G723_PLAYER_PAUSE);
}

void CDlgG723PlayCtrl::OnButStop() 
{
	// TODO: Add your control notification handler code here
	
	glPDlgPB->ChangeCurG723PlayerMode (G723_PLAYER_REQ_STOP);
}

void CDlgG723PlayCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

LRESULT CDlgG723PlayCtrl::OnIndexButtonClick (WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your control notification handler code here

	if (wParam == m_idxSelCh)
	{
		m_bufCButCh[m_idxSelCh].SetCheck (TRUE);
	}
	else
	{
		m_bufCButCh[m_idxSelCh].SetCheck (FALSE);
		m_idxSelCh = wParam;
		m_bufCButCh[m_idxSelCh].SetCheck (TRUE);

		glPDlgPB->ChangeCurG723PlayCh (m_idxSelCh);
	}

	return 0;
}

void CDlgG723PlayCtrl::ChangeSliderState ()
{
	m_maxPosSli = (glPDlgPB->GetNumG723Frm ()) /SLIDER_RESOLUTION;
	m_curPosSli = (glPDlgPB->GetCurPosG723Frm ()) /SLIDER_RESOLUTION;

	m_sliPlayer.SetRange (0, m_maxPosSli);
	m_sliPlayer.SetPos (m_curPosSli);
}

void CDlgG723PlayCtrl::OnOK() 
{
	// TODO: Add extra validation here
	
	glPDlgPB->S6010StreamPlayerQuit ();

	CDialog::OnOK();
}

LRESULT CDlgG723PlayCtrl::OnUpdateFilePath (WPARAM wParam, LPARAM lParam)
{
	m_edStrG723FilePath = *((CString *)lParam);

	UpdateData (FALSE);

	if (glPDlgPB->OpenG723File (m_edStrG723FilePath) == FALSE)
	{
		AfxMessageBox ("ERROR: Invalid Stream File");
	}
	else
	{
		ChangeSliderState ();
	}
	
	return 0;
}

LRESULT CDlgG723PlayCtrl::OnUpdateSliderPos (WPARAM wParam, LPARAM lParam)
{
	int tmpCurPosSli = (glPDlgPB->GetCurPosG723Frm ()) /SLIDER_RESOLUTION;

	if (m_curPosSli != tmpCurPosSli)
	{
		m_curPosSli = tmpCurPosSli;
		m_sliPlayer.SetPos (m_curPosSli);
	}
	return 0;
}

void CDlgG723PlayCtrl::OnButRawSave() 
{
	// TODO: Add your control notification handler code here
	
	char strFileName[MAX_PATH];
	sprintf (strFileName, "Rec_Audio_%d", m_idxSelCh);
	
	CFileDialog dlg(FALSE, "g723", strFileName);
	if (dlg.DoModal () != IDOK)
	{
		return;
	}

	if (glPDlgPB->WriteG723RAW (m_idxSelCh, LPCTSTR(dlg.GetPathName ())) == FALSE)
	{
		AfxMessageBox ("An error was occured during save");
	}
	else
	{
		AfxMessageBox ("Success");
	}
}
