// DlgCtrlI2C.cpp : implementation file
//

#include "stdafx.h"
#include "solo6010app.h"
#include "DlgCtrlI2C.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlI2C dialog


CDlgCtrlI2C::CDlgCtrlI2C(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCtrlI2C::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCtrlI2C)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	int i;
	for (i=0; i<NUM_I2C_IO_ITEM; i++)
	{
		m_edBufSubAddr[i] = _T("00");
		m_edBufData[i] = _T("");
	}
}


void CDlgCtrlI2C::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCtrlI2C)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	DDX_Radio (pDX, IDC_RAD_DEVICE_0, m_radIdxDevice);
	DDX_Text(pDX, IDC_ED_CHANNEL, m_edChannel);
	DDX_Text(pDX, IDC_ED_SLAVE_ADDR, m_edSlaveAddr);
	DDX_Text(pDX, IDC_ED_SUB_ADDR_0, m_edBufSubAddr[0]);
	DDX_Text(pDX, IDC_ED_SUB_ADDR_1, m_edBufSubAddr[1]);
	DDX_Text(pDX, IDC_ED_SUB_ADDR_2, m_edBufSubAddr[2]);
	DDX_Text(pDX, IDC_ED_SUB_ADDR_3, m_edBufSubAddr[3]);
	DDX_Text(pDX, IDC_ED_DATA_0, m_edBufData[0]);
	DDX_Text(pDX, IDC_ED_DATA_1, m_edBufData[1]);
	DDX_Text(pDX, IDC_ED_DATA_2, m_edBufData[2]);
	DDX_Text(pDX, IDC_ED_DATA_3, m_edBufData[3]);
}

BEGIN_MESSAGE_MAP(CDlgCtrlI2C, CDialog)
	ON_BN_CLICKED(IDC_RAD_DEVICE_0, OnRadDevice)
	ON_BN_CLICKED(IDC_RAD_DEVICE_1, OnRadDevice)
	ON_BN_CLICKED(IDC_RAD_DEVICE_2, OnRadDevice)
	ON_BN_CLICKED(IDC_RAD_DEVICE_3, OnRadDevice)
	ON_BN_CLICKED(IDC_RAD_DEVICE_4, OnRadDevice)
	ON_BN_CLICKED(IDC_RAD_DEVICE_5, OnRadDevice)
	//{{AFX_MSG_MAP(CDlgCtrlI2C)
	ON_BN_CLICKED(IDC_BUT_READ_0, OnButRead0)
	ON_BN_CLICKED(IDC_BUT_WRITE_0, OnButWrite0)
	ON_BN_CLICKED(IDC_BUT_READ_1, OnButRead1)
	ON_BN_CLICKED(IDC_BUT_WRITE_1, OnButWrite1)
	ON_BN_CLICKED(IDC_BUT_READ_2, OnButRead2)
	ON_BN_CLICKED(IDC_BUT_WRITE_2, OnButWrite2)
	ON_BN_CLICKED(IDC_BUT_READ_3, OnButRead3)
	ON_BN_CLICKED(IDC_BUT_WRITE_3, OnButWrite3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlI2C message handlers

BOOL CDlgCtrlI2C::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	m_radIdxDevice = IDX_I2C_DEV_TW2815_A;

	UpdateData (FALSE);

	OnRadDevice ();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCtrlI2C::OnButRead0() 
{
	// TODO: Add your control notification handler code here
	
	ReadData (0);
}

void CDlgCtrlI2C::OnButRead1() 
{
	// TODO: Add your control notification handler code here
	
	ReadData (1);
}

void CDlgCtrlI2C::OnButRead2() 
{
	// TODO: Add your control notification handler code here
	
	ReadData (2);
}

void CDlgCtrlI2C::OnButRead3() 
{
	// TODO: Add your control notification handler code here
	
	ReadData (3);
}

void CDlgCtrlI2C::OnButWrite0() 
{
	// TODO: Add your control notification handler code here
	
	WriteData (0);
}

void CDlgCtrlI2C::OnButWrite1() 
{
	// TODO: Add your control notification handler code here
	
	WriteData (1);
}

void CDlgCtrlI2C::OnButWrite2() 
{
	// TODO: Add your control notification handler code here
	
	WriteData (2);
}

void CDlgCtrlI2C::OnButWrite3() 
{
	// TODO: Add your control notification handler code here
	
	WriteData (3);
}

void CDlgCtrlI2C::ReadData (int idxItem)
{
	UpdateData (TRUE);

	DWORD nChannel, nSlaveAddr, nSubAddr, nData;

	nChannel = GetHexValueFromHexString (LPCTSTR(m_edChannel));
	nSlaveAddr = GetHexValueFromHexString (LPCTSTR(m_edSlaveAddr));
	nSubAddr = GetHexValueFromHexString (LPCTSTR(m_edBufSubAddr[idxItem]));
	nData = glPCurSelS6010->I2C_Read (nChannel, nSlaveAddr, nSubAddr);
	m_edBufData[idxItem].Format ("%02x", nData);

	UpdateData (FALSE);
}

void CDlgCtrlI2C::WriteData (int idxItem)
{
	UpdateData (TRUE);

	DWORD nChannel, nSlaveAddr, nSubAddr, nData;

	nChannel = GetHexValueFromHexString (LPCTSTR(m_edChannel));
	nSlaveAddr = GetHexValueFromHexString (LPCTSTR(m_edSlaveAddr));
	nSubAddr = GetHexValueFromHexString (LPCTSTR(m_edBufSubAddr[idxItem]));
	nData = GetHexValueFromHexString (LPCTSTR(m_edBufData[idxItem]));

	glPCurSelS6010->I2C_Write (nChannel, nSlaveAddr, nSubAddr, nData);
}

void CDlgCtrlI2C::OnRadDevice() 
{
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);

	if (m_radIdxDevice == IDX_I2C_DEV_CUSTOM)
	{
		GetDlgItem (IDC_ED_CHANNEL)->EnableWindow (TRUE);
		GetDlgItem (IDC_ED_SLAVE_ADDR)->EnableWindow (TRUE);
	}
	else
	{
		GetDlgItem (IDC_ED_CHANNEL)->EnableWindow (FALSE);
		GetDlgItem (IDC_ED_SLAVE_ADDR)->EnableWindow (FALSE);

		switch (m_radIdxDevice)
		{
		case IDX_I2C_DEV_TW2815_A:
			m_edChannel.Format ("%d", TW2815_A_IIC_CHANNEL);
			m_edSlaveAddr.Format ("%02x", TW2815_A_IIC_ID);
			break;
		case IDX_I2C_DEV_TW2815_B:
			m_edChannel.Format ("%d", TW2815_B_IIC_CHANNEL);
			m_edSlaveAddr.Format ("%02x", TW2815_B_IIC_ID);
			break;
		case IDX_I2C_DEV_TW2815_C:
			m_edChannel.Format ("%d", TW2815_C_IIC_CHANNEL);
			m_edSlaveAddr.Format ("%02x", TW2815_C_IIC_ID);
			break;
		case IDX_I2C_DEV_TW2815_D:
			m_edChannel.Format ("%d", TW2815_D_IIC_CHANNEL);
			m_edSlaveAddr.Format ("%02x", TW2815_D_IIC_ID);
			break;
		case IDX_I2C_DEV_SAA7128:
			m_edChannel.Format ("%d", SAA7128_IIC_CHANNEL);
			m_edSlaveAddr.Format ("%02x", SAA7128_IIC_ID);
			break;
		}

		UpdateData (FALSE);
	}
}
