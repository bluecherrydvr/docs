// DlgSettingRecord.cpp : implementation file
//

#include "stdafx.h"
#include "SOLO6010App.h"
#include "DlgSettingRecord.h"
#include "SOLO6010.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int GetBitrateComIdxFromRealVal (int nBitrate)
{
	int idxComBR;
	if (nBitrate == 0)
	{
		idxComBR = 0;
	}
	else
	{
		int i;

		idxComBR = NUM_CBR_BR -1;
		for (i=1; i<NUM_CBR_BR; i++)
		{
			if (nBitrate <= BUF_BITRATE_COM_IDX_TO_REAL_VAL[i])
			{
				idxComBR = i;
				break;
			}
		}
	}

	return idxComBR;
}

int GetCBR_BufSzComIdxFromRealVal (int nBitrate, int szCBR_Buf)
{
	int idxComSzCBR_Buf, i;


	if (nBitrate == 0)
	{
		idxComSzCBR_Buf = IDX_CBR_BUF_SZ_5_SEC;
	}
	else
	{
		idxComSzCBR_Buf = NUM_CBR_BUF_SZ -1;
		for (i=0; i<NUM_CBR_BUF_SZ; i++)
		{
			if (szCBR_Buf /nBitrate <= BUF_CBR_BUF_SZ_COM_IDX_TO_REAL_VAL[i])
			{
				idxComSzCBR_Buf = i;
				break;
			}
		}
	}

	return idxComSzCBR_Buf;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgSettingRecord dialog

CDlgSettingRecord::CDlgSettingRecord(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSettingRecord::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSettingRecord)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	memcpy (&m_tmpSetRec, &glPCurGInfo->setRec, sizeof(SETTING_REC));

	memset (m_chBIBP, 0, sizeof(BOOL) *NUM_VID_REAL_ENC_CH);
	memset (m_chBRec, 0, sizeof(BOOL) *NUM_VID_REAL_ENC_CH);

	memset (m_chJPEG_BRec, 0, sizeof(BOOL) *NUM_VID_REAL_ENC_CH);

	memset (m_chG723BRec, 0, sizeof(BOOL) *NUM_G723_CH_PAIR);

	m_idxModeVideoCh = IDX_MODE_V_CH_REAL;
}


