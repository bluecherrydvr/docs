// DecodeSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PlayDvrFCD.h"
#include "PlayDvrFCDDlg.h"
#include "DecodeSetupDlg.h"


// CDecodeSetupDlg dialog

IMPLEMENT_DYNAMIC(CDecodeSetupDlg, CDialog)

CDecodeSetupDlg::CDecodeSetupDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDecodeSetupDlg::IDD, pParent)
	, m_bInputType(FALSE)
	, m_sFileFullPath(_T(""))
	, m_bDecodeType(FALSE)
{
	m_pMainDlg = (CDvrFCDDlg*) pParent;

	m_iDecodeChIdx = -1;
	m_iSelEncodeCHIdx = -1;
}

CDecodeSetupDlg::~CDecodeSetupDlg()
{
}

void CDecodeSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_FILENAME, m_bInputType);
	DDX_Text(pDX, IDC_STATIC_FILE_PATH, m_sFileFullPath);
	DDX_Control(pDX, IDC_COMBO_CAMERA, m_cEncodeChComboBox);
	DDX_Radio(pDX, IDC_RADIO_HW_DECODE, m_bDecodeType);
}


BEGIN_MESSAGE_MAP(CDecodeSetupDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CDecodeSetupDlg::OnBnClickedButtonBrowse)
	ON_BN_CLICKED(IDC_RADIO_FILENAME, &CDecodeSetupDlg::OnBnClickedRadioSourceInput)
	ON_BN_CLICKED(IDC_RADIO_CAMERA, &CDecodeSetupDlg::OnBnClickedRadioSourceInput)
	
	ON_BN_CLICKED(IDOK, &CDecodeSetupDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO_HW_DECODE, &CDecodeSetupDlg::OnBnClickedRadioDecodeType)
	ON_BN_CLICKED(IDC_RADIO_SW_DECODE, &CDecodeSetupDlg::OnBnClickedRadioDecodeType)

END_MESSAGE_MAP()


// CDecodeSetupDlg message handlers

void CDecodeSetupDlg::OnBnClickedButtonBrowse()
{
	// TODO: Add your control notification handler code here

	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);

	CString MyStr;

	TCHAR szFilters[] = _T ("*.mov");
	MyStr.Format("Select a movie file to decode\n");

	CFileDialog dlg(TRUE, MyStr, szFilters,
						OFN_HIDEREADONLY, szFilters);

	if (dlg.DoModal () == IDOK)
	{
		m_sFileFullPath = dlg.GetPathName();

		UpdateData(FALSE);
	}
}

void CDecodeSetupDlg::OnBnClickedRadioSourceInput()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	if(m_iTotalHWEncodeChannel == 2) //Already max two Encode & Two Decode Channel
	{
		GetDlgItem(IDC_STATIC_FILE_PATH)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(FALSE);

		GetDlgItem(IDOK)->EnableWindow(FALSE);

		GetDlgItem(IDC_RADIO_CAMERA)->EnableWindow(FALSE);
		GetDlgItem(IDC_COMBO_CAMERA)->EnableWindow(FALSE);
	}
	else
	{
		if(m_bInputType == FALSE) //FileName
		{
			GetDlgItem(IDC_STATIC_FILE_PATH)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(TRUE);

			GetDlgItem(IDC_COMBO_CAMERA)->EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_STATIC_FILE_PATH)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(FALSE);

			GetDlgItem(IDC_COMBO_CAMERA)->EnableWindow(TRUE);
		}
	}
}

