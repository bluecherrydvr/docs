// DlgCtrlPlayer.cpp : implementation file
//

#include "stdafx.h"
#include "solo6010app.h"
#include "DlgCtrlPlayer.h"
#include "PlayerManager.h"
#include "SOLO6010AppDlg.h"
#include "WndPlayer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CALLBACK CbFn_UpdatePlayerStat (void *pContext, CPlayerManager *pPlayer, DWORD posFrm)
{
	CDlgCtrlPlayer *pDlg = (CDlgCtrlPlayer *)pContext;

	pDlg->m_curPosFrm = posFrm;
	pDlg->m_curSzFrm = pPlayer->GetTotalVideoFrameNum ();
	pDlg->m_curTimeMSec = pPlayer->GetVideoElapsedTime (posFrm);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlPlayer dialog


CDlgCtrlPlayer::CDlgCtrlPlayer(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCtrlPlayer::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCtrlPlayer)
	m_edVidFilePath = _T("");
	m_edAudFilePath = _T("");
	//}}AFX_DATA_INIT

	m_bUseDefCS = FALSE;
	m_idxPlaySpeed = PM_PLAY_SPEED_NORMAL;

	m_bDispCh = TRUE;
	m_bDispFrm = TRUE;
	m_bDispTime = TRUE;

	m_curPosFrm = 0;
	m_curSzFrm = 0;
	m_curTimeMSec = -1;
	m_oldTimeMSec = -1;

	m_numStepFBW = 5;
}