void CDlgSettingRecord::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSettingRecord)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	// MPEG4[S]
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_0, m_comQuality[0]);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_1, m_comQuality[1]);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_2, m_comQuality[2]);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_3, m_comQuality[3]);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_4, m_comQuality[4]);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_5, m_comQuality[5]);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_6, m_comQuality[6]);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_7, m_comQuality[7]);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_8, m_comQuality[8]);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_9, m_comQuality[9]);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_10, m_comQuality[10]);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_11, m_comQuality[11]);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_12, m_comQuality[12]);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_13, m_comQuality[13]);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_14, m_comQuality[14]);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_15, m_comQuality[15]);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY_A, m_comQuality[16]);

	DDX_Control(pDX, IDC_COM_SZ_IMG_0, m_comSzImg[0]);
	DDX_Control(pDX, IDC_COM_SZ_IMG_1, m_comSzImg[1]);
	DDX_Control(pDX, IDC_COM_SZ_IMG_2, m_comSzImg[2]);
	DDX_Control(pDX, IDC_COM_SZ_IMG_3, m_comSzImg[3]);
	DDX_Control(pDX, IDC_COM_SZ_IMG_4, m_comSzImg[4]);
	DDX_Control(pDX, IDC_COM_SZ_IMG_5, m_comSzImg[5]);
	DDX_Control(pDX, IDC_COM_SZ_IMG_6, m_comSzImg[6]);
	DDX_Control(pDX, IDC_COM_SZ_IMG_7, m_comSzImg[7]);
	DDX_Control(pDX, IDC_COM_SZ_IMG_8, m_comSzImg[8]);
	DDX_Control(pDX, IDC_COM_SZ_IMG_9, m_comSzImg[9]);
	DDX_Control(pDX, IDC_COM_SZ_IMG_10, m_comSzImg[10]);
	DDX_Control(pDX, IDC_COM_SZ_IMG_11, m_comSzImg[11]);
	DDX_Control(pDX, IDC_COM_SZ_IMG_12, m_comSzImg[12]);
	DDX_Control(pDX, IDC_COM_SZ_IMG_13, m_comSzImg[13]);
	DDX_Control(pDX, IDC_COM_SZ_IMG_14, m_comSzImg[14]);
	DDX_Control(pDX, IDC_COM_SZ_IMG_15, m_comSzImg[15]);
	DDX_Control(pDX, IDC_COM_SZ_IMG_A, m_comSzImg[16]);

	DDX_Control(pDX, IDC_COM_FPS_0, m_comFPS[0]);
	DDX_Control(pDX, IDC_COM_FPS_1, m_comFPS[1]);
	DDX_Control(pDX, IDC_COM_FPS_2, m_comFPS[2]);
	DDX_Control(pDX, IDC_COM_FPS_3, m_comFPS[3]);
	DDX_Control(pDX, IDC_COM_FPS_4, m_comFPS[4]);
	DDX_Control(pDX, IDC_COM_FPS_5, m_comFPS[5]);
	DDX_Control(pDX, IDC_COM_FPS_6, m_comFPS[6]);
	DDX_Control(pDX, IDC_COM_FPS_7, m_comFPS[7]);
	DDX_Control(pDX, IDC_COM_FPS_8, m_comFPS[8]);
	DDX_Control(pDX, IDC_COM_FPS_9, m_comFPS[9]);
	DDX_Control(pDX, IDC_COM_FPS_10, m_comFPS[10]);
	DDX_Control(pDX, IDC_COM_FPS_11, m_comFPS[11]);
	DDX_Control(pDX, IDC_COM_FPS_12, m_comFPS[12]);
	DDX_Control(pDX, IDC_COM_FPS_13, m_comFPS[13]);
	DDX_Control(pDX, IDC_COM_FPS_14, m_comFPS[14]);
	DDX_Control(pDX, IDC_COM_FPS_15, m_comFPS[15]);
	DDX_Control(pDX, IDC_COM_FPS_A, m_comFPS[16]);

	DDX_Control(pDX, IDC_CH_B_IBP_0, m_cbutBIBP[0]);
	DDX_Control(pDX, IDC_CH_B_IBP_1, m_cbutBIBP[1]);
	DDX_Control(pDX, IDC_CH_B_IBP_2, m_cbutBIBP[2]);
	DDX_Control(pDX, IDC_CH_B_IBP_3, m_cbutBIBP[3]);
	DDX_Control(pDX, IDC_CH_B_IBP_4, m_cbutBIBP[4]);
	DDX_Control(pDX, IDC_CH_B_IBP_5, m_cbutBIBP[5]);
	DDX_Control(pDX, IDC_CH_B_IBP_6, m_cbutBIBP[6]);
	DDX_Control(pDX, IDC_CH_B_IBP_7, m_cbutBIBP[7]);
	DDX_Control(pDX, IDC_CH_B_IBP_8, m_cbutBIBP[8]);
	DDX_Control(pDX, IDC_CH_B_IBP_9, m_cbutBIBP[9]);
	DDX_Control(pDX, IDC_CH_B_IBP_10, m_cbutBIBP[10]);
	DDX_Control(pDX, IDC_CH_B_IBP_11, m_cbutBIBP[11]);
	DDX_Control(pDX, IDC_CH_B_IBP_12, m_cbutBIBP[12]);
	DDX_Control(pDX, IDC_CH_B_IBP_13, m_cbutBIBP[13]);
	DDX_Control(pDX, IDC_CH_B_IBP_14, m_cbutBIBP[14]);
	DDX_Control(pDX, IDC_CH_B_IBP_15, m_cbutBIBP[15]);
	DDX_Control(pDX, IDC_CH_B_IBP_A, m_cbutBIBP[16]);

	DDX_Check(pDX, IDC_CH_B_IBP_0, m_chBIBP[0]);
	DDX_Check(pDX, IDC_CH_B_IBP_1, m_chBIBP[1]);
	DDX_Check(pDX, IDC_CH_B_IBP_2, m_chBIBP[2]);
	DDX_Check(pDX, IDC_CH_B_IBP_3, m_chBIBP[3]);
	DDX_Check(pDX, IDC_CH_B_IBP_4, m_chBIBP[4]);
	DDX_Check(pDX, IDC_CH_B_IBP_5, m_chBIBP[5]);
	DDX_Check(pDX, IDC_CH_B_IBP_6, m_chBIBP[6]);
	DDX_Check(pDX, IDC_CH_B_IBP_7, m_chBIBP[7]);
	DDX_Check(pDX, IDC_CH_B_IBP_8, m_chBIBP[8]);
	DDX_Check(pDX, IDC_CH_B_IBP_9, m_chBIBP[9]);
	DDX_Check(pDX, IDC_CH_B_IBP_10, m_chBIBP[10]);
	DDX_Check(pDX, IDC_CH_B_IBP_11, m_chBIBP[11]);
	DDX_Check(pDX, IDC_CH_B_IBP_12, m_chBIBP[12]);
	DDX_Check(pDX, IDC_CH_B_IBP_13, m_chBIBP[13]);
	DDX_Check(pDX, IDC_CH_B_IBP_14, m_chBIBP[14]);
	DDX_Check(pDX, IDC_CH_B_IBP_15, m_chBIBP[15]);
	DDX_Check(pDX, IDC_CH_B_IBP_A, m_chBIBP[16]);

	DDX_Text(pDX, IDC_ED_SZ_GOP_0, m_edSzGOP[0]);
	DDX_Text(pDX, IDC_ED_SZ_GOP_1, m_edSzGOP[1]);
	DDX_Text(pDX, IDC_ED_SZ_GOP_2, m_edSzGOP[2]);
	DDX_Text(pDX, IDC_ED_SZ_GOP_3, m_edSzGOP[3]);
	DDX_Text(pDX, IDC_ED_SZ_GOP_4, m_edSzGOP[4]);
	DDX_Text(pDX, IDC_ED_SZ_GOP_5, m_edSzGOP[5]);
	DDX_Text(pDX, IDC_ED_SZ_GOP_6, m_edSzGOP[6]);
	DDX_Text(pDX, IDC_ED_SZ_GOP_7, m_edSzGOP[7]);
	DDX_Text(pDX, IDC_ED_SZ_GOP_8, m_edSzGOP[8]);
	DDX_Text(pDX, IDC_ED_SZ_GOP_9, m_edSzGOP[9]);
	DDX_Text(pDX, IDC_ED_SZ_GOP_10, m_edSzGOP[10]);
	DDX_Text(pDX, IDC_ED_SZ_GOP_11, m_edSzGOP[11]);
	DDX_Text(pDX, IDC_ED_SZ_GOP_12, m_edSzGOP[12]);
	DDX_Text(pDX, IDC_ED_SZ_GOP_13, m_edSzGOP[13]);
	DDX_Text(pDX, IDC_ED_SZ_GOP_14, m_edSzGOP[14]);
	DDX_Text(pDX, IDC_ED_SZ_GOP_15, m_edSzGOP[15]);
	DDX_Text(pDX, IDC_ED_SZ_GOP_A, m_edSzGOP[16]);

	DDX_Check(pDX, IDC_CH_B_REC_0, m_chBRec[0]);
	DDX_Check(pDX, IDC_CH_B_REC_1, m_chBRec[1]);
	DDX_Check(pDX, IDC_CH_B_REC_2, m_chBRec[2]);
	DDX_Check(pDX, IDC_CH_B_REC_3, m_chBRec[3]);
	DDX_Check(pDX, IDC_CH_B_REC_4, m_chBRec[4]);
	DDX_Check(pDX, IDC_CH_B_REC_5, m_chBRec[5]);
	DDX_Check(pDX, IDC_CH_B_REC_6, m_chBRec[6]);
	DDX_Check(pDX, IDC_CH_B_REC_7, m_chBRec[7]);
	DDX_Check(pDX, IDC_CH_B_REC_8, m_chBRec[8]);
	DDX_Check(pDX, IDC_CH_B_REC_9, m_chBRec[9]);
	DDX_Check(pDX, IDC_CH_B_REC_10, m_chBRec[10]);
	DDX_Check(pDX, IDC_CH_B_REC_11, m_chBRec[11]);
	DDX_Check(pDX, IDC_CH_B_REC_12, m_chBRec[12]);
	DDX_Check(pDX, IDC_CH_B_REC_13, m_chBRec[13]);
	DDX_Check(pDX, IDC_CH_B_REC_14, m_chBRec[14]);
	DDX_Check(pDX, IDC_CH_B_REC_15, m_chBRec[15]);
	DDX_Check(pDX, IDC_CH_B_REC_A, m_chBRec[16]);

	DDX_Control(pDX, IDC_COM_BR_0, m_comBR[0]);
	DDX_Control(pDX, IDC_COM_BR_1, m_comBR[1]);
	DDX_Control(pDX, IDC_COM_BR_2, m_comBR[2]);
	DDX_Control(pDX, IDC_COM_BR_3, m_comBR[3]);
	DDX_Control(pDX, IDC_COM_BR_4, m_comBR[4]);
	DDX_Control(pDX, IDC_COM_BR_5, m_comBR[5]);
	DDX_Control(pDX, IDC_COM_BR_6, m_comBR[6]);
	DDX_Control(pDX, IDC_COM_BR_7, m_comBR[7]);
	DDX_Control(pDX, IDC_COM_BR_8, m_comBR[8]);
	DDX_Control(pDX, IDC_COM_BR_9, m_comBR[9]);
	DDX_Control(pDX, IDC_COM_BR_10, m_comBR[10]);
	DDX_Control(pDX, IDC_COM_BR_11, m_comBR[11]);
	DDX_Control(pDX, IDC_COM_BR_12, m_comBR[12]);
	DDX_Control(pDX, IDC_COM_BR_13, m_comBR[13]);
	DDX_Control(pDX, IDC_COM_BR_14, m_comBR[14]);
	DDX_Control(pDX, IDC_COM_BR_15, m_comBR[15]);
	DDX_Control(pDX, IDC_COM_BR_A, m_comBR[16]);

	DDX_Control(pDX, IDC_COM_CBR_BUF_0, m_comCBR_Buf[0]);
	DDX_Control(pDX, IDC_COM_CBR_BUF_1, m_comCBR_Buf[1]);
	DDX_Control(pDX, IDC_COM_CBR_BUF_2, m_comCBR_Buf[2]);
	DDX_Control(pDX, IDC_COM_CBR_BUF_3, m_comCBR_Buf[3]);
	DDX_Control(pDX, IDC_COM_CBR_BUF_4, m_comCBR_Buf[4]);
	DDX_Control(pDX, IDC_COM_CBR_BUF_5, m_comCBR_Buf[5]);
	DDX_Control(pDX, IDC_COM_CBR_BUF_6, m_comCBR_Buf[6]);
	DDX_Control(pDX, IDC_COM_CBR_BUF_7, m_comCBR_Buf[7]);
	DDX_Control(pDX, IDC_COM_CBR_BUF_8, m_comCBR_Buf[8]);
	DDX_Control(pDX, IDC_COM_CBR_BUF_9, m_comCBR_Buf[9]);
	DDX_Control(pDX, IDC_COM_CBR_BUF_10, m_comCBR_Buf[10]);
	DDX_Control(pDX, IDC_COM_CBR_BUF_11, m_comCBR_Buf[11]);
	DDX_Control(pDX, IDC_COM_CBR_BUF_12, m_comCBR_Buf[12]);
	DDX_Control(pDX, IDC_COM_CBR_BUF_13, m_comCBR_Buf[13]);
	DDX_Control(pDX, IDC_COM_CBR_BUF_14, m_comCBR_Buf[14]);
	DDX_Control(pDX, IDC_COM_CBR_BUF_15, m_comCBR_Buf[15]);
	DDX_Control(pDX, IDC_COM_CBR_BUF_A, m_comCBR_Buf[16]);
	// MPEG4[E]

	// JPEG[S]
	DDX_Check(pDX, IDC_CH_B_J_REC_0, m_chJPEG_BRec[0]);
	DDX_Check(pDX, IDC_CH_B_J_REC_1, m_chJPEG_BRec[1]);
	DDX_Check(pDX, IDC_CH_B_J_REC_2, m_chJPEG_BRec[2]);
	DDX_Check(pDX, IDC_CH_B_J_REC_3, m_chJPEG_BRec[3]);
	DDX_Check(pDX, IDC_CH_B_J_REC_4, m_chJPEG_BRec[4]);
	DDX_Check(pDX, IDC_CH_B_J_REC_5, m_chJPEG_BRec[5]);
	DDX_Check(pDX, IDC_CH_B_J_REC_6, m_chJPEG_BRec[6]);
	DDX_Check(pDX, IDC_CH_B_J_REC_7, m_chJPEG_BRec[7]);
	DDX_Check(pDX, IDC_CH_B_J_REC_8, m_chJPEG_BRec[8]);
	DDX_Check(pDX, IDC_CH_B_J_REC_9, m_chJPEG_BRec[9]);
	DDX_Check(pDX, IDC_CH_B_J_REC_10, m_chJPEG_BRec[10]);
	DDX_Check(pDX, IDC_CH_B_J_REC_11, m_chJPEG_BRec[11]);
	DDX_Check(pDX, IDC_CH_B_J_REC_12, m_chJPEG_BRec[12]);
	DDX_Check(pDX, IDC_CH_B_J_REC_13, m_chJPEG_BRec[13]);
	DDX_Check(pDX, IDC_CH_B_J_REC_14, m_chJPEG_BRec[14]);
	DDX_Check(pDX, IDC_CH_B_J_REC_15, m_chJPEG_BRec[15]);
	DDX_Check(pDX, IDC_CH_B_J_REC_A, m_chJPEG_BRec[16]);
	// JPEG[E]

	// G.723[S]
	DDX_Control(pDX, IDC_COM_G723_S_RATE, m_comG723SR);
	
	DDX_Check(pDX, IDC_CH_B_A_REC_0_1, m_chG723BRec[0]);
	DDX_Check(pDX, IDC_CH_B_A_REC_2_3, m_chG723BRec[1]);
	DDX_Check(pDX, IDC_CH_B_A_REC_4_5, m_chG723BRec[2]);
	DDX_Check(pDX, IDC_CH_B_A_REC_6_7, m_chG723BRec[3]);
	DDX_Check(pDX, IDC_CH_B_A_REC_8_9, m_chG723BRec[4]);
	DDX_Check(pDX, IDC_CH_B_A_REC_10_11, m_chG723BRec[5]);
	DDX_Check(pDX, IDC_CH_B_A_REC_12_13, m_chG723BRec[6]);
	DDX_Check(pDX, IDC_CH_B_A_REC_14_15, m_chG723BRec[7]);
	// G.723[E]
}


