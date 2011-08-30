// DlgCtrlUART.cpp : implementation file
//

#include "stdafx.h"
#include "solo6010app.h"
#include "DlgCtrlUART.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlUART dialog


CDlgCtrlUART::CDlgCtrlUART(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCtrlUART::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCtrlUART)
	m_edReadData = _T("");
	m_staSzRead = _T("");
	m_edWriteData = _T("");
	m_radBaudRate = IDX_UART_BAUD_RATE_9600;
	m_radDataSize = IDX_UART_DATA_SZ_8_BIT;
	m_radParity = IDX_UART_PARITY_NONE;
	m_radStopBit = IDX_UART_STOP_BIT_1;
	m_radPort = UART_PORT_1;
	//}}AFX_DATA_INIT

	int i;

	for (i=0; i<NUM_UART_PORT; i++)
	{
		m_bufBaudRate[i] = IDX_UART_BAUD_RATE_9600;
		m_bufDataSize[i] = IDX_UART_DATA_SZ_8_BIT;
		m_bufParity[i] = IDX_UART_PARITY_NONE;
		m_bufStopBit[i] = IDX_UART_STOP_BIT_1;

		glPCurSelS6010->UART_Setup (i, m_bufBaudRate[i], m_bufDataSize[i], m_bufStopBit[i], m_bufParity[i]);
	}

	m_idxOldPort = -1;
}


void CDlgCtrlUART::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCtrlUART)
	DDX_Text(pDX, IDC_ED_READ_DATA, m_edReadData);
	DDX_Text(pDX, IDC_STA_SZ_READ, m_staSzRead);
	DDX_Text(pDX, IDC_ED_WRITE_DATA, m_edWriteData);
	DDX_Radio(pDX, IDC_RAD_BAUDRATE_0, m_radBaudRate);
	DDX_Radio(pDX, IDC_RAD_DATA_SIZE_0, m_radDataSize);
	DDX_Radio(pDX, IDC_RAD_PARITY_0, m_radParity);
	DDX_Radio(pDX, IDC_RAD_STOP_BIT_0, m_radStopBit);
	DDX_Radio(pDX, IDC_RAD_PORT_0, m_radPort);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCtrlUART, CDialog)
	//{{AFX_MSG_MAP(CDlgCtrlUART)
	ON_BN_CLICKED(IDC_BUT_READ, OnButRead)
	ON_BN_CLICKED(IDC_BUT_SET_PROPERTY, OnButSetProperty)
	ON_BN_CLICKED(IDC_BUT_WRITE, OnButWrite)
	ON_BN_CLICKED(IDC_RAD_PORT_0, OnRadPort0)
	ON_BN_CLICKED(IDC_RAD_PORT_1, OnRadPort1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlUART message handlers

BOOL CDlgCtrlUART::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCtrlUART::OnButRead() 
{
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);

	char strTmp[SZ_UART_DATA_BUF +1];
	int szBufRead = SZ_UART_DATA_BUF;
	int szActualRead;

	glPCurSelS6010->UART_Read (m_idxOldPort, (BYTE *)strTmp, szBufRead, &szActualRead);

	strTmp[szActualRead] = 0;
	m_edReadData = strTmp;
	m_staSzRead.Format ("%d", szActualRead);

	UpdateData (FALSE);
}

void CDlgCtrlUART::OnButSetProperty() 
{
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);

	glPCurSelS6010->UART_Setup (m_idxOldPort, m_radBaudRate, m_radDataSize, m_radStopBit, m_radParity);
}

void CDlgCtrlUART::OnButWrite() 
{
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);

	char strTmp[SZ_UART_DATA_BUF +1];
	int szBufWrite = m_edWriteData.GetLength ();
	int szActualWrite;

	memcpy (strTmp, LPCTSTR(m_edWriteData), szBufWrite);

	glPCurSelS6010->UART_Write (m_idxOldPort, (BYTE *)strTmp, szBufWrite, &szActualWrite);
}

void CDlgCtrlUART::OnRadPort0() 
{
	// TODO: Add your control notification handler code here
	
	UpdateDlgItemFromCurPort ();
}

void CDlgCtrlUART::OnRadPort1() 
{
	// TODO: Add your control notification handler code here
	
	UpdateDlgItemFromCurPort ();
}

void CDlgCtrlUART::UpdateDlgItemFromCurPort ()
{
	UpdateData (TRUE);

	if (m_idxOldPort != -1)
	{
		m_bufBaudRate[m_idxOldPort] = m_radBaudRate;
		m_bufDataSize[m_idxOldPort] = m_radDataSize;
		m_bufParity[m_idxOldPort] = m_radParity;
		m_bufStopBit[m_idxOldPort] = m_radStopBit;
	}

	m_idxOldPort = m_radPort;

	m_edReadData = _T("");
	m_staSzRead = _T("");
	m_edWriteData = _T("");

	m_radBaudRate = m_bufBaudRate[m_radPort];
	m_radDataSize = m_bufDataSize[m_radPort];
	m_radParity = m_bufParity[m_radPort];
	m_radStopBit = m_bufStopBit[m_radPort];

	UpdateData (FALSE);
}