void CDlgCtrlPlayer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCtrlPlayer)
	DDX_Control(pDX, IDC_SP_NUM_FW_BW_STEP, m_spNumFBWStep);
	DDX_Control(pDX, IDC_STA_PLAY_SPEED, m_staPlaySpeed);
	DDX_Control(pDX, IDC_STA_NUM_FW_BW_STEP, m_staNumStepFBW);
	DDX_Control(pDX, IDC_STA_CUR_TIME, m_staCurTime);
	DDX_Control(pDX, IDC_STA_CUR_FRM, m_staCurFrm);
	DDX_Control(pDX, IDC_SLI_PLAYER, m_sliPlayer);
	DDX_Text(pDX, IDC_ED_VID_FILE_PATH, m_edVidFilePath);
	DDX_Text(pDX, IDC_ED_AUD_FILE_PATH, m_edAudFilePath);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_BUT_OPEN_VID_FILE, m_butAll[DLCP_BUT_OPEN_VID_FILE]);
	DDX_Control(pDX, IDC_BUT_OPEN_AUD_FILE, m_butAll[DLCP_BUT_OPEN_AUD_FILE]);
	DDX_Control(pDX, IDC_CH_SPLIT_MODE_1, m_butAll[DLCP_BUT_SPLIT_1]);
	DDX_Control(pDX, IDC_CH_SPLIT_MODE_4, m_butAll[DLCP_BUT_SPLIT_4]);
	DDX_Control(pDX, IDC_CH_SPLIT_MODE_6, m_butAll[DLCP_BUT_SPLIT_6]);
	DDX_Control(pDX, IDC_CH_SPLIT_MODE_7, m_butAll[DLCP_BUT_SPLIT_7]);
	DDX_Control(pDX, IDC_CH_SPLIT_MODE_8, m_butAll[DLCP_BUT_SPLIT_8]);
	DDX_Control(pDX, IDC_CH_SPLIT_MODE_9, m_butAll[DLCP_BUT_SPLIT_9]);
	DDX_Control(pDX, IDC_CH_SPLIT_MODE_10, m_butAll[DLCP_BUT_SPLIT_10]);
	DDX_Control(pDX, IDC_CH_SPLIT_MODE_13, m_butAll[DLCP_BUT_SPLIT_13]);
	DDX_Control(pDX, IDC_CH_SPLIT_MODE_16, m_butAll[DLCP_BUT_SPLIT_16]);
	DDX_Control(pDX, IDC_BUT_PLAY, m_butAll[DLCP_BUT_PLAY]);
	DDX_Control(pDX, IDC_BUT_PAUSE, m_butAll[DLCP_BUT_PAUSE]);
	DDX_Control(pDX, IDC_BUT_STOP, m_butAll[DLCP_BUT_STOP]);
	DDX_Control(pDX, IDC_BUT_BW_1_FRM, m_butAll[DLCP_BUT_BW_1_FRM]);
	DDX_Control(pDX, IDC_BUT_FW_1_FRM, m_butAll[DLCP_BUT_FW_1_FRM]);
	DDX_Control(pDX, IDC_BUT_BW_N_FRM, m_butAll[DLCP_BUT_BW_N_FRM]);
	DDX_Control(pDX, IDC_BUT_FW_N_FRM, m_butAll[DLCP_BUT_FW_N_FRM]);
	DDX_Control(pDX, IDC_BUT_SET_POS_START, m_butAll[DLCP_BUT_SET_POS_START]);
	DDX_Control(pDX, IDC_BUT_SET_POS_END, m_butAll[DLCP_BUT_SET_POS_END]);
	DDX_Control(pDX, IDC_BUT_PS_SLOW, m_butAll[DLCP_BUT_PS_SLOW]);
	DDX_Control(pDX, IDC_BUT_PS_NORMAL, m_butAll[DLCP_BUT_PS_NORMAL]);
	DDX_Control(pDX, IDC_BUT_PS_FAST, m_butAll[DLCP_BUT_PS_FAST]);
	DDX_Control(pDX, IDC_CH_REAL_CH_0, m_butAll[DLCP_BUT_CH_REAL_0]);
	DDX_Control(pDX, IDC_CH_REAL_CH_1, m_butAll[DLCP_BUT_CH_REAL_1]);
	DDX_Control(pDX, IDC_CH_REAL_CH_2, m_butAll[DLCP_BUT_CH_REAL_2]);
	DDX_Control(pDX, IDC_CH_REAL_CH_3, m_butAll[DLCP_BUT_CH_REAL_3]);
	DDX_Control(pDX, IDC_CH_REAL_CH_4, m_butAll[DLCP_BUT_CH_REAL_4]);
	DDX_Control(pDX, IDC_CH_REAL_CH_5, m_butAll[DLCP_BUT_CH_REAL_5]);
	DDX_Control(pDX, IDC_CH_REAL_CH_6, m_butAll[DLCP_BUT_CH_REAL_6]);
	DDX_Control(pDX, IDC_CH_REAL_CH_7, m_butAll[DLCP_BUT_CH_REAL_7]);
	DDX_Control(pDX, IDC_CH_REAL_CH_8, m_butAll[DLCP_BUT_CH_REAL_8]);
	DDX_Control(pDX, IDC_CH_REAL_CH_9, m_butAll[DLCP_BUT_CH_REAL_9]);
	DDX_Control(pDX, IDC_CH_REAL_CH_10, m_butAll[DLCP_BUT_CH_REAL_10]);
	DDX_Control(pDX, IDC_CH_REAL_CH_11, m_butAll[DLCP_BUT_CH_REAL_11]);
	DDX_Control(pDX, IDC_CH_REAL_CH_12, m_butAll[DLCP_BUT_CH_REAL_12]);
	DDX_Control(pDX, IDC_CH_REAL_CH_13, m_butAll[DLCP_BUT_CH_REAL_13]);
	DDX_Control(pDX, IDC_CH_REAL_CH_14, m_butAll[DLCP_BUT_CH_REAL_14]);
	DDX_Control(pDX, IDC_CH_REAL_CH_15, m_butAll[DLCP_BUT_CH_REAL_15]);
	DDX_Control(pDX, IDC_CH_VIRT_CH_0, m_butAll[DLCP_BUT_CH_VIRT_0]);
	DDX_Control(pDX, IDC_CH_VIRT_CH_1, m_butAll[DLCP_BUT_CH_VIRT_1]);
	DDX_Control(pDX, IDC_CH_VIRT_CH_2, m_butAll[DLCP_BUT_CH_VIRT_2]);
	DDX_Control(pDX, IDC_CH_VIRT_CH_3, m_butAll[DLCP_BUT_CH_VIRT_3]);
	DDX_Control(pDX, IDC_CH_VIRT_CH_4, m_butAll[DLCP_BUT_CH_VIRT_4]);
	DDX_Control(pDX, IDC_CH_VIRT_CH_5, m_butAll[DLCP_BUT_CH_VIRT_5]);
	DDX_Control(pDX, IDC_CH_VIRT_CH_6, m_butAll[DLCP_BUT_CH_VIRT_6]);
	DDX_Control(pDX, IDC_CH_VIRT_CH_7, m_butAll[DLCP_BUT_CH_VIRT_7]);
	DDX_Control(pDX, IDC_CH_VIRT_CH_8, m_butAll[DLCP_BUT_CH_VIRT_8]);
	DDX_Control(pDX, IDC_CH_VIRT_CH_9, m_butAll[DLCP_BUT_CH_VIRT_9]);
	DDX_Control(pDX, IDC_CH_VIRT_CH_10, m_butAll[DLCP_BUT_CH_VIRT_10]);
	DDX_Control(pDX, IDC_CH_VIRT_CH_11, m_butAll[DLCP_BUT_CH_VIRT_11]);
	DDX_Control(pDX, IDC_CH_VIRT_CH_12, m_butAll[DLCP_BUT_CH_VIRT_12]);
	DDX_Control(pDX, IDC_CH_VIRT_CH_13, m_butAll[DLCP_BUT_CH_VIRT_13]);
	DDX_Control(pDX, IDC_CH_VIRT_CH_14, m_butAll[DLCP_BUT_CH_VIRT_14]);
	DDX_Control(pDX, IDC_CH_VIRT_CH_15, m_butAll[DLCP_BUT_CH_VIRT_15]);
	DDX_Control(pDX, IDC_CH_INFO_CH, m_butAll[DLCP_BUT_INFO_CH]);
	DDX_Control(pDX, IDC_CH_INFO_FRM, m_butAll[DLCP_BUT_INFO_FRM]);
	DDX_Control(pDX, IDC_CH_INFO_TIME, m_butAll[DLCP_BUT_INFO_TIME]);
	DDX_Control(pDX, IDC_CH_USE_DEF_CS, m_butAll[DLCP_BUT_USE_DEF_CS]);
	DDX_Control(pDX, IDC_BUT_SZ_D4, m_butAll[DLCP_BUT_SZ_D4]);
	DDX_Control(pDX, IDC_BUT_SZ_D2, m_butAll[DLCP_BUT_SZ_D2]);
	DDX_Control(pDX, IDC_BUT_SZ_X1, m_butAll[DLCP_BUT_SZ_X1]);
	DDX_Control(pDX, IDC_BUT_SZ_X2, m_butAll[DLCP_BUT_SZ_X2]);
	DDX_Control(pDX, IDC_BUT_SZ_X3, m_butAll[DLCP_BUT_SZ_X3]);
	DDX_Control(pDX, IDC_BUT_SZ_FULL, m_butAll[DLCP_BUT_SZ_FULL]);
	DDX_Control(pDX, IDC_CH_M4_POSTF_DEBLK, m_butAll[DLCP_BUT_CH_M4_FLT_DEBLK]);
	DDX_Control(pDX, IDC_CH_M4_POSTF_DERING, m_butAll[DLCP_BUT_CH_M4_FLT_DERNG]);
	DDX_Control(pDX, IDC_CH_IMG_POSTF_DEINT, m_butAll[DLCP_BUT_CH_IMG_FLT_DEINT]);
	DDX_Control(pDX, IDC_CH_IMG_POSTF_UPSC, m_butAll[DLCP_BUT_CH_IMG_FLT_UPSC]);
	DDX_Control(pDX, IDC_CH_IMG_POSTF_DNSC, m_butAll[DLCP_BUT_CH_IMG_FLT_DNSC]);
	DDX_Control(pDX, IDC_BUT_SNAPSHOT, m_butAll[DLCP_BUT_SNAPSHOT]);
	DDX_Control(pDX, IDC_BUT_QUIT, m_butAll[DLCP_BUT_QUIT]);
}