BEGIN_MESSAGE_MAP(CDlgSettingRecord, CDialog)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_0, OnChangeEdSzGOP)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_1, OnChangeEdSzGOP)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_2, OnChangeEdSzGOP)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_3, OnChangeEdSzGOP)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_4, OnChangeEdSzGOP)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_5, OnChangeEdSzGOP)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_6, OnChangeEdSzGOP)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_7, OnChangeEdSzGOP)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_8, OnChangeEdSzGOP)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_9, OnChangeEdSzGOP)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_10, OnChangeEdSzGOP)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_11, OnChangeEdSzGOP)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_12, OnChangeEdSzGOP)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_13, OnChangeEdSzGOP)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_14, OnChangeEdSzGOP)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_15, OnChangeEdSzGOP)
	
	//{{AFX_MSG_MAP(CDlgSettingRecord)
	ON_BN_CLICKED(IDC_CH_B_IBP_A, OnChBIbpA)
	ON_BN_CLICKED(IDC_CH_B_REC_A, OnChBRecA)
	ON_CBN_SELCHANGE(IDC_COM_BR_A, OnSelchangeComBrA)
	ON_CBN_SELCHANGE(IDC_COM_CBR_BUF_A, OnSelchangeComCbrBufA)
	ON_CBN_SELCHANGE(IDC_COM_FPS_A, OnSelchangeComFpsA)
	ON_CBN_SELCHANGE(IDC_COM_IMG_QUALITY_A, OnSelchangeComImgQualityA)
	ON_CBN_SELCHANGE(IDC_COM_SZ_IMG_A, OnSelchangeComSzImgA)
	ON_EN_CHANGE(IDC_ED_SZ_GOP_A, OnChangeEdSzGopA)
	ON_BN_CLICKED(IDC_RAD_REAL_CHANNEL, OnRadRealChannel)
	ON_BN_CLICKED(IDC_RAD_VIRT_CHANNEL, OnRadVirtChannel)
	ON_BN_CLICKED(IDC_CH_B_J_REC_A, OnChBJRecA)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSettingRecord message handlers

