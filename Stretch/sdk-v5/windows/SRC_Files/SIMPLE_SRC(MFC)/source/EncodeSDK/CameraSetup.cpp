// CameraSetup.cpp : implementation file
//

#include "stdafx.h"
#include "EncodeDvrFCD.h"
#include "CameraSetup.h"
#include "EncodeDvrFCDDlg.h"



// CCameraSetup dialog

IMPLEMENT_DYNAMIC(CCameraSetup, CDialog)

CCameraSetup::CCameraSetup(CWnd* pParent /*=NULL*/)
	: CDialog(CCameraSetup::IDD, pParent)
	,m_sFileFullPath(_T(""))
{
	//m_iFrame = 30 ;
	m_iAverbit = 2000 ;
	m_iMaxbit = 4000 ;
	m_iQuality = 50 ;
	m_iGOPSize = 15 ;
	m_chan_def.video_format_primary = SDVR_VIDEO_ENC_H264 ;
	m_chan_def.video_format_secondary = SDVR_VIDEO_ENC_H264 ;
	m_chan_def.audio_format = SDVR_AUDIO_ENC_G711 ;
	strFormat = "";
	m_iSubEn = 0 ;
	m_iCh = 0 ;	
}

CCameraSetup::~CCameraSetup()
{
}

void CCameraSetup::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FRAME_RATE, m_iFrame);
	DDX_Control(pDX, IDC_COMBO_BITRATE, m_cBitRateLocationComboBox);
	DDX_Text(pDX, IDC_STATIC_SAVE, m_sFileFullPath);
	DDX_Text(pDX, IDC_EDIT_AVERBIT, m_iAverbit);
	DDX_Text(pDX, IDC_EDIT_MAXBIT, m_iMaxbit);
	DDX_Text(pDX, IDC_EDIT_QUALITY, m_iQuality);
	DDX_Text(pDX, IDC_EDIT_GOP, m_iGOPSize);
	DDX_Text(pDX, IDC_STATIC_SAVE, m_sFileFullPath);
	DDX_Text(pDX, IDC_STATIC_ENCODE, strFormat);
}

BEGIN_MESSAGE_MAP(CCameraSetup, CDialog)

	ON_CBN_SELCHANGE(IDC_COMBO_BITRATE, &CCameraSetup::OnCbnSelchangeComboBitrate)
	ON_EN_CHANGE(IDC_EDIT_FRAME_RATE, &CCameraSetup::OnEnChangeEditFrameRate)
	ON_EN_CHANGE(IDC_EDIT_AVERBIT, &CCameraSetup::OnEnChangeEditAverbit)
	ON_EN_CHANGE(IDC_EDIT_MAXBIT, &CCameraSetup::OnEnChangeEditMaxbit)
	ON_EN_CHANGE(IDC_EDIT_QUALITY, &CCameraSetup::OnEnChangeEditQuality)
	ON_EN_CHANGE(IDC_EDIT_GOP, &CCameraSetup::OnEnChangeEditGop)
	ON_BN_CLICKED(IDOK, &CCameraSetup::OnBnClickedOk)	
	ON_STN_CLICKED(IDC_STATIC_SAVE, &CCameraSetup::OnStnClickedStaticSave)
	ON_EN_CHANGE(IDC_STATIC_ENCODE, &CCameraSetup::OnEnChangeEditFormat)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL CCameraSetup::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_iBitrate = 0;
	m_cBitRateLocationComboBox.AddString("VBR");
	m_cBitRateLocationComboBox.AddString("CBR");
	m_cBitRateLocationComboBox.AddString("CQ");
	
	m_cBitRateLocationComboBox.SetCurSel(m_iBitrate);
	GetDlgItem(IDC_EDIT_AVERBIT)->EnableWindow(TRUE);

	if (m_pMainDlg->m_pDVRSystem[m_pMainDlg->m_iEventOpenBoard]->m_bIsPAL)
	{
		m_iFrame = 25;		
	}
	else
	{
		m_iFrame = 30;
	}

	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);

	m_sFileFullPath = szCurDir ;

	if(m_iSubEn == 0)
	{
		if(m_pMainDlg->bCheckPriH264[m_iCh])
		{
			GetDlgItem(IDC_EDIT_QUALITY)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_MAXBIT)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_AVERBIT)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_GOP)->EnableWindow(TRUE);
			GetDlgItem(IDC_COMBO_BITRATE)->EnableWindow(TRUE);
			//strFormat = "Primary Format : H.264 Setting";
			char sz[100];
			sprintf(sz , "Channel %ld Primary Format : H.264 Setting",m_iCh+1);
			strFormat = sz ;
		}
		else if(m_pMainDlg->bCheckPriMJPEG[m_iCh])
		{
			GetDlgItem(IDC_EDIT_QUALITY)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_MAXBIT)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_AVERBIT)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_GOP)->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO_BITRATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_FRAME_RATE)->EnableWindow(FALSE);
			char sz[100];
			sprintf(sz , "Channel %ld Primary Format : MJPEG Setting",m_iCh+1);
			strFormat = sz ;
		}
	}
	else if(m_iSubEn == 1)
	{
		if(m_pMainDlg->bCheckSecH264[m_iCh])
		{
			GetDlgItem(IDC_EDIT_QUALITY)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_MAXBIT)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_AVERBIT)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_GOP)->EnableWindow(TRUE);
			GetDlgItem(IDC_COMBO_BITRATE)->EnableWindow(TRUE);
			char sz[100];
			sprintf(sz , "Channel %ld Secondary Format : H.264 Setting",m_iCh+1);
			strFormat = sz ;
		}
		else if(m_pMainDlg->bCheckSecMJPEG[m_iCh])
		{
			GetDlgItem(IDC_EDIT_QUALITY)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_MAXBIT)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_AVERBIT)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_GOP)->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO_BITRATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_FRAME_RATE)->EnableWindow(FALSE);
			char sz[100];
			sprintf(sz , "Channel %ld Secondary Format : MJPEG Setting",m_iCh+1);
			strFormat = sz ;
		}
	}
	

	UpdateData(false);

	return TRUE;
}

