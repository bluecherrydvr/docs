// BoardSettingDlg.cpp :
//

#include "stdafx.h"
#include "DvrFCD.h"
#include "DvrFCDDlg.h"
#include "BoardSettingDlg.h"
#include <math.h>

#define PCI_TIMEOUT_MAX 20
#define PCI_TIMEOUT_MIN  1
#define FONT_CODE_MAX 65535
#define FONT_CODE_MIN   0
#define COLOR_MIN		   0
#define COLOR_MAX      255

// CBoardSettingDlg

IMPLEMENT_DYNAMIC(CBoardSettingDlg, CDialog)

CBoardSettingDlg::CBoardSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBoardSettingDlg::IDD, pParent)
{	
	m_iBtnVideoStandard = 0;
	m_iBoardIndex = 0;
	m_iPCITimeout = 10;	

	m_iFont_Start_Code = 0;
	m_iFont_End_Code = FONT_CODE_MAX;
	m_iFont_Y_Color = 255;
	m_iFont_U_Color = 128;
	m_iFont_V_Color = 128;	

	memset(&m_font_table, 0, sizeof(m_font_table));
	m_font_table.font_table_format = SDVR_FT_FORMAT_BDF;
	m_strFontTable = "";
}

CBoardSettingDlg::~CBoardSettingDlg()
{
}

void CBoardSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);	
	int i=0;
	for (i=0; i<MAX_BOARD_SUPPORT_VIDEO_STANDARD; i++)
	{
		DDX_Control(pDX, IDC_RADIO_STD_D1_PAL+i, m_BtnVideoStds[i]);
	}

	DDX_Text(pDX, IDC_EDIT_PCI_TIMEOUT, m_iPCITimeout);
	DDV_MinMaxInt(pDX, m_iPCITimeout, PCI_TIMEOUT_MIN, PCI_TIMEOUT_MAX);

	DDX_Text(pDX, IDC_EDIT_FONT_START_CODE, m_iFont_Start_Code);
	DDV_MinMaxInt(pDX, m_iFont_Start_Code, FONT_CODE_MIN, FONT_CODE_MAX);

	DDX_Text(pDX, IDC_EDIT_FONT_END_CODE, m_iFont_End_Code);
	DDV_MinMaxInt(pDX, m_iFont_End_Code, FONT_CODE_MIN, FONT_CODE_MAX);

	DDX_Text(pDX, IDC_EDIT_Y_COLOR, m_iFont_Y_Color);
	DDV_MinMaxInt(pDX, m_iFont_Y_Color, COLOR_MIN, COLOR_MAX);

	DDX_Text(pDX, IDC_EDIT_U_COLOR, m_iFont_U_Color);
	DDV_MinMaxInt(pDX, m_iFont_U_Color, COLOR_MIN, COLOR_MAX);

	DDX_Text(pDX, IDC_EDIT_V_COLOR, m_iFont_V_Color);
	DDV_MinMaxInt(pDX, m_iFont_V_Color, COLOR_MIN, COLOR_MAX);

	DDX_Control(pDX, IDC_CHECK_CUSTOM_FONT_TABLE, m_BtnEnableCustomFontTable);
}


BEGIN_MESSAGE_MAP(CBoardSettingDlg, CDialog)				
	ON_BN_CLICKED(ID_SET_BOARD, &CBoardSettingDlg::OnBnClickedSetBoard)
	ON_BN_CLICKED(IDC_CHECK_CUSTOM_FONT_TABLE, &CBoardSettingDlg::OnBnClickedCheckCustomFontTable)
END_MESSAGE_MAP()


// CBoardSettingDlg
void CBoardSettingDlg::ConfigInitBoardData()
{		
	int i=0;	
	for (i=0; i<MAX_BOARD_SUPPORT_VIDEO_STANDARD; i++)
	{
		if (m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[i])
		{
			m_BtnVideoStds[i].EnableWindow(TRUE);			
		}
		else
		{
			m_BtnVideoStds[i].EnableWindow(FALSE);
		}
	}	

	m_BtnEnableCustomFontTable.SetCheck(BST_UNCHECKED);
	GetDlgItem(IDC_EDIT_FONT_START_CODE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_FONT_END_CODE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_Y_COLOR)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_U_COLOR)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_V_COLOR)->EnableWindow(FALSE);	


	if ((m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[BOARD_VIDEO_STD_D1_NTSC]) ||
		(m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[BOARD_VIDEO_STD_CIF_NTSC]) ||
		(m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[BOARD_VIDEO_STD_2CIF_NTSC]) ||
		(m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[BOARD_VIDEO_STD_4CIF_NTSC]) ||
		(m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[BOARD_VIDEO_STD_QCIF_NTSC]))
	{
		m_BtnVideoStds[BOARD_VIDEO_STD_D1_NTSC].SetCheck(BST_CHECKED);		
	}
	else
	{
		for (i=0; i<MAX_BOARD_SUPPORT_VIDEO_STANDARD; i++)
		{

			if (m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[i])
			{
				if ((m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[BOARD_VIDEO_STD_D1_NTSC]) ||
					(m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[BOARD_VIDEO_STD_CIF_NTSC]) ||
					(m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[BOARD_VIDEO_STD_2CIF_NTSC]) ||
					(m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[BOARD_VIDEO_STD_4CIF_NTSC]) ||
					(m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[BOARD_VIDEO_STD_QCIF_NTSC]))
				{
					m_BtnVideoStds[i].SetCheck(BST_CHECKED);
					break;
				}
			}
		}
	}