BOOL CDecodeSetupDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	
	int iBoardCount = 0, i = 0;
	CString	sTemp;

	m_iTotalHWEncodeChannel = m_pMainDlg->m_pDVRSystem[m_pMainDlg->m_iEventOpenBoard]->m_iTotalHWEncodeChannel[(m_iDecodeChIdx / MAX_SUPPORT_BOARD)];
	m_iTotalHWDecodeChannel = m_pMainDlg->m_pDVRSystem[m_pMainDlg->m_iEventOpenBoard]->m_iTotalHWDecodeChannel[(m_iDecodeChIdx / MAX_SUPPORT_BOARD)];

	if((m_iTotalHWEncodeChannel < 2) &&
		((m_iTotalHWEncodeChannel + m_iTotalHWDecodeChannel) < 4)) //MAX Encode + Decode Channel
	{
		for (i = 0; i < MAX_BOARD_CHANNEL; i++)
		{
			//To find out the decode channel which suit to Enocde channel range depending on which chip in the board
			if((i / MAX_SUPPORT_BOARD) == (m_iDecodeChIdx / MAX_SUPPORT_BOARD)) //They are the Same Chip in the board
			{
				//The Encode channel already pass through to one Decode channel.
				if(((m_pMainDlg->m_pDVRSystem[m_pMainDlg->m_iEventOpenBoard]->m_pEncodeChParam[i]) &&
					(m_pMainDlg->m_pDVRSystem[m_pMainDlg->m_iEventOpenBoard]->m_pEncodeChParam[i]->m_iDecodeCHIdx != -1)))// ||
					//(i == m_iDecodeChIdx))
				{

				}
				else
				{
					sTemp.Format("Encode Channel%02d", i+1);
					m_cEncodeChComboBox.AddString(sTemp);
				}
			}
		}
	}

	int iTotalCnt = m_cEncodeChComboBox.GetCount();
	if(iTotalCnt == 0)
	{
		if(m_iTotalHWEncodeChannel == 2) //Already max two Encode & Two Decode Channel
		{
			GetDlgItem(IDC_STATIC_FILE_PATH)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(FALSE);

			GetDlgItem(IDOK)->EnableWindow(FALSE);
		}
		else
		{
			GetDlgItem(IDC_STATIC_FILE_PATH)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(TRUE);
		}

		GetDlgItem(IDC_RADIO_CAMERA)->EnableWindow(FALSE);
		GetDlgItem(IDC_COMBO_CAMERA)->EnableWindow(FALSE);
	}
	else
	{
		m_cEncodeChComboBox.SetCurSel(0);

		CString	sTemp;
				
		m_cEncodeChComboBox.GetLBText(0, sTemp);
		sTemp.Left(2);

		m_iSelEncodeCHIdx = atoi(sTemp.operator LPCTSTR());
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CDecodeSetupDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	if(m_bInputType == FALSE) //FileName
	{

	}
	else
	{
		int iTotalCnt = m_cEncodeChComboBox.GetCount();

		if(iTotalCnt > 0)
		{
			int iCurSel = m_cEncodeChComboBox.GetCurSel();

			if(iCurSel != -1)
			{
				CString	sTemp;
				
				m_cEncodeChComboBox.GetLBText(iCurSel, sTemp);
				sTemp = sTemp.Right(2);

				m_iSelEncodeCHIdx = atoi(sTemp.operator LPCTSTR())-1; //Index from '0'
			}
		}
	}

	OnOK();
}

void CDecodeSetupDlg::OnBnClickedRadioDecodeType() //0: HW Decode, 1: SW Decode
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);

	if(m_iTotalHWEncodeChannel == 2) //Already max two Encode & Two Decode Channel
	{
		GetDlgItem(IDC_STATIC_FILE_PATH)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_BROWSE)->EnableWindow(FALSE);

		GetDlgItem(IDOK)->EnableWindow(FALSE);

		GetDlgItem(IDC_RADIO_CAMERA)->EnableWindow(FALSE);
		GetDlgItem(IDC_COMBO_CAMERA)->EnableWindow(FALSE);
	}
	else
	{
		if(m_bDecodeType == FALSE) //HW Decode
		{
			GetDlgItem(IDC_RADIO_CAMERA)->EnableWindow(TRUE);
			GetDlgItem(IDC_COMBO_CAMERA)->EnableWindow(TRUE);
		}
		else //SW Decode
		{
			GetDlgItem(IDC_RADIO_CAMERA)->EnableWindow(FALSE);
			GetDlgItem(IDC_COMBO_CAMERA)->EnableWindow(FALSE);
		}
	}
}
