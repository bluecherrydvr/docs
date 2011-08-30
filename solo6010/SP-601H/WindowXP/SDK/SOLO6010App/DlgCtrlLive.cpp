// DlgCtrlLive.cpp : implementation file
//

#include "stdafx.h"
#include "SOLO6010App.h"
#include "DlgCtrlLive.h"
#include "SOLO6010.h"
#include "WndLiveDisp.h"
#include "SOLO6010AppDlg.h"
#include "HWDecManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CALLBACK CbFn_UpdatePlayerStat (void *pContext, CHWDecManager *pPlayer, DWORD posFrm)
{
	CDlgCtrlLive *pDlg = (CDlgCtrlLive *)pContext;

	pDlg->m_curPosFrm = posFrm;
	pDlg->m_curSzFrm = pPlayer->GetTotalVideoFrameNum ();
	pDlg->m_curTimeMSec = pPlayer->GetVideoElapsedTime (posFrm);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlLive dialog


CDlgCtrlLive::CDlgCtrlLive(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCtrlLive::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCtrlLive)
	m_edVidFilePath = _T("");
	m_edAudFilePath = _T("");
	//}}AFX_DATA_INIT

	m_idxPlaySpeed = PM_PLAY_SPEED_NORMAL;

	m_curPosFrm = 0;
	m_curSzFrm = 0;
	m_curTimeMSec = -1;
	m_oldTimeMSec = -1;

	m_numStepFBW = 5;

	glPCurHWDecManager = NULL;

	m_bChangeLiveModeAuto = TRUE;
}