/*	for (i=0; i<MAX_BOARD_SUPPORT_VIDEO_STANDARD; i++)
	{
	
		if (m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[i])
		{
			if ((m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[BOARD_VIDEO_STD_D1_NTSC]) ||
			   (m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[BOARD_VIDEO_STD_CIF_NTSC]) ||
			   (m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[BOARD_VIDEO_STD_2CIF_NTSC]) ||
			   (m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[BOARD_VIDEO_STD_4CIF_NTSC]) ||
			   (m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bSupportVideoStds[BOARD_VIDEO_STD_QCIF_NTSC]))
			{
				
		}
	}*/

}

void CBoardSettingDlg::OnBnClickedSetBoard()
{	
	UpdateData(TRUE);

	int iTemp = 0;
	int i=0;
	for (i=0; i<MAX_BOARD_SUPPORT_VIDEO_STANDARD; i++)
	{
		if (m_BtnVideoStds[i].GetCheck() == BST_CHECKED)
		{
			iTemp = pow(2.0, i);	
			break;
		}
	}

	m_iPCITimeout<PCI_TIMEOUT_MIN ? m_iPCITimeout=PCI_TIMEOUT_MIN : m_iPCITimeout=m_iPCITimeout;
	m_iPCITimeout>PCI_TIMEOUT_MAX ? m_iPCITimeout=PCI_TIMEOUT_MAX : m_iPCITimeout=m_iPCITimeout;

	m_iFont_Start_Code<FONT_CODE_MIN ? m_iFont_Start_Code=FONT_CODE_MIN : m_iFont_Start_Code=m_iFont_Start_Code;
	m_iFont_Start_Code>FONT_CODE_MAX ? m_iFont_Start_Code=FONT_CODE_MAX : m_iFont_Start_Code=m_iFont_Start_Code;
	
	m_iFont_End_Code<FONT_CODE_MIN ? m_iFont_End_Code=FONT_CODE_MIN : m_iFont_End_Code=m_iFont_End_Code;	
	m_iFont_End_Code>FONT_CODE_MAX ? m_iFont_End_Code=FONT_CODE_MAX : m_iFont_End_Code=m_iFont_End_Code;

	m_iFont_Y_Color<COLOR_MIN ? m_iFont_Y_Color=COLOR_MIN : m_iFont_Y_Color=m_iFont_Y_Color;
	m_iFont_Y_Color>COLOR_MAX ? m_iFont_Y_Color=COLOR_MAX : m_iFont_Y_Color=m_iFont_Y_Color;

	m_iFont_U_Color<COLOR_MIN ? m_iFont_U_Color=COLOR_MIN : m_iFont_U_Color=m_iFont_U_Color;
	m_iFont_U_Color>COLOR_MAX ? m_iFont_U_Color=COLOR_MAX : m_iFont_U_Color=m_iFont_U_Color;

	m_iFont_V_Color<COLOR_MIN ? m_iFont_V_Color=COLOR_MIN : m_iFont_V_Color=m_iFont_V_Color;	
	m_iFont_V_Color>COLOR_MAX ? m_iFont_V_Color=COLOR_MAX : m_iFont_V_Color=m_iFont_V_Color;		

	m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_VideoStandard = (sdvr_video_std_e)iTemp;
	m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_iTimeOut = m_iPCITimeout;	

	m_font_table.start_font_code = m_iFont_Start_Code;
	m_font_table.end_font_code = m_iFont_End_Code;
	m_font_table.color_y = m_iFont_Y_Color;
	m_font_table.color_u = m_iFont_U_Color;
	m_font_table.color_v = m_iFont_V_Color;
	m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_strFontTable = m_strFontTable;
	m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_font_table = m_font_table;	

	UpdateData(FALSE);

	OnOK();
}


BOOL CBoardSettingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ConfigInitBoardData();

	return TRUE;  // return TRUE unless you set the focus to a control	
}

void CBoardSettingDlg::OnBnClickedCheckCustomFontTable()
{
	bool bShowEdit = false;	
	
	if (m_BtnEnableCustomFontTable.GetCheck() == BST_CHECKED)
	{
		CString MyStr;
		CString strFilePath;
		char szFilePath[MAX_PATH];
		memset(&szFilePath, 0, sizeof(szFilePath));

		TCHAR szFilters[] =
			_T ("Font files (*.bdf)|*.bdf||");
		MyStr.Format("Select Font File(*.bdf) for upload.\n");
		CFileDialog dlg (TRUE, MyStr, NULL,
			OFN_HIDEREADONLY, szFilters);		

		if (dlg.DoModal () == IDOK)
		{
			m_strFontTable = dlg.GetPathName();
			m_font_table.font_table_id = 8;		// custom id = 8~15;	
			
			bShowEdit = true;
		}
		else
		{
			m_BtnEnableCustomFontTable.SetCheck(BST_UNCHECKED);
		}
	}	

	if (bShowEdit)
	{
		GetDlgItem(IDC_EDIT_FONT_START_CODE)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_FONT_END_CODE)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_Y_COLOR)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_U_COLOR)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_V_COLOR)->EnableWindow(TRUE);
	}
	else
	{
		m_strFontTable = "";
		m_font_table.font_table_id = 0;

		GetDlgItem(IDC_EDIT_FONT_START_CODE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_FONT_END_CODE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_Y_COLOR)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_U_COLOR)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_V_COLOR)->EnableWindow(FALSE);
	}
}