void CCameraSetup::OnCbnSelchangeComboBitrate()
{
	m_iBitrate = m_cBitRateLocationComboBox.GetCurSel();

	GetDlgItem(IDC_EDIT_QUALITY)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_MAXBIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_AVERBIT)->EnableWindow(FALSE);

	if(m_iBitrate == 0)//VBR
	{
		GetDlgItem(IDC_EDIT_AVERBIT)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_MAXBIT)->EnableWindow(TRUE);
	}
	else if(m_iBitrate == 1)//CBR
	{
		GetDlgItem(IDC_EDIT_AVERBIT)->EnableWindow(TRUE);		
	}
	else if(m_iBitrate == 2)//CQ
	{
		GetDlgItem(IDC_EDIT_MAXBIT)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_QUALITY)->EnableWindow(TRUE);
	}		
}

void CCameraSetup::OnEnChangeEditFrameRate()
{
	UpdateData(true);
	UpdateData(false);
}

void CCameraSetup::OnEnChangeEditAverbit()
{
	UpdateData(true);
	UpdateData(false);
}

void CCameraSetup::OnEnChangeEditMaxbit()
{
	UpdateData(true);
	UpdateData(false);
}

void CCameraSetup::OnEnChangeEditQuality()
{
	UpdateData(true);
	UpdateData(false);
}

void CCameraSetup::OnBnClickedOk()
{
	UpdateData(false);
	
	OnOK();
}

void CCameraSetup::OnEnChangeEditGop()
{
	
}

void CCameraSetup::OnStnClickedStaticSave()
{
	// TODO: Add your control notification handler code here
	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);

	CString MyStr;

	TCHAR szFilters[] = _T ("*.mov");
	MyStr.Format("Select a movie file to decode\n");

	CFileDialog dlg(TRUE, MyStr, szFilters,
						OFN_HIDEREADONLY, szFilters);

	if (dlg.DoModal () == IDOK)
	{
		m_sFileFullPath = dlg.GetPathName();
		//m_sFileFullPath.Replace(_T("\\"), _T("//"));

		UpdateData(FALSE);
	}
	
}

void CCameraSetup::OnEnChangeEditFormat()
{
	
}

HBRUSH CCameraSetup::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_ENCODE)
	{
		pDC-> SetTextColor(RGB(91, 74, 247)); 
	}
	return hbr;
}
