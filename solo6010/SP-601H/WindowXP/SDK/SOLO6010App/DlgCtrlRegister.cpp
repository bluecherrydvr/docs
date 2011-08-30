// DlgCtrlRegister.cpp : implementation file
//

#include "stdafx.h"
#include "solo6010app.h"
#include "DlgCtrlRegister.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlRegister dialog


CDlgCtrlRegister::CDlgCtrlRegister(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCtrlRegister::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCtrlRegister)
	m_edRegMapAddrFrom = _T("00000000");
	m_edRegMapAddrTo = _T("00000000");
	//}}AFX_DATA_INIT

	int i;
	for (i=0; i<NUM_REG_IO_ITEM; i++)
	{
		m_edBufAddr[i] = _T("00000000");
		m_edBufData[i] = _T("");
	}
}


void CDlgCtrlRegister::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCtrlRegister)
	DDX_Control(pDX, IDC_LI_REG_MAP, m_liRegMap);
	DDX_Text(pDX, IDC_ED_REG_MAP_ADDR_FROM, m_edRegMapAddrFrom);
	DDX_Text(pDX, IDC_ED_REG_MAP_ADDR_TO, m_edRegMapAddrTo);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_ED_ADDR_0, m_edBufAddr[0]);
	DDX_Text(pDX, IDC_ED_DATA_0, m_edBufData[0]);
	DDX_Text(pDX, IDC_ED_ADDR_1, m_edBufAddr[1]);
	DDX_Text(pDX, IDC_ED_DATA_1, m_edBufData[1]);
	DDX_Text(pDX, IDC_ED_ADDR_2, m_edBufAddr[2]);
	DDX_Text(pDX, IDC_ED_DATA_2, m_edBufData[2]);
	DDX_Text(pDX, IDC_ED_ADDR_3, m_edBufAddr[3]);
	DDX_Text(pDX, IDC_ED_DATA_3, m_edBufData[3]);
}


BEGIN_MESSAGE_MAP(CDlgCtrlRegister, CDialog)
	//{{AFX_MSG_MAP(CDlgCtrlRegister)
	ON_BN_CLICKED(IDC_BUT_READ_0, OnButRead0)
	ON_BN_CLICKED(IDC_BUT_WRITE_0, OnButWrite0)
	ON_BN_CLICKED(IDC_BUT_READ_1, OnButRead1)
	ON_BN_CLICKED(IDC_BUT_WRITE_1, OnButWrite1)
	ON_BN_CLICKED(IDC_BUT_READ_2, OnButRead2)
	ON_BN_CLICKED(IDC_BUT_WRITE_2, OnButWrite2)
	ON_BN_CLICKED(IDC_BUT_READ_3, OnButRead3)
	ON_BN_CLICKED(IDC_BUT_WRITE_3, OnButWrite3)
	ON_BN_CLICKED(IDC_BUT_READ_REG_MAP, OnButReadRegMap)
	ON_BN_CLICKED(IDC_CH_SET_ENTIRE_ADDR, OnChSetEntireAddr)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlRegister message handlers

BOOL CDlgCtrlRegister::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	m_fntRegMap.CreatePointFont (110, "Courier New");
	m_liRegMap.SetFont (&m_fntRegMap);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCtrlRegister::OnButRead0() 
{
	// TODO: Add your control notification handler code here
	
	CheckAddrAlign ();

	UpdateData (TRUE);

	DWORD nAddr, nData;

	nAddr = GetHexValueFromHexString (LPCTSTR(m_edBufAddr[0]));
	nData = glPCurSelS6010->REGISTER_Get (nAddr);
	m_edBufData[0].Format ("%08x", nData);

	UpdateData (FALSE);
}

void CDlgCtrlRegister::OnButWrite0() 
{
	// TODO: Add your control notification handler code here
	
	CheckAddrAlign ();

	UpdateData (TRUE);

	DWORD nAddr, nData;
	nAddr = GetHexValueFromHexString (LPCTSTR(m_edBufAddr[0]));
	nData = GetHexValueFromHexString (LPCTSTR(m_edBufData[0]));

	glPCurSelS6010->REGISTER_Set (nAddr, nData);
}

void CDlgCtrlRegister::OnButRead1() 
{
	// TODO: Add your control notification handler code here
	
	CheckAddrAlign ();

	UpdateData (TRUE);

	DWORD nAddr, nData;

	nAddr = GetHexValueFromHexString (LPCTSTR(m_edBufAddr[1]));
	nData = glPCurSelS6010->REGISTER_Get (nAddr);
	m_edBufData[1].Format ("%08x", nData);

	UpdateData (FALSE);
}