void CDlgSettingRecord::OnOK() 
{
	// TODO: Add extra validation here
	
	if (GetMP4RecInfoFromDlgItem () == FALSE)
	{
		AfxMessageBox ("MPEG4 Setting ERROR: FPS Invalid");
		return;
	}
	if (GetJPEGRecInfoFromDlgItem () == FALSE)
	{
		AfxMessageBox ("JPEG Setting ERROR: Same channel MPEG4 must be set.");
		return;
	}
	GetG723DlgItem ();

	if (glPCurSelS6010->IsValidInfoRec (&m_tmpSetRec) == FALSE)
	{
		AfxMessageBox ("ERROR: Record Setting Exceed Solo6010 Limit");
		return;
	}

	memcpy (&glPCurGInfo->setRec, &m_tmpSetRec, sizeof(SETTING_REC));

	CDialog::OnOK();
}

BOOL CDlgSettingRecord::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	int i, j;
	CString strTmp;

	// MPEG4[S]
	for (i=0; i<NUM_VID_REAL_ENC_CH +1; i++)
	{
		for (j=MP4_IMG_QUALITY_BEST; j<=MP4_IMG_QUALITY_WORST; j++)
		{
			if (j == MP4_IMG_QUALITY_BEST)
			{
				strTmp.Format ("%d Best", MP4_IMG_QUALITY_BEST +1);
			}
			else if (j == MP4_IMG_QUALITY_WORST)
			{
				strTmp.Format ("%d Worst", MP4_IMG_QUALITY_WORST +1);
			}
			else
			{
				strTmp.Format ("%d", j +1);
			}
			m_comQuality[i].AddString (strTmp);
		}
		for (j=0; j<NUM_IMG_SZ; j++)
		{
			m_comSzImg[i].AddString (GL_BUF_IMG_SZ[glPCurGInfo->typeVideo][j]);
		}
		for (j=0; j<NUM_FPS; j++)
		{
			m_comFPS[i].AddString (GL_STR_FPS[glPCurGInfo->typeVideo][j]);
		}
		for (j=0; j<NUM_CBR_BR; j++)
		{
			m_comBR[i].AddString (STR_BITRATE_BUF[j]);
		}
		for (j=0; j<NUM_CBR_BUF_SZ; j++)
		{
			m_comCBR_Buf[i].AddString (STR_CBR_BUF_SZ_BUF[j]);
		}
	}

	m_chBRec[NUM_VID_REAL_ENC_CH] = 0;
	m_edSzGOP[NUM_VID_REAL_ENC_CH] = 0;
	// MPEG4[E]

	// JPEG[S]
	// JPEG[S]
	
	// G723[S]
	for (j=0; j<NUM_G723_SR; j++)
	{
		m_comG723SR.AddString (STR_G723_SR[j]);
	}
	// G723[E]


	((CButton *)GetDlgItem (IDC_RAD_REAL_CHANNEL))->SetCheck (TRUE);

	UpdateData (FALSE);

	SetMP4DlgItemFromVideoChMode ();
	SetJPEGDlgItemFromVideoChMode ();
	SetG723DlgItem ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSettingRecord::SetMP4DlgItemFromVideoChMode ()
{
	int i, bEnable;
	SETTING_MP4_REC *pSetMP4Rec;

	if (m_idxModeVideoCh == IDX_MODE_V_CH_REAL)
	{
		pSetMP4Rec = &m_tmpSetRec.setMP4[0];
		bEnable = TRUE;
	}
	else
	{
		pSetMP4Rec = &m_tmpSetRec.setMP4[NUM_VID_REAL_ENC_CH];
		bEnable = FALSE;
	}

	GetDlgItem (IDC_STA_IMG_SIZE)->EnableWindow (bEnable);
	GetDlgItem (IDC_STA_IBP)->EnableWindow (bEnable);
	for (i=0; i<NUM_VID_REAL_ENC_CH +1; i++)
	{
		m_comSzImg[i].EnableWindow (bEnable);
		m_cbutBIBP[i].EnableWindow (bEnable);
	}

	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		m_chBRec[i] = pSetMP4Rec->bSet;
		m_comSzImg[i].SetCurSel (pSetMP4Rec->idxImgSz);

		m_comFPS[i].SetCurSel (pSetMP4Rec->idxFPS);
		m_edSzGOP[i] = pSetMP4Rec->nKeyFrmInterval;
		m_comQuality[i].SetCurSel (pSetMP4Rec->nQuality);
		m_chBIBP[i] = pSetMP4Rec->bIBasedPred;

		if (pSetMP4Rec->bUseCBR == TRUE)
		{
			m_comBR[i].SetCurSel (GetBitrateComIdxFromRealVal (
				pSetMP4Rec->nBitrate));
		}
		else
		{
			m_comBR[i].SetCurSel (GetBitrateComIdxFromRealVal (0));
		}
		m_comCBR_Buf[i].SetCurSel (GetCBR_BufSzComIdxFromRealVal (
			pSetMP4Rec->nBitrate,
			pSetMP4Rec->szVirtualDecBuf));

		pSetMP4Rec++;
	}

	UpdateData (FALSE);
}

BOOL CDlgSettingRecord::GetMP4RecInfoFromDlgItem ()
{
	UpdateData (TRUE);

	int i, nRet;
	CString strTmp;

	SETTING_MP4_REC *pSetMP4Rec;

	nRet = TRUE;
	if (m_idxModeVideoCh == IDX_MODE_V_CH_REAL)
	{
		pSetMP4Rec = &m_tmpSetRec.setMP4[0];
	}
	else
	{
		pSetMP4Rec = &m_tmpSetRec.setMP4[NUM_VID_REAL_ENC_CH];
	}

	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		pSetMP4Rec->bSet = m_chBRec[i];
		pSetMP4Rec->idxImgSz = m_comSzImg[i].GetCurSel ();
		pSetMP4Rec->idxFPS = m_comFPS[i].GetCurSel ();
		pSetMP4Rec->nKeyFrmInterval = m_edSzGOP[i];
		pSetMP4Rec->nQuality = m_comQuality[i].GetCurSel ();
		pSetMP4Rec->bIBasedPred = m_chBIBP[i];
		if (m_comBR[i].GetCurSel () == IDX_CBR_BR_VBR)
		{
			pSetMP4Rec->bUseCBR = 0;
			pSetMP4Rec->nBitrate = 0;
			pSetMP4Rec->szVirtualDecBuf = 0;
		}
		else
		{
			pSetMP4Rec->bUseCBR = 1;
			pSetMP4Rec->nBitrate = 
				BUF_BITRATE_COM_IDX_TO_REAL_VAL[m_comBR[i].GetCurSel ()];
			pSetMP4Rec->szVirtualDecBuf =
				BUF_CBR_BUF_SZ_COM_IDX_TO_REAL_VAL[m_comCBR_Buf[i].GetCurSel ()]
				*pSetMP4Rec->nBitrate;
		}

		if (pSetMP4Rec->idxImgSz == IMG_SZ_704X480_704X576 &&
			pSetMP4Rec->idxFPS == FPS_60)
		{
			nRet = FALSE;
		}
		if (pSetMP4Rec->idxImgSz != IMG_SZ_704X480_704X576 &&
			pSetMP4Rec->idxFPS == FPS_0_1)
		{
			nRet = FALSE;
		}

		pSetMP4Rec++;
	}

	if (m_idxModeVideoCh == IDX_MODE_V_CH_REAL)
	{
		for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
		{
			// Virtual Channel Set[S]...
			m_tmpSetRec.setMP4[i +NUM_VID_REAL_ENC_CH].idxImgSz = 
				m_tmpSetRec.setMP4[i].idxImgSz;
			m_tmpSetRec.setMP4[i +NUM_VID_REAL_ENC_CH].bIBasedPred = 
				m_tmpSetRec.setMP4[i].bIBasedPred;
			m_tmpSetRec.setMP4[i +NUM_VID_REAL_ENC_CH].nMotionTh = 
				m_tmpSetRec.setMP4[i].nMotionTh;
			// Virtual Channel Set[E]...
		}
	}
	return nRet;
}

