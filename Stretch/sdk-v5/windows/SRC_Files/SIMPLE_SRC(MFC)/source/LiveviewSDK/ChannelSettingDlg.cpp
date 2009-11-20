// ChannelSettingDlg.cpp : 
//

#include "stdafx.h"
#include "DvrFCD.h"
#include "DvrFCDDlg.h"
#include "ChannelSettingDlg.h"
#include "sdvr_sdk.h"

#define FRAMERATE_NTSC_MAX 30
#define FRAMERATE_PAL_MAX 25
#define FRAMERATE_MIN   1
#define OSD_TRANSLUCENT_MAX 255
#define OSD_TRANSLUCENT_MIN   0

#define SLIDER_VIDEO_ADJUST_MIN 0
#define SLIDER_VIDEO_ADJUST_MAX 255

#define SLIDER_VIDEO_ADJUST_SHARPNESS_MIN 1
#define SLIDER_VIDEO_ADJUST_SHARPNESS_MAX 15


// CChannelSettingDlg 

IMPLEMENT_DYNAMIC(CChannelSettingDlg, CDialog)

CChannelSettingDlg::CChannelSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CChannelSettingDlg::IDD, pParent)
{
	int i=0;
	m_iBoardIndex = 0;
	m_iChannelIndex = 0;
	m_iFrameRate = 30;
	m_bIsModify = false;
	m_bMute = TRUE;
	
	for (i=0; i<SDVR_MAX_OSD; i++)
	{
		m_bBtnEnableOSD[i] = FALSE;
		m_iOSD_XPos[i] = 0;
		m_iOSD_YPos[i] = 0;
		m_szOSD_Text[i] = "";
		memset(&m_OSD_Config_ex[i], 0, sizeof(m_OSD_Config_ex[i]));		
		m_szOSD_Text[i] = "";		
	}	

	m_bBtnEnableOSD[0] = TRUE;

	m_pDirectDrawDisplay = NULL;
	m_iVdoWidth_Raw = 0;
	m_iVdoHeight_Raw = 0;	
}

CChannelSettingDlg::~CChannelSettingDlg()
{
}

void CChannelSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	int i=0;
	for (i=0; i<2; i++)
	{
		DDX_Control(pDX, IDC_RADIO_SLATERAL+i, m_BtnVpp[i]);
	}

	for (i=0; i<5; i++)
	{
		DDX_Control(pDX, IDC_RADIO_EQUAL+i, m_BtnRes[i]);
	}
	
	DDX_Text(pDX, IDC_EDIT_FRAMERATE, m_iFrameRate);
	DDV_MinMaxInt(pDX, m_iFrameRate, FRAMERATE_MIN, FRAMERATE_NTSC_MAX);	
	
	DDX_Check(pDX, IDC_CHECK_ENABLE_OSD1, m_bBtnEnableOSD[0]);
	DDX_Check(pDX, IDC_CHECK_ENABLE_OSD2, m_bBtnEnableOSD[1]);	
	DDX_Check(pDX, IDC_CHECK_MUTE, m_bMute);

	DDX_Control(pDX, IDC_EDIT_OSD1_TEXT, m_edit_OSDText[0]);
	DDX_Control(pDX, IDC_EDIT_OSD2_TEXT, m_edit_OSDText[1]);	

	DDX_Control(pDX, IDC_COMBO_OSD1_POS, m_Cbo_OSD_Pos[0]);
	DDX_Control(pDX, IDC_COMBO_OSD2_POS, m_Cbo_OSD_Pos[1]);	

	DDX_Control(pDX, IDC_COMBO_OSD1_TIMESTAMP, m_Cbo_OSD_TimeStamp[0]);
	DDX_Control(pDX, IDC_COMBO_OSD2_TIMESTAMP, m_Cbo_OSD_TimeStamp[1]);	

	DDX_Text(pDX, IDC_EDIT_OSD1_X_POS, m_iOSD_XPos[0]);
	DDV_MinMaxInt(pDX, m_iOSD_XPos[0], 0, MAX_VDO_WIDTH);
	DDX_Text(pDX, IDC_EDIT_OSD2_X_POS, m_iOSD_XPos[1]);
	DDV_MinMaxInt(pDX, m_iOSD_XPos[1], 0, MAX_VDO_WIDTH);

	DDX_Text(pDX, IDC_EDIT_OSD1_Y_POS, m_iOSD_YPos[0]);
	DDV_MinMaxInt(pDX, m_iOSD_YPos[0], 0, MAX_VDO_HEIGHT);
	DDX_Text(pDX, IDC_EDIT_OSD2_Y_POS, m_iOSD_YPos[1]);
	DDV_MinMaxInt(pDX, m_iOSD_YPos[1], 0, MAX_VDO_HEIGHT);

	DDX_Control(pDX, IDC_SLIDER_OSD1_TRANSLUCENT, m_Slider_OSD_Translucent[0]);
	DDX_Control(pDX, IDC_SLIDER_OSD2_TRANSLUCENT, m_Slider_OSD_Translucent[1]);	

	DDX_Control(pDX, IDC_EDIT_OSD1_X_POS, m_Edit_OSD_XPos[0]);
	DDX_Control(pDX, IDC_EDIT_OSD2_X_POS, m_Edit_OSD_XPos[1]);

	DDX_Control(pDX, IDC_EDIT_OSD1_Y_POS, m_Edit_OSD_YPos[0]);
	DDX_Control(pDX, IDC_EDIT_OSD2_Y_POS, m_Edit_OSD_YPos[1]);

	DDX_Control(pDX, IDC_SLIDER_BRIGHTNESS, m_Slider_Brightness);
	DDX_Control(pDX, IDC_SLIDER_CONTRAST, m_Slider_Contrast);
	DDX_Control(pDX, IDC_SLIDER_SATURATION, m_Slider_Saturation);
	DDX_Control(pDX, IDC_SLIDER_HUE, m_Slider_Hue);
	DDX_Control(pDX, IDC_SLIDER_SHARPNESS, m_Slider_Sharpness);
}


