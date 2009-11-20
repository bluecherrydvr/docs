// VideoEncoder.cpp : implementation file
//

#include "stdafx.h"
#include "EncodeDvrFCD.h"
#include "VideoEncoder.h"
#include "EncodeDvrFCDDlg.h"


// CVideoEncoder dialog

IMPLEMENT_DYNAMIC(CVideoEncoder, CDialog)

CVideoEncoder::CVideoEncoder(CWnd* pParent /*=NULL*/)
	: CDialog(CVideoEncoder::IDD, pParent)
{
	m_iGOP = 15 ;
	m_iAverBitRate = 2000 ;
	m_iMaxBitRate = 4000;
	m_iQuality = 50 ;
	strFormatEncoding = "";
	m_iSubEncoder = 0 ;
	m_iChEncoding = 0 ;
	m_iSelBitRate = 0 ;
}

CVideoEncoder::~CVideoEncoder()
{
}

void CVideoEncoder::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FRAME, m_iFrameRate);
	DDX_Text(pDX, IDC_EDIT_GOP_SIZE, m_iGOP);
	DDX_Text(pDX, IDC_EDIT_AVG_BITRATE, m_iAverBitRate);
	DDX_Text(pDX, IDC_EDIT_MAX_BITRATE, m_iMaxBitRate);
	DDX_Text(pDX, IDC_EDIT_QUALITY, m_iQuality);
	DDX_Control(pDX, IDC_COMBO_BITRATE_CTL, m_cBitRateComboBox);
	DDX_Text(pDX, IDC_STATIC_ENCODING, strFormatEncoding) ;
}


BEGIN_MESSAGE_MAP(CVideoEncoder, CDialog)
	ON_BN_CLICKED(IDC_ENCODER_PRIMARY, &CVideoEncoder::OnBnClickedEncoderPrimary)
	ON_BN_CLICKED(IDC_ENCODER_SECONDARY, &CVideoEncoder::OnBnClickedEncoderSecondary)
	ON_CBN_SELCHANGE(IDC_COMBO_DECIMATION, &CVideoEncoder::OnCbnSelchangeComboDecimation)
	ON_EN_CHANGE(IDC_EDIT_FRAME, &CVideoEncoder::OnEnChangeEditFrame)
	ON_EN_CHANGE(IDC_EDIT_GOP_SIZE, &CVideoEncoder::OnEnChangeEditGopSize)
	ON_CBN_SELCHANGE(IDC_COMBO_BITRATE_CTL, &CVideoEncoder::OnCbnSelchangeComboBitrateCtl)
	ON_EN_CHANGE(IDC_EDIT_AVG_BITRATE, &CVideoEncoder::OnEnChangeEditAvgBitrate)
	ON_EN_CHANGE(IDC_EDIT_MAX_BITRATE, &CVideoEncoder::OnEnChangeEditMaxBitrate)
	ON_EN_CHANGE(IDC_EDIT_QUALITY, &CVideoEncoder::OnEnChangeEditQuality)
	ON_EN_CHANGE(IDC_STATIC_ENCODING, &CVideoEncoder::OnEnChangeEditFormatEncode)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CVideoEncoder message handlers