void CDlgSettingRecord::SetG723DlgItem ()
{
	m_comG723SR.SetCurSel (m_tmpSetRec.setG723.idxSampRate);

	int i;
	for (i=0; i<NUM_G723_CH_PAIR; i++)
	{
		m_chG723BRec[i] = m_tmpSetRec.setG723.bufBEnc[2 *i];
	}

	UpdateData (FALSE);
}

BOOL CDlgSettingRecord::GetG723DlgItem ()
{
	UpdateData (TRUE);

	m_tmpSetRec.setG723.idxSampRate= m_comG723SR.GetCurSel ();

	int i;
	for (i=0; i<NUM_G723_CH_PAIR; i++)
	{
		m_tmpSetRec.setG723.bufBEnc[2 *i +0] = m_chG723BRec[i];
		m_tmpSetRec.setG723.bufBEnc[2 *i +1] = m_chG723BRec[i];
	}
	return TRUE;
}

void CDlgSettingRecord::OnChBIbpA() 
{
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);

	int i;
	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		m_chBIBP[i] = m_chBIBP[NUM_VID_REAL_ENC_CH];
	}
	
	UpdateData (FALSE);
}

void CDlgSettingRecord::OnChBRecA() 
{
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);

	int i;
	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		m_chBRec[i] = m_chBRec[NUM_VID_REAL_ENC_CH];
	}
	
	UpdateData (FALSE);
}