BEGIN_MESSAGE_MAP(CChannelSettingDlg, CDialog)
	ON_BN_CLICKED(ID_SET_CHANNEL, &CChannelSettingDlg::OnBnClickedSetChannel)			
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_OSD1_TRANSLUCENT, &CChannelSettingDlg::OnNMCustomdrawSliderOsd1Translucent)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_OSD2_TRANSLUCENT, &CChannelSettingDlg::OnNMCustomdrawSliderOsd2Translucent)	
	ON_CBN_SELCHANGE(IDC_COMBO_OSD1_POS, &CChannelSettingDlg::OnCbnSelchangeComboOsd1Pos)
	ON_CBN_SELCHANGE(IDC_COMBO_OSD2_POS, &CChannelSettingDlg::OnCbnSelchangeComboOsd2Pos)	
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_BRIGHTNESS, &CChannelSettingDlg::OnNMCustomdrawSliderBrightness)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_CONTRAST, &CChannelSettingDlg::OnNMCustomdrawSliderContrast)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_SATURATION, &CChannelSettingDlg::OnNMCustomdrawSliderSaturation)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_HUE, &CChannelSettingDlg::OnNMCustomdrawSliderHue)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_SHARPNESS, &CChannelSettingDlg::OnNMCustomdrawSliderSharpness)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CChannelSettingDlg

BOOL CChannelSettingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ConfigInitChannelData();	

	CString strTitle = "";
	strTitle.Format("Channel %d Setting.", m_iChannelIndex+1);
	SetWindowText(strTitle);

	if (!m_bIsModify)
	{
		GetDlgItem(IDC_STATIC_DISPLAY_RAW_SECONDARY)->SetWindowText("Raw video secondary not connected yet.");
	}
	else
	{		
		InitDrawDib();
	}

	return TRUE; 
}