void CDlgCtrlLive::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCtrlLive)
	DDX_Control(pDX, IDC_SP_NUM_FW_BW_STEP, m_spNumFBWStep);
	DDX_Control(pDX, IDC_STA_PLAY_SPEED, m_staPlaySpeed);
	DDX_Control(pDX, IDC_STA_NUM_FW_BW_STEP, m_staNumStepFBW);
	DDX_Control(pDX, IDC_STA_CUR_TIME, m_staCurTime);
	DDX_Control(pDX, IDC_STA_CUR_FRM, m_staCurFrm);
	DDX_Control(pDX, IDC_SLI_PLAYER, m_sliPlayer);
	DDX_Text(pDX, IDC_ED_VID_FILE_PATH, m_edVidFilePath);
	DDX_Text(pDX, IDC_ED_AUD_FILE_PATH, m_edAudFilePath);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_CH_LIVE_MODE_1, m_butAll[IDX_DCL_BUT_LIVE_DISP_1]);
	DDX_Control(pDX, IDC_CH_LIVE_MODE_4, m_butAll[IDX_DCL_BUT_LIVE_DISP_4]);
	DDX_Control(pDX, IDC_CH_LIVE_MODE_6, m_butAll[IDX_DCL_BUT_LIVE_DISP_6]);
	DDX_Control(pDX, IDC_CH_LIVE_MODE_7, m_butAll[IDX_DCL_BUT_LIVE_DISP_7]);
	DDX_Control(pDX, IDC_CH_LIVE_MODE_8, m_butAll[IDX_DCL_BUT_LIVE_DISP_8]);
	DDX_Control(pDX, IDC_CH_LIVE_MODE_9, m_butAll[IDX_DCL_BUT_LIVE_DISP_9]);
	DDX_Control(pDX, IDC_CH_LIVE_MODE_10, m_butAll[IDX_DCL_BUT_LIVE_DISP_10]);
	DDX_Control(pDX, IDC_CH_LIVE_MODE_13, m_butAll[IDX_DCL_BUT_LIVE_DISP_13]);
	DDX_Control(pDX, IDC_CH_LIVE_MODE_16, m_butAll[IDX_DCL_BUT_LIVE_DISP_16]);
	DDX_Control(pDX, IDC_CH_CHANNEL_0, m_butAll[IDX_DCL_BUT_CH_0]);
	DDX_Control(pDX, IDC_CH_CHANNEL_1, m_butAll[IDX_DCL_BUT_CH_1]);
	DDX_Control(pDX, IDC_CH_CHANNEL_2, m_butAll[IDX_DCL_BUT_CH_2]);
	DDX_Control(pDX, IDC_CH_CHANNEL_3, m_butAll[IDX_DCL_BUT_CH_3]);
	DDX_Control(pDX, IDC_CH_CHANNEL_4, m_butAll[IDX_DCL_BUT_CH_4]);
	DDX_Control(pDX, IDC_CH_CHANNEL_5, m_butAll[IDX_DCL_BUT_CH_5]);
	DDX_Control(pDX, IDC_CH_CHANNEL_6, m_butAll[IDX_DCL_BUT_CH_6]);
	DDX_Control(pDX, IDC_CH_CHANNEL_7, m_butAll[IDX_DCL_BUT_CH_7]);
	DDX_Control(pDX, IDC_CH_CHANNEL_8, m_butAll[IDX_DCL_BUT_CH_8]);
	DDX_Control(pDX, IDC_CH_CHANNEL_9, m_butAll[IDX_DCL_BUT_CH_9]);
	DDX_Control(pDX, IDC_CH_CHANNEL_10, m_butAll[IDX_DCL_BUT_CH_10]);
	DDX_Control(pDX, IDC_CH_CHANNEL_11, m_butAll[IDX_DCL_BUT_CH_11]);
	DDX_Control(pDX, IDC_CH_CHANNEL_12, m_butAll[IDX_DCL_BUT_CH_12]);
	DDX_Control(pDX, IDC_CH_CHANNEL_13, m_butAll[IDX_DCL_BUT_CH_13]);
	DDX_Control(pDX, IDC_CH_CHANNEL_14, m_butAll[IDX_DCL_BUT_CH_14]);
	DDX_Control(pDX, IDC_CH_CHANNEL_15, m_butAll[IDX_DCL_BUT_CH_15]);
	DDX_Control(pDX, IDC_BUT_DISZ_D4, m_butAll[IDX_DCL_BUT_DISZ_DIV_4]);
	DDX_Control(pDX, IDC_BUT_DISZ_D2, m_butAll[IDX_DCL_BUT_DISZ_DIV_2]);
	DDX_Control(pDX, IDC_BUT_DISZ_X1, m_butAll[IDX_DCL_BUT_DISZ_X_1]);
	DDX_Control(pDX, IDC_BUT_DISZ_X2, m_butAll[IDX_DCL_BUT_DISZ_X_2]);
	DDX_Control(pDX, IDC_BUT_DISZ_X3, m_butAll[IDX_DCL_BUT_DISZ_X_3]);
	DDX_Control(pDX, IDC_BUT_DISZ_FULL, m_butAll[IDX_DCL_BUT_DISZ_FULL]);
	DDX_Control(pDX, IDC_CH_B_DISP_CH, m_butAll[IDX_DCL_BUT_CH_DISP]);
	DDX_Control(pDX, IDC_CH_B_ALWAYS_TOP, m_butAll[IDX_DCL_BUT_CH_ALWAYS_TOP]);
	DDX_Control(pDX, IDC_BUT_HIDE, m_butAll[IDX_DCL_BUT_HIDE]);
	DDX_Control(pDX, IDC_BUT_QUIT, m_butAll[IDX_DCL_BUT_QUIT]);
	
	DDX_Control(pDX, IDC_STA_COLOR_HUE, m_bufStaColor[IDX_COLOR_HUE]);
	DDX_Control(pDX, IDC_SLI_COLOR_HUE, m_bufSliColor[IDX_COLOR_HUE]);
	DDX_Control(pDX, IDC_STA_COLOR_SAT, m_bufStaColor[IDX_COLOR_SAT]);
	DDX_Control(pDX, IDC_SLI_COLOR_SAT, m_bufSliColor[IDX_COLOR_SAT]);
	DDX_Control(pDX, IDC_STA_COLOR_CON, m_bufStaColor[IDX_COLOR_CON]);
	DDX_Control(pDX, IDC_SLI_COLOR_CON, m_bufSliColor[IDX_COLOR_CON]);
	DDX_Control(pDX, IDC_STA_COLOR_BRI, m_bufStaColor[IDX_COLOR_BRI]);
	DDX_Control(pDX, IDC_SLI_COLOR_BRI, m_bufSliColor[IDX_COLOR_BRI]);

	DDX_Control(pDX, IDC_BUT_COLOR_DEF_HUE, m_butAll[IDX_DCL_BUT_CLR_DEF_HUE]);
	DDX_Control(pDX, IDC_BUT_COLOR_DEF_SAT, m_butAll[IDX_DCL_BUT_CLR_DEF_SAT]);
	DDX_Control(pDX, IDC_BUT_COLOR_DEF_CON, m_butAll[IDX_DCL_BUT_CLR_DEF_CON]);
	DDX_Control(pDX, IDC_BUT_COLOR_DEF_BRI, m_butAll[IDX_DCL_BUT_CLR_DEF_BRI]);

	DDX_Control(pDX, IDC_BUT_IS_L_OR_D_0, m_butAll[IDX_DCL_BUT_L_OR_D_0]);
	DDX_Control(pDX, IDC_BUT_IS_L_OR_D_1, m_butAll[IDX_DCL_BUT_L_OR_D_1]);
	DDX_Control(pDX, IDC_BUT_IS_L_OR_D_2, m_butAll[IDX_DCL_BUT_L_OR_D_2]);
	DDX_Control(pDX, IDC_BUT_IS_L_OR_D_3, m_butAll[IDX_DCL_BUT_L_OR_D_3]);
	DDX_Control(pDX, IDC_BUT_IS_L_OR_D_4, m_butAll[IDX_DCL_BUT_L_OR_D_4]);
	DDX_Control(pDX, IDC_BUT_IS_L_OR_D_5, m_butAll[IDX_DCL_BUT_L_OR_D_5]);
	DDX_Control(pDX, IDC_BUT_IS_L_OR_D_6, m_butAll[IDX_DCL_BUT_L_OR_D_6]);
	DDX_Control(pDX, IDC_BUT_IS_L_OR_D_7, m_butAll[IDX_DCL_BUT_L_OR_D_7]);
	DDX_Control(pDX, IDC_BUT_IS_L_OR_D_8, m_butAll[IDX_DCL_BUT_L_OR_D_8]);
	DDX_Control(pDX, IDC_BUT_IS_L_OR_D_9, m_butAll[IDX_DCL_BUT_L_OR_D_9]);
	DDX_Control(pDX, IDC_BUT_IS_L_OR_D_10, m_butAll[IDX_DCL_BUT_L_OR_D_10]);
	DDX_Control(pDX, IDC_BUT_IS_L_OR_D_11, m_butAll[IDX_DCL_BUT_L_OR_D_11]);
	DDX_Control(pDX, IDC_BUT_IS_L_OR_D_12, m_butAll[IDX_DCL_BUT_L_OR_D_12]);
	DDX_Control(pDX, IDC_BUT_IS_L_OR_D_13, m_butAll[IDX_DCL_BUT_L_OR_D_13]);
	DDX_Control(pDX, IDC_BUT_IS_L_OR_D_14, m_butAll[IDX_DCL_BUT_L_OR_D_14]);
	DDX_Control(pDX, IDC_BUT_IS_L_OR_D_15, m_butAll[IDX_DCL_BUT_L_OR_D_15]);

	DDX_Control(pDX, IDC_BUT_OPEN_VID_FILE, m_butAll[IDX_DCL_BUT_HWD_OPEN_VID_FILE]);
	DDX_Control(pDX, IDC_BUT_OPEN_AUD_FILE, m_butAll[IDX_DCL_BUT_HWD_OPEN_AUD_FILE]);
	DDX_Control(pDX, IDC_BUT_PLAY, m_butAll[IDX_DCL_BUT_HWD_PLAY]);
	DDX_Control(pDX, IDC_BUT_PAUSE, m_butAll[IDX_DCL_BUT_HWD_PAUSE]);
	DDX_Control(pDX, IDC_BUT_STOP, m_butAll[IDX_DCL_BUT_HWD_STOP]);
	DDX_Control(pDX, IDC_BUT_BACK_PLAY, m_butAll[IDX_DCL_BUT_HWD_BACK_PLAY]);
	DDX_Control(pDX, IDC_BUT_BW_1_FRM, m_butAll[IDX_DCL_BUT_HWD_BW_1_FRM]);
	DDX_Control(pDX, IDC_BUT_FW_1_FRM, m_butAll[IDX_DCL_BUT_HWD_FW_1_FRM]);
	DDX_Control(pDX, IDC_BUT_BW_N_FRM, m_butAll[IDX_DCL_BUT_HWD_BW_N_FRM]);
	DDX_Control(pDX, IDC_BUT_FW_N_FRM, m_butAll[IDX_DCL_BUT_HWD_FW_N_FRM]);
	DDX_Control(pDX, IDC_BUT_SET_POS_START, m_butAll[IDX_DCL_BUT_HWD_SET_POS_START]);
	DDX_Control(pDX, IDC_BUT_SET_POS_END, m_butAll[IDX_DCL_BUT_HWD_SET_POS_END]);
	DDX_Control(pDX, IDC_BUT_PS_SLOW, m_butAll[IDX_DCL_BUT_HWD_PS_SLOW]);
	DDX_Control(pDX, IDC_BUT_PS_NORMAL, m_butAll[IDX_DCL_BUT_HWD_PS_NORMAL]);
	DDX_Control(pDX, IDC_BUT_PS_FAST, m_butAll[IDX_DCL_BUT_HWD_PS_FAST]);
	DDX_Control(pDX, IDC_CH_DEC_R_CH_0, m_butAll[IDX_DCL_BUT_HWD_CH_REAL_0]);
	DDX_Control(pDX, IDC_CH_DEC_R_CH_1, m_butAll[IDX_DCL_BUT_HWD_CH_REAL_1]);
	DDX_Control(pDX, IDC_CH_DEC_R_CH_2, m_butAll[IDX_DCL_BUT_HWD_CH_REAL_2]);
	DDX_Control(pDX, IDC_CH_DEC_R_CH_3, m_butAll[IDX_DCL_BUT_HWD_CH_REAL_3]);
	DDX_Control(pDX, IDC_CH_DEC_R_CH_4, m_butAll[IDX_DCL_BUT_HWD_CH_REAL_4]);
	DDX_Control(pDX, IDC_CH_DEC_R_CH_5, m_butAll[IDX_DCL_BUT_HWD_CH_REAL_5]);
	DDX_Control(pDX, IDC_CH_DEC_R_CH_6, m_butAll[IDX_DCL_BUT_HWD_CH_REAL_6]);
	DDX_Control(pDX, IDC_CH_DEC_R_CH_7, m_butAll[IDX_DCL_BUT_HWD_CH_REAL_7]);
	DDX_Control(pDX, IDC_CH_DEC_R_CH_8, m_butAll[IDX_DCL_BUT_HWD_CH_REAL_8]);
	DDX_Control(pDX, IDC_CH_DEC_R_CH_9, m_butAll[IDX_DCL_BUT_HWD_CH_REAL_9]);
	DDX_Control(pDX, IDC_CH_DEC_R_CH_10, m_butAll[IDX_DCL_BUT_HWD_CH_REAL_10]);
	DDX_Control(pDX, IDC_CH_DEC_R_CH_11, m_butAll[IDX_DCL_BUT_HWD_CH_REAL_11]);
	DDX_Control(pDX, IDC_CH_DEC_R_CH_12, m_butAll[IDX_DCL_BUT_HWD_CH_REAL_12]);
	DDX_Control(pDX, IDC_CH_DEC_R_CH_13, m_butAll[IDX_DCL_BUT_HWD_CH_REAL_13]);
	DDX_Control(pDX, IDC_CH_DEC_R_CH_14, m_butAll[IDX_DCL_BUT_HWD_CH_REAL_14]);
	DDX_Control(pDX, IDC_CH_DEC_R_CH_15, m_butAll[IDX_DCL_BUT_HWD_CH_REAL_15]);
	DDX_Control(pDX, IDC_CH_DEC_V_CH_0, m_butAll[IDX_DCL_BUT_HWD_CH_VIRT_0]);
	DDX_Control(pDX, IDC_CH_DEC_V_CH_1, m_butAll[IDX_DCL_BUT_HWD_CH_VIRT_1]);
	DDX_Control(pDX, IDC_CH_DEC_V_CH_2, m_butAll[IDX_DCL_BUT_HWD_CH_VIRT_2]);
	DDX_Control(pDX, IDC_CH_DEC_V_CH_3, m_butAll[IDX_DCL_BUT_HWD_CH_VIRT_3]);
	DDX_Control(pDX, IDC_CH_DEC_V_CH_4, m_butAll[IDX_DCL_BUT_HWD_CH_VIRT_4]);
	DDX_Control(pDX, IDC_CH_DEC_V_CH_5, m_butAll[IDX_DCL_BUT_HWD_CH_VIRT_5]);
	DDX_Control(pDX, IDC_CH_DEC_V_CH_6, m_butAll[IDX_DCL_BUT_HWD_CH_VIRT_6]);
	DDX_Control(pDX, IDC_CH_DEC_V_CH_7, m_butAll[IDX_DCL_BUT_HWD_CH_VIRT_7]);
	DDX_Control(pDX, IDC_CH_DEC_V_CH_8, m_butAll[IDX_DCL_BUT_HWD_CH_VIRT_8]);
	DDX_Control(pDX, IDC_CH_DEC_V_CH_9, m_butAll[IDX_DCL_BUT_HWD_CH_VIRT_9]);
	DDX_Control(pDX, IDC_CH_DEC_V_CH_10, m_butAll[IDX_DCL_BUT_HWD_CH_VIRT_10]);
	DDX_Control(pDX, IDC_CH_DEC_V_CH_11, m_butAll[IDX_DCL_BUT_HWD_CH_VIRT_11]);
	DDX_Control(pDX, IDC_CH_DEC_V_CH_12, m_butAll[IDX_DCL_BUT_HWD_CH_VIRT_12]);
	DDX_Control(pDX, IDC_CH_DEC_V_CH_13, m_butAll[IDX_DCL_BUT_HWD_CH_VIRT_13]);
	DDX_Control(pDX, IDC_CH_DEC_V_CH_14, m_butAll[IDX_DCL_BUT_HWD_CH_VIRT_14]);
	DDX_Control(pDX, IDC_CH_DEC_V_CH_15, m_butAll[IDX_DCL_BUT_HWD_CH_VIRT_15]);

	DDX_Control(pDX, IDC_CH_USE_S6010_DI, m_butAll[IDC_DCL_BUT_USE_S6010_DI_FLT]);
	DDX_Control(pDX, IDC_CH_CHANGE_LIVE_MODE_AUTO, m_butAll[IDC_DCL_BUT_CHANGE_LIVE_MODE_AUTO]);
	
	DDX_Control(pDX, IDC_CH_VLOSS_0, m_butAll[IDC_DCL_BUT_VLOSS_CAM_0]);
	DDX_Control(pDX, IDC_CH_VLOSS_1, m_butAll[IDC_DCL_BUT_VLOSS_CAM_1]);
	DDX_Control(pDX, IDC_CH_VLOSS_2, m_butAll[IDC_DCL_BUT_VLOSS_CAM_2]);
	DDX_Control(pDX, IDC_CH_VLOSS_3, m_butAll[IDC_DCL_BUT_VLOSS_CAM_3]);
	DDX_Control(pDX, IDC_CH_VLOSS_4, m_butAll[IDC_DCL_BUT_VLOSS_CAM_4]);
	DDX_Control(pDX, IDC_CH_VLOSS_5, m_butAll[IDC_DCL_BUT_VLOSS_CAM_5]);
	DDX_Control(pDX, IDC_CH_VLOSS_6, m_butAll[IDC_DCL_BUT_VLOSS_CAM_6]);
	DDX_Control(pDX, IDC_CH_VLOSS_7, m_butAll[IDC_DCL_BUT_VLOSS_CAM_7]);
	DDX_Control(pDX, IDC_CH_VLOSS_8, m_butAll[IDC_DCL_BUT_VLOSS_CAM_8]);
	DDX_Control(pDX, IDC_CH_VLOSS_9, m_butAll[IDC_DCL_BUT_VLOSS_CAM_9]);
	DDX_Control(pDX, IDC_CH_VLOSS_10, m_butAll[IDC_DCL_BUT_VLOSS_CAM_10]);
	DDX_Control(pDX, IDC_CH_VLOSS_11, m_butAll[IDC_DCL_BUT_VLOSS_CAM_11]);
	DDX_Control(pDX, IDC_CH_VLOSS_12, m_butAll[IDC_DCL_BUT_VLOSS_CAM_12]);
	DDX_Control(pDX, IDC_CH_VLOSS_13, m_butAll[IDC_DCL_BUT_VLOSS_CAM_13]);
	DDX_Control(pDX, IDC_CH_VLOSS_14, m_butAll[IDC_DCL_BUT_VLOSS_CAM_14]);
	DDX_Control(pDX, IDC_CH_VLOSS_15, m_butAll[IDC_DCL_BUT_VLOSS_CAM_15]);
}


BEGIN_MESSAGE_MAP(CDlgCtrlLive, CDialog)
	ON_MESSAGE(WM_DOO_INDEX_BUTTON_CLICK, OnIndexButtonClick)
	ON_MESSAGE (WM_DOO_LIVE_DISP_WND_CLICK, OnDispWndClick)
	//{{AFX_MSG_MAP(CDlgCtrlLive)
	ON_WM_DESTROY()
	ON_WM_NCHITTEST()
	ON_WM_SHOWWINDOW()
	ON_WM_HSCROLL()
	ON_WM_TIMER()
	ON_NOTIFY(UDN_DELTAPOS, IDC_SP_NUM_FW_BW_STEP, OnDeltaposSpNumFwBwStep)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlLive message handlers

BOOL CDlgCtrlLive::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	int i;

	for (i=0; i<NUM_DCL_BUTTONS; i++)
	{
		m_butAll[i].SetID (i);
	}

	for (i=0; i<NUM_COLOR_ITEM; i++)
	{
		m_bufSliColor[i].SetRange (0, 255);
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

	m_butAll[IDC_DCL_BUT_CHANGE_LIVE_MODE_AUTO].SetCheck (TRUE);

	SetTimer (IDX_DCL_TIMER_UPDATE_PLAYER_STAT, 50, NULL);

	glPDlgCtrlLive = this;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCtrlLive::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
	KillTimer (IDX_DCL_TIMER_UPDATE_PLAYER_STAT);
}

