// RegionSetting.cpp :
//

#include "stdafx.h"
#include "DvrFCD.h"
#include "DvrFCDDlg.h"
#include "RegionSetting.h"


#define MD_THRESHOLD_MIN		0
#define MD_THRESHOLD_MAX		99

COLORREF MyRGB[SDVR_MAX_MD_REGIONS] = {  RGB(255, 255, 30),
												RGB(255, 30, 255),
												RGB(30, 255, 255),
												RGB(100, 255, 100)
											 };


// CRegionSetting

IMPLEMENT_DYNAMIC(CRegionSetting, CDialog)

CRegionSetting::CRegionSetting(CWnd* pParent /*=NULL*/)
	: CDialog(CRegionSetting::IDD, pParent)
{
	m_pDirectDrawDisplay = NULL;

	m_iVdoWidth_Raw = 0;
	m_iVdoHeight_Raw = 0;	
	m_bIsDrawArea = FALSE;
	m_bLBtnDown = FALSE;	

	int i=0;
	for (i=0; i<SDVR_MAX_MD_REGIONS; i++)
	{
		m_iMDThreshold[i] = 20;
	}

	m_iMDValueFrequency = 0;
	m_iMDRectLayer = 0;
	m_bIsRect = FALSE;	
	m_bEnableMD = FALSE;
	m_iRegionCount = 0;	
	m_iNextRegionID = 0;
	m_bStatusIsAddRegion = FALSE;

	m_pChannelParam = NULL;
	m_iListCurSelectNum = 0;
	m_iCurListPos = -1;
}

CRegionSetting::~CRegionSetting()
{	
}

void CRegionSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	int i=0;
	for (i=0; i<SDVR_MAX_MD_REGIONS; i++)
	{
		DDX_Control(pDX, IDC_RADIO_MD_0+i, m_BtnMD[i]);

		DDX_Text(pDX, IDC_EDIT_THRESHOLD1+i, m_iMDThreshold[i]);
		DDV_MinMaxInt(pDX, m_iMDThreshold[i], MD_THRESHOLD_MIN, MD_THRESHOLD_MAX);
	}

	DDX_Text(pDX, IDC_EDIT_MOTION_FREQUENCY, m_iMDValueFrequency);
	DDV_MinMaxInt(pDX, m_iMDValueFrequency, 0, 255);

	DDX_Control(pDX, IDC_BUTTON_REMOVE_REGION, m_BtnRemove);
	DDX_Check(pDX, IDC_CHECK_ENABLE_MD, m_bEnableMD);

	DDX_Control(pDX, IDC_COMBO_BAUD_RATE, m_Cbo_BaudRate);
	DDX_Control(pDX, IDC_COMBO_ADDRESS, m_Cbo_Address);
	DDX_Control(pDX, IDC_COMBO_SPEED, m_Cbo_Speed);

	DDX_Control(pDX, IDC_LIST_RECT_ID, m_ListRectID);
}