BEGIN_MESSAGE_MAP(CDlgCtrlPlayer, CDialog)
	ON_MESSAGE(WM_DOO_INDEX_BUTTON_CLICK, OnIndexButtonClick)
	//{{AFX_MSG_MAP(CDlgCtrlPlayer)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_WM_NCHITTEST()
	ON_WM_HSCROLL()
	ON_WM_TIMER()
	ON_NOTIFY(UDN_DELTAPOS, IDC_SP_NUM_FW_BW_STEP, OnDeltaposSpNumFwBwStep)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlPlayer message handlers

BOOL CDlgCtrlPlayer::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	int i;

	for (i=0; i<NUM_DLCP_BUTTONS; i++)
	{
		m_butAll[i].SetID (i);
	}

	m_sliPlayer.SetRange (0, 0);
	m_sliPlayer.SetPos (0);

	UDACCEL uda[1];
	uda[0].nSec = 0;
	uda[0].nInc = 1;
	m_spNumFBWStep.SetRange (0, 1);
	m_spNumFBWStep.SetAccel (1, uda);

	char strTmp[MAX_PATH];
	sprintf (strTmp, "%d", m_numStepFBW);
	m_staNumStepFBW.SetWindowText (strTmp);
	
	SetTimer (1, 50, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCtrlPlayer::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
	m_pPlayer->UnregisterCB_UpdatePlayerStat ();
}