void CChannelSettingDlg::ConfigInitChannelData()
{	
	CDVRSystem* pDVRSystem = m_pMainDlg->m_pDVRSystem[m_iBoardIndex];
	int i=0;
	for (i=0; i<SDVR_MAX_OSD; i++)
	{
		m_edit_OSDText[i].SetLimitText(SDVR_MAX_OSD_EX_TEXT);

		m_Cbo_OSD_Pos[i].AddString("Top-Left");
		m_Cbo_OSD_Pos[i].AddString("Bottom-Left");
		m_Cbo_OSD_Pos[i].AddString("Top-Right");		
		m_Cbo_OSD_Pos[i].AddString("Bottom-Right");		
		m_Cbo_OSD_Pos[i].AddString("Custom");

		m_Cbo_OSD_TimeStamp[i].AddString("None");
		m_Cbo_OSD_TimeStamp[i].AddString("Debug Information");
		m_Cbo_OSD_TimeStamp[i].AddString("MDY 12H");
		m_Cbo_OSD_TimeStamp[i].AddString("DMY 12H");
		m_Cbo_OSD_TimeStamp[i].AddString("YMD 12H");
		m_Cbo_OSD_TimeStamp[i].AddString("MDY 24H");
		m_Cbo_OSD_TimeStamp[i].AddString("DMY 24H");
		m_Cbo_OSD_TimeStamp[i].AddString("YMD 24H");
		m_Slider_OSD_Translucent[i].SetRange(OSD_TRANSLUCENT_MIN, OSD_TRANSLUCENT_MAX);		
	}	

	m_Slider_Brightness.SetRange(SLIDER_VIDEO_ADJUST_MIN, SLIDER_VIDEO_ADJUST_MAX);
	m_Slider_Contrast.SetRange(SLIDER_VIDEO_ADJUST_MIN, SLIDER_VIDEO_ADJUST_MAX);
	m_Slider_Saturation.SetRange(SLIDER_VIDEO_ADJUST_MIN, SLIDER_VIDEO_ADJUST_MAX);
	m_Slider_Hue.SetRange(SLIDER_VIDEO_ADJUST_MIN, SLIDER_VIDEO_ADJUST_MAX);
	m_Slider_Sharpness.SetRange(SLIDER_VIDEO_ADJUST_SHARPNESS_MIN, SLIDER_VIDEO_ADJUST_SHARPNESS_MAX);

	if (m_bIsModify)
	{		
		ReloadData(); // some control items parameters need reload from last setting.			
	}
	else
	{
		CreateDefaultData();		
	}

	CString strValue;

	if (m_pMainDlg->m_bIsConnectChannelOK[m_iBoardIndex][m_iChannelIndex])
	{
		m_pMainDlg->ConfigBrightness(m_iChannelIndex, TRUE);
		m_Slider_Brightness.SetPos(m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].brightness);
		strValue.Format("%d", m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].brightness);	
		GetDlgItem(IDC_STATIC_SHOW_BRIGHTNESS)->SetWindowText(strValue);	

		m_pMainDlg->ConfigContrast(m_iChannelIndex, TRUE);
		m_Slider_Contrast.SetPos(m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].contrast);
		strValue.Format("%d", m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].contrast);	
		GetDlgItem(IDC_STATIC_SHOW_CONTRAST)->SetWindowText(strValue);	

		m_pMainDlg->ConfigSaturation(m_iChannelIndex, TRUE);
		m_Slider_Saturation.SetPos(m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].saturation);
		strValue.Format("%d", m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].saturation);	
		GetDlgItem(IDC_STATIC_SHOW_SATURATION)->SetWindowText(strValue);	

		m_pMainDlg->ConfigHue(m_iChannelIndex, TRUE);	
		m_Slider_Hue.SetPos(m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].hue);
		strValue.Format("%d", m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].hue);	
		GetDlgItem(IDC_STATIC_SHOW_HUE)->SetWindowText(strValue);	

		m_pMainDlg->ConfigSharpness(m_iChannelIndex, TRUE);	
		m_Slider_Sharpness.SetPos(m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].sharpness);
		strValue.Format("%d", m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].sharpness);	
		GetDlgItem(IDC_STATIC_SHOW_SHARPNESS)->SetWindowText(strValue);
	}

	UpdateData(FALSE);
}


