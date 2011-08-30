// DlgSettingGeneral.cpp : implementation file
//

#include "stdafx.h"
#include "SOLO6010App.h"
#include "DlgSettingGeneral.h"
#include "SOLO6010AppDlg.h"
#include "DlgCtrlLive.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSettingGeneral dialog


CDlgSettingGeneral::CDlgSettingGeneral(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSettingGeneral::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSettingGeneral)
	m_radVideoMode = -1;
	m_edStrRecDir = _T("");
	m_radVCard = -1;
	m_radLiveDispMode = -1;
	m_radModeDeinterlacing = -1;
	m_radRecMethodMP4 = -1;
	m_radRecMethodG723 = -1;
	m_staNumOverSurf = _T("");
	m_chEncOSD_bUseCh = FALSE;
	m_chEncOSD_bUseTime = FALSE;
	m_chEncOSD_bUseDate = FALSE;
	m_chEncOSD_bUseShadowText = FALSE;
	m_edEncOSD_colorY = 0;
	m_edEncOSD_colorU = 0;
	m_edEncOSD_colorV = 0;
	m_edEncOSD_offsetH = 0;
	m_edEncOSD_offsetV = 0;
	//}}AFX_DATA_INIT

	m_bChangedGlGInfo = FALSE;
}


void CDlgSettingGeneral::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSettingGeneral)
	DDX_Control(pDX, IDC_SP_NUM_OVER_SURF, m_spNumOverSurf);
	DDX_Radio(pDX, IDC_RAD_V_TYPE_NTSC, m_radVideoMode);
	DDX_Text(pDX, IDC_ED_REC_DIR, m_edStrRecDir);
	DDX_Radio(pDX, IDC_RAD_VCARD_INTEL_ONBOARD, m_radVCard);
	DDX_Radio(pDX, IDC_RAD_LIVE_DISP_MODE_0, m_radLiveDispMode);
	DDX_Radio(pDX, IDC_RAD_LIVE_DEINTERLACING_0, m_radModeDeinterlacing);
	DDX_Radio(pDX, IDC_RAD_REC_METHOD_MP4_0, m_radRecMethodMP4);
	DDX_Radio(pDX, IDC_RAD_REC_METHOD_G723_0, m_radRecMethodG723);
	DDX_Text(pDX, IDC_STA_NUM_OVER_SURF, m_staNumOverSurf);
	DDX_Check(pDX, IDC_CH_ENC_OSD_USE_CHANNEL, m_chEncOSD_bUseCh);
	DDX_Check(pDX, IDC_CH_ENC_OSD_USE_TIME, m_chEncOSD_bUseTime);
	DDX_Check(pDX, IDC_CH_ENC_OSD_USE_DATE, m_chEncOSD_bUseDate);
	DDX_Check(pDX, IDC_CH_ENC_OSD_B_USE_SHADOW, m_chEncOSD_bUseShadowText);
	DDX_Text(pDX, IDC_ED_ENC_OSD_COLOR_Y, m_edEncOSD_colorY);
	DDV_MinMaxByte(pDX, m_edEncOSD_colorY, 0, 255);
	DDX_Text(pDX, IDC_ED_ENC_OSD_COLOR_U, m_edEncOSD_colorU);
	DDV_MinMaxByte(pDX, m_edEncOSD_colorU, 0, 255);
	DDX_Text(pDX, IDC_ED_ENC_OSD_COLOR_V, m_edEncOSD_colorV);
	DDV_MinMaxByte(pDX, m_edEncOSD_colorV, 0, 255);
	DDX_Text(pDX, IDC_ED_ENC_OSD_OFFSET_H, m_edEncOSD_offsetH);
	DDV_MinMaxByte(pDX, m_edEncOSD_offsetH, 0, 127);
	DDX_Text(pDX, IDC_ED_ENC_OSD_OFFSET_V, m_edEncOSD_offsetV);
	DDV_MinMaxByte(pDX, m_edEncOSD_offsetV, 0, 127);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_COM_VD_MATRIX_CH_0, m_comVDInCh[0]);
	DDX_Control(pDX, IDC_COM_VD_MATRIX_CH_1, m_comVDInCh[1]);
	DDX_Control(pDX, IDC_COM_VD_MATRIX_CH_2, m_comVDInCh[2]);
	DDX_Control(pDX, IDC_COM_VD_MATRIX_CH_3, m_comVDInCh[3]);
	DDX_Control(pDX, IDC_COM_VD_MATRIX_CH_4, m_comVDInCh[4]);
	DDX_Control(pDX, IDC_COM_VD_MATRIX_CH_5, m_comVDInCh[5]);
	DDX_Control(pDX, IDC_COM_VD_MATRIX_CH_6, m_comVDInCh[6]);
	DDX_Control(pDX, IDC_COM_VD_MATRIX_CH_7, m_comVDInCh[7]);
	DDX_Control(pDX, IDC_COM_VD_MATRIX_CH_8, m_comVDInCh[8]);
	DDX_Control(pDX, IDC_COM_VD_MATRIX_CH_9, m_comVDInCh[9]);
	DDX_Control(pDX, IDC_COM_VD_MATRIX_CH_10, m_comVDInCh[10]);
	DDX_Control(pDX, IDC_COM_VD_MATRIX_CH_11, m_comVDInCh[11]);
	DDX_Control(pDX, IDC_COM_VD_MATRIX_CH_12, m_comVDInCh[12]);
	DDX_Control(pDX, IDC_COM_VD_MATRIX_CH_13, m_comVDInCh[13]);
	DDX_Control(pDX, IDC_COM_VD_MATRIX_CH_14, m_comVDInCh[14]);
	DDX_Control(pDX, IDC_COM_VD_MATRIX_CH_15, m_comVDInCh[15]);
}


BEGIN_MESSAGE_MAP(CDlgSettingGeneral, CDialog)
	//{{AFX_MSG_MAP(CDlgSettingGeneral)
	ON_BN_CLICKED(IDC_BUT_OPEN_REC_DIR, OnButOpenRecDir)
	ON_BN_CLICKED(IDC_BUT_SET_MOSAIC_MOTION, OnButSetMosaicMotion)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SP_NUM_OVER_SURF, OnDeltaposSpNumOverSurf)
	ON_BN_CLICKED(IDC_BUT_DEFAULT_NUM_SURF, OnButDefaultNumSurf)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSettingGeneral message handlers

void CDlgSettingGeneral::OnButOpenRecDir() 
{
	// TODO: Add your control notification handler code here
	
	CString strTmp;
	if (MiscBrowseForFolder (&strTmp, STR_APP_TITLE, GetSafeHwnd ()) == TRUE)
	{
		if (strTmp.GetAt (strTmp.GetLength () -1) != '\\')
		{
			strTmp += "\\";
		}
		m_edStrRecDir = strTmp;
		UpdateData (FALSE);
	}
}

BOOL CDlgSettingGeneral::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	int i, j;

	m_radVideoMode = glPCurGInfo->typeVideo;	// Video type

	// Live display mode[S]
	UDACCEL uda[1];
	uda[0].nSec = 0;
	uda[0].nInc = 1;
	m_spNumOverSurf.SetRange (0, 1);
	m_spNumOverSurf.SetAccel (1, uda);

	m_radVCard = glPCurGInfo->idxVCard;
	m_radLiveDispMode = glPCurGInfo->modeLiveDisp;
	m_radModeDeinterlacing = glPCurGInfo->modeDeinterlacing;
	m_numOverSurf = glPCurGInfo->numOverSurf;

	m_staNumOverSurf.Format ("%d", m_numOverSurf);
	// Live display mode[E]

	// MPEG4[S]
	// Rec data rcv mode MP4[S]
	if (glPCurGInfo->setRec.modeRecMP4 == REC_DATA_RCV_BY_RAW_DATA)
	{	// record raw data
		m_radRecMethodMP4 = 0;
	}
	else
	{
		m_radRecMethodMP4 = 1;
	}
	// Rec data rcv mode MP4[E]

	// Encoder OSD[S]
	m_chEncOSD_bUseCh = glPCurGInfo->ENC_OSD_bShowCh;
	m_chEncOSD_bUseDate = glPCurGInfo->ENC_OSD_bShowDate;
	m_chEncOSD_bUseTime = glPCurGInfo->ENC_OSD_bShowTime;

	m_chEncOSD_bUseShadowText = glPCurGInfo->ENC_OSD_bUseShadowText;

	m_edEncOSD_colorY = glPCurGInfo->ENC_OSD_colorY;
	m_edEncOSD_colorU = glPCurGInfo->ENC_OSD_colorU;
	m_edEncOSD_colorV = glPCurGInfo->ENC_OSD_colorV;

	m_edEncOSD_offsetH = glPCurGInfo->ENC_OSD_offsetH;
	m_edEncOSD_offsetV = glPCurGInfo->ENC_OSD_offsetV;
	// Encoder OSD[E]
	// MPEG4[E]
	
	// G.723[S]
	// Rec data rcv mode G723[S]
	if (glPCurGInfo->setRec.modeRecG723 == REC_DATA_RCV_BY_RAW_DATA)
	{	// record raw data
		m_radRecMethodG723 = 0;
	}
	else
	{
		m_radRecMethodG723 = 1;
	}
	// Rec data rcv mode G723[E]
	// G.723[E]

	m_edStrRecDir = glPCurGInfo->strRecDir;

	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		for (j=0; j<NUM_LIVE_DISP_WND; j++)
		{
			m_comVDInCh[i].AddString (GL_STR_CAM[j]);
		}
		m_comVDInCh[i].SetCurSel (glPCurGInfo->bufIdxCamToWndCh[i]);
	}

	memcpy (&m_tmpGlGInfo, glPCurGInfo, sizeof(INFO_APP_GLOBAL));	// for glGInfo change test

	UpdateData (FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSettingGeneral::OnOK() 
{
	// TODO: Add extra validation here
	
	if (glPCurGInfo->bOnLive == TRUE && glPCurGInfo->bSetMosaicVM == TRUE)
	{	// quitting mosaic and v-motion setting
		glPDlgApp->SendMessage (WM_DOO_APP_DLG_CTRL, IDX_APP_MSG_ON_BUT_LIVE, 0);
	}

	UpdateData (TRUE);

	// Video type[S]
	glPCurGInfo->typeVideo = m_radVideoMode;
	glPCurGInfo->szVideo.cx = VIDEO_IN_H_SIZE;
	if (glPCurGInfo->typeVideo == VIDEO_TYPE_NTSC)
	{
		glPCurGInfo->szVideo.cy = 480;
	}
	else
	{
		glPCurGInfo->szVideo.cy = 576;
	}
	// Video type[E]

	strcpy (glPCurGInfo->strRecDir, LPCTSTR(m_edStrRecDir));	// Record directory

	int i;

	// Video matrix[S]
	for (i=0; i<NUM_LIVE_DISP_WND; i++)
	{
		glPCurGInfo->bufIdxCamToWndCh[i] = m_comVDInCh[i].GetCurSel ();
	}
	// Video matrix[E]

	// Live display mode[S]
	glPCurGInfo->idxVCard = m_radVCard;
	glPCurGInfo->modeLiveDisp = m_radLiveDispMode;
	glPCurGInfo->modeDeinterlacing = m_radModeDeinterlacing;
	glPCurGInfo->numOverSurf = m_numOverSurf;

	if (glPCurGInfo->idxVCard == VCARD_INTEL_ONBOARD)
	{
		glPCurGInfo->clrOverlay = COLOR_OVERLAY_DEST_INTEL;
	}
	else	// VCARD_OTHER
	{
		glPCurGInfo->clrOverlay = COLOR_OVERLAY_DEST;
	}
	// Live display mode[E]

	// MPEG4[S]
	// Rec data rcv mode MP4[S]
	if (m_radRecMethodMP4 == 0)
	{
		glPCurGInfo->setRec.modeRecMP4 = REC_DATA_RCV_BY_RAW_DATA;
	}
	else
	{
		glPCurGInfo->setRec.modeRecMP4 = REC_DATA_RCV_BY_FRAME;
	}
	// Rec data rcv mode MP4[E]

	// Encoder OSD[S]
	glPCurGInfo->ENC_OSD_bShowCh = m_chEncOSD_bUseCh;
	glPCurGInfo->ENC_OSD_bShowDate = m_chEncOSD_bUseDate;
	glPCurGInfo->ENC_OSD_bShowTime = m_chEncOSD_bUseTime;

	glPCurGInfo->ENC_OSD_bUseShadowText = m_chEncOSD_bUseShadowText;

	glPCurGInfo->ENC_OSD_colorY = m_edEncOSD_colorY;
	glPCurGInfo->ENC_OSD_colorU = m_edEncOSD_colorU;
	glPCurGInfo->ENC_OSD_colorV = m_edEncOSD_colorV;

	glPCurGInfo->ENC_OSD_offsetH = m_edEncOSD_offsetH;
	glPCurGInfo->ENC_OSD_offsetV = m_edEncOSD_offsetV;
	// Encoder OSD[E]
	// MPEG4[E]

	// G.723[S]
	// Rec data rcv mode G723[S]
	if (m_radRecMethodG723 == 0)
	{
		glPCurGInfo->setRec.modeRecG723 = REC_DATA_RCV_BY_RAW_DATA;
	}
	else
	{
		glPCurGInfo->setRec.modeRecG723 = REC_DATA_RCV_BY_FRAME;
	}
	// Rec data rcv mode G723[E]
	// G.723[E]

	if (memcmp (glPCurGInfo, &m_tmpGlGInfo, sizeof(INFO_APP_GLOBAL)) != 0)
	{
		m_bChangedGlGInfo = TRUE;
	}

	CDialog::OnOK();
}

void CDlgSettingGeneral::OnCancel() 
{
	// TODO: Add extra validation here

	if (glPCurGInfo->bOnLive == TRUE && glPCurGInfo->bSetMosaicVM == TRUE)
	{	// quitting mosaic and v-motion setting
		glPDlgApp->SendMessage (WM_DOO_APP_DLG_CTRL, IDX_APP_MSG_ON_BUT_LIVE, 0);
	}

	CDialog::OnCancel();
}


void CDlgSettingGeneral::OnButSetMosaicMotion() 
{
	// TODO: Add your control notification handler code here

	if (glPCurGInfo->bOnLive == TRUE && glPCurGInfo->bSetMosaicVM == FALSE)
	{
		glPDlgApp->SendMessage (WM_DOO_APP_DLG_CTRL, IDX_APP_MSG_ON_BUT_LIVE, 0);
	}

	glPDlgCtrlLive->SendMessage (WM_DOO_INDEX_BUTTON_CLICK, IDX_DCL_BUT_LIVE_DISP_1, 0);

	glPCurGInfo->bSetMosaicVM = TRUE;
	glPDlgApp->SendMessage (WM_DOO_APP_DLG_CTRL, IDX_APP_MSG_ON_BUT_LIVE, 0);
}

void CDlgSettingGeneral::OnDeltaposSpNumOverSurf(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);
	if (pNMUpDown->iDelta > 0)
	{
		m_numOverSurf++;
		if (m_numOverSurf > MAX_DDRAW_SURF)
		{
			m_numOverSurf = MAX_DDRAW_SURF;
		}
	}
	else
	{
		m_numOverSurf--;
		if (m_numOverSurf < 1)
		{
			m_numOverSurf = 1;
		}
	}
	m_staNumOverSurf.Format ("%d", m_numOverSurf);

	UpdateData (FALSE);

	*pResult = 0;
}

void CDlgSettingGeneral::OnButDefaultNumSurf() 
{
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);
	switch (m_radLiveDispMode)
	{
	case LIVE_DISP_1_OVERLAY_60_FI:
		m_numOverSurf = 1;
		break;
	case LIVE_DISP_1_OVERLAY_30_FR:
		m_numOverSurf = 1;
		break;
	case LIVE_DISP_N_OVERLAY_60_FI_FLIP:
		m_numOverSurf = NUM_DEF_FLIP_OVER_60FPS_SURF;
		break;
	case LIVE_DISP_N_OVERLAY_30_FR_FLIP:
		m_numOverSurf = NUM_DEF_FLIP_OVER_30FPS_SURF;
		break;
	case LIVE_DISP_SYS_TO_OVERLAY_60_FI_FLIP:
		m_numOverSurf = NUM_DEF_FLIP_OVER_30FPS_SURF;
		break;
	case LIVE_DISP_SYS_TO_OVERLAY_30_FR_FLIP:
		m_numOverSurf = NUM_DEF_FLIP_OVER_30FPS_SURF;
		break;
	case LIVE_DISP_N_OVERLAY_30_FI_FLIP:
		m_numOverSurf = NUM_DEF_FLIP_OVER_30FPS_SURF;
		break;
	}

	m_staNumOverSurf.Format ("%d", m_numOverSurf);

	UpdateData (FALSE);
}
