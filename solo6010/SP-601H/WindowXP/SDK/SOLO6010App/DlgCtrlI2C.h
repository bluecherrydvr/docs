#if !defined(AFX_DLGCTRLI2C_H__D1B45E90_224E_4B2F_9AD5_AC174986BC76__INCLUDED_)
#define AFX_DLGCTRLI2C_H__D1B45E90_224E_4B2F_9AD5_AC174986BC76__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCtrlI2C.h : header file
//

#define NUM_I2C_IO_ITEM			4

enum
{
	IDX_I2C_DEV_TW2815_A,
	IDX_I2C_DEV_TW2815_B,
	IDX_I2C_DEV_TW2815_C,
	IDX_I2C_DEV_TW2815_D,
	IDX_I2C_DEV_SAA7128,
	IDX_I2C_DEV_CUSTOM
};

// Video In/Out[S]	// From "SOLO6010_DrvGlobal.h"
#define TW2815_A_IIC_ID			0x50
#define TW2815_A_IIC_CHANNEL	0
#define TW2815_B_IIC_ID			0x52
#define TW2815_B_IIC_CHANNEL	0
#define TW2815_C_IIC_ID			0x54
#define TW2815_C_IIC_CHANNEL	0
#define TW2815_D_IIC_ID			0x56
#define TW2815_D_IIC_CHANNEL	0

#define SAA7128_IIC_ID			0x8c
#define SAA7128_IIC_CHANNEL		1
// Video In/Out[E]	// From "SOLO6010_DrvGlobal.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlI2C dialog

class CDlgCtrlI2C : public CDialog
{
// Construction
public:
	CDlgCtrlI2C(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgCtrlI2C)
	enum { IDD = IDD_DLG_CTRL_I2C };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	CString	m_edBufSubAddr[NUM_I2C_IO_ITEM];
	CString	m_edBufData[NUM_I2C_IO_ITEM];

	CString m_edChannel;
	CString m_edSlaveAddr;

	int m_radIdxDevice;

	void ReadData (int idxItem);
	void WriteData (int idxItem);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCtrlI2C)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	afx_msg void OnRadDevice();
	//{{AFX_MSG(CDlgCtrlI2C)
	virtual BOOL OnInitDialog();
	afx_msg void OnButRead0();
	afx_msg void OnButWrite0();
	afx_msg void OnButRead1();
	afx_msg void OnButWrite1();
	afx_msg void OnButRead2();
	afx_msg void OnButWrite2();
	afx_msg void OnButRead3();
	afx_msg void OnButWrite3();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCTRLI2C_H__D1B45E90_224E_4B2F_9AD5_AC174986BC76__INCLUDED_)