void CDlgCtrlPlayer::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here

	if (bShow == TRUE)
	{
		m_edVidFilePath.Empty ();
		m_edAudFilePath.Empty ();

		UpdateChannelButtons ();
		UpdateInfoDispButtons ();
		UpdateMP4PostFilterButtons ();

		UpdateData (FALSE);
	}	
}

UINT CDlgCtrlPlayer::OnNcHitTest(CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	return HTCAPTION;
	return CDialog::OnNcHitTest(point);
}

void CDlgCtrlPlayer::SetPlayerManager (CPlayerManager *pPlayer)
{
	m_pPlayer = pPlayer;

	m_pPlayer->RegisterCB_UpdatePlayerStat (CbFn_UpdatePlayerStat, this);
}

void CDlgCtrlPlayer::SetPlayerWnd (CWndPlayer *pWndPlayer)
{
	m_pWndPlayer = pWndPlayer;
}

LRESULT CDlgCtrlPlayer::OnIndexButtonClick (WPARAM wParam, LPARAM lParam)
{
	if (wParam == DLCP_BUT_OPEN_VID_FILE)
	{
		CFileDialog dlg(TRUE);
		if (dlg.DoModal () == IDOK)
		{
			m_edVidFilePath = dlg.GetPathName ();
			if (m_pPlayer->OpenFile (m_edVidFilePath, m_edAudFilePath) == TRUE)
			{
				UpdateData (FALSE);

				UpdateDlgCtrlFromFile ();
			}
		}
	}
	else if (wParam == DLCP_BUT_OPEN_AUD_FILE)
	{
		CFileDialog dlg(TRUE);
		if (dlg.DoModal () == IDOK)
		{
			m_edAudFilePath = dlg.GetPathName ();
			if (m_pPlayer->OpenFile (m_edVidFilePath, m_edAudFilePath) == TRUE)
			{
				UpdateData (FALSE);

				UpdateDlgCtrlFromFile ();
			}
		}
	}
	else if (wParam <= DLCP_BUT_SPLIT_16)
	{
		m_butAll[DLCP_BUT_SPLIT_1 +m_pPlayer->GetSplitMode ()].SetCheck (FALSE);

		m_pPlayer->SetSplitMode (wParam -DLCP_BUT_SPLIT_1);

		m_butAll[DLCP_BUT_SPLIT_1 +m_pPlayer->GetSplitMode ()].SetCheck (TRUE);
		
		UpdateChannelButtons ();
	}
	else if (wParam == DLCP_BUT_PLAY)
	{
		m_pPlayer->Play ();
	}
	else if (wParam == DLCP_BUT_PAUSE)
	{
		m_pPlayer->Pause ();
	}
	else if (wParam == DLCP_BUT_STOP)
	{
		m_pPlayer->Stop ();
	}
	else if (wParam == DLCP_BUT_BW_1_FRM)
	{
		m_pPlayer->OffsetFramePos (m_pPlayer->GetSelWnd (), -1);
	}
	else if (wParam == DLCP_BUT_FW_1_FRM)
	{
		m_pPlayer->OffsetFramePos (m_pPlayer->GetSelWnd (), 1);
	}
	else if (wParam == DLCP_BUT_BW_N_FRM)
	{
		m_pPlayer->OffsetFramePos (m_pPlayer->GetSelWnd (), -m_numStepFBW);
	}
	else if (wParam == DLCP_BUT_FW_N_FRM)
	{
		m_pPlayer->OffsetFramePos (m_pPlayer->GetSelWnd (), m_numStepFBW);
	}
	else if (wParam == DLCP_BUT_SET_POS_START)
	{
		m_pPlayer->SetFramePos (0);
	}
	else if (wParam == DLCP_BUT_SET_POS_END)
	{
		m_pPlayer->SetFramePos (m_pPlayer->GetTotalVideoFrameNum () -1);
	}
	else if (wParam == DLCP_BUT_PS_SLOW)
	{
		m_pPlayer->SetPlaySpeed (m_pPlayer->GetPlaySpeed () -1);
		UpdatePlaySpeed (m_pPlayer->GetPlaySpeed ());
	}
	else if (wParam == DLCP_BUT_PS_NORMAL)
	{
		m_pPlayer->SetPlaySpeed (PM_PLAY_SPEED_NORMAL);
		UpdatePlaySpeed (m_pPlayer->GetPlaySpeed ());
	}
	else if (wParam == DLCP_BUT_PS_FAST)
	{
		m_pPlayer->SetPlaySpeed (m_pPlayer->GetPlaySpeed () +1);
		UpdatePlaySpeed (m_pPlayer->GetPlaySpeed ());
	}
	else if (wParam <= DLCP_BUT_CH_VIRT_15)
	{
		m_pPlayer->SetChannel (m_pPlayer->GetSelWnd (), wParam -DLCP_BUT_CH_REAL_0);
		UpdateChannelButtons ();
	}
	else if (wParam <= DLCP_BUT_INFO_TIME)
	{
		switch (wParam)
		{
		case DLCP_BUT_INFO_CH:
			m_bDispCh = 1 -m_bDispCh;
			break;
		case DLCP_BUT_INFO_FRM:
			m_bDispFrm = 1 -m_bDispFrm;
			break;
		case DLCP_BUT_INFO_TIME:
			m_bDispTime = 1 -m_bDispTime;
			break;
		}

		m_pWndPlayer->SetInfoDispState (m_bDispCh, m_bDispFrm, m_bDispTime);

		BOOL bTmpPlaying = FALSE;
		if (m_pPlayer->GetPlayMode () == PM_MODE_PLAYING)
		{
			bTmpPlaying = TRUE;
		}

		m_pPlayer->SetFramePos (m_pPlayer->GetFramePos ());
		
		if (bTmpPlaying == TRUE)
		{
			m_pPlayer->Play ();
		}

		UpdateInfoDispButtons ();
	}
	else if (wParam == DLCP_BUT_USE_DEF_CS)
	{
		m_bUseDefCS = 1 -m_bUseDefCS;
		m_pPlayer->SetDefaultColorspace (m_bUseDefCS);
	}
	else if (wParam <= DLCP_BUT_SZ_FULL)
	{
		m_pWndPlayer->ChangeWndSize (wParam -DLCP_BUT_SZ_D4);
	}
	else if (wParam == DLCP_BUT_CH_IMG_FLT_DEINT)
	{
		glPCurGInfo->bUseImgFlt_Deint = 1 -glPCurGInfo->bUseImgFlt_Deint;
	}
	else if (wParam == DLCP_BUT_CH_IMG_FLT_UPSC)
	{
		glPCurGInfo->bUseImgFlt_UpSc = 1 -glPCurGInfo->bUseImgFlt_UpSc;
	}
	else if (wParam == DLCP_BUT_CH_IMG_FLT_DNSC)
	{
		glPCurGInfo->bUseImgFlt_DnSc = 1 -glPCurGInfo->bUseImgFlt_DnSc;
	}
	else if (wParam == DLCP_BUT_CH_M4_FLT_DEBLK)
	{
		glPCurGInfo->bUseMP4Flt_Deblk = 1 -glPCurGInfo->bUseMP4Flt_Deblk;

		UpdateMP4PostFilterButtons ();
	}
	else if (wParam == DLCP_BUT_CH_M4_FLT_DERNG)
	{
		glPCurGInfo->bUseMP4Flt_Dering = 1 -glPCurGInfo->bUseMP4Flt_Dering;

		UpdateMP4PostFilterButtons ();
	}
	else if (wParam == DLCP_BUT_SNAPSHOT)
	{
	}
	else if (wParam == DLCP_BUT_QUIT)
	{
		m_pPlayer->Pause ();
		glPDlgApp->SendMessage (WM_DOO_APP_DLG_CTRL, IDX_APP_MSG_ON_BUT_PLAYER, 0);
	}

	return 0;
}