void CChannelSettingDlg::ReloadData()
{
	CDVRSystem* pDVRSystem = m_pMainDlg->m_pDVRSystem[m_iBoardIndex];
	int i=0;

	for (i=0; i<SDVR_MAX_OSD; i++)
	{
		m_bBtnEnableOSD[i] = pDVRSystem->m_bEnableOSD[m_iChannelIndex][i];
		m_OSD_Config_ex[i] = pDVRSystem->m_OSD_Config_ex[m_iChannelIndex][i];	
		m_szOSD_Text[i] = pDVRSystem->m_szOSD_Text[m_iChannelIndex][i];
	}	

	m_iFrameRate = pDVRSystem->m_iFrameRate[m_iChannelIndex];
	m_bMute     = pDVRSystem->m_bMute[m_iChannelIndex];

	int iSelect = 0;
	for (i=0; i<2; i++)
	{
		if (pDVRSystem->m_pChannelParam[m_iChannelIndex]->m_chan_def.vpp_mode == i)
		{
			iSelect = i;
			break;
		}		
	}
	m_BtnVpp[iSelect].SetCheck(BST_CHECKED);
	
	for (i=0; i<2; i++)
	{
		m_BtnVpp[i].EnableWindow(FALSE);
	}

	for (i=0; i<5; i++)
	{
		m_BtnRes[i].EnableWindow(FALSE);
	}

	if (pDVRSystem->m_video_res_decimation[m_iChannelIndex] == SDVR_VIDEO_RES_DECIMATION_EQUAL)
	{
		m_BtnRes[0].SetCheck(BST_CHECKED);
	}
	else if (pDVRSystem->m_video_res_decimation[m_iChannelIndex] == SDVR_VIDEO_RES_DECIMATION_CIF)
	{
		m_BtnRes[1].SetCheck(BST_CHECKED);
	}
	else if (pDVRSystem->m_video_res_decimation[m_iChannelIndex] == SDVR_VIDEO_RES_DECIMATION_QCIF)
	{
		m_BtnRes[2].SetCheck(BST_CHECKED);
	}
	else if (pDVRSystem->m_video_res_decimation[m_iChannelIndex] == SDVR_VIDEO_RES_DECIMATION_HALF)
	{
		m_BtnRes[3].SetCheck(BST_CHECKED);
	}
	else if (pDVRSystem->m_video_res_decimation[m_iChannelIndex] == SDVR_VIDEO_RES_DECIMATION_DCIF)
	{
		m_BtnRes[4].SetCheck(BST_CHECKED);
	}

	for (i=0; i<SDVR_MAX_OSD; i++)
	{
		m_Cbo_OSD_Pos[i].SetCurSel(m_OSD_Config_ex[i].location_ctrl);
		m_Cbo_OSD_TimeStamp[i].SetCurSel(m_OSD_Config_ex[i].dts_format);
		m_iOSD_XPos[i] = m_OSD_Config_ex[i].top_left_x;
		m_iOSD_YPos[i] = m_OSD_Config_ex[i].top_left_y;
		m_Slider_OSD_Translucent[i].SetPos(m_OSD_Config_ex[i].translucent);

		m_edit_OSDText[i].SetWindowText(m_szOSD_Text[i]);

		if (m_OSD_Config_ex[i].location_ctrl == SDVR_LOC_CUSTOM)
		{
			m_Edit_OSD_XPos[i].EnableWindow(TRUE);
			m_Edit_OSD_YPos[i].EnableWindow(TRUE);
		}
		else
		{
			m_Edit_OSD_XPos[i].EnableWindow(FALSE);
			m_Edit_OSD_YPos[i].EnableWindow(FALSE);
		}
	}

	UpdateData(FALSE);
}


