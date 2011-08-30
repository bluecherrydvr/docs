// DlgCtrlGPIO.cpp : implementation file
//

#include "stdafx.h"
#include "solo6010app.h"
#include "DlgCtrlGPIO.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static char *GL_STR_GPIO_INT_MODE[NUM_GPIO_INT_CFG +1] = 
{
	"Rising Edge",
	"Falling Edge",
	"Both Edge",
	"Low Level",
	"Disable Interrupt"
};

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlGPIO dialog


CDlgCtrlGPIO::CDlgCtrlGPIO(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgCtrlGPIO::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCtrlGPIO)
	//}}AFX_DATA_INIT

	int i;
	for (i=0; i<NUM_GPIO_ALL_PORT; i++)
	{
		m_bufRadMode[i] = GPIO_MODE_IN_OR_INT;
		m_bufChOut[i] = FALSE;
	}
	for (i=0; i<NUM_GPIO_ALL_PORT; i++)
	{
		m_bufChIn[i] = FALSE;
	}
}


void CDlgCtrlGPIO::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCtrlGPIO)
	//}}AFX_DATA_MAP
	DDX_Radio(pDX, IDC_RAD_MODE_IN_0, m_bufRadMode[0]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_1, m_bufRadMode[1]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_2, m_bufRadMode[2]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_3, m_bufRadMode[3]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_4, m_bufRadMode[4]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_5, m_bufRadMode[5]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_6, m_bufRadMode[6]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_7, m_bufRadMode[7]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_8, m_bufRadMode[8]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_9, m_bufRadMode[9]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_10, m_bufRadMode[10]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_11, m_bufRadMode[11]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_12, m_bufRadMode[12]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_13, m_bufRadMode[13]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_14, m_bufRadMode[14]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_15, m_bufRadMode[15]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_16, m_bufRadMode[16]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_17, m_bufRadMode[17]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_18, m_bufRadMode[18]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_19, m_bufRadMode[19]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_20, m_bufRadMode[20]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_21, m_bufRadMode[21]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_22, m_bufRadMode[22]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_23, m_bufRadMode[23]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_24, m_bufRadMode[24]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_25, m_bufRadMode[25]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_26, m_bufRadMode[26]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_27, m_bufRadMode[27]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_28, m_bufRadMode[28]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_29, m_bufRadMode[29]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_30, m_bufRadMode[30]);
	DDX_Radio(pDX, IDC_RAD_MODE_IN_31, m_bufRadMode[31]);

	DDX_Check(pDX, IDC_CH_OUT_0, m_bufChOut[0]);
	DDX_Check(pDX, IDC_CH_OUT_1, m_bufChOut[1]);
	DDX_Check(pDX, IDC_CH_OUT_2, m_bufChOut[2]);
	DDX_Check(pDX, IDC_CH_OUT_3, m_bufChOut[3]);
	DDX_Check(pDX, IDC_CH_OUT_4, m_bufChOut[4]);
	DDX_Check(pDX, IDC_CH_OUT_5, m_bufChOut[5]);
	DDX_Check(pDX, IDC_CH_OUT_6, m_bufChOut[6]);
	DDX_Check(pDX, IDC_CH_OUT_7, m_bufChOut[7]);
	DDX_Check(pDX, IDC_CH_OUT_8, m_bufChOut[8]);
	DDX_Check(pDX, IDC_CH_OUT_9, m_bufChOut[9]);
	DDX_Check(pDX, IDC_CH_OUT_10, m_bufChOut[10]);
	DDX_Check(pDX, IDC_CH_OUT_11, m_bufChOut[11]);
	DDX_Check(pDX, IDC_CH_OUT_12, m_bufChOut[12]);
	DDX_Check(pDX, IDC_CH_OUT_13, m_bufChOut[13]);
	DDX_Check(pDX, IDC_CH_OUT_14, m_bufChOut[14]);
	DDX_Check(pDX, IDC_CH_OUT_15, m_bufChOut[15]);
	DDX_Check(pDX, IDC_CH_OUT_16, m_bufChOut[16]);
	DDX_Check(pDX, IDC_CH_OUT_17, m_bufChOut[17]);
	DDX_Check(pDX, IDC_CH_OUT_18, m_bufChOut[18]);
	DDX_Check(pDX, IDC_CH_OUT_19, m_bufChOut[19]);
	DDX_Check(pDX, IDC_CH_OUT_20, m_bufChOut[20]);
	DDX_Check(pDX, IDC_CH_OUT_21, m_bufChOut[21]);
	DDX_Check(pDX, IDC_CH_OUT_22, m_bufChOut[22]);
	DDX_Check(pDX, IDC_CH_OUT_23, m_bufChOut[23]);
	DDX_Check(pDX, IDC_CH_OUT_24, m_bufChOut[24]);
	DDX_Check(pDX, IDC_CH_OUT_25, m_bufChOut[25]);
	DDX_Check(pDX, IDC_CH_OUT_26, m_bufChOut[26]);
	DDX_Check(pDX, IDC_CH_OUT_27, m_bufChOut[27]);
	DDX_Check(pDX, IDC_CH_OUT_28, m_bufChOut[28]);
	DDX_Check(pDX, IDC_CH_OUT_29, m_bufChOut[29]);
	DDX_Check(pDX, IDC_CH_OUT_30, m_bufChOut[30]);
	DDX_Check(pDX, IDC_CH_OUT_31, m_bufChOut[31]);

	DDX_Check(pDX, IDC_CH_IN_0, m_bufChIn[0]);
	DDX_Check(pDX, IDC_CH_IN_1, m_bufChIn[1]);
	DDX_Check(pDX, IDC_CH_IN_2, m_bufChIn[2]);
	DDX_Check(pDX, IDC_CH_IN_3, m_bufChIn[3]);
	DDX_Check(pDX, IDC_CH_IN_4, m_bufChIn[4]);
	DDX_Check(pDX, IDC_CH_IN_5, m_bufChIn[5]);
	DDX_Check(pDX, IDC_CH_IN_6, m_bufChIn[6]);
	DDX_Check(pDX, IDC_CH_IN_7, m_bufChIn[7]);
	DDX_Check(pDX, IDC_CH_IN_8, m_bufChIn[8]);
	DDX_Check(pDX, IDC_CH_IN_9, m_bufChIn[9]);
	DDX_Check(pDX, IDC_CH_IN_10, m_bufChIn[10]);
	DDX_Check(pDX, IDC_CH_IN_11, m_bufChIn[11]);
	DDX_Check(pDX, IDC_CH_IN_12, m_bufChIn[12]);
	DDX_Check(pDX, IDC_CH_IN_13, m_bufChIn[13]);
	DDX_Check(pDX, IDC_CH_IN_14, m_bufChIn[14]);
	DDX_Check(pDX, IDC_CH_IN_15, m_bufChIn[15]);
	DDX_Check(pDX, IDC_CH_IN_16, m_bufChIn[16]);
	DDX_Check(pDX, IDC_CH_IN_17, m_bufChIn[17]);
	DDX_Check(pDX, IDC_CH_IN_18, m_bufChIn[18]);
	DDX_Check(pDX, IDC_CH_IN_19, m_bufChIn[19]);
	DDX_Check(pDX, IDC_CH_IN_20, m_bufChIn[20]);
	DDX_Check(pDX, IDC_CH_IN_21, m_bufChIn[21]);
	DDX_Check(pDX, IDC_CH_IN_22, m_bufChIn[22]);
	DDX_Check(pDX, IDC_CH_IN_23, m_bufChIn[23]);
	DDX_Check(pDX, IDC_CH_IN_24, m_bufChIn[24]);
	DDX_Check(pDX, IDC_CH_IN_25, m_bufChIn[25]);
	DDX_Check(pDX, IDC_CH_IN_26, m_bufChIn[26]);
	DDX_Check(pDX, IDC_CH_IN_27, m_bufChIn[27]);
	DDX_Check(pDX, IDC_CH_IN_28, m_bufChIn[28]);
	DDX_Check(pDX, IDC_CH_IN_29, m_bufChIn[29]);
	DDX_Check(pDX, IDC_CH_IN_30, m_bufChIn[30]);
	DDX_Check(pDX, IDC_CH_IN_31, m_bufChIn[31]);

	DDX_Control(pDX, IDC_COM_INT_MODE_0, m_bufComIntMode[0]);
	DDX_Control(pDX, IDC_COM_INT_MODE_1, m_bufComIntMode[1]);
	DDX_Control(pDX, IDC_COM_INT_MODE_2, m_bufComIntMode[2]);
	DDX_Control(pDX, IDC_COM_INT_MODE_3, m_bufComIntMode[3]);
	DDX_Control(pDX, IDC_COM_INT_MODE_4, m_bufComIntMode[4]);
	DDX_Control(pDX, IDC_COM_INT_MODE_5, m_bufComIntMode[5]);
	DDX_Control(pDX, IDC_COM_INT_MODE_6, m_bufComIntMode[6]);
	DDX_Control(pDX, IDC_COM_INT_MODE_7, m_bufComIntMode[7]);
	DDX_Control(pDX, IDC_COM_INT_MODE_8, m_bufComIntMode[8]);
	DDX_Control(pDX, IDC_COM_INT_MODE_9, m_bufComIntMode[9]);
	DDX_Control(pDX, IDC_COM_INT_MODE_10, m_bufComIntMode[10]);
	DDX_Control(pDX, IDC_COM_INT_MODE_11, m_bufComIntMode[11]);
	DDX_Control(pDX, IDC_COM_INT_MODE_12, m_bufComIntMode[12]);
	DDX_Control(pDX, IDC_COM_INT_MODE_13, m_bufComIntMode[13]);
	DDX_Control(pDX, IDC_COM_INT_MODE_14, m_bufComIntMode[14]);
	DDX_Control(pDX, IDC_COM_INT_MODE_15, m_bufComIntMode[15]);
	DDX_Control(pDX, IDC_COM_INT_MODE_16, m_bufComIntMode[16]);
	DDX_Control(pDX, IDC_COM_INT_MODE_17, m_bufComIntMode[17]);
	DDX_Control(pDX, IDC_COM_INT_MODE_18, m_bufComIntMode[18]);
	DDX_Control(pDX, IDC_COM_INT_MODE_19, m_bufComIntMode[19]);
	DDX_Control(pDX, IDC_COM_INT_MODE_20, m_bufComIntMode[20]);
	DDX_Control(pDX, IDC_COM_INT_MODE_21, m_bufComIntMode[21]);
	DDX_Control(pDX, IDC_COM_INT_MODE_22, m_bufComIntMode[22]);
	DDX_Control(pDX, IDC_COM_INT_MODE_23, m_bufComIntMode[23]);
	DDX_Control(pDX, IDC_COM_INT_MODE_24, m_bufComIntMode[24]);
	DDX_Control(pDX, IDC_COM_INT_MODE_25, m_bufComIntMode[25]);
	DDX_Control(pDX, IDC_COM_INT_MODE_26, m_bufComIntMode[26]);
	DDX_Control(pDX, IDC_COM_INT_MODE_27, m_bufComIntMode[27]);
	DDX_Control(pDX, IDC_COM_INT_MODE_28, m_bufComIntMode[28]);
	DDX_Control(pDX, IDC_COM_INT_MODE_29, m_bufComIntMode[29]);
	DDX_Control(pDX, IDC_COM_INT_MODE_30, m_bufComIntMode[30]);
	DDX_Control(pDX, IDC_COM_INT_MODE_31, m_bufComIntMode[31]);
}