void CDlgCtrlPlayer::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	
	CSliderCtrl *pSlider = (CSliderCtrl *)pScrollBar;

	switch (pScrollBar->GetDlgCtrlID ())
	{
	case IDC_SLI_PLAYER:
		m_pPlayer->SetFramePos (pSlider->GetPos ());
		break;
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CDlgCtrlPlayer::UpdateDlgCtrlFromFile ()
{
	m_sliPlayer.SetRange (0, m_pPlayer->GetTotalVideoFrameNum () -1);
	m_sliPlayer.SetPos (0);

	int i;
	for (i=0; i<NUM_LIVE_SPLIT_MODE; i++)
	{
		m_butAll[DLCP_BUT_SPLIT_1 +i].SetCheck (FALSE);
	}

	m_butAll[DLCP_BUT_SPLIT_1 +m_pPlayer->GetSplitMode ()].SetCheck (TRUE);

	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		if (m_pPlayer->GetVideoFrameNum (i) > 0)
		{
			m_butAll[DLCP_BUT_CH_REAL_0 +i].EnableWindow (TRUE);
		}
		else
		{
			m_butAll[DLCP_BUT_CH_REAL_0 +i].EnableWindow (FALSE);
		}
	}

	UpdateChannelButtons ();
	UpdateInfoDispButtons ();
}