void CDlgSettingRecord::OnSelchangeComBrA() 
{
	// TODO: Add your control notification handler code here
	
	int i, idxSel;

	idxSel = m_comBR[NUM_VID_REAL_ENC_CH].GetCurSel ();
	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		m_comBR[i].SetCurSel (idxSel);
	}
}

void CDlgSettingRecord::OnSelchangeComCbrBufA() 
{
	// TODO: Add your control notification handler code here
	
	int i, idxSel;

	idxSel = m_comCBR_Buf[NUM_VID_REAL_ENC_CH].GetCurSel ();
	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		m_comCBR_Buf[i].SetCurSel (idxSel);
	}
}

void CDlgSettingRecord::OnSelchangeComFpsA() 
{
	// TODO: Add your control notification handler code here
	
	int i, idxSel;

	idxSel = m_comFPS[NUM_VID_REAL_ENC_CH].GetCurSel ();
	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		m_comFPS[i].SetCurSel (idxSel);
	}
}

void CDlgSettingRecord::OnSelchangeComImgQualityA() 
{
	// TODO: Add your control notification handler code here
	
	int i, idxSel;

	idxSel = m_comQuality[NUM_VID_REAL_ENC_CH].GetCurSel ();
	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		m_comQuality[i].SetCurSel (idxSel);
	}
}