UINT CDlgCtrlLive::OnNcHitTest(CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	return HTCAPTION;
	return CDialog::OnNcHitTest(point);
}

void CDlgCtrlLive::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here
	
	if (bShow == TRUE)
	{
		int i;
		for (i=IDX_DCL_BUT_LIVE_DISP_1; i<=IDX_DCL_BUT_LIVE_DISP_16; i++)
		{
			m_butAll[i].SetCheck (FALSE);
		}
		m_butAll[glPCurSelS6010->LIVE_GetSplitMode ()].SetCheck (TRUE);
		m_butAll[IDX_DCL_BUT_CH_DISP].SetCheck (glPCurGInfo->bLiveDispCh);
		m_butAll[glPCurSelS6010->LIVE_GetChFromIdxWnd (glPCurGInfo->idxLiveSelWnd)
			+IDX_DCL_BUT_CH_0].SetCheck (TRUE);

		GetLiveColorAndUpdateCtrl ();	// Color Info Update

		SetTimer (IDX_DCL_TIMER_DET_VLOSS, glPCurGInfo->VL_checkInterval, NULL);

		glPCurHWDecManager->RegisterCB_UpdatePlayerStat (CbFn_UpdatePlayerStat, this);
	}
	else
	{
		KillTimer (IDX_DCL_TIMER_DET_VLOSS);
		
		glPCurHWDecManager->UnregisterCB_UpdatePlayerStat ();
		glPCurHWDecManager->CloseFile ();
	}
}