BEGIN_MESSAGE_MAP(CRegionSetting, CDialog)
	ON_BN_CLICKED(ID_REGION_SETTING_OK, &CRegionSetting::OnBnClickedRegionSettingOk)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()		
	ON_BN_CLICKED(IDC_RADIO_MD_0, &CRegionSetting::OnBnClickedRadioMd0)
	ON_BN_CLICKED(IDC_RADIO_MD_1, &CRegionSetting::OnBnClickedRadioMd1)
	ON_BN_CLICKED(IDC_RADIO_MD_2, &CRegionSetting::OnBnClickedRadioMd2)
	ON_BN_CLICKED(IDC_RADIO_MD_3, &CRegionSetting::OnBnClickedRadioMd3)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_REGION, &CRegionSetting::OnBnClickedButtonRemoveRegion)	
	ON_BN_CLICKED(IDC_BUTTON_ADD_REGION, &CRegionSetting::OnBnClickedButtonAddRegion)
	ON_BN_CLICKED(IDC_BUTTON_SET_EXIT, &CRegionSetting::OnBnClickedButtonSetExit)	
	ON_CBN_SELCHANGE(IDC_COMBO_BAUD_RATE, &CRegionSetting::OnCbnSelchangeComboBaudRate)
	ON_CBN_SELCHANGE(IDC_COMBO_ADDRESS, &CRegionSetting::OnCbnSelchangeComboAddress)
	ON_CBN_SELCHANGE(IDC_COMBO_SPEED, &CRegionSetting::OnCbnSelchangeComboSpeed)
	ON_BN_CLICKED(IDC_BUTTON_INIT_RS485, &CRegionSetting::OnBnClickedButtonInitRs485)
	ON_BN_CLICKED(IDC_BUTTON_UP, &CRegionSetting::OnBnClickedButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_LEFT, &CRegionSetting::OnBnClickedButtonLeft)
	ON_BN_CLICKED(IDC_BUTTON_RIGHT, &CRegionSetting::OnBnClickedButtonRight)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, &CRegionSetting::OnBnClickedButtonDown)
	ON_BN_CLICKED(IDC_BUTTON_ZOOM_IN, &CRegionSetting::OnBnClickedButtonZoomIn)
	ON_BN_CLICKED(IDC_BUTTON_ZOOM_OUT, &CRegionSetting::OnBnClickedButtonZoomOut)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS_NEAR, &CRegionSetting::OnBnClickedButtonFocusNear)
	ON_BN_CLICKED(IDC_BUTTON_FOCUS_FAR, &CRegionSetting::OnBnClickedButtonFocusFar)
	ON_BN_CLICKED(IDC_BUTTON_IRIS_LARGE, &CRegionSetting::OnBnClickedButtonIrisLarge)
	ON_BN_CLICKED(IDC_BUTTON_IRIS_SMALL, &CRegionSetting::OnBnClickedButtonIrisSmall)
	ON_MESSAGE(WM_PTZ_ACTION_STOP, OnPTZ_ActionStop)
	ON_LBN_SELCHANGE(IDC_LIST_RECT_ID, &CRegionSetting::OnLbnSelchangeListRectId)	
END_MESSAGE_MAP()


// CRegionSetting

BOOL CRegionSetting::OnInitDialog()
{
	CDialog::OnInitDialog();

	int i=0;

	m_pChannelParam = m_pMainDlg->m_pDVRSystem[m_pMainDlg->m_iRadioBtnBoard]->m_pChannelParam[m_iChannelIndex];

	GetDlgItem(IDC_STATIC_DRAW_REGION_AREA)->GetClientRect(&m_rcDrawArea);

	m_pMainDlg->m_rcRegionSettingDlgDrawArea = m_rcDrawArea;	
	m_bEnableMD = m_pChannelParam->m_bEnableMD;	

	for (i=0; i<SDVR_MAX_MD_REGIONS; i++)
	{
		m_iMDThreshold[i] = m_pChannelParam->m_iMDThreshold[i];
	}	

	m_iMDValueFrequency =  m_pChannelParam->m_iMDValueFrequency;
	m_BtnMD[m_iMDRectLayer].SetCheck(BST_CHECKED);
		
	m_Cbo_BaudRate.AddString("2400");
	m_Cbo_BaudRate.AddString("4800");
	m_Cbo_BaudRate.AddString("9600");
	if (m_pMainDlg->m_iBaudRate == 2400)
	{
		m_Cbo_BaudRate.SetCurSel(0);
	}
	else if (m_pMainDlg->m_iBaudRate == 4800)
	{
		m_Cbo_BaudRate.SetCurSel(1);
	}
	else if (m_pMainDlg->m_iBaudRate == 9600)
	{
		m_Cbo_BaudRate.SetCurSel(2);
	}	

	m_Cbo_Speed.AddString("1");
	m_Cbo_Speed.AddString("2");
	m_Cbo_Speed.AddString("3");	
	m_Cbo_Speed.SetCurSel(m_pMainDlg->m_iSpeed);

	CString strAddress;
	for (i=0; i<255; i++)
	{
		strAddress.Format("%d", i);
		m_Cbo_Address.AddString(strAddress);
	}
	m_Cbo_Address.SetCurSel(m_pMainDlg->m_iAddress);

	if (m_pMainDlg->m_bRS485Init[m_pMainDlg->m_iRadioBtnBoard])
	{
		GetDlgItem(IDC_BUTTON_INIT_RS485)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_INIT_RS485)->EnableWindow(TRUE);
	}
	
	FindEmptyRegion();