BEGIN_MESSAGE_MAP(CDlgCtrlGPIO, CDialog)
	//{{AFX_MSG_MAP(CDlgCtrlGPIO)
	ON_BN_CLICKED(IDC_BUT_WRITE, OnButWrite)
	ON_BN_CLICKED(IDC_BUT_READ, OnButRead)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_RAD_MODE_IN_0, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_1, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_2, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_3, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_4, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_5, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_6, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_7, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_8, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_9, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_10, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_11, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_12, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_13, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_14, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_15, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_16, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_17, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_18, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_19, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_20, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_21, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_22, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_23, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_24, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_25, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_26, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_27, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_28, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_29, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_30, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_IN_31, OnModeChange)

	ON_BN_CLICKED(IDC_RAD_MODE_OUT_0, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_1, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_2, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_3, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_4, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_5, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_6, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_7, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_8, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_9, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_10, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_11, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_12, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_13, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_14, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_15, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_16, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_17, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_18, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_19, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_20, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_21, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_22, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_23, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_24, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_25, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_26, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_27, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_28, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_29, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_30, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_OUT_31, OnModeChange)

	ON_BN_CLICKED(IDC_RAD_MODE_I2C_0, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_I2C_1, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_I2C_2, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_I2C_3, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_I2C_4, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_I2C_5, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_I2C_6, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_I2C_7, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_I2C_8, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_I2C_9, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_I2C_10, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_I2C_11, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_I2C_12, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_I2C_13, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_I2C_14, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_I2C_15, OnModeChange)

	ON_BN_CLICKED(IDC_RAD_MODE_ETC_0, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_ETC_1, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_ETC_2, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_ETC_3, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_ETC_4, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_ETC_5, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_ETC_6, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_ETC_7, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_ETC_8, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_ETC_9, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_ETC_10, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_ETC_11, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_ETC_12, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_ETC_13, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_ETC_14, OnModeChange)
	ON_BN_CLICKED(IDC_RAD_MODE_ETC_15, OnModeChange)

	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_0, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_1, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_2, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_3, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_4, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_5, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_6, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_7, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_8, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_9, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_10, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_11, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_12, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_13, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_14, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_15, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_16, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_17, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_18, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_19, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_20, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_21, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_22, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_23, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_24, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_25, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_26, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_27, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_28, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_29, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_30, OnModeChange)
	ON_CBN_SELCHANGE(IDC_COM_INT_MODE_31, OnModeChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCtrlGPIO message handlers

BOOL CDlgCtrlGPIO::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	int i, j;
	for (i=0; i<NUM_GPIO_ALL_PORT; i++)
	{
		for (j=0; j<NUM_GPIO_INT_CFG; j++)
		{
			m_bufComIntMode[i].AddString (GL_STR_GPIO_INT_MODE[j]);
		}

		m_bufComIntMode[i].SetCurSel (glPCurGInfo->GPIO_bufIntCfg[i]);
	}

	for (i=0; i<NUM_GPIO_ALL_PORT; i++)
	{
		m_bufRadMode[i] = glPCurGInfo->GPIO_bufMode[i];

		m_bufChIn[i] = FALSE;
		m_bufChOut[i] = FALSE;
	}

	UpdateData (FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCtrlGPIO::OnButWrite() 
{
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);
	
	int i;
	unsigned long maskPort, nData;

	maskPort = 0;
	nData = 0;
	for (i=0; i<NUM_GPIO_ALL_PORT; i++)
	{
		if (m_bufRadMode[i] == GPIO_MODE_OUT)
		{
			maskPort |= GPIO_PORT(i);

			if (m_bufChOut[i] == TRUE)
			{
				nData |= GPIO_PORT(i);
			}
		}
	}
	glPCurSelS6010->GPIO_Write (maskPort, nData);
}

void CDlgCtrlGPIO::OnButRead() 
{
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);
	
	int i;
	unsigned long nData;

	nData = glPCurSelS6010->GPIO_Read ();
	for (i=0; i<NUM_GPIO_ALL_PORT; i++)
	{
		if (nData &(1 <<i))
		{
			m_bufChIn[i] = TRUE;
		}
		else
		{
			m_bufChIn[i] = FALSE;
		}
	}

	UpdateData (FALSE);
}