void CDlgSettingRecord::OnSelchangeComSzImgA() 
{
	// TODO: Add your control notification handler code here
	
	int i, idxSel;

	idxSel = m_comSzImg[NUM_VID_REAL_ENC_CH].GetCurSel ();
	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		m_comSzImg[i].SetCurSel (idxSel);
	}
}

void CDlgSettingRecord::OnChangeEdSzGopA() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);

	int i;

	if (m_edSzGOP[NUM_VID_REAL_ENC_CH] < 1)
	{
		m_edSzGOP[NUM_VID_REAL_ENC_CH] = 1;
	}
	if (m_edSzGOP[NUM_VID_REAL_ENC_CH] > 255)
	{
		m_edSzGOP[NUM_VID_REAL_ENC_CH] = 255;
	}
	
	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		m_edSzGOP[i] = m_edSzGOP[NUM_VID_REAL_ENC_CH];
	}
	
	UpdateData (FALSE);
}

void CDlgSettingRecord::OnRadRealChannel() 
{
	// TODO: Add your control notification handler code here
	
	if (m_idxModeVideoCh == IDX_MODE_V_CH_REAL)
	{
		return;
	}

	((CButton *)GetDlgItem (IDC_RAD_REAL_CHANNEL))->SetCheck (TRUE);
	((CButton *)GetDlgItem (IDC_RAD_VIRT_CHANNEL))->SetCheck (FALSE);

	GetMP4RecInfoFromDlgItem ();
	GetJPEGRecInfoFromDlgItem ();

	m_idxModeVideoCh = IDX_MODE_V_CH_REAL;

	SetMP4DlgItemFromVideoChMode ();
	SetJPEGDlgItemFromVideoChMode ();
}

void CDlgSettingRecord::OnRadVirtChannel() 
{
	// TODO: Add your control notification handler code here
	
	if (m_idxModeVideoCh == IDX_MODE_V_CH_VIRTUAL)
	{
		return;
	}

	((CButton *)GetDlgItem (IDC_RAD_REAL_CHANNEL))->SetCheck (FALSE);
	((CButton *)GetDlgItem (IDC_RAD_VIRT_CHANNEL))->SetCheck (TRUE);

	GetMP4RecInfoFromDlgItem ();
	GetJPEGRecInfoFromDlgItem ();

	m_idxModeVideoCh = IDX_MODE_V_CH_VIRTUAL;

	SetMP4DlgItemFromVideoChMode ();
	SetJPEGDlgItemFromVideoChMode ();
}

void CDlgSettingRecord::OnChangeEdSzGOP() 
{
	UpdateData (TRUE);

	int i;
	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		if (m_edSzGOP[i] < 1)
		{
			m_edSzGOP[i] = 1;
		}
		if (m_edSzGOP[i] > 255)
		{
			m_edSzGOP[i] = 255;
		}
	}
	
	UpdateData (FALSE);
}

void CDlgSettingRecord::SetJPEGDlgItemFromVideoChMode ()
{
	int i;
	SETTING_JPEG_REC *pSetJPEGRec;

	if (m_idxModeVideoCh == IDX_MODE_V_CH_REAL)
	{
		pSetJPEGRec = &m_tmpSetRec.setJPEG[0];
	}
	else
	{
		pSetJPEGRec = &m_tmpSetRec.setJPEG[NUM_VID_REAL_ENC_CH];
	}

	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		m_chJPEG_BRec[i] = pSetJPEGRec->bRec;

		pSetJPEGRec++;
	}

	UpdateData (FALSE);
}

BOOL CDlgSettingRecord::GetJPEGRecInfoFromDlgItem ()
{
	UpdateData (TRUE);

	int i, nRet;
	CString strTmp;

	SETTING_JPEG_REC *pSetJPEGRec;

	nRet = TRUE;
	if (m_idxModeVideoCh == IDX_MODE_V_CH_REAL)
	{
		pSetJPEGRec = &m_tmpSetRec.setJPEG[0];
	}
	else
	{
		pSetJPEGRec = &m_tmpSetRec.setJPEG[NUM_VID_REAL_ENC_CH];
	}

	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		pSetJPEGRec->bRec = m_chJPEG_BRec[i];

		if (pSetJPEGRec->bRec == TRUE && m_chBRec[i] == FALSE)
		{
			nRet = FALSE;
		}

		pSetJPEGRec++;
	}

	return nRet;
}


void CDlgSettingRecord::OnChBJRecA() 
{
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);

	int i;
	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		m_chJPEG_BRec[i] = m_chJPEG_BRec[NUM_VID_REAL_ENC_CH];
	}
	
	UpdateData (FALSE);
}