BOOL CVideoEncoder::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_bVdoEncoderPri = true;
	m_cBitRateComboBox.AddString("VBR");
	m_cBitRateComboBox.AddString("CBR");
	m_cBitRateComboBox.AddString("CQ");	
	m_cBitRateComboBox.SetCurSel(m_iSelBitRate);

	if (m_pMainDlg->m_pDVRSystem[0]->m_bIsPAL)
	{
		m_iFrameRate = 25;		
	}
	else
	{
		m_iFrameRate = 30;
	}
	
	if(m_iSubEncoder == 0 )
	{
		if(m_pMainDlg->bCheckPriH264[m_iChEncoding])
		{
			GetDlgItem(IDC_EDIT_QUALITY)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_MAX_BITRATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_AVG_BITRATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_GOP_SIZE)->EnableWindow(TRUE);
			GetDlgItem(IDC_COMBO_BITRATE_CTL)->EnableWindow(TRUE);
			char sz[100];
			sprintf(sz , "Channel %ld Primary Format : H.264 Setting",m_iChEncoding+1);
			strFormatEncoding = sz ;
		}
		else if(m_pMainDlg->bCheckPriMJPEG[m_iChEncoding])
		{
			GetDlgItem(IDC_EDIT_QUALITY)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_MAX_BITRATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_AVG_BITRATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_GOP_SIZE)->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO_BITRATE_CTL)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_FRAME)->EnableWindow(FALSE);
			char sz[100];
			sprintf(sz , "Channel %ld Primary Format : MJPEG Setting",m_iChEncoding+1);
			strFormatEncoding = sz ;
		}
	}
	else if(m_iSubEncoder == 1 )
	{
		if(m_pMainDlg->bCheckSecH264[m_iChEncoding])
		{
			GetDlgItem(IDC_EDIT_QUALITY)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_MAX_BITRATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_AVG_BITRATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_GOP_SIZE)->EnableWindow(TRUE);
			GetDlgItem(IDC_COMBO_BITRATE_CTL)->EnableWindow(TRUE);
			char sz[100];
			sprintf(sz , "Channel %ld Secondary Format : H.264 Setting",m_iChEncoding+1);
			strFormatEncoding = sz ;
		}	
		else if(m_pMainDlg->bCheckSecMJPEG[m_iChEncoding])
		{
			GetDlgItem(IDC_EDIT_QUALITY)->EnableWindow(TRUE);
			GetDlgItem(IDC_EDIT_MAX_BITRATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_AVG_BITRATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_GOP_SIZE)->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO_BITRATE_CTL)->EnableWindow(FALSE);
			GetDlgItem(IDC_EDIT_FRAME)->EnableWindow(FALSE);
			char sz[100];
			sprintf(sz , "Channel %ld Secondary Format : MJPEG Setting",m_iChEncoding+1);
			strFormatEncoding = sz ;
		}
	}

	UpdateData(false);


	return TRUE;
}

void CVideoEncoder::OnBnClickedEncoderPrimary()
{
	UpdateData(true);
	m_bVdoEncoderPri = true;
	m_bVdoEncoderSec = false;
	UpdateData(false);
}

void CVideoEncoder::OnBnClickedEncoderSecondary()
{
	UpdateData(true);
	m_bVdoEncoderPri = false;
	m_bVdoEncoderSec = true ;
	
	UpdateData(false);
}

void CVideoEncoder::OnCbnSelchangeComboDecimation()
{
	UpdateData(true);
	UpdateData(false);
}

void CVideoEncoder::OnEnChangeEditFrame()
{
	UpdateData(true);
	UpdateData(false);
}

void CVideoEncoder::OnEnChangeEditGopSize()
{
	UpdateData(true);
	UpdateData(false);
}

void CVideoEncoder::OnCbnSelchangeComboBitrateCtl()
{
	UpdateData(true);
	m_iSelBitRate = m_cBitRateComboBox.GetCurSel();

	GetDlgItem(IDC_EDIT_QUALITY)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_MAX_BITRATE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_AVG_BITRATE)->EnableWindow(FALSE);

	if(m_iSelBitRate == 0)//VBR
	{
		GetDlgItem(IDC_EDIT_AVG_BITRATE)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_MAX_BITRATE)->EnableWindow(TRUE);
	}
	else if(m_iSelBitRate == 1)//CBR
	{
		GetDlgItem(IDC_EDIT_AVG_BITRATE)->EnableWindow(TRUE);		
	}
	else if(m_iSelBitRate == 2)//CQ
	{
		GetDlgItem(IDC_EDIT_MAX_BITRATE)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_QUALITY)->EnableWindow(TRUE);
	}	
	UpdateData(false);
}

void CVideoEncoder::OnEnChangeEditAvgBitrate()
{
	UpdateData(true);
	UpdateData(false);
}

void CVideoEncoder::OnEnChangeEditMaxBitrate()
{
	UpdateData(true);
	UpdateData(false);
}

void CVideoEncoder::OnEnChangeEditQuality()
{
	UpdateData(true);
	UpdateData(false);
}

void CVideoEncoder::OnEnChangeEditFormatEncode()
{

}

HBRUSH CVideoEncoder::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->GetDlgCtrlID() == IDC_STATIC_ENCODING)
	{
		pDC-> SetTextColor(RGB(91, 74, 247)); 
	}
	return hbr;
}