void CDlgCtrlRegister::OnButWrite1() 
{
	// TODO: Add your control notification handler code here
	
	CheckAddrAlign ();

	UpdateData (TRUE);

	DWORD nAddr, nData;
	nAddr = GetHexValueFromHexString (LPCTSTR(m_edBufAddr[1]));
	nData = GetHexValueFromHexString (LPCTSTR(m_edBufData[1]));

	glPCurSelS6010->REGISTER_Set (nAddr, nData);
}

void CDlgCtrlRegister::OnButRead2() 
{
	// TODO: Add your control notification handler code here
	
	CheckAddrAlign ();

	UpdateData (TRUE);

	DWORD nAddr, nData;

	nAddr = GetHexValueFromHexString (LPCTSTR(m_edBufAddr[2]));
	nData = glPCurSelS6010->REGISTER_Get (nAddr);
	m_edBufData[2].Format ("%08x", nData);

	UpdateData (FALSE);
}

void CDlgCtrlRegister::OnButWrite2() 
{
	// TODO: Add your control notification handler code here
	
	CheckAddrAlign ();

	UpdateData (TRUE);

	DWORD nAddr, nData;
	nAddr = GetHexValueFromHexString (LPCTSTR(m_edBufAddr[2]));
	nData = GetHexValueFromHexString (LPCTSTR(m_edBufData[2]));

	glPCurSelS6010->REGISTER_Set (nAddr, nData);
}

void CDlgCtrlRegister::OnButRead3() 
{
	// TODO: Add your control notification handler code here
	
	CheckAddrAlign ();

	UpdateData (TRUE);

	DWORD nAddr, nData;

	nAddr = GetHexValueFromHexString (LPCTSTR(m_edBufAddr[3]));
	nData = glPCurSelS6010->REGISTER_Get (nAddr);
	m_edBufData[3].Format ("%08x", nData);

	UpdateData (FALSE);
}

void CDlgCtrlRegister::OnButWrite3() 
{
	// TODO: Add your control notification handler code here
	
	CheckAddrAlign ();
	
	UpdateData (TRUE);

	DWORD nAddr, nData;
	nAddr = GetHexValueFromHexString (LPCTSTR(m_edBufAddr[3]));
	nData = GetHexValueFromHexString (LPCTSTR(m_edBufData[3]));

	glPCurSelS6010->REGISTER_Set (nAddr, nData);
}

void CDlgCtrlRegister::OnButReadRegMap() 
{
	// TODO: Add your control notification handler code here
	
	CheckAddrAlign ();

	DWORD i, addrFrom, addrTo, bufRegMap[ADDR_LAST_REG /4 +1];

	addrFrom = GetHexValueFromHexString (LPCTSTR(m_edRegMapAddrFrom));
	addrTo = GetHexValueFromHexString (LPCTSTR(m_edRegMapAddrTo));

	for (i=addrFrom; i<=addrTo; i+=4)
	{
		bufRegMap[i /4] = glPCurSelS6010->REGISTER_Get (i);
	}

	char strTmp[MAX_PATH];

	m_liRegMap.ResetContent ();

	for (i=addrFrom; i<=addrTo; i+=4)
	{
		sprintf (strTmp, "%08x : %08x", i, bufRegMap[i /4]);
		m_liRegMap.AddString (strTmp);
	}
}

void CDlgCtrlRegister::OnChSetEntireAddr() 
{
	// TODO: Add your control notification handler code here
	
	if (((CButton *)GetDlgItem (IDC_CH_SET_ENTIRE_ADDR))->GetCheck () == TRUE)
	{
		m_edRegMapAddrFrom.Format ("%08x", ADDR_FIRST_REG);
		m_edRegMapAddrTo.Format ("%08x", ADDR_LAST_REG);

		UpdateData (FALSE);
	}
}

void CDlgCtrlRegister::CheckAddrAlign ()
{
	UpdateData (TRUE);

	DWORD nAddr;
	int i;

	for (i=0; i<NUM_REG_IO_ITEM; i++)
	{
		nAddr = GetHexValueFromHexString (LPCTSTR(m_edBufAddr[i]));
		if (nAddr /4 *4 != nAddr)
		{
			m_edBufAddr[i].Format ("%08x", nAddr /4 *4);
		}
	}
	nAddr = GetHexValueFromHexString (LPCTSTR(m_edRegMapAddrFrom));
	if (nAddr /4 *4 != nAddr)
	{
		m_edRegMapAddrFrom.Format ("%08x", nAddr /4 *4);
	}
	nAddr = GetHexValueFromHexString (LPCTSTR(m_edRegMapAddrTo));
	if (nAddr /4 *4 != nAddr)
	{
		m_edRegMapAddrTo.Format ("%08x", nAddr /4 *4);
	}

	UpdateData (FALSE);
}