void CChannelSettingDlg::CreateDefaultData()
{
	CDVRSystem* pDVRSystem = m_pMainDlg->m_pDVRSystem[m_iBoardIndex];
	int i=0;

	if (m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bIsPAL)
	{
		m_iFrameRate = 25;		
	}
	else
	{
		m_iFrameRate = 30;
	}	

	m_BtnVpp[0].SetCheck(BST_CHECKED);
	m_BtnRes[1].SetCheck(BST_CHECKED);
	pDVRSystem->m_video_res_decimation[m_iChannelIndex] = SDVR_VIDEO_RES_DECIMATION_CIF;

	for (i=0; i<SDVR_MAX_OSD; i++)
	{
		m_Cbo_OSD_Pos[i].SetCurSel(0);		
		m_Cbo_OSD_TimeStamp[i].SetCurSel(0);
		m_iOSD_XPos[i] = 0;
		m_iOSD_YPos[i] = 0;		
		m_Slider_OSD_Translucent[i].SetPos(100);
		pDVRSystem->m_szOSD_Text[m_iChannelIndex][i] = m_szOSD_Text[i] = "";
		m_OSD_Config_ex[i].translucent = m_Slider_OSD_Translucent[i].GetPos();
		pDVRSystem->m_OSD_Config_ex[m_iChannelIndex][i] = m_OSD_Config_ex[i];
	}

	pDVRSystem->m_bEnableOSD[m_iChannelIndex][0] = m_bBtnEnableOSD[0] = TRUE;
	m_Cbo_OSD_Pos[0].SetCurSel(3);
	m_Cbo_OSD_TimeStamp[0].SetCurSel(2);	
	
	pDVRSystem->m_OSD_Config_ex[m_iChannelIndex][0].location_ctrl = m_OSD_Config_ex[0].location_ctrl = (sdvr_location_e)m_Cbo_OSD_Pos[0].GetCurSel();
	pDVRSystem->m_OSD_Config_ex[m_iChannelIndex][0].dts_format = m_OSD_Config_ex[0].dts_format = (sdvr_dts_style_e)m_Cbo_OSD_TimeStamp[0].GetCurSel();

	OnCbnSelchangeComboOsd1Pos();
	OnCbnSelchangeComboOsd2Pos();	

	CString strValue;
	
	m_Slider_Brightness.SetPos(m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].brightness);
	strValue.Format("%d", m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].brightness);	
	GetDlgItem(IDC_STATIC_SHOW_BRIGHTNESS)->SetWindowText(strValue);	
	
	m_Slider_Contrast.SetPos(m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].contrast);
	strValue.Format("%d", m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].contrast);	
	GetDlgItem(IDC_STATIC_SHOW_CONTRAST)->SetWindowText(strValue);	
	
	m_Slider_Saturation.SetPos(m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].saturation);
	strValue.Format("%d", m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].saturation);	
	GetDlgItem(IDC_STATIC_SHOW_SATURATION)->SetWindowText(strValue);	
	
	m_Slider_Hue.SetPos(m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].hue);
	strValue.Format("%d", m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].hue);	
	GetDlgItem(IDC_STATIC_SHOW_HUE)->SetWindowText(strValue);	
	
	m_Slider_Sharpness.SetPos(m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].sharpness);
	strValue.Format("%d", m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].sharpness);	
	GetDlgItem(IDC_STATIC_SHOW_SHARPNESS)->SetWindowText(strValue);
}


