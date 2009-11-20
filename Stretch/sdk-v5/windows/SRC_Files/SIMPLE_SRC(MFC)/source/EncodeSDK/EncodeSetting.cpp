// EncodeSetting.cpp : implementation file
//

#include "stdafx.h"
#include "EncodeDvrFCD.h"
#include "EncodeSetting.h"


// CEncodeSetting dialog

IMPLEMENT_DYNAMIC(CEncodeSetting, CDialog)

CEncodeSetting::CEncodeSetting(CWnd* pParent /*=NULL*/)
	: CDialog(CEncodeSetting::IDD, pParent)
{
	m_iVdoEncoderPri = 1 ;
	m_iAdoEncoder = 1 ;
	m_iPriH264 = 0 ;
	m_iSecH264 = 0 ;
	m_iAdoG711 = 0 ;
	m_chan_def.video_format_primary = SDVR_VIDEO_ENC_H264 ;
	m_chan_def.video_format_secondary = SDVR_VIDEO_ENC_H264 ;
	m_chan_def.audio_format = SDVR_AUDIO_ENC_G711 ;
	m_iEncodeCh = 0 ;
	
}

CEncodeSetting::~CEncodeSetting()
{
}

void CEncodeSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_PRIMARY_VDO_ENCODER, m_iVdoEncoderPri);
	DDX_Check(pDX, IDC_CHECK_ADO_FORMAT, m_iAdoEncoder);
	DDX_Radio(pDX, IDC_RADIO_PRI_H264, m_iPriH264);
	DDX_Radio(pDX, IDC_RADIO_SEC_H264, m_iSecH264);
	DDX_Radio(pDX, IDC_RADIO_G711, m_iAdoG711);
}
BEGIN_MESSAGE_MAP(CEncodeSetting, CDialog)
	ON_BN_CLICKED(IDC_CHECK_PRIMARY_VDO_ENCODER, &CEncodeSetting::OnBnClickedCheckPrimaryVdoEncoder)
	ON_BN_CLICKED(IDC_RADIO_PRI_H264, &CEncodeSetting::OnBnClickedRadioPriH264)
	ON_BN_CLICKED(IDC_RADIO_PRI_MJPEG, &CEncodeSetting::OnBnClickedRadioPriMjpeg)
	ON_BN_CLICKED(IDC_RADIO_PRI_MPEG4, &CEncodeSetting::OnBnClickedRadioPriMpeg4)
	ON_BN_CLICKED(IDC_RADIO_SEC_H264, &CEncodeSetting::OnBnClickedRadioSecH264)
	ON_BN_CLICKED(IDC_RADIO_SEC_MJPEG, &CEncodeSetting::OnBnClickedRadioSecMjpeg)
	ON_BN_CLICKED(IDC_RADIO_SEC_MPEG4, &CEncodeSetting::OnBnClickedRadioSecMpeg4)
	ON_BN_CLICKED(IDC_CHECK_SECONDARY_VDO_ENCODER, &CEncodeSetting::OnBnClickedCheckSecondaryVdoEncoder)
	ON_BN_CLICKED(IDC_CHECK_ADO_FORMAT, &CEncodeSetting::OnBnClickedCheckAdoFormat)
	ON_BN_CLICKED(IDC_RADIO_G711, &CEncodeSetting::OnBnClickedRadioG711)
	ON_BN_CLICKED(IDC_RADIO_G726, &CEncodeSetting::OnBnClickedRadioG726)
	ON_BN_CLICKED(IDOK, &CEncodeSetting::OnBnClickedOk)
	ON_WM_CTLCOLOR()
	ON_WM_FONTCHANGE()
END_MESSAGE_MAP()

BOOL CEncodeSetting::OnInitDialog()
{
	CDialog::OnInitDialog();

	//default
	m_bCheckPrimary[m_iEncodeCh] = true;
	m_bCheckSecondary[m_iEncodeCh] =false;
	m_bCheckPriH264[m_iEncodeCh] = true;
	m_bCheckPriMJPEG[m_iEncodeCh] = false;
	m_bCheckSecH264[m_iEncodeCh] = true;
	m_bCheckSecMJPEG[m_iEncodeCh] = false;
	

	UpdateData(false);
	
	return TRUE;
}

void CEncodeSetting::OnBnClickedCheckPrimaryVdoEncoder()
{
	UpdateData(true);
	m_bCheckPrimary[m_iEncodeCh] = false ;
	UpdateData(false);
}

void CEncodeSetting::OnBnClickedRadioPriH264()
{
	UpdateData(true);
	m_chan_def.video_format_primary = SDVR_VIDEO_ENC_H264 ;
	m_bCheckPriH264[m_iEncodeCh] = true;
	m_bCheckPriMJPEG[m_iEncodeCh] = false ;
	UpdateData(false);	
}

void CEncodeSetting::OnBnClickedRadioPriMjpeg()
{
	UpdateData(true);
	m_chan_def.video_format_primary = SDVR_VIDEO_ENC_JPEG ;
	m_bCheckPriH264[m_iEncodeCh] = false;
	m_bCheckPriMJPEG[m_iEncodeCh] = true ;
	UpdateData(false);
}

void CEncodeSetting::OnBnClickedRadioPriMpeg4()
{
	UpdateData(true);
	m_chan_def.video_format_primary = SDVR_VIDEO_ENC_MPEG4 ;
	UpdateData(false);
}

void CEncodeSetting::OnBnClickedRadioSecH264()
{
	UpdateData(true);
	m_chan_def.video_format_secondary = SDVR_VIDEO_ENC_H264 ;
	m_bCheckSecH264[m_iEncodeCh] = true;
	m_bCheckSecMJPEG[m_iEncodeCh] = false;
	UpdateData(false);
}

void CEncodeSetting::OnBnClickedRadioSecMjpeg()
{
	UpdateData(true);
	m_chan_def.video_format_secondary = SDVR_VIDEO_ENC_JPEG ;
	m_bCheckSecH264[m_iEncodeCh] = false;
	m_bCheckSecMJPEG[m_iEncodeCh] = true;
	UpdateData(false);
}

void CEncodeSetting::OnBnClickedRadioSecMpeg4()
{
	UpdateData(true);
	m_chan_def.video_format_secondary = SDVR_VIDEO_ENC_MPEG4 ;
	UpdateData(false);
}

void CEncodeSetting::OnBnClickedCheckSecondaryVdoEncoder()
{
	UpdateData(true);
	m_bCheckSecondary[m_iEncodeCh] = true ;
	GetDlgItem(IDC_RADIO_SEC_H264)->EnableWindow(TRUE);
	GetDlgItem(IDC_RADIO_SEC_MJPEG)->EnableWindow(TRUE);
	UpdateData(false);
}

void CEncodeSetting::OnBnClickedCheckAdoFormat()
{
	
}

void CEncodeSetting::OnBnClickedRadioG711()
{
	UpdateData(true);
	m_chan_def.audio_format = SDVR_AUDIO_ENC_G711 ;
	UpdateData(false);
}

void CEncodeSetting::OnBnClickedRadioG726()
{
	
}

void CEncodeSetting::OnBnClickedOk()
{
	UpdateData(false);

	OnOK();
}

HBRUSH CEncodeSetting::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if ((pWnd->GetDlgCtrlID() == IDC_CHECK_PRIMARY_VDO_ENCODER)||
		(pWnd->GetDlgCtrlID() == IDC_CHECK_SECONDARY_VDO_ENCODER))		
	{
		pDC-> SetTextColor(RGB(91, 74, 247));
	}
	
	return hbr;
}
