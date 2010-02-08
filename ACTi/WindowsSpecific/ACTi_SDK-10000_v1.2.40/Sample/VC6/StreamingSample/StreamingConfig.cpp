// StreamingConfig.cpp : implementation file
//

#include "stdafx.h"
#include "AdapterTester.h"
#include "StreamingConfig.h"


// CStreamingConfig dialog

IMPLEMENT_DYNAMIC(CStreamingConfig, CDialog)
CStreamingConfig::CStreamingConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CStreamingConfig::IDD, pParent)
	, m_IP(0)
	, m_ID(_T("Admin"))
	, m_PW(_T("123456"))
	, m_SP(6002)
	, m_CP(6001)
{
	BYTE* p = (BYTE*)&m_IP;
	p[0] = 1;
	p[1] = 2;
	p[2] = 168;
	p[3] = 192;
}

CStreamingConfig::~CStreamingConfig()
{
}

void CStreamingConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_IPADDRESS1, m_IP);
	DDX_Text(pDX, IDC_EDIT2, m_ID);
	DDX_Text(pDX, IDC_EDIT42, m_PW);
	DDX_Text(pDX, IDC_EDIT43, m_SP);
	DDX_Text(pDX, IDC_EDIT44, m_CP);
}


BEGIN_MESSAGE_MAP(CStreamingConfig, CDialog)
END_MESSAGE_MAP()


// CStreamingConfig message handlers