void CChannelSettingDlg::OnBnClickedSetChannel()
{	
	UpdateData(TRUE);	

	CDVRSystem* pDVRSystem = m_pMainDlg->m_pDVRSystem[m_iBoardIndex];

	int i=0;
	int iSelect = 0;
	for (i=0; i<2; i++)
	{
		if (m_BtnVpp[i].GetCheck() == BST_CHECKED)
		{
			iSelect = i;
			break;
		}
	}
	pDVRSystem->m_pChannelParam[m_iChannelIndex]->m_chan_def.vpp_mode = iSelect;

	iSelect = 0;
	for (i=0; i<5; i++)
	{
		if (m_BtnRes[i].GetCheck() == BST_CHECKED)
		{
			iSelect = i;
			break;
		}
	}

	if (iSelect == 0)
	{
		pDVRSystem->m_video_res_decimation[m_iChannelIndex] = SDVR_VIDEO_RES_DECIMATION_EQUAL;
	}
	else if (iSelect == 1)
	{
		pDVRSystem->m_video_res_decimation[m_iChannelIndex] = SDVR_VIDEO_RES_DECIMATION_CIF;
	}
	else if (iSelect == 2)
	{
		pDVRSystem->m_video_res_decimation[m_iChannelIndex] = SDVR_VIDEO_RES_DECIMATION_QCIF;
	}
	else if (iSelect == 3)
	{
		pDVRSystem->m_video_res_decimation[m_iChannelIndex] = SDVR_VIDEO_RES_DECIMATION_HALF;
	}	
	else if (iSelect == 4)
	{
		pDVRSystem->m_video_res_decimation[m_iChannelIndex] = SDVR_VIDEO_RES_DECIMATION_DCIF;
	}	

	if (pDVRSystem->m_bIsPAL)
	{
		if (m_iFrameRate>FRAMERATE_PAL_MAX)
		{
			m_iFrameRate = FRAMERATE_PAL_MAX;
		}
	}
	else
	{
		if (m_iFrameRate>FRAMERATE_NTSC_MAX)
		{
			m_iFrameRate = FRAMERATE_NTSC_MAX;
		}
	}

	if (m_iFrameRate<FRAMERATE_MIN)
	{
		m_iFrameRate = FRAMERATE_MIN;
	}
	pDVRSystem->m_iFrameRate[m_iChannelIndex] = m_iFrameRate;
	pDVRSystem->m_bMute[m_iChannelIndex] = m_bMute;

	for (i=0; i<SDVR_MAX_OSD; i++)
	{
		m_edit_OSDText[i].GetWindowText(m_szOSD_Text[i]);

		int iLength = m_szOSD_Text[i].GetLength();
		
		char szText[SDVR_MAX_OSD_EX_TEXT];
		memset(&szText, 0, sizeof(szText));

		memset(&m_OSD_Config_ex[i].text, 0, sizeof(m_OSD_Config_ex[i].text));

		if (iLength>0)
		{			
			memcpy(&szText, m_szOSD_Text[i].GetString(), iLength);			

			// ansi to unicode
			char* szAnsi = szText;		
			int wcsLen = ::MultiByteToWideChar(CP_ACP, NULL, szAnsi, strlen(szAnsi), NULL, 0);		
			wchar_t* wszString = new wchar_t[wcsLen + 1];
			::MultiByteToWideChar(CP_ACP, NULL, szAnsi, strlen(szAnsi), wszString, wcsLen);		
			wszString[wcsLen] = '\0';	

			memcpy(&m_OSD_Config_ex[i].text, wszString, wcsLen*2);			
			
			if (wszString)
			{
				delete[] wszString;
				wszString = NULL;
			}
		}			

		pDVRSystem->m_bEnableOSD[m_iChannelIndex][i] = m_bBtnEnableOSD[i];
		m_OSD_Config_ex[i].dts_format = (sdvr_dts_style_e)m_Cbo_OSD_TimeStamp[i].GetCurSel();
		m_OSD_Config_ex[i].location_ctrl = (sdvr_location_e)m_Cbo_OSD_Pos[i].GetCurSel();
		m_OSD_Config_ex[i].text_len = iLength;		

		m_OSD_Config_ex[i].top_left_x = m_iOSD_XPos[i];
		m_OSD_Config_ex[i].top_left_y = m_iOSD_YPos[i];
		m_OSD_Config_ex[i].translucent = m_Slider_OSD_Translucent[i].GetPos();

		pDVRSystem->m_OSD_Config_ex[m_iChannelIndex][i] = m_OSD_Config_ex[i];		

		pDVRSystem->m_szOSD_Text[m_iChannelIndex][i] = m_szOSD_Text[i];		
	}

	UpdateData(FALSE);	

	m_pMainDlg->m_bCreateAndInitialChannelSettingDlgOK = FALSE;		

	Sleep(50);

	CloseDrawDib();	

	OnOK();
}


void CChannelSettingDlg::OnNMCustomdrawSliderOsd1Translucent(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);	

	CString strValue;
	strValue.Format("%d", m_Slider_OSD_Translucent[0].GetPos());
	GetDlgItem(IDC_STATIC_OSD1_SHOW_TRANSLUCENT)->SetWindowText(strValue);

	*pResult = 0;
}


void CChannelSettingDlg::OnNMCustomdrawSliderOsd2Translucent(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	
	CString strValue;
	strValue.Format("%d", m_Slider_OSD_Translucent[1].GetPos());
	GetDlgItem(IDC_STATIC_OSD2_SHOW_TRANSLUCENT)->SetWindowText(strValue);	
	
	*pResult = 0;
}


void CChannelSettingDlg::OnCbnSelchangeComboOsd1Pos()
{
	UpdateData(TRUE);
	int iSelect = m_Cbo_OSD_Pos[0].GetCurSel();

	if (iSelect == 4)
	{
		m_Edit_OSD_XPos[0].EnableWindow(TRUE);
		m_Edit_OSD_YPos[0].EnableWindow(TRUE);
	}
	else
	{
		m_Edit_OSD_XPos[0].EnableWindow(FALSE);
		m_Edit_OSD_YPos[0].EnableWindow(FALSE);
	}
	UpdateData(FALSE);
}

void CChannelSettingDlg::OnCbnSelchangeComboOsd2Pos()
{
	UpdateData(TRUE);
	int iSelect = m_Cbo_OSD_Pos[1].GetCurSel();

	if (iSelect == 4)
	{
		m_Edit_OSD_XPos[1].EnableWindow(TRUE);
		m_Edit_OSD_YPos[1].EnableWindow(TRUE);
	}
	else
	{
		m_Edit_OSD_XPos[1].EnableWindow(FALSE);
		m_Edit_OSD_YPos[1].EnableWindow(FALSE);
	}	
	UpdateData(FALSE);
}


void CChannelSettingDlg::OnNMCustomdrawSliderBrightness(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	CString strValue;
	int iValue = m_Slider_Brightness.GetPos();
	strValue.Format("%d", iValue);	
	GetDlgItem(IDC_STATIC_SHOW_BRIGHTNESS)->SetWindowText(strValue);
	m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].brightness = (sx_uint8)iValue;

	if (m_pMainDlg->m_bIsConnectChannelOK[m_iBoardIndex][m_iChannelIndex])
	{		
		m_pMainDlg->ConfigBrightness(m_iChannelIndex, FALSE);
	}
	*pResult = 0;
}


void CChannelSettingDlg::OnNMCustomdrawSliderContrast(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	CString strValue;
	int iValue = m_Slider_Contrast.GetPos();
	strValue.Format("%d", iValue);	
	GetDlgItem(IDC_STATIC_SHOW_CONTRAST)->SetWindowText(strValue);
	m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].contrast = (sx_uint8)iValue;

	if (m_pMainDlg->m_bIsConnectChannelOK[m_iBoardIndex][m_iChannelIndex])
	{		
		m_pMainDlg->ConfigContrast(m_iChannelIndex, FALSE);
	}
	*pResult = 0;
}


void CChannelSettingDlg::OnNMCustomdrawSliderSaturation(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	CString strValue;
	int iValue = m_Slider_Saturation.GetPos();
	strValue.Format("%d", iValue);	
	GetDlgItem(IDC_STATIC_SHOW_SATURATION)->SetWindowText(strValue);
	m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].saturation = (sx_uint8)iValue;

	if (m_pMainDlg->m_bIsConnectChannelOK[m_iBoardIndex][m_iChannelIndex])
	{		
		m_pMainDlg->ConfigSaturation(m_iChannelIndex, FALSE);
	}
	*pResult = 0;
}


void CChannelSettingDlg::OnNMCustomdrawSliderHue(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	CString strValue;
	int iValue = m_Slider_Hue.GetPos();
	strValue.Format("%d", iValue);	
	GetDlgItem(IDC_STATIC_SHOW_HUE)->SetWindowText(strValue);
	m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].hue = (sx_uint8)iValue;

	if (m_pMainDlg->m_bIsConnectChannelOK[m_iBoardIndex][m_iChannelIndex])
	{		
		m_pMainDlg->ConfigHue(m_iChannelIndex, FALSE);
	}
	*pResult = 0;
}


void CChannelSettingDlg::OnNMCustomdrawSliderSharpness(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

	CString strValue;
	int iValue = m_Slider_Sharpness.GetPos();
	strValue.Format("%d", iValue);	
	GetDlgItem(IDC_STATIC_SHOW_SHARPNESS)->SetWindowText(strValue);
	m_pMainDlg->m_image_ctrl[m_iBoardIndex][m_iChannelIndex].sharpness = (sx_uint8)iValue;

	if (m_pMainDlg->m_bIsConnectChannelOK[m_iBoardIndex][m_iChannelIndex])
	{		
		m_pMainDlg->ConfigSharpness(m_iChannelIndex, FALSE);
	}
	*pResult = 0;
}