//	RefreshListID();

	CString strTitle = "";
	strTitle.Format("Channel %d Region Setting & PTZ Control.", m_iChannelIndex+1);
	SetWindowText(strTitle);	

	UpdateData(FALSE);

	InitDrawDib();

	return TRUE;  // return TRUE unless you set the focus to a control	
}


BOOL CRegionSetting::InitDrawDib()
{
	m_pDirectDrawDisplay = NULL;
	m_pDirectDrawDisplay = new CDirectDraw();

	m_pDirectDrawDisplay->m_iVdo_YBufSize = m_pMainDlg->m_iVdo_YBufSize[m_pMainDlg->m_iRadioBtnBoard][m_iChannelIndex];
	m_pDirectDrawDisplay->m_iVdo_UBufSize = m_pMainDlg->m_iVdo_UBufSize[m_pMainDlg->m_iRadioBtnBoard][m_iChannelIndex];
	m_pDirectDrawDisplay->m_iVdo_VBufSize = m_pMainDlg->m_iVdo_VBufSize[m_pMainDlg->m_iRadioBtnBoard][m_iChannelIndex];

	TRACE("RegionSetting initDrawDib(%d) width = %d, height = %d\n", m_iChannelIndex+1, m_iVdoWidth_Raw, m_iVdoHeight_Raw);

	m_pMainDlg->m_bCreateAndInitialRegionSettingDlgOK = m_pDirectDrawDisplay->InitDirectDraw(m_iVdoWidth_Raw, m_iVdoHeight_Raw, GetDlgItem(IDC_STATIC_DRAW_REGION_AREA));		

	return m_pMainDlg->m_bCreateAndInitialRegionSettingDlgOK;
}


BOOL CRegionSetting::CloseDrawDib()
{
	if(m_pDirectDrawDisplay)
	{	
		m_pDirectDrawDisplay->CloseDirectDraw();

		delete m_pDirectDrawDisplay;
		m_pDirectDrawDisplay = NULL;
	}

	return TRUE;
}


void CRegionSetting::DrawVideoYUV(char * y, char * u, char *v)
{
	if (m_pMainDlg->m_bCreateAndInitialRegionSettingDlgOK && m_pDirectDrawDisplay)
	{
		m_pDirectDrawDisplay->DrawYUV(y, u, v, m_iVdoWidth_Raw, m_iVdoHeight_Raw, GetDlgItem(IDC_STATIC_DRAW_REGION_AREA));
		DrawRegion();
	}	
}


void CRegionSetting::OnBnClickedRegionSettingOk()
{		
	UpdateData(TRUE);

	m_pChannelParam->m_bEnableMD = m_bEnableMD;
	
	int i=0;
	for (i=0; i<SDVR_MAX_MD_REGIONS; i++)
	{
		m_pChannelParam->m_iMDThreshold[i] = m_iMDThreshold[i];
	}	

	m_pChannelParam->m_iMDValueFrequency = m_iMDValueFrequency;

	UpdateData(FALSE);
	
	m_pMainDlg->PostMessage(WM_MD_ENABLE, m_iChannelIndex, m_bEnableMD);	
}