void CDlgCtrlGPIO::OnModeChange() 
{
	UpdateData (TRUE);

	int i, j;
	unsigned long maskPort, tmpMask1, tmpMask2, tmpMask3, tmpMask4;

	for (i=0; i<NUM_GPIO_MODE; i++)
	{
		maskPort = 0;
		for (j=0; j<NUM_GPIO_ALL_PORT; j++)
		{
			if (m_bufRadMode[j] == i)
			{
				maskPort |= GPIO_PORT(j);
			}
		}
		glPCurSelS6010->GPIO_SetMode (i, maskPort);
	}

	tmpMask1 = 0;
	tmpMask2 = 0;
	tmpMask3 = 0;
	tmpMask4 = 0;
	for (i=0; i<NUM_GPIO_ALL_PORT; i++)
	{
		if (m_bufRadMode[i] == GPIO_MODE_IN_OR_INT)
		{
			switch (m_bufComIntMode[i].GetCurSel ())
			{
			case GPIO_INT_CFG_RISING_EDGE:
				tmpMask1 |= GPIO_PORT(i);
				break;
			case GPIO_INT_CFG_FALLING_EDGE:
				tmpMask2 |= GPIO_PORT(i);
				break;
			case GPIO_INT_CFG_BOTH_EDGE:
				tmpMask3 |= GPIO_PORT(i);
				break;
			case GPIO_INT_CFG_LOW_LEVEL:
				tmpMask4 |= GPIO_PORT(i);
				break;
			}
		}
	}
	glPCurSelS6010->GPIO_EnableInterrupt (tmpMask1, GPIO_INT_CFG_RISING_EDGE);
	glPCurSelS6010->GPIO_EnableInterrupt (tmpMask2, GPIO_INT_CFG_FALLING_EDGE);
	glPCurSelS6010->GPIO_EnableInterrupt (tmpMask3, GPIO_INT_CFG_BOTH_EDGE);
	glPCurSelS6010->GPIO_EnableInterrupt (tmpMask4, GPIO_INT_CFG_LOW_LEVEL);

	glPCurSelS6010->GPIO_EnableInterrupt (~(tmpMask1 |tmpMask2 |tmpMask3 |tmpMask4), GPIO_INT_CFG_DISABLE_INTERRUPT);
}