void CDlgCtrlPlayer::UpdatePlaySpeed (int idxPlaySpeed)
{
	CString strPlaySpeed;

	switch (idxPlaySpeed)
	{
	case PM_PLAY_SPEED_DIV16:
		strPlaySpeed = "/16";
		break;
	case PM_PLAY_SPEED_DIV8:
		strPlaySpeed = "/8";
		break;
	case PM_PLAY_SPEED_DIV4:
		strPlaySpeed = "/4";
		break;
	case PM_PLAY_SPEED_DIV2:
		strPlaySpeed = "/2";
		break;
	case PM_PLAY_SPEED_NORMAL:
		strPlaySpeed = "x1";
		break;
	case PM_PLAY_SPEED_X2:
		strPlaySpeed = "x2";
		break;
	case PM_PLAY_SPEED_X4:
		strPlaySpeed = "x4";
		break;
	case PM_PLAY_SPEED_X8:
		strPlaySpeed = "x8";
		break;
	case PM_PLAY_SPEED_X16:
		strPlaySpeed = "x16";
		break;
	}

	m_staPlaySpeed.SetWindowText (LPCTSTR(strPlaySpeed));
}

void CDlgCtrlPlayer::UpdateChannelButtons ()
{
	int i;
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		m_butAll[DLCP_BUT_CH_REAL_0 +i].SetCheck (FALSE);
	}

	m_butAll[DLCP_BUT_CH_REAL_0 +m_pPlayer->GetChannel (m_pPlayer->GetSelWnd ())].SetCheck (TRUE);
}