void CRegionSetting::OnLButtonDown(UINT nFlags, CPoint point)
{
	int i=0;

	if((point.x<=m_rcDrawArea.right) && (point.y<=m_rcDrawArea.bottom))
	{
		m_ptStart = m_ptEnd = point;
		m_bIsDrawArea = TRUE;	
		m_bLBtnDown = TRUE;
		m_bIsRect = TRUE;	

		m_pChannelParam->m_bFinishMDRectOK[m_iMDRectLayer][m_iNextRegionID] = FALSE;	// Let DrawRegion() knows ready to draw new region.
	}
	else
	{
		m_bIsDrawArea = FALSE;
	}


	CDialog::OnLButtonDown(nFlags, point);
}


void CRegionSetting::OnLButtonUp(UINT nFlags, CPoint point)
{
	int i=0;

	if((point.x<=m_rcDrawArea.right) && (point.y<=m_rcDrawArea.bottom))
	{
		m_ptEnd = point;
		m_bIsDrawArea = TRUE;		
		m_bLBtnDown = FALSE;		
//		TRACE("OnLButtonUp(%d, %d)\n", point.x, point.y);
	}
	else
	{
		if (point.x>m_rcDrawArea.right)
		{
			m_ptEnd.x = m_rcDrawArea.right;
		}

		if (point.y>m_rcDrawArea.bottom)
		{
			m_ptEnd.y = m_rcDrawArea.bottom;
		}		
		
		m_bIsDrawArea = FALSE;
		m_bLBtnDown = FALSE;
	}
		
	if (m_bIsRect && (m_iNextRegionID<MAX_REGION_PER_LAYER))
	{
		m_pChannelParam->m_rcMDRect[m_iMDRectLayer][m_iNextRegionID].left = m_ptStart.x;
		m_pChannelParam->m_rcMDRect[m_iMDRectLayer][m_iNextRegionID].top = m_ptStart.y;
		m_pChannelParam->m_rcMDRect[m_iMDRectLayer][m_iNextRegionID].right = m_ptEnd.x;
		m_pChannelParam->m_rcMDRect[m_iMDRectLayer][m_iNextRegionID].bottom = m_ptEnd.y;

//		m_pMainDlg->m_rcMDRect[m_iChannelIndex][iRegionID] = m_rcMDRect[iRegionID];
	//	m_pMainDlg->m_bFinishMDRectOK[m_iChannelIndex][iRegionID] = m_bFinishMDRectOK[iRegionID] = TRUE;
		m_pChannelParam->m_bFinishMDRectOK[m_iMDRectLayer][m_iNextRegionID] = TRUE;
		CString strID;
		strID.Format("%04d", m_iNextRegionID+1);
		m_ListRectID.AddString(strID);	

		m_pMainDlg->PostMessage(WM_MD_MODIFY_REGION, m_iChannelIndex, 0);

		FindEmptyRegion();
		
		m_bIsRect = FALSE;		
	}	

	CDialog::OnLButtonUp(nFlags, point);
}


void CRegionSetting::OnMouseMove(UINT nFlags, CPoint point)
{
	if((point.x<=m_rcDrawArea.right) && (point.y<=m_rcDrawArea.bottom))
	{
		if (m_bLBtnDown)
		{
			m_ptEnd = point;		
		}	
		
		m_bIsDrawArea = TRUE;		
	
//		TRACE("OnMouseMove(%d, %d)\n", point.x, point.y);
	}
	else
	{
		m_bIsDrawArea = FALSE;
		m_bLBtnDown = FALSE;

		if (point.x>m_rcDrawArea.right)
		{
			m_ptEnd.x = m_rcDrawArea.right;
		}

		if (point.y>m_rcDrawArea.bottom)
		{
			m_ptEnd.y = m_rcDrawArea.bottom;
		}		
	}	

	CDialog::OnMouseMove(nFlags, point);
}