void CDlgCtrlLive::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	
	if (nIDEvent == IDX_DCL_TIMER_DET_VLOSS)
	{	// Video Loss Check
		DWORD maskVLoss = glPCurSelS6010->VLOSS_GetState ();

		int i;
		for (i=0; i<NUM_LIVE_DISP_WND; i++)
		{
			m_butAll[IDC_DCL_BUT_VLOSS_CAM_0 +i].EnableWindow ((maskVLoss >>i) &0x1);
		}
	}
	else if (nIDEvent == IDX_DCL_TIMER_UPDATE_PLAYER_STAT)
	{
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
	}

	CDialog::OnTimer(nIDEvent);
}

void CDlgCtrlLive::OnDeltaposSpNumFwBwStep(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);
	if (pNMUpDown->iDelta > 0)
	{
		m_numStepFBW++;
		if (m_numStepFBW > int(glPCurHWDecManager->GetTotalVideoFrameNum ()))
		{
			m_numStepFBW = glPCurHWDecManager->GetTotalVideoFrameNum ();
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

LRESULT CDlgCtrlLive::OnIndexButtonClick (WPARAM wParam, LPARAM lParam)
{
	int i;

	if (wParam <= IDX_DCL_BUT_LIVE_DISP_16)	// Live Mode Change
	{
		glPCurHWDecManager->Pause ();

		m_butAll[glPCurSelS6010->LIVE_GetSplitMode () -LIVE_SPLIT_MODE_1 +IDX_DCL_BUT_LIVE_DISP_1].SetCheck (FALSE);
		m_butAll[glPCurSelS6010->LIVE_GetChFromIdxWnd (glPCurGInfo->idxLiveSelWnd) +IDX_DCL_BUT_CH_0].SetCheck (FALSE);

		glPCurGInfo->idxLiveSelWnd = 0;	// Selected Window Init
		glPCurSelS6010->LIVE_SetSplitMode (wParam -IDX_DCL_BUT_LIVE_DISP_1 +LIVE_SPLIT_MODE_1);
		glPWndLiveDisp->Invalidate ();

		m_butAll[glPCurSelS6010->LIVE_GetSplitMode () -LIVE_SPLIT_MODE_1 +IDX_DCL_BUT_LIVE_DISP_1].SetCheck (TRUE);
		m_butAll[glPCurSelS6010->LIVE_GetChFromIdxWnd (glPCurGInfo->idxLiveSelWnd) +IDX_DCL_BUT_CH_0].SetCheck (TRUE);

		if (glPCurHWDecManager->IsWndStreamCIF () == TRUE)
		{
			glPWndLiveDisp->ZoomOverlayDestHSize (2);
		}
		else
		{
			glPWndLiveDisp->ZoomOverlayDestHSize (1);
		}

		glPCurHWDecManager->SetFramePos (glPCurHWDecManager->GetFramePos ());
	}
	else if (wParam <= IDX_DCL_BUT_CH_15)	// Live Channel Change
	{
		for (i=IDX_DCL_BUT_CH_0; i<=IDX_DCL_BUT_CH_15; i++)
		{
			m_butAll[i].SetCheck (FALSE);
		}

		if (lParam != 1)
		{
			glPCurSelS6010->LIVE_ChangeLiveCh (glPCurGInfo->idxLiveSelWnd, wParam -IDX_DCL_BUT_CH_0);
			glPWndLiveDisp->Invalidate ();
		}

		m_butAll[glPCurSelS6010->LIVE_GetChFromIdxWnd (glPCurGInfo->idxLiveSelWnd) +IDX_DCL_BUT_CH_0].SetCheck (TRUE);

		GetLiveColorAndUpdateCtrl ();	// Color Info Update
	}
	else if (wParam <= IDX_DCL_BUT_DISZ_FULL)	// Display Size Change
	{
		int hRes = GetSystemMetrics (SM_CXSCREEN);
		int vRes = GetSystemMetrics (SM_CYSCREEN);
		int sizeFrmH = GetSystemMetrics (SM_CXDLGFRAME);
		int sizeFrmV = GetSystemMetrics (SM_CYDLGFRAME);
		int szWndH, szWndV, posX, posY;

		CRect rcWindow;
		glPWndLiveDisp->GetWindowRect (&rcWindow);

		switch (wParam)
		{
		case IDX_DCL_BUT_DISZ_DIV_4:
			szWndH = glPCurGInfo->szVideo.cx /4 +sizeFrmH *2;
			szWndV = glPCurGInfo->szVideo.cy /4 +sizeFrmV *2;
			break;
		case IDX_DCL_BUT_DISZ_DIV_2:
			szWndH = glPCurGInfo->szVideo.cx /2 +sizeFrmH *2;
			szWndV = glPCurGInfo->szVideo.cy /2 +sizeFrmV *2;
			break;
		case IDX_DCL_BUT_DISZ_X_1:
			szWndH = glPCurGInfo->szVideo.cx +sizeFrmH *2;
			szWndV = glPCurGInfo->szVideo.cy +sizeFrmV *2;
			break;
		case IDX_DCL_BUT_DISZ_X_2:
			szWndH = glPCurGInfo->szVideo.cx *2 +sizeFrmH *2;
			szWndV = glPCurGInfo->szVideo.cy *2 +sizeFrmV *2;
			break;
		case IDX_DCL_BUT_DISZ_X_3:
			szWndH = glPCurGInfo->szVideo.cx *3 +sizeFrmH *2;
			szWndV = glPCurGInfo->szVideo.cy *3 +sizeFrmV *2;
			break;
		case IDX_DCL_BUT_DISZ_FULL:
			szWndH = hRes;
			szWndV = vRes;
			break;
		}
		posX = rcWindow.left +(rcWindow.Width () -szWndH) /2;
		posY = rcWindow.top +(rcWindow.Height () -szWndV) /2;

		if (szWndH > hRes)
		{
			szWndH = hRes;
		}
		if (szWndV > vRes)
		{
			szWndV = vRes;
		}
		if (posX < 0)
		{
			posX = 0;
		}
		if (posY < 0)
		{
			posY = 0;
		}
	
		if (wParam == IDX_DCL_BUT_DISZ_FULL)
		{
			glPWndLiveDisp->MoveWindow (-sizeFrmH, -sizeFrmV, hRes +sizeFrmH *2, vRes +sizeFrmV *2);
		}
		else
		{
			glPWndLiveDisp->MoveWindow (posX, posY, szWndH, szWndV);
		}
		glPWndLiveDisp->UpdateOverlay ();
	}
	else if (wParam == IDX_DCL_BUT_CH_DISP)
	{
		glPCurGInfo->bLiveDispCh = 1 -glPCurGInfo->bLiveDispCh;
		glPWndLiveDisp->Invalidate ();
	}
	else if (wParam == IDX_DCL_BUT_CH_ALWAYS_TOP)
	{
		glPCurGInfo->bAlwaysTop = 1 -glPCurGInfo->bAlwaysTop;

		if (glPCurGInfo->bAlwaysTop == 1)	// Set topmost window
		{
			glPWndLiveDisp->SetWindowPos (&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE |SWP_NOSIZE);
		}
		else	// Release
		{
			glPWndLiveDisp->SetWindowPos (&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE |SWP_NOSIZE);
		}
	}
	else if (wParam == IDX_DCL_BUT_HIDE)
	{
		OnOK ();
	}
	else if (wParam == IDX_DCL_BUT_QUIT)
	{
		glPDlgApp->SendMessage (WM_DOO_APP_DLG_CTRL, IDX_APP_MSG_ON_BUT_LIVE, 0);
	}
	else if (wParam <= IDX_DCL_BUT_CLR_DEF_BRI)
	{	// Default color value (HUE, SAT, BRI, CON)
		OnColorDefButDn (wParam);
	}
	else if (wParam <= IDX_DCL_BUT_L_OR_D_15)
	{	// Window Mode (Live Display or MP4 Decode)
		glPCurHWDecManager->Pause ();

		OnDispWndModeButDn (wParam);

		if (glPCurHWDecManager->IsWndStreamCIF () == TRUE)
		{
			glPWndLiveDisp->ZoomOverlayDestHSize (2);
		}
		else
		{
			glPWndLiveDisp->ZoomOverlayDestHSize (1);
		}

		glPCurHWDecManager->SetFramePos (glPCurHWDecManager->GetFramePos ());
	}
	else if (wParam <= IDC_DCL_BUT_CHANGE_LIVE_MODE_AUTO)
	{	// MP4 HW Decoder Player
		if (glPCurHWDecManager != NULL)
		{
			OnHWPlayerCtrlButDn (wParam);
		}
	}
	else if (wParam <= IDC_DCL_BUT_VLOSS_CAM_15)
	{
		m_butAll[wParam].SetCheck (FALSE);
	}

	return 0;
}

LRESULT CDlgCtrlLive::OnDispWndClick (WPARAM wParam, LPARAM lParam)
{
	OnIndexButtonClick (glPCurSelS6010->LIVE_GetChFromIdxWnd (glPCurGInfo->idxLiveSelWnd) +IDX_DCL_BUT_CH_0, 1);

	return 0;
}

void CDlgCtrlLive::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	
	CSliderCtrl *pSlider = (CSliderCtrl *)pScrollBar;

	switch (pSlider->GetDlgCtrlID ())
	{
	case IDC_SLI_COLOR_HUE:
		SetLiveColorAndUpdateCtrl (IDX_COLOR_HUE, pSlider->GetPos ());
		break;
	case IDC_SLI_COLOR_SAT:
		SetLiveColorAndUpdateCtrl (IDX_COLOR_SAT, pSlider->GetPos ());
		break;
	case IDC_SLI_COLOR_CON:
		SetLiveColorAndUpdateCtrl (IDX_COLOR_CON, pSlider->GetPos ());
		break;
	case IDC_SLI_COLOR_BRI:
		SetLiveColorAndUpdateCtrl (IDX_COLOR_BRI, pSlider->GetPos ());
		break;
	case IDC_SLI_PLAYER:
		glPCurHWDecManager->SetFramePos (pSlider->GetPos ());
		break;
	}
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CDlgCtrlLive::GetLiveColorAndUpdateCtrl ()
{
	BYTE nHue, nSaturation, nContrast, nBrightness;

	glPCurSelS6010->COLOR_Get (glPCurSelS6010->LIVE_GetChFromIdxWnd (glPCurGInfo->idxLiveSelWnd),
		nHue, nSaturation, nContrast, nBrightness);

	CString strTmp;

	strTmp.Format ("%d", nHue);
	m_bufStaColor[IDX_COLOR_HUE].SetWindowText (LPCTSTR(strTmp));
	m_bufSliColor[IDX_COLOR_HUE].SetPos (nHue);

	strTmp.Format ("%d", nSaturation);
	m_bufStaColor[IDX_COLOR_SAT].SetWindowText (LPCTSTR(strTmp));
	m_bufSliColor[IDX_COLOR_SAT].SetPos (nSaturation);

	strTmp.Format ("%d", nContrast);
	m_bufStaColor[IDX_COLOR_CON].SetWindowText (LPCTSTR(strTmp));
	m_bufSliColor[IDX_COLOR_CON].SetPos (nContrast);

	strTmp.Format ("%d", nBrightness);
	m_bufStaColor[IDX_COLOR_BRI].SetWindowText (LPCTSTR(strTmp));
	m_bufSliColor[IDX_COLOR_BRI].SetPos (nBrightness);
}

void CDlgCtrlLive::SetLiveColorAndUpdateCtrl (int idxColor, int nValue)
{
	switch (idxColor)
	{
	case IDX_COLOR_HUE:
		glPCurSelS6010->COLOR_SetHue (
			glPCurSelS6010->LIVE_GetChFromIdxWnd (glPCurGInfo->idxLiveSelWnd), nValue);
		break;
	case IDX_COLOR_SAT:
		glPCurSelS6010->COLOR_SetSaturation (
			glPCurSelS6010->LIVE_GetChFromIdxWnd (glPCurGInfo->idxLiveSelWnd), nValue);
		break;
	case IDX_COLOR_CON:
		glPCurSelS6010->COLOR_SetContrast (
			glPCurSelS6010->LIVE_GetChFromIdxWnd (glPCurGInfo->idxLiveSelWnd), nValue);
		break;
	case IDX_COLOR_BRI:
		glPCurSelS6010->COLOR_SetBrightness (
			glPCurSelS6010->LIVE_GetChFromIdxWnd (glPCurGInfo->idxLiveSelWnd), nValue);
		break;
	}

	CString strTmp;
	strTmp.Format ("%d", nValue);
	m_bufStaColor[idxColor].SetWindowText (LPCTSTR(strTmp));
}

void CDlgCtrlLive::OnColorDefButDn (WPARAM wParam)
{
	switch (wParam)
	{
	case IDX_DCL_BUT_CLR_DEF_HUE:
		SetLiveColorAndUpdateCtrl (IDX_COLOR_HUE, NUM_DEF_COLOR_VAL);
		m_bufSliColor[IDX_COLOR_HUE].SetPos (NUM_DEF_COLOR_VAL);
		break;
	case IDX_DCL_BUT_CLR_DEF_SAT:
		SetLiveColorAndUpdateCtrl (IDX_COLOR_SAT, NUM_DEF_COLOR_VAL);
		m_bufSliColor[IDX_COLOR_SAT].SetPos (NUM_DEF_COLOR_VAL);
		break;
	case IDX_DCL_BUT_CLR_DEF_CON:
		SetLiveColorAndUpdateCtrl (IDX_COLOR_CON, NUM_DEF_COLOR_VAL);
		m_bufSliColor[IDX_COLOR_CON].SetPos (NUM_DEF_COLOR_VAL);
		break;
	case IDX_DCL_BUT_CLR_DEF_BRI:
		SetLiveColorAndUpdateCtrl (IDX_COLOR_BRI, NUM_DEF_COLOR_VAL);
		m_bufSliColor[IDX_COLOR_BRI].SetPos (NUM_DEF_COLOR_VAL);
		break;
	}
}

void CDlgCtrlLive::SetDispWndModeFromFlag ()
{
	int i;
	char strCaption[100];

	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		if (glPCurSelS6010->LIVE_IsWndLiveOrDec (i) == TRUE)
		{
			sprintf (strCaption, "%dL", i +1);
		}
		else
		{
			sprintf (strCaption, "%dD", i +1);
		}
		m_butAll[IDX_DCL_BUT_L_OR_D_0 +i].SetWindowText (strCaption);
	}
}

void CDlgCtrlLive::OnDispWndModeButDn (WPARAM wParam)
{
	int idxWnd = wParam -IDX_DCL_BUT_L_OR_D_0;
	BOOL bLive;

	if (glPCurSelS6010->LIVE_IsWndLiveOrDec (idxWnd) == TRUE)
	{
		bLive = FALSE;
	}
	else
	{
		bLive = TRUE;
	}
	glPCurSelS6010->LIVE_SetWndLiveOrDec (idxWnd, bLive);

	SetDispWndModeFromFlag ();
	glPWndLiveDisp->Invalidate ();
}

void CDlgCtrlLive::OnHWPlayerCtrlButDn (WPARAM wParam)
{
	if (wParam == IDX_DCL_BUT_HWD_OPEN_VID_FILE ||
		wParam == IDX_DCL_BUT_HWD_OPEN_AUD_FILE)
	{
		CFileDialog dlg(TRUE);
		if (dlg.DoModal () == IDOK)
		{
			if (wParam == IDX_DCL_BUT_HWD_OPEN_VID_FILE)
			{
				m_edVidFilePath = dlg.GetPathName ();
			}
			else if (wParam == IDX_DCL_BUT_HWD_OPEN_AUD_FILE)
			{
				m_edAudFilePath = dlg.GetPathName ();
			}
			if (glPCurHWDecManager->OpenFile (m_edVidFilePath, m_edAudFilePath) == TRUE)
			{
				if (m_bChangeLiveModeAuto == TRUE)
				{
					int i, j;
					for (i=0, j=0; i<NUM_VID_TOTAL_ENC_CH; i++)
					{
						if (glPCurHWDecManager->GetVideoFrameNum (i) != 0)
						{
							glPCurSelS6010->LIVE_SetWndLiveOrDec (j, FALSE);
							glPCurSelS6010->LIVE_ChangeDecCh (j, i);
							j++;
						}

						if (j >= GL_NUM_DISP_FROM_DISP_MODE[glPCurSelS6010->LIVE_GetSplitMode ()])
						{
							break;
						}
					}
					for (; j<GL_NUM_DISP_FROM_DISP_MODE[glPCurSelS6010->LIVE_GetSplitMode ()]; j++)
					{
						glPCurSelS6010->LIVE_SetWndLiveOrDec (j, TRUE);
					}
					glPWndLiveDisp->Invalidate ();
					OnDispWndModeButDn (wParam);
					Sleep (500);
				}

				if (glPCurHWDecManager->IsWndStreamCIF () == TRUE)
				{
					glPWndLiveDisp->ZoomOverlayDestHSize (2);
				}
				else
				{
					glPWndLiveDisp->ZoomOverlayDestHSize (1);
				}

				UpdateData (FALSE);

				glPCurHWDecManager->Stop ();
				UpdateHWPlayerCtrl ();
			}
		}
	}
	else if (wParam == IDX_DCL_BUT_HWD_PLAY)
	{
		glPCurHWDecManager->Play ();
	}
	else if (wParam == IDX_DCL_BUT_HWD_PAUSE)
	{
		glPCurHWDecManager->Pause ();
	}
	else if (wParam == IDX_DCL_BUT_HWD_STOP)
	{
		glPCurHWDecManager->Stop ();
	}
	else if (wParam == IDX_DCL_BUT_HWD_BACK_PLAY)
	{
		glPCurHWDecManager->ReversePlay ();
	}
	else if (wParam == IDX_DCL_BUT_HWD_BW_1_FRM)
	{
		glPCurHWDecManager->OffsetFramePos (glPCurGInfo->idxLiveSelWnd, -1);
	}
	else if (wParam == IDX_DCL_BUT_HWD_FW_1_FRM)
	{
		glPCurHWDecManager->OffsetFramePos (glPCurGInfo->idxLiveSelWnd, 1);
	}
	else if (wParam == IDX_DCL_BUT_HWD_BW_N_FRM)
	{
		glPCurHWDecManager->OffsetFramePos (glPCurGInfo->idxLiveSelWnd, -m_numStepFBW);
	}
	else if (wParam == IDX_DCL_BUT_HWD_FW_N_FRM)
	{
		glPCurHWDecManager->OffsetFramePos (glPCurGInfo->idxLiveSelWnd, m_numStepFBW);
	}
	else if (wParam == IDX_DCL_BUT_HWD_SET_POS_START)
	{
		glPCurHWDecManager->SetFramePos (0);
	}
	else if (wParam == IDX_DCL_BUT_HWD_SET_POS_END)
	{
		glPCurHWDecManager->SetFramePos (glPCurHWDecManager->GetTotalVideoFrameNum () -1);
	}
	else if (wParam == IDX_DCL_BUT_HWD_PS_SLOW)
	{
		glPCurHWDecManager->SetPlaySpeed (glPCurHWDecManager->GetPlaySpeed () -1);
		UpdateHWPlayerCtrl (FALSE);
	}
	else if (wParam == IDX_DCL_BUT_HWD_PS_NORMAL)
	{
		glPCurHWDecManager->SetPlaySpeed (PM_PLAY_SPEED_NORMAL);
		UpdateHWPlayerCtrl (FALSE);
	}
	else if (wParam == IDX_DCL_BUT_HWD_PS_FAST)
	{
		glPCurHWDecManager->SetPlaySpeed (glPCurHWDecManager->GetPlaySpeed () +1);
		UpdateHWPlayerCtrl (FALSE);
	}
	else if (wParam <= IDX_DCL_BUT_HWD_CH_VIRT_15)
	{	// MP4 HW Decoder Real/Virtual Channel Click
		glPCurSelS6010->LIVE_ChangeDecCh (glPCurGInfo->idxLiveSelWnd, wParam -IDX_DCL_BUT_HWD_CH_REAL_0);
		glPWndLiveDisp->Invalidate ();

		m_butAll[wParam].SetCheck (FALSE);
		
		glPCurHWDecManager->Stop ();
	}
	else if (wParam == IDC_DCL_BUT_USE_S6010_DI_FLT)
	{
		glPCurHWDecManager->EnableS6010DeinterlaceFilter (m_butAll[IDC_DCL_BUT_USE_S6010_DI_FLT].GetCheck ());
		
		glPCurHWDecManager->Stop ();
	}
	else if (wParam == IDC_DCL_BUT_CHANGE_LIVE_MODE_AUTO)
	{
		m_bChangeLiveModeAuto = 1 -m_bChangeLiveModeAuto;
	}
}

void CDlgCtrlLive::UpdateHWPlayerCtrl (BOOL bInitSlider)
{
	int i;
	
	// Slider[S]
	if (bInitSlider == TRUE)
	{
		m_sliPlayer.SetRange (0, glPCurHWDecManager->GetTotalVideoFrameNum () -1);
		m_sliPlayer.SetPos (0);
	}
	// Slider[E]
	
	// Play Speed[S]
	CString strPlaySpeed;

	switch (glPCurHWDecManager->GetPlaySpeed ())
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
	// Play Speed[E]

	// Channel Buttons[S]
	for (i=0; i<NUM_VID_TOTAL_ENC_CH; i++)
	{
		if (glPCurHWDecManager->GetVideoFrameNum (i) > 0)
		{
			m_butAll[IDX_DCL_BUT_HWD_CH_REAL_0 +i].EnableWindow (TRUE);
		}
		else
		{
			m_butAll[IDX_DCL_BUT_HWD_CH_REAL_0 +i].EnableWindow (FALSE);
		}
	}
	// Channel Buttons[E]
}