void CDlgCtrlPlayer::UpdateInfoDispButtons ()
{
	m_butAll[DLCP_BUT_INFO_CH].SetCheck (m_bDispCh);
	m_butAll[DLCP_BUT_INFO_FRM].SetCheck (m_bDispFrm);
	m_butAll[DLCP_BUT_INFO_TIME].SetCheck (m_bDispTime);
}

void CDlgCtrlPlayer::UpdateMP4PostFilterButtons ()
{
	m_butAll[DLCP_BUT_CH_M4_FLT_DEBLK].SetCheck (glPCurGInfo->bUseMP4Flt_Deblk);
	m_butAll[DLCP_BUT_CH_M4_FLT_DERNG].SetCheck (glPCurGInfo->bUseMP4Flt_Dering);

	m_butAll[DLCP_BUT_CH_IMG_FLT_DEINT].SetCheck (FALSE /*glPCurGInfo->bUseImgFlt_Deint*/);
	m_butAll[DLCP_BUT_CH_IMG_FLT_UPSC].SetCheck (glPCurGInfo->bUseImgFlt_UpSc);
	m_butAll[DLCP_BUT_CH_IMG_FLT_DNSC].SetCheck (glPCurGInfo->bUseImgFlt_DnSc);

	m_pPlayer->SetMP4PostFilterState (glPCurGInfo->bUseMP4Flt_Deblk, glPCurGInfo->bUseMP4Flt_Deblk,
		glPCurGInfo->bUseMP4Flt_Dering, glPCurGInfo->bUseMP4Flt_Dering);
	m_pPlayer->SetImgPostFilterState (glPCurGInfo->bUseImgFlt_Deint, glPCurGInfo->bUseImgFlt_UpSc, glPCurGInfo->bUseImgFlt_DnSc);
}

void CDlgCtrlPlayer::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	
	if (m_curTimeMSec != m_oldTimeMSec)
	{
		char strTmp[MAX_PATH];

		sprintf (strTmp, "%d / %d", m_curPosFrm +1, m_curSzFrm);
		m_staCurFrm.SetWindowText (strTmp);

		sprintf (strTmp, "%02d : %02d : %02d.%03d",
			m_curTimeMSec /1000 /60 /60, (m_curTimeMSec /1000 /60) %60, (m_curTimeMSec /1000) %60, m_curTimeMSec %1000);
		m_staCurTime.SetWindowText (strTmp);

		m_sliPlayer.SetPos (m_curPosFrm);

		m_oldTimeMSec = m_curTimeMSec;
	}

	CDialog::OnTimer(nIDEvent);
}

void CDlgCtrlPlayer::OnDeltaposSpNumFwBwStep(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);
	if (pNMUpDown->iDelta > 0)
	{
		m_numStepFBW++;
		if (m_numStepFBW > int(m_pPlayer->GetTotalVideoFrameNum ()))
		{
			m_numStepFBW = m_pPlayer->GetTotalVideoFrameNum ();
		}
	}
	else
	{
		m_numStepFBW--;
		if (m_numStepFBW < 1)
		{
			m_numStepFBW = 1;
		}
	}
	char strTmp[MAX_PATH];
	sprintf (strTmp, "%d", m_numStepFBW);
	m_staNumStepFBW.SetWindowText (strTmp);

	UpdateData (FALSE);

	*pResult = 0;
}