void CRegionSetting::DrawRegion()
{
	CWnd* pcWndItemDrawArea = GetDlgItem(IDC_STATIC_DRAW_REGION_AREA);
	CClientDC dcDrawArea(pcWndItemDrawArea);	

	int i=0, j=0;	

	CBrush brush[SDVR_MAX_MD_REGIONS];

	for (i=0; i<SDVR_MAX_MD_REGIONS; i++)
	{
		brush[i].CreateSolidBrush(MyRGB[i]);		
	}
	
	CBrush* oldBrush;

	CRect rcRect(m_ptStart.x, m_ptStart.y, m_ptEnd.x, m_ptEnd.y);
	CString strText[SDVR_MAX_MD_REGIONS];

	oldBrush = dcDrawArea.SelectObject(&brush[m_iMDRectLayer]);
	if ((m_iNextRegionID<MAX_REGION_PER_LAYER) && m_bIsRect)
	{
		dcDrawArea.FrameRect(rcRect, &brush[m_iMDRectLayer]);		
		strText[m_iMDRectLayer].Format("*%d", m_iNextRegionID+1);
		dcDrawArea.SetTextColor(MyRGB[m_iMDRectLayer]);
		dcDrawArea.SetBkColor(CLR_NONE);
		dcDrawArea.TextOut(rcRect.left+1, rcRect.top+1, strText[m_iMDRectLayer]);			
	}
	
	for (i=0; i<SDVR_MAX_MD_REGIONS; i++)
	{
		for (j=0; j<MAX_REGION_PER_LAYER; j++)
		{
			if (m_pChannelParam->m_bFinishMDRectOK[i][j])
			{
				dcDrawArea.FrameRect(m_pChannelParam->m_rcMDRect[i][j], &brush[i]);					
				
				if ((m_iListCurSelectNum == (j+1)) && (m_iMDRectLayer == i) && !m_bLBtnDown)
				{
					strText[i].Format("*%d", j+1);
				}
				else
				{
					strText[i].Format("%d", j+1);
				}

				dcDrawArea.SetTextColor(MyRGB[i]);
				dcDrawArea.SetBkColor(CLR_NONE);
				dcDrawArea.TextOut(m_pChannelParam->m_rcMDRect[i][j].left+1, m_pChannelParam->m_rcMDRect[i][j].top+1, strText[i]);
			}
		}
	}

	dcDrawArea.SelectObject(oldBrush);
}


void CRegionSetting::OnBnClickedRadioMd0()
{
	m_iMDRectLayer = 0;
	FindEmptyRegion();
//	RefreshListID();
}

void CRegionSetting::OnBnClickedRadioMd1()
{
	m_iMDRectLayer = 1;
	FindEmptyRegion();
//	RefreshListID();
}

void CRegionSetting::OnBnClickedRadioMd2()
{
	m_iMDRectLayer = 2;
	FindEmptyRegion();
//	RefreshListID();
}

void CRegionSetting::OnBnClickedRadioMd3()
{
	m_iMDRectLayer = 3;
	FindEmptyRegion();
//	RefreshListID();
}


void CRegionSetting::OnBnClickedButtonRemoveRegion()
{	
	int i=0, iRegionID=0;

	int iRemoveID = 0;
	int iSelect = m_ListRectID.GetCurSel();

	if (iSelect<0)
	{
		return;
	}

	CString strGetRemoveID="";
	m_ListRectID.GetText(iSelect, strGetRemoveID);
	
	iRemoveID = atoi(strGetRemoveID);
	iRemoveID-=1;

	m_ListRectID.DeleteString(iSelect);

	if (!m_pChannelParam->m_bFinishMDRectOK[m_iMDRectLayer][iRemoveID])	// no this region.
	{
		return;
	}

	m_pChannelParam->m_bFinishMDRectOK[m_iMDRectLayer][iRemoveID] = false;	
	
	m_pChannelParam->m_rcMDRect[m_iMDRectLayer][iRemoveID].left = -1;
	m_pChannelParam->m_rcMDRect[m_iMDRectLayer][iRemoveID].top = -1;
	m_pChannelParam->m_rcMDRect[m_iMDRectLayer][iRemoveID].right = -1;
	m_pChannelParam->m_rcMDRect[m_iMDRectLayer][iRemoveID].bottom = -1;		
	
	m_pMainDlg->PostMessage(WM_MD_MODIFY_REGION, m_iChannelIndex, 0);

	FindEmptyRegion();			
}


