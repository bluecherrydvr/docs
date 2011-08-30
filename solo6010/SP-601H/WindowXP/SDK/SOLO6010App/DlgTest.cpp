// DlgTest.cpp : implementation file
//

#include "stdafx.h"
#include "solo6010app.h"
#include "DlgTest.h"
#include "DlgSettingRecord.h"
#include "SOLO6010.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgTest dialog


CDlgTest::CDlgTest(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgTest::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgTest)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgTest::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgTest)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_COM_CHANNEL, m_comChannel);
	DDX_Control(pDX, IDC_COM_SZ_IMG, m_comSzImg);
	DDX_Check(pDX, IDC_CH_B_REC, m_chBRec);
	DDX_Control(pDX, IDC_COM_IMG_QUALITY, m_comQuality);
	DDX_Control(pDX, IDC_COM_FPS, m_comFPS);
	DDX_Text(pDX, IDC_ED_SZ_GOP, m_edSzGOP);
	DDX_Control(pDX, IDC_COM_BR, m_comBR);
	DDX_Control(pDX, IDC_COM_CBR_BUF, m_comCBR_Buf);
}


BEGIN_MESSAGE_MAP(CDlgTest, CDialog)
	//{{AFX_MSG_MAP(CDlgTest)
	ON_BN_CLICKED(IDC_BUT_CHANGE_REC_SETTING, OnButChangeRecSetting)
	ON_CBN_SELCHANGE(IDC_COM_CHANNEL, OnSelchangeComChannel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgTest message handlers

BOOL CDlgTest::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	int i;
	CString strTmp;

	// Change MPEG4 Setting Test[S]
	for (i=0; i<NUM_VID_REAL_ENC_CH; i++)
	{
		strTmp.Format ("%d", i +1);
		m_comChannel.AddString (strTmp);
	}
	for (i=0; i<NUM_IMG_SZ; i++)
	{
		m_comSzImg.AddString (GL_BUF_IMG_SZ[glPCurGInfo->typeVideo][i]);
	}
	for (i=MP4_IMG_QUALITY_BEST; i<=MP4_IMG_QUALITY_WORST; i++)
	{
		if (i == MP4_IMG_QUALITY_BEST)
		{
			strTmp.Format ("%d Best", MP4_IMG_QUALITY_BEST +1);
		}
		else if (i == MP4_IMG_QUALITY_WORST)
		{
			strTmp.Format ("%d Worst", MP4_IMG_QUALITY_WORST +1);
		}
		else
		{
			strTmp.Format ("%d", i +1);
		}
		m_comQuality.AddString (strTmp);
	}
	for (i=0; i<NUM_FPS; i++)
	{
		m_comFPS.AddString (GL_STR_FPS[glPCurGInfo->typeVideo][i]);
	}
	for (i=0; i<NUM_CBR_BR; i++)
	{
		m_comBR.AddString (STR_BITRATE_BUF[i]);
	}
	for (i=0; i<NUM_CBR_BUF_SZ; i++)
	{
		m_comCBR_Buf.AddString (STR_CBR_BUF_SZ_BUF[i]);
	}
	for (i=0; i<NUM_FPS; i++)
	{
		m_comFPS.AddString (GL_STR_FPS[glPCurGInfo->typeVideo][i]);
	}
	m_edSzGOP = 0;
	// Change MPEG4 Setting Test[E]

	UpdateData (FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgTest::OnButChangeRecSetting() 
{
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);

	CString strTmp;

	if ((m_setMP4Rec.idxImgSz == IMG_SZ_704X480_704X576 && m_comFPS.GetCurSel () == FPS_60) ||
		(m_setMP4Rec.idxImgSz != IMG_SZ_704X480_704X576 && m_comFPS.GetCurSel () == FPS_0_1))
	{
		AfxMessageBox ("Invalid FPS, Please check FPS.");
		return;
	}

	m_setMP4Rec.bSet = m_chBRec;
	m_setMP4Rec.idxImgSz = m_comSzImg.GetCurSel ();
	m_setMP4Rec.idxFPS = m_comFPS.GetCurSel ();
	m_setMP4Rec.nKeyFrmInterval = m_edSzGOP;
	m_setMP4Rec.nQuality = m_comQuality.GetCurSel ();
	if (m_comBR.GetCurSel () == IDX_CBR_BR_VBR)
	{
		m_setMP4Rec.bUseCBR = 0;
		m_setMP4Rec.nBitrate = 0;
		m_setMP4Rec.szVirtualDecBuf = 0;
	}
	else
	{
		m_setMP4Rec.bUseCBR = 1;
		m_setMP4Rec.nBitrate = BUF_BITRATE_COM_IDX_TO_REAL_VAL[m_comBR.GetCurSel ()];
		m_setMP4Rec.szVirtualDecBuf = BUF_CBR_BUF_SZ_COM_IDX_TO_REAL_VAL[m_comCBR_Buf.GetCurSel ()] *m_setMP4Rec.nBitrate;
	}

	glPCurSelS6010->REC_MP4_RealChChangePropOnRec (m_comChannel.GetCurSel (),
		m_setMP4Rec.bSet,
		m_setMP4Rec.idxImgSz,
		m_setMP4Rec.nKeyFrmInterval,
		m_setMP4Rec.nQuality,
		m_setMP4Rec.bIBasedPred,
		m_setMP4Rec.idxFPS,
		m_setMP4Rec.bUseCBR,
		m_setMP4Rec.nBitrate,
		m_setMP4Rec.szVirtualDecBuf);
}


void CDlgTest::OnSelchangeComChannel() 
{
	// TODO: Add your control notification handler code here
	
	glPCurSelS6010->REC_MP4_RealChGetProp (m_comChannel.GetCurSel (),
		m_setMP4Rec.bSet,
		m_setMP4Rec.idxImgSz,
		m_setMP4Rec.nKeyFrmInterval,
		m_setMP4Rec.nQuality,
		m_setMP4Rec.bIBasedPred,
		m_setMP4Rec.idxFPS,
		m_setMP4Rec.nMotionTh,
		m_setMP4Rec.bUseCBR,
		m_setMP4Rec.nBitrate,
		m_setMP4Rec.szVirtualDecBuf);	// for Getting Current Image Size

	m_chBRec = m_setMP4Rec.bSet;
	m_comSzImg.SetCurSel (m_setMP4Rec.idxImgSz);
	m_comQuality.SetCurSel (m_setMP4Rec.nQuality);
	m_comFPS.SetCurSel (m_setMP4Rec.idxFPS);
	m_edSzGOP = m_setMP4Rec.nKeyFrmInterval;
	if (m_setMP4Rec.bUseCBR == TRUE)
	{
		m_comBR.SetCurSel (GetBitrateComIdxFromRealVal (m_setMP4Rec.nBitrate));
	}
	else
	{
		m_comBR.SetCurSel (GetBitrateComIdxFromRealVal (0));
	}
	m_comCBR_Buf.SetCurSel (GetCBR_BufSzComIdxFromRealVal (m_setMP4Rec.nBitrate, m_setMP4Rec.szVirtualDecBuf));

	UpdateData (FALSE);
}
