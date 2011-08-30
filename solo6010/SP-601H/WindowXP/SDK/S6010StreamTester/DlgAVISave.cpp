// DlgAVISave.cpp : implementation file
//

#include "stdafx.h"
#include "S6010StreamTester.h"
#include "DlgAVISave.h"

#include "DlgPlayerBase.h"
#include "InfoS6010StreamIdx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgAVISave dialog


CDlgAVISave::CDlgAVISave(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgAVISave::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgAVISave)
	m_radSelCh = -1;
	m_radOutMode = -1;
	//}}AFX_DATA_INIT
}


void CDlgAVISave::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAVISave)
	DDX_Radio(pDX, IDC_RAD_CH_R_0, m_radSelCh);
	DDX_Radio(pDX, IDC_RAD_OUT_RAW, m_radOutMode);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_RAD_CH_R_0, m_bufRadCh[0]);
	DDX_Control(pDX, IDC_RAD_CH_R_1, m_bufRadCh[1]);
	DDX_Control(pDX, IDC_RAD_CH_R_2, m_bufRadCh[2]);
	DDX_Control(pDX, IDC_RAD_CH_R_3, m_bufRadCh[3]);
	DDX_Control(pDX, IDC_RAD_CH_R_4, m_bufRadCh[4]);
	DDX_Control(pDX, IDC_RAD_CH_R_5, m_bufRadCh[5]);
	DDX_Control(pDX, IDC_RAD_CH_R_6, m_bufRadCh[6]);
	DDX_Control(pDX, IDC_RAD_CH_R_7, m_bufRadCh[7]);
	DDX_Control(pDX, IDC_RAD_CH_R_8, m_bufRadCh[8]);
	DDX_Control(pDX, IDC_RAD_CH_R_9, m_bufRadCh[9]);
	DDX_Control(pDX, IDC_RAD_CH_R_10, m_bufRadCh[10]);
	DDX_Control(pDX, IDC_RAD_CH_R_11, m_bufRadCh[11]);
	DDX_Control(pDX, IDC_RAD_CH_R_12, m_bufRadCh[12]);
	DDX_Control(pDX, IDC_RAD_CH_R_13, m_bufRadCh[13]);
	DDX_Control(pDX, IDC_RAD_CH_R_14, m_bufRadCh[14]);
	DDX_Control(pDX, IDC_RAD_CH_R_15, m_bufRadCh[15]);
	DDX_Control(pDX, IDC_RAD_CH_V_0, m_bufRadCh[16]);
	DDX_Control(pDX, IDC_RAD_CH_V_1, m_bufRadCh[17]);
	DDX_Control(pDX, IDC_RAD_CH_V_2, m_bufRadCh[18]);
	DDX_Control(pDX, IDC_RAD_CH_V_3, m_bufRadCh[19]);
	DDX_Control(pDX, IDC_RAD_CH_V_4, m_bufRadCh[20]);
	DDX_Control(pDX, IDC_RAD_CH_V_5, m_bufRadCh[21]);
	DDX_Control(pDX, IDC_RAD_CH_V_6, m_bufRadCh[22]);
	DDX_Control(pDX, IDC_RAD_CH_V_7, m_bufRadCh[23]);
	DDX_Control(pDX, IDC_RAD_CH_V_8, m_bufRadCh[24]);
	DDX_Control(pDX, IDC_RAD_CH_V_9, m_bufRadCh[25]);
	DDX_Control(pDX, IDC_RAD_CH_V_10, m_bufRadCh[26]);
	DDX_Control(pDX, IDC_RAD_CH_V_11, m_bufRadCh[27]);
	DDX_Control(pDX, IDC_RAD_CH_V_12, m_bufRadCh[28]);
	DDX_Control(pDX, IDC_RAD_CH_V_13, m_bufRadCh[29]);
	DDX_Control(pDX, IDC_RAD_CH_V_14, m_bufRadCh[30]);
	DDX_Control(pDX, IDC_RAD_CH_V_15, m_bufRadCh[31]);
}


BEGIN_MESSAGE_MAP(CDlgAVISave, CDialog)
	//{{AFX_MSG_MAP(CDlgAVISave)
	ON_BN_CLICKED(IDC_BUT_MAKE, OnButMake)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAVISave message handlers

void CDlgAVISave::OnButMake() 
{
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);

	if (m_radSelCh == -1)
	{
		return;
	}

	char strFilePath[MAX_PATH], strFileExt[MAX_PATH];

	sprintf (strFilePath, "SMP4_CH%d", m_radSelCh +1);

	switch (m_radOutMode)
	{
	case IDX_OUT_RAW:
		sprintf (strFileExt, "raw");
		break;
	case IDX_OUT_VOL_RAW:
		sprintf (strFileExt, "sm4");
		break;
	case IDX_OUT_AVI:
		sprintf (strFileExt, "avi");
		break;
	default:
		sprintf (strFileExt, "avi");
		break;
	}

	CFileDialog dlg (FALSE, strFileExt, strFilePath, OFN_HIDEREADONLY |OFN_OVERWRITEPROMPT,
		NULL, this);
	if (dlg.DoModal () != IDOK)
	{
		return;
	}

	switch (m_radOutMode)
	{
	case IDX_OUT_RAW:
		glPDlgPB->MakeRAWStream (dlg.GetPathName (), m_radSelCh, FALSE);
		break;
	case IDX_OUT_VOL_RAW:
		glPDlgPB->MakeRAWStream (dlg.GetPathName (), m_radSelCh, TRUE);
		break;
	case IDX_OUT_AVI:
	default:
		glPDlgPB->MakeAVIFile (dlg.GetPathName (), m_radSelCh);
		break;
	}
}

BOOL CDlgAVISave::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	int i;
	for (i=0; i<MAX_ENC_CHANNEL; i++)
	{
		if (glInfoStreamIdx->GetMaxFrameNum (i) > 0)
		{
			m_bufRadCh[i].EnableWindow (TRUE);
		}
		else
		{
			m_bufRadCh[i].EnableWindow (FALSE);
		}
	}

	m_radSelCh = -1;
	for (i=0; i<MAX_ENC_CHANNEL; i++)
	{
		if (glInfoStreamIdx->GetMaxFrameNum (i) > 0)
		{
			m_radSelCh = i;
			break;
		}
	}

	m_radOutMode = IDX_OUT_AVI;

	UpdateData (FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgAVISave::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
}