void CRegionSetting::OnBnClickedButtonAddRegion()
{
	GetDlgItem(IDC_BUTTON_ADD_REGION)->EnableWindow(FALSE);
	
	m_bStatusIsAddRegion = TRUE;	
}


void CRegionSetting::DrawMotionAlarm()
{
	CWnd* pcWndItemDisplay = GetDlgItem(IDC_STATIC_DRAW_REGION_AREA);
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


void CRegionSetting::OnBnClickedButtonSetExit()
{
	OnBnClickedRegionSettingOk();

	m_pMainDlg->m_bCreateAndInitialRegionSettingDlgOK = FALSE;

	Sleep(50);

	CloseDrawDib();	

	OnOK();
}

void CRegionSetting::OnCbnSelchangeComboBaudRate()
{
	if (m_Cbo_BaudRate.GetCurSel() == 0)
	{
		m_pMainDlg->m_iBaudRate = 2400;
	}
	else if (m_Cbo_BaudRate.GetCurSel() == 1)
	{
		m_pMainDlg->m_iBaudRate = 4800;
	}
	else if (m_Cbo_BaudRate.GetCurSel() == 2)
	{
		m_pMainDlg->m_iBaudRate = 9600;
	}
}

void CRegionSetting::OnCbnSelchangeComboSpeed()
{
	m_pMainDlg->m_iSpeed = m_Cbo_Speed.GetCurSel();
}

void CRegionSetting::OnCbnSelchangeComboAddress()
{
	m_pMainDlg->m_iAddress = m_Cbo_Address.GetCurSel();
}

void CRegionSetting::OnBnClickedButtonInitRs485()
{
	if (m_pMainDlg->SendMessage(WM_PTZ_ACTION, m_iChannelIndex, PTZ_INIT) == S_OK)
	{		
		GetDlgItem(IDC_BUTTON_INIT_RS485)->EnableWindow(FALSE);
	}
	else
	{		
		GetDlgItem(IDC_BUTTON_INIT_RS485)->EnableWindow(TRUE);
	}
}

void CRegionSetting::OnBnClickedButtonUp()
{	
	m_pMainDlg->PostMessage(WM_PTZ_ACTION, m_iChannelIndex, PTZ_UP);
	PostMessage(WM_PTZ_ACTION_STOP, m_iChannelIndex, 0);
}

void CRegionSetting::OnBnClickedButtonLeft()
{
	m_pMainDlg->PostMessage(WM_PTZ_ACTION, m_iChannelIndex, PTZ_LEFT);
	PostMessage(WM_PTZ_ACTION_STOP, m_iChannelIndex, 0);
}

void CRegionSetting::OnBnClickedButtonRight()
{
	m_pMainDlg->PostMessage(WM_PTZ_ACTION, m_iChannelIndex, PTZ_RIGHT);
	PostMessage(WM_PTZ_ACTION_STOP, m_iChannelIndex, 0);
}

void CRegionSetting::OnBnClickedButtonDown()
{
	m_pMainDlg->PostMessage(WM_PTZ_ACTION, m_iChannelIndex, PTZ_DOWN);
	PostMessage(WM_PTZ_ACTION_STOP, m_iChannelIndex, 0);
}

void CRegionSetting::OnBnClickedButtonZoomIn()
{
	m_pMainDlg->PostMessage(WM_PTZ_ACTION, m_iChannelIndex, PTZ_ZOOM_IN);
	PostMessage(WM_PTZ_ACTION_STOP, m_iChannelIndex, 0);
}

void CRegionSetting::OnBnClickedButtonZoomOut()
{
	m_pMainDlg->PostMessage(WM_PTZ_ACTION, m_iChannelIndex, PTZ_ZOOM_OUT);
	PostMessage(WM_PTZ_ACTION_STOP, m_iChannelIndex, 0);
}

void CRegionSetting::OnBnClickedButtonFocusNear()
{
	m_pMainDlg->PostMessage(WM_PTZ_ACTION, m_iChannelIndex, PTZ_FOCUS_NEAR);
	PostMessage(WM_PTZ_ACTION_STOP, m_iChannelIndex, 0);
}

void CRegionSetting::OnBnClickedButtonFocusFar()
{
	m_pMainDlg->PostMessage(WM_PTZ_ACTION, m_iChannelIndex, PTZ_FOCUS_FAR);
	PostMessage(WM_PTZ_ACTION_STOP, m_iChannelIndex, 0);
}

void CRegionSetting::OnBnClickedButtonIrisLarge()
{
	m_pMainDlg->PostMessage(WM_PTZ_ACTION, m_iChannelIndex, PTZ_IRIS_LARGE);
	PostMessage(WM_PTZ_ACTION_STOP, m_iChannelIndex, 0);
}

void CRegionSetting::OnBnClickedButtonIrisSmall()
{
	m_pMainDlg->PostMessage(WM_PTZ_ACTION, m_iChannelIndex, PTZ_IRIS_SMALL);
	PostMessage(WM_PTZ_ACTION_STOP, m_iChannelIndex, 0);
}


HRESULT CRegionSetting::OnPTZ_ActionStop(WPARAM wParam, LPARAM lParam)
{
	if (!m_pMainDlg->m_bRS485Init[m_pMainDlg->m_iRadioBtnBoard])
	{
		return S_OK;
	}

	GetDlgItem(IDC_BUTTON_UP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_LEFT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_RIGHT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_FOCUS_FAR)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_FOCUS_NEAR)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_ZOOM_IN)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_ZOOM_OUT)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_IRIS_LARGE)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_IRIS_SMALL)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_SET_EXIT)->EnableWindow(FALSE);
	
	Sleep(500+m_pMainDlg->m_iSpeed*500);

	m_pMainDlg->PostMessage(WM_PTZ_ACTION, m_iChannelIndex, PTZ_ACTION_STOP);

	Sleep(50);

	GetDlgItem(IDC_BUTTON_UP)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_LEFT)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_RIGHT)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_DOWN)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_FOCUS_FAR)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_FOCUS_NEAR)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_ZOOM_IN)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_ZOOM_OUT)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_IRIS_LARGE)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_IRIS_SMALL)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_SET_EXIT)->EnableWindow(TRUE);

	return S_OK;
}