BOOL CChannelSettingDlg::InitDrawDib()
{
	m_pDirectDrawDisplay = NULL;
	m_pDirectDrawDisplay = new CDirectDraw();

	m_pDirectDrawDisplay->m_iVdo_YBufSize = m_pMainDlg->m_iVdo_YBufSize[m_iBoardIndex][m_iChannelIndex];
	m_pDirectDrawDisplay->m_iVdo_UBufSize = m_pMainDlg->m_iVdo_UBufSize[m_iBoardIndex][m_iChannelIndex];
	m_pDirectDrawDisplay->m_iVdo_VBufSize = m_pMainDlg->m_iVdo_VBufSize[m_iBoardIndex][m_iChannelIndex];

	TRACE("CChannelSettingDlg initDrawDib(%d) width = %d, height = %d.", m_iChannelIndex+1, m_iVdoWidth_Raw, m_iVdoHeight_Raw);

	m_pMainDlg->m_bCreateAndInitialChannelSettingDlgOK = m_pDirectDrawDisplay->InitDirectDraw(m_iVdoWidth_Raw, m_iVdoHeight_Raw, GetDlgItem(IDC_STATIC_DISPLAY_RAW_SECONDARY));		

	return m_pMainDlg->m_bCreateAndInitialChannelSettingDlgOK;
}


BOOL CChannelSettingDlg::CloseDrawDib()
{
	if(m_pDirectDrawDisplay)
	{	
		m_pDirectDrawDisplay->CloseDirectDraw();

		delete m_pDirectDrawDisplay;
		m_pDirectDrawDisplay = NULL;
	}

	return TRUE;
}


void CChannelSettingDlg::DrawVideoYUV(char * y, char * u, char *v)
{
	if (m_pMainDlg->m_bCreateAndInitialChannelSettingDlgOK && m_pDirectDrawDisplay)
	{
		m_pDirectDrawDisplay->DrawYUV(y, u, v, m_iVdoWidth_Raw, m_iVdoHeight_Raw, GetDlgItem(IDC_STATIC_DISPLAY_RAW_SECONDARY));		
	}	
}


void CChannelSettingDlg::DrawMotionAlarm()
{
	CWnd* pcWndItemDisplay = GetDlgItem(IDC_STATIC_DISPLAY_RAW_SECONDARY);
	CClientDC dcDisplay(pcWndItemDisplay);	

	CBrush brushMDAlarm(RGB(255, 0, 0));
	CBrush* oldBrush;
	CString strText;
	CRect rcDisplay;
	pcWndItemDisplay->GetClientRect(&rcDisplay);	

	rcDisplay.left = rcDisplay.left+1;
	rcDisplay.top = rcDisplay.top+1;
	rcDisplay.right = rcDisplay.right-1;
	rcDisplay.bottom = rcDisplay.bottom-1;

	oldBrush = dcDisplay.SelectObject(&brushMDAlarm);	

	dcDisplay.FrameRect(rcDisplay, &brushMDAlarm);
	rcDisplay.left = rcDisplay.left+1;
	rcDisplay.top = rcDisplay.top+1;
	rcDisplay.right = rcDisplay.right-1;
	rcDisplay.bottom = rcDisplay.bottom-1;
	dcDisplay.FrameRect(rcDisplay, &brushMDAlarm);

	strText.Format("CH %d Motion Detect Alarm!!!", m_iChannelIndex+1);
	dcDisplay.TextOut(rcDisplay.left+3, rcDisplay.top+3, strText);	

	dcDisplay.SelectObject(oldBrush);	
}


HBRUSH CChannelSettingDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if ((pWnd->GetDlgCtrlID() == IDC_STATIC_SHOW_HINT) || 
		(pWnd->GetDlgCtrlID() == IDC_STATIC_SECONDARY) ||
		(pWnd->GetDlgCtrlID() == IDC_STATIC_DISPLAY_RAW_SECONDARY))
	{
		pDC-> SetTextColor(RGB(255, 0, 0)); 
	}

	return hbr;
}
