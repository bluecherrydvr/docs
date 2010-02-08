// MulticastAddrDlg.cpp : implementation file
//

#include "stdafx.h"
#include "goclient.h"
#include "MulticastAddrDlg.h"
#include ".\multicastaddrdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMulticastAddrDlg dialog


CMulticastAddrDlg::CMulticastAddrDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMulticastAddrDlg::IDD, pParent)
	, m_radio(0)
    , DoAudio(FALSE)
    , m_Audio_rtp_port(_T(""))
{
	//{{AFX_DATA_INIT(CMulticastAddrDlg)
	m_rtp_port = _T("6000");
	m_Audio_rtp_port = _T("6002");
	//m_mcast_addr = _T("228.5.6.1");
	m_mcast_addr = _T("rtsp://");
	//}}AFX_DATA_INIT
}


void CMulticastAddrDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CMulticastAddrDlg)
    DDX_Text(pDX, IDC_ADDREDIT, m_rtp_port);
    DDX_Text(pDX, IDC_EDIT_MCAST, m_mcast_addr);
    DDX_Radio(pDX, IDC_RADIO1, m_radio);
    //}}AFX_DATA_MAP
    DDX_Check(pDX, IDC_CHECK1, DoAudio);
    DDX_Control(pDX, IDC_COMBO1, AudioSetting);
    DDX_Text(pDX, IDC_ADDREDIT2, m_Audio_rtp_port);
}


BEGIN_MESSAGE_MAP(CMulticastAddrDlg, CDialog)
	//{{AFX_MSG_MAP(CMulticastAddrDlg)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	//}}AFX_MSG_MAP
    ON_CBN_SELCHANGE(IDC_COMBO1, OnCbnSelchangeCombo1)
END_MESSAGE_MAP()
#define AUDIOFORMATNUM 2
NAME_ENUM_PAIR AudioFormatList[AUDIOFORMATNUM] = 
{
	{"8KHz PCM(Mono)", 1},
	{"48KHz PCM(Stero)", 2},
};
void CMulticastAddrDlg::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
	if(AudioFormatList[AudioSetting.GetCurSel()].enumValue == 1)
    {
		AudioSetting.SetCurSel(0);
        AudioIdx = 1;
    }
	else
    {
		AudioSetting.SetCurSel(1);
        AudioIdx = 2;
    }
}


BOOL CMulticastAddrDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_rtp_port = AfxGetApp()->GetProfileString("Setting","LOCAL_RTP_PORT","6000");
	m_Audio_rtp_port = AfxGetApp()->GetProfileString("Setting","LOCAL_AUDIORTP_PORT","6002");
    for(int i=0; i<AUDIOFORMATNUM; i++)
	{
		AudioSetting.AddString(AudioFormatList[i].name);
	}
	AudioSetting.SetCurSel(0);
    AudioIdx = 1;
	return TRUE;  // return TRUE  unless you set the focus to a control
}void CMulticastAddrDlg::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	GetDlgItem(IDC_ADDREDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_ADDREDIT2)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK1)->EnableWindow(FALSE);
	GetDlgItem(IDC_COMBO1)->EnableWindow(FALSE);
	m_mcast_addr = _T("rtsp://");
	UpdateData(FALSE);
}

void CMulticastAddrDlg::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	GetDlgItem(IDC_ADDREDIT)->EnableWindow(TRUE);
	GetDlgItem(IDC_ADDREDIT2)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK1)->EnableWindow(TRUE);
	GetDlgItem(IDC_COMBO1)->EnableWindow(TRUE);
	m_mcast_addr = _T("");
	UpdateData(FALSE);
}

void CMulticastAddrDlg::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	OnRadio2();
}