BOOL CRegionSetting::FindEmptyRegion()
{
	int i=0;
	int iOldNextRegionID = m_iNextRegionID;

	for (i=0; i<MAX_REGION_PER_LAYER; i++)
	{
		if (!m_pChannelParam->m_bFinishMDRectOK[m_iMDRectLayer][i])
		{
			m_iNextRegionID = i;
			break;
		}
	}

	RefreshListID();

	return TRUE;
}

void CRegionSetting::RefreshListID()
{
	m_ListRectID.ResetContent();
	
	CString strID;
	int i=0;

	for (i=0; i<MAX_REGION_PER_LAYER; i++)
	{
		if (m_pChannelParam->m_bFinishMDRectOK[m_iMDRectLayer][i])
		{
			strID.Format("%04d", i+1);
			m_ListRectID.AddString(strID);
		}
	}
	
	m_ListRectID.SetCurSel(m_ListRectID.GetCount()-1);		

	OnLbnSelchangeListRectId();
}


void CRegionSetting::OnLbnSelchangeListRectId()
{	
	m_iCurListPos = m_ListRectID.GetCurSel();

	if (m_iCurListPos<0)
	{
		return;
	}

	CString strGetID="";
	m_ListRectID.GetText(m_iCurListPos, strGetID);

	m_iListCurSelectNum = atoi(strGetID);

	TRACE("OnLbnSelchangeListRectId() = %d\n", m_iListCurSelectNum+1);
}