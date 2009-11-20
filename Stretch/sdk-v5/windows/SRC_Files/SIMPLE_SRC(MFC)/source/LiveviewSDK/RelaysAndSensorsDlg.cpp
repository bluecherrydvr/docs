// RelaysAndSensorsDlg.cpp :
//

#include "stdafx.h"
#include "DvrFCD.h"
#include "DvrFCDDlg.h"
#include "RelaysAndSensorsDlg.h"
#include <math.h>



IMPLEMENT_DYNAMIC(CRelaysAndSensorsDlg, CDialog)

CRelaysAndSensorsDlg::CRelaysAndSensorsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRelaysAndSensorsDlg::IDD, pParent)
{
	m_bIsTrigger = true;
}

CRelaysAndSensorsDlg::~CRelaysAndSensorsDlg()
{
}

void CRelaysAndSensorsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	int i=0;
	for (i=0; i<MAX_RELAYS_SENSORS; i++)
	{
		DDX_Control(pDX, IDC_CHECK_SENSOR1+i, m_BtnSensors[i]);
		DDX_Control(pDX, IDC_COMBO_SENSOR1+i, m_Cbo_SensorTrigger[i]);
		DDX_Control(pDX, IDC_CHECK_TRIGGER1+i, m_BtnRelays[i]);
	}

	DDX_Control(pDX, IDC_LIST_SENSORS, m_ListSensor);
}


BEGIN_MESSAGE_MAP(CRelaysAndSensorsDlg, CDialog)
	ON_BN_CLICKED(ID_RELAYS_SENSORS_SET_EXIT, &CRelaysAndSensorsDlg::OnBnClickedRelaysSensorsSetExit)
	ON_BN_CLICKED(IDC_BUTTON_TRIGGER, &CRelaysAndSensorsDlg::OnBnClickedButtonTrigger)
	ON_BN_CLICKED(ID_SENSORS_SET, &CRelaysAndSensorsDlg::OnBnClickedSensorsSet)
END_MESSAGE_MAP()



BOOL CRelaysAndSensorsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	int i=0;
	for (i=0; i<MAX_RELAYS_SENSORS; i++)
	{
		m_Cbo_SensorTrigger[i].AddString("Edge Trigger");
		m_Cbo_SensorTrigger[i].AddString("Level Sensitive");
		m_Cbo_SensorTrigger[i].SetCurSel(0);
		
		m_BtnRelays[i].SetCheck(BST_UNCHECKED);

		if (m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bEnableSensors[i])		
		{
			m_BtnSensors[i].SetCheck(BST_CHECKED);
		}
		else
		{
			m_BtnSensors[i].SetCheck(BST_UNCHECKED);
		}

		if (m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bEnableRelays[i])		
		{
			m_BtnRelays[i].SetCheck(BST_CHECKED);
		}
		else
		{
			m_BtnRelays[i].SetCheck(BST_UNCHECKED);
		}
		
		m_Cbo_SensorTrigger[i].SetCurSel(m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_iTriggerMode[i]);		
	}	

	m_ListSensor.ResetContent();
	m_ListSensor.AddString("Sensors Alarm Record::");

	return TRUE;  // return TRUE unless you set the focus to a control	
}

void CRelaysAndSensorsDlg::OnBnClickedRelaysSensorsSetExit()
{
	OnBnClickedSensorsSet();

	OnOK();
}

void CRelaysAndSensorsDlg::OnBnClickedButtonTrigger()
{
	GetDlgItem(IDC_BUTTON_TRIGGER)->EnableWindow(FALSE);
	int i=0;
	for (i=0; i<MAX_RELAYS_SENSORS; i++)
	{
		if (m_BtnRelays[i].GetCheck() == BST_CHECKED)
		{
			m_pMainDlg->PostMessage(WM_RELAY_TRIGGER, i, m_bIsTrigger);			
			Sleep(50);
		}				
	}

	m_bIsTrigger =! m_bIsTrigger;

	GetDlgItem(IDC_BUTTON_TRIGGER)->EnableWindow(TRUE);
}

void CRelaysAndSensorsDlg::OnBnClickedSensorsSet()
{

	int sensor_enable_map = 0;
	int edge_triggered_map = 0;
	int i=0;

	for (i=0; i<MAX_RELAYS_SENSORS; i++)
	{
		if (m_BtnSensors[i].GetCheck() == BST_CHECKED)
		{
			m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bEnableSensors[i] = TRUE;

			sensor_enable_map |= (int) pow(2.0, i);

			if (m_Cbo_SensorTrigger[i].GetCurSel() == SENSOR_EDGE_TRIGGER)
			{
				edge_triggered_map |= (int) pow(2.0, i);
			}
		}
		else
		{
			m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bEnableSensors[i] = FALSE;
		}

		if (m_BtnRelays[i].GetCheck() == BST_CHECKED)
		{
			m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bEnableRelays[i] = TRUE;
		}
		else
		{
			m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_bEnableRelays[i] = FALSE;
		}

		m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_iTriggerMode[i] = m_Cbo_SensorTrigger[i].GetCurSel();
	}	

	m_pMainDlg->PostMessage(WM_CONFIG_SENSORS, sensor_enable_map, edge_triggered_map);
}


void CRelaysAndSensorsDlg::ShowSensorAlarm(int iSensor)
{
	if (m_ListSensor.GetCount()>50)
	{
		m_ListSensor.ResetContent();
		m_ListSensor.AddString("Sensors Alarm Record::");
	}

	CString strText;
	SYSTEMTIME stTime;
	GetLocalTime(&stTime);

	if (m_pMainDlg->m_pDVRSystem[m_iBoardIndex]->m_iTriggerMode[iSensor] == SENSOR_EDGE_TRIGGER)
	{
		strText.Format("Sensor(%d) Edge Trigger Alarm!(%d/%02d/%02d-%02d:%02d:%02d:%03d)", iSensor+1, 
						stTime.wYear, stTime.wMonth, stTime.wDay, stTime.wHour, stTime.wMinute, stTime.wSecond, stTime.wMilliseconds);
	}
	else
	{
		strText.Format("Sensor(%d) Level Sensitive Alarm!(%d/%02d/%02d-%02d:%02d:%02d:%03d)", iSensor+1, 
			stTime.wYear, stTime.wMonth, stTime.wDay, stTime.wHour, stTime.wMinute, stTime.wSecond, stTime.wMilliseconds);
	}

	m_ListSensor.AddString(strText);	
	m_ListSensor.SetCurSel(m_ListSensor.GetCount()-1);
}