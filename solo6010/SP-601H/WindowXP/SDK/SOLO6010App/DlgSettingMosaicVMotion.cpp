// DlgSettingMosaicVMotion.cpp : implementation file
//

#include "stdafx.h"
#include "SOLO6010App.h"
#include "DlgSettingMosaicVMotion.h"
#include "WndLiveDisp.h"
#include "SOLO6010.h"
#include "SOLO6010AppDlg.h"
#include "DlgCtrlLive.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgSettingMosaicVMotion dialog


CDlgSettingMosaicVMotion::CDlgSettingMosaicVMotion(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSettingMosaicVMotion::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgSettingMosaicVMotion)
	m_VM_edCntMinDet = 0;
	//}}AFX_DATA_INIT

	m_VM_idxCurSelTh = NUM_DEF_V_MOTION_TH;
	m_ADVMD_idxCurSelTh = NUM_DEF_V_MOTION_TH;

	m_ADVMD_idxCurSelCh = 0;
}


void CDlgSettingMosaicVMotion::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgSettingMosaicVMotion)
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_BUT_OK, m_butAll[IDX_DSMVM_BUT_OK]);
	DDX_Control(pDX, IDC_BUT_CANCEL, m_butAll[IDX_DSMVM_BUT_CANCEL]);

	DDX_Control(pDX, IDC_MOSAIC_CH_B_0, m_butAll[IDX_DSMVM_BUT_MO_CH_0]);
	DDX_Control(pDX, IDC_MOSAIC_CH_B_1, m_butAll[IDX_DSMVM_BUT_MO_CH_1]);
	DDX_Control(pDX, IDC_MOSAIC_CH_B_2, m_butAll[IDX_DSMVM_BUT_MO_CH_2]);
	DDX_Control(pDX, IDC_MOSAIC_CH_B_3, m_butAll[IDX_DSMVM_BUT_MO_CH_3]);
	DDX_Control(pDX, IDC_MOSAIC_CH_B_4, m_butAll[IDX_DSMVM_BUT_MO_CH_4]);
	DDX_Control(pDX, IDC_MOSAIC_CH_B_5, m_butAll[IDX_DSMVM_BUT_MO_CH_5]);
	DDX_Control(pDX, IDC_MOSAIC_CH_B_6, m_butAll[IDX_DSMVM_BUT_MO_CH_6]);
	DDX_Control(pDX, IDC_MOSAIC_CH_B_7, m_butAll[IDX_DSMVM_BUT_MO_CH_7]);
	DDX_Control(pDX, IDC_MOSAIC_CH_B_8, m_butAll[IDX_DSMVM_BUT_MO_CH_8]);
	DDX_Control(pDX, IDC_MOSAIC_CH_B_9, m_butAll[IDX_DSMVM_BUT_MO_CH_9]);
	DDX_Control(pDX, IDC_MOSAIC_CH_B_10, m_butAll[IDX_DSMVM_BUT_MO_CH_10]);
	DDX_Control(pDX, IDC_MOSAIC_CH_B_11, m_butAll[IDX_DSMVM_BUT_MO_CH_11]);
	DDX_Control(pDX, IDC_MOSAIC_CH_B_12, m_butAll[IDX_DSMVM_BUT_MO_CH_12]);
	DDX_Control(pDX, IDC_MOSAIC_CH_B_13, m_butAll[IDX_DSMVM_BUT_MO_CH_13]);
	DDX_Control(pDX, IDC_MOSAIC_CH_B_14, m_butAll[IDX_DSMVM_BUT_MO_CH_14]);
	DDX_Control(pDX, IDC_MOSAIC_CH_B_15, m_butAll[IDX_DSMVM_BUT_MO_CH_15]);

	DDX_Control(pDX, IDC_MOSAIC_COM_STRENGTH, m_MOSAIC_comStrength);

	DDX_Control(pDX, IDC_VM_CH_B_0, m_butAll[IDX_DSMVM_BUT_VM_CH_0]);
	DDX_Control(pDX, IDC_VM_CH_B_1, m_butAll[IDX_DSMVM_BUT_VM_CH_1]);
	DDX_Control(pDX, IDC_VM_CH_B_2, m_butAll[IDX_DSMVM_BUT_VM_CH_2]);
	DDX_Control(pDX, IDC_VM_CH_B_3, m_butAll[IDX_DSMVM_BUT_VM_CH_3]);
	DDX_Control(pDX, IDC_VM_CH_B_4, m_butAll[IDX_DSMVM_BUT_VM_CH_4]);
	DDX_Control(pDX, IDC_VM_CH_B_5, m_butAll[IDX_DSMVM_BUT_VM_CH_5]);
	DDX_Control(pDX, IDC_VM_CH_B_6, m_butAll[IDX_DSMVM_BUT_VM_CH_6]);
	DDX_Control(pDX, IDC_VM_CH_B_7, m_butAll[IDX_DSMVM_BUT_VM_CH_7]);
	DDX_Control(pDX, IDC_VM_CH_B_8, m_butAll[IDX_DSMVM_BUT_VM_CH_8]);
	DDX_Control(pDX, IDC_VM_CH_B_9, m_butAll[IDX_DSMVM_BUT_VM_CH_9]);
	DDX_Control(pDX, IDC_VM_CH_B_10, m_butAll[IDX_DSMVM_BUT_VM_CH_10]);
	DDX_Control(pDX, IDC_VM_CH_B_11, m_butAll[IDX_DSMVM_BUT_VM_CH_11]);
	DDX_Control(pDX, IDC_VM_CH_B_12, m_butAll[IDX_DSMVM_BUT_VM_CH_12]);
	DDX_Control(pDX, IDC_VM_CH_B_13, m_butAll[IDX_DSMVM_BUT_VM_CH_13]);
	DDX_Control(pDX, IDC_VM_CH_B_14, m_butAll[IDX_DSMVM_BUT_VM_CH_14]);
	DDX_Control(pDX, IDC_VM_CH_B_15, m_butAll[IDX_DSMVM_BUT_VM_CH_15]);

	DDX_Control(pDX, IDC_VM_COM_CLR_BOX_Y, m_VM_bufComClr[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_Y]);
	DDX_Control(pDX, IDC_VM_COM_CLR_BOX_CB, m_VM_bufComClr[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CB]);
	DDX_Control(pDX, IDC_VM_COM_CLR_BOX_CR, m_VM_bufComClr[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CR]);
	DDX_Control(pDX, IDC_VM_COM_CLR_AREA_Y, m_VM_bufComClr[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_Y]);
	DDX_Control(pDX, IDC_VM_COM_CLR_AREA_CB, m_VM_bufComClr[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CB]);
	DDX_Control(pDX, IDC_VM_COM_CLR_AREA_CR, m_VM_bufComClr[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CR]);
	DDX_Text(pDX, IDC_VM_ED_CLR_BOX_Y, m_VM_bufEdClr[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_Y]);
	DDX_Text(pDX, IDC_VM_ED_CLR_BOX_CB, m_VM_bufEdClr[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CB]);
	DDX_Text(pDX, IDC_VM_ED_CLR_BOX_CR, m_VM_bufEdClr[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CR]);
	DDX_Text(pDX, IDC_VM_ED_CLR_AREA_Y, m_VM_bufEdClr[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_Y]);
	DDX_Text(pDX, IDC_VM_ED_CLR_AREA_CB, m_VM_bufEdClr[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CB]);
	DDX_Text(pDX, IDC_VM_ED_CLR_AREA_CR, m_VM_bufEdClr[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CR]);
	DDX_Control(pDX, IDC_VM_RAD_TH_0, m_VM_bufRadTh[0]);
	DDX_Control(pDX, IDC_VM_RAD_TH_1, m_VM_bufRadTh[1]);
	DDX_Control(pDX, IDC_VM_RAD_TH_2, m_VM_bufRadTh[2]);
	DDX_Control(pDX, IDC_VM_RAD_TH_3, m_VM_bufRadTh[3]);
	DDX_Control(pDX, IDC_VM_RAD_TH_4, m_VM_bufRadTh[4]);
	DDX_Control(pDX, IDC_VM_RAD_TH_5, m_VM_bufRadTh[5]);
	DDX_Control(pDX, IDC_VM_RAD_TH_6, m_VM_bufRadTh[6]);
	DDX_Control(pDX, IDC_VM_RAD_TH_7, m_VM_bufRadTh[7]);
	DDX_Control(pDX, IDC_VM_RAD_TH_8, m_VM_bufRadTh[8]);
	DDX_Control(pDX, IDC_VM_CH_B_SHOW_GRID, m_VM_cbutBShowGrid);
	DDX_Text(pDX, IDC_VM_ED_CNT_MIN_DET, m_VM_edCntMinDet);

	DDX_Control(pDX, IDC_VM_CBD_CH_B_0, m_butAll[IDX_DSMVM_BUT_CBD_CH_0]);
	DDX_Control(pDX, IDC_VM_CBD_CH_B_1, m_butAll[IDX_DSMVM_BUT_CBD_CH_1]);
	DDX_Control(pDX, IDC_VM_CBD_CH_B_2, m_butAll[IDX_DSMVM_BUT_CBD_CH_2]);
	DDX_Control(pDX, IDC_VM_CBD_CH_B_3, m_butAll[IDX_DSMVM_BUT_CBD_CH_3]);
	DDX_Control(pDX, IDC_VM_CBD_CH_B_4, m_butAll[IDX_DSMVM_BUT_CBD_CH_4]);
	DDX_Control(pDX, IDC_VM_CBD_CH_B_5, m_butAll[IDX_DSMVM_BUT_CBD_CH_5]);
	DDX_Control(pDX, IDC_VM_CBD_CH_B_6, m_butAll[IDX_DSMVM_BUT_CBD_CH_6]);
	DDX_Control(pDX, IDC_VM_CBD_CH_B_7, m_butAll[IDX_DSMVM_BUT_CBD_CH_7]);
	DDX_Control(pDX, IDC_VM_CBD_CH_B_8, m_butAll[IDX_DSMVM_BUT_CBD_CH_8]);
	DDX_Control(pDX, IDC_VM_CBD_CH_B_9, m_butAll[IDX_DSMVM_BUT_CBD_CH_9]);
	DDX_Control(pDX, IDC_VM_CBD_CH_B_10, m_butAll[IDX_DSMVM_BUT_CBD_CH_10]);
	DDX_Control(pDX, IDC_VM_CBD_CH_B_11, m_butAll[IDX_DSMVM_BUT_CBD_CH_11]);
	DDX_Control(pDX, IDC_VM_CBD_CH_B_12, m_butAll[IDX_DSMVM_BUT_CBD_CH_12]);
	DDX_Control(pDX, IDC_VM_CBD_CH_B_13, m_butAll[IDX_DSMVM_BUT_CBD_CH_13]);
	DDX_Control(pDX, IDC_VM_CBD_CH_B_14, m_butAll[IDX_DSMVM_BUT_CBD_CH_14]);
	DDX_Control(pDX, IDC_VM_CBD_CH_B_15, m_butAll[IDX_DSMVM_BUT_CBD_CH_15]);

	DDX_Control(pDX, IDC_ADVMD_CH_B_MOTION_0, m_butAll[IDX_DSMVM_BUT_AMD_CH_0]);
	DDX_Control(pDX, IDC_ADVMD_CH_B_MOTION_1, m_butAll[IDX_DSMVM_BUT_AMD_CH_1]);
	DDX_Control(pDX, IDC_ADVMD_CH_B_MOTION_2, m_butAll[IDX_DSMVM_BUT_AMD_CH_2]);
	DDX_Control(pDX, IDC_ADVMD_CH_B_MOTION_3, m_butAll[IDX_DSMVM_BUT_AMD_CH_3]);
	DDX_Control(pDX, IDC_ADVMD_CH_B_MOTION_4, m_butAll[IDX_DSMVM_BUT_AMD_CH_4]);
	DDX_Control(pDX, IDC_ADVMD_CH_B_MOTION_5, m_butAll[IDX_DSMVM_BUT_AMD_CH_5]);
	DDX_Control(pDX, IDC_ADVMD_CH_B_MOTION_6, m_butAll[IDX_DSMVM_BUT_AMD_CH_6]);
	DDX_Control(pDX, IDC_ADVMD_CH_B_MOTION_7, m_butAll[IDX_DSMVM_BUT_AMD_CH_7]);
	DDX_Control(pDX, IDC_ADVMD_CH_B_MOTION_8, m_butAll[IDX_DSMVM_BUT_AMD_CH_8]);
	DDX_Control(pDX, IDC_ADVMD_CH_B_MOTION_9, m_butAll[IDX_DSMVM_BUT_AMD_CH_9]);
	DDX_Control(pDX, IDC_ADVMD_CH_B_MOTION_10, m_butAll[IDX_DSMVM_BUT_AMD_CH_10]);
	DDX_Control(pDX, IDC_ADVMD_CH_B_MOTION_11, m_butAll[IDX_DSMVM_BUT_AMD_CH_11]);
	DDX_Control(pDX, IDC_ADVMD_CH_B_MOTION_12, m_butAll[IDX_DSMVM_BUT_AMD_CH_12]);
	DDX_Control(pDX, IDC_ADVMD_CH_B_MOTION_13, m_butAll[IDX_DSMVM_BUT_AMD_CH_13]);
	DDX_Control(pDX, IDC_ADVMD_CH_B_MOTION_14, m_butAll[IDX_DSMVM_BUT_AMD_CH_14]);
	DDX_Control(pDX, IDC_ADVMD_CH_B_MOTION_15, m_butAll[IDX_DSMVM_BUT_AMD_CH_15]);
	DDX_Control(pDX, IDC_ADVMD_COM_CHK_INTERVAL, m_ADVMD_comChkInterval);
	DDX_Text(pDX, IDC_ADVMD_ED_CNT_MIN_DET, m_ADVMD_edCntMinDet);
	DDX_Control(pDX, IDC_ADVMD_RAD_TH_0, m_ADVMD_bufRadTh[0]);
	DDX_Control(pDX, IDC_ADVMD_RAD_TH_1, m_ADVMD_bufRadTh[1]);
	DDX_Control(pDX, IDC_ADVMD_RAD_TH_2, m_ADVMD_bufRadTh[2]);
	DDX_Control(pDX, IDC_ADVMD_RAD_TH_3, m_ADVMD_bufRadTh[3]);
	DDX_Control(pDX, IDC_ADVMD_RAD_TH_4, m_ADVMD_bufRadTh[4]);
	DDX_Control(pDX, IDC_ADVMD_RAD_TH_5, m_ADVMD_bufRadTh[5]);
	DDX_Control(pDX, IDC_ADVMD_RAD_TH_6, m_ADVMD_bufRadTh[6]);
	DDX_Control(pDX, IDC_ADVMD_RAD_TH_7, m_ADVMD_bufRadTh[7]);
	DDX_Control(pDX, IDC_ADVMD_RAD_TH_8, m_ADVMD_bufRadTh[8]);
	DDX_Control(pDX, IDC_ADVMD_RAD_NUM_CHK_ITEM_0, m_ADVMD_bufRadNumChkItem[0]);
	DDX_Control(pDX, IDC_ADVMD_RAD_NUM_CHK_ITEM_1, m_ADVMD_bufRadNumChkItem[1]);
	DDX_Control(pDX, IDC_ADVMD_RAD_NUM_CHK_ITEM_2, m_ADVMD_bufRadNumChkItem[2]);
	DDX_Control(pDX, IDC_ADVMD_RAD_NUM_CHK_ITEM_3, m_ADVMD_bufRadNumChkItem[3]);
	DDX_Control(pDX, IDC_ADVMD_RAD_CUR_CHK_ITEM_0, m_ADVMD_bufRadCurChkItem[0]);
	DDX_Control(pDX, IDC_ADVMD_RAD_CUR_CHK_ITEM_1, m_ADVMD_bufRadCurChkItem[1]);
	DDX_Control(pDX, IDC_ADVMD_RAD_CUR_CHK_ITEM_2, m_ADVMD_bufRadCurChkItem[2]);
	DDX_Control(pDX, IDC_ADVMD_RAD_CUR_CHK_ITEM_3, m_ADVMD_bufRadCurChkItem[3]);
	DDX_Control(pDX, IDC_ADVMD_CH_B_SHOW_GRID, m_ADVMD_cbutBShowGrid);
}


BEGIN_MESSAGE_MAP(CDlgSettingMosaicVMotion, CDialog)
	ON_MESSAGE(WM_DOO_INDEX_BUTTON_CLICK, OnIndexButtonClick)
	ON_MESSAGE (WM_DOO_LIVE_DISP_WND_CLICK, OnDispWndClick)
	//{{AFX_MSG_MAP(CDlgSettingMosaicVMotion)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_WM_NCHITTEST()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_VM_CH_B_SHOW_GRID, OnVmChBShowGrid)
	ON_BN_CLICKED(IDC_VM_RAD_TH_0, OnVmRadTh0)
	ON_BN_CLICKED(IDC_VM_RAD_TH_1, OnVmRadTh1)
	ON_BN_CLICKED(IDC_VM_RAD_TH_2, OnVmRadTh2)
	ON_BN_CLICKED(IDC_VM_RAD_TH_3, OnVmRadTh3)
	ON_BN_CLICKED(IDC_VM_RAD_TH_4, OnVmRadTh4)
	ON_BN_CLICKED(IDC_VM_RAD_TH_5, OnVmRadTh5)
	ON_BN_CLICKED(IDC_VM_RAD_TH_6, OnVmRadTh6)
	ON_BN_CLICKED(IDC_VM_RAD_TH_7, OnVmRadTh7)
	ON_BN_CLICKED(IDC_VM_RAD_TH_8, OnVmRadTh8)
	ON_CBN_SELCHANGE(IDC_VM_COM_CLR_BOX_Y, OnSelchangeVmComClrBoxY)
	ON_CBN_SELCHANGE(IDC_VM_COM_CLR_BOX_CR, OnSelchangeVmComClrBoxCr)
	ON_CBN_SELCHANGE(IDC_VM_COM_CLR_BOX_CB, OnSelchangeVmComClrBoxCb)
	ON_CBN_SELCHANGE(IDC_VM_COM_CLR_AREA_Y, OnSelchangeVmComClrAreaY)
	ON_CBN_SELCHANGE(IDC_VM_COM_CLR_AREA_CR, OnSelchangeVmComClrAreaCr)
	ON_CBN_SELCHANGE(IDC_VM_COM_CLR_AREA_CB, OnSelchangeVmComClrAreaCb)
	ON_EN_CHANGE(IDC_VM_ED_CLR_BOX_Y, OnChangeVmEdClrBoxY)
	ON_EN_CHANGE(IDC_VM_ED_CLR_BOX_CR, OnChangeVmEdClrBoxCr)
	ON_EN_CHANGE(IDC_VM_ED_CLR_BOX_CB, OnChangeVmEdClrBoxCb)
	ON_EN_CHANGE(IDC_VM_ED_CLR_AREA_Y, OnChangeVmEdClrAreaY)
	ON_EN_CHANGE(IDC_VM_ED_CLR_AREA_CR, OnChangeVmEdClrAreaCr)
	ON_EN_CHANGE(IDC_VM_ED_CLR_AREA_CB, OnChangeVmEdClrAreaCb)
	ON_BN_CLICKED(IDC_VM_CH_B_EXEC_SET_TH, OnVmChBExecSetTh)
	ON_EN_CHANGE(IDC_VM_ED_CNT_MIN_DET, OnChangeVmEdCntMinDet)
	ON_CBN_SELCHANGE(IDC_MOSAIC_COM_STRENGTH, OnSelchangeMosaicComStrength)
	ON_BN_CLICKED(IDC_ADVMD_CH_B_EXEC_SET_TH, OnAdvmdChBExecSetTh)
	ON_CBN_SELCHANGE(IDC_ADVMD_COM_CHK_INTERVAL, OnSelchangeAdvmdComChkInterval)
	ON_EN_CHANGE(IDC_ADVMD_ED_CNT_MIN_DET, OnChangeAdvmdEdCntMinDet)
	ON_BN_CLICKED(IDC_ADVMD_RAD_TH_0, OnAdvmdRadTh0)
	ON_BN_CLICKED(IDC_ADVMD_RAD_TH_1, OnAdvmdRadTh1)
	ON_BN_CLICKED(IDC_ADVMD_RAD_TH_2, OnAdvmdRadTh2)
	ON_BN_CLICKED(IDC_ADVMD_RAD_TH_3, OnAdvmdRadTh3)
	ON_BN_CLICKED(IDC_ADVMD_RAD_TH_4, OnAdvmdRadTh4)
	ON_BN_CLICKED(IDC_ADVMD_RAD_TH_5, OnAdvmdRadTh5)
	ON_BN_CLICKED(IDC_ADVMD_RAD_TH_6, OnAdvmdRadTh6)
	ON_BN_CLICKED(IDC_ADVMD_RAD_TH_7, OnAdvmdRadTh7)
	ON_BN_CLICKED(IDC_ADVMD_RAD_TH_8, OnAdvmdRadTh8)
	ON_BN_CLICKED(IDC_ADVMD_RAD_NUM_CHK_ITEM_0, OnAdvmdRadNumChkItem0)
	ON_BN_CLICKED(IDC_ADVMD_RAD_NUM_CHK_ITEM_1, OnAdvmdRadNumChkItem1)
	ON_BN_CLICKED(IDC_ADVMD_RAD_NUM_CHK_ITEM_2, OnAdvmdRadNumChkItem2)
	ON_BN_CLICKED(IDC_ADVMD_RAD_NUM_CHK_ITEM_3, OnAdvmdRadNumChkItem3)
	ON_BN_CLICKED(IDC_ADVMD_RAD_CUR_CHK_ITEM_0, OnAdvmdRadCurChkItem0)
	ON_BN_CLICKED(IDC_ADVMD_RAD_CUR_CHK_ITEM_1, OnAdvmdRadCurChkItem1)
	ON_BN_CLICKED(IDC_ADVMD_RAD_CUR_CHK_ITEM_2, OnAdvmdRadCurChkItem2)
	ON_BN_CLICKED(IDC_ADVMD_RAD_CUR_CHK_ITEM_3, OnAdvmdRadCurChkItem3)
	ON_BN_CLICKED(IDC_ADVMD_CH_B_SHOW_GRID, OnAdvmdChBShowGrid)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgSettingMosaicVMotion message handlers

BOOL CDlgSettingMosaicVMotion::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	int i, j;

	for (i=0; i<NUM_DSMVM_BUT; i++)
	{
		m_butAll[i].SetID (i);
	}

	char strTmp[MAX_PATH];

	// Mosaic[S]
	for (i=MIN_MOSAIC_STRENGTH; i<=MAX_MOSAIC_STRENGTH; i++)
	{
		sprintf (strTmp, "%2d", i);
		m_MOSAIC_comStrength.AddString (strTmp);
	}
	m_MOSAIC_comStrength.SetCurSel (glPCurGInfo->MOSAIC_nStrength);
	// Mosaic[E]

	// Video Motion[S]
	for (i=0; i<2 *NUM_VM_CLR; i++)
	{
		for (j=0; j<NUM_VM_GRID_CLR; j++)
		{
			m_VM_bufComClr[i /NUM_VM_CLR][i %NUM_VM_CLR].AddString (GL_COM_STR_IDX_VM_GRID_CLR[j]);
		}
	}
	// Video Motion[E]

	// Advanced Motion Detection[S]
	for (i=0; i<MAX_MOT_DATA_DISTANCE -1; i++)
	{
		sprintf (strTmp, "%d /15 sec", i +1);
		m_ADVMD_comChkInterval.AddString (strTmp);
	}
	// Advanced Motion Detection[E]

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgSettingMosaicVMotion::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	
}

void CDlgSettingMosaicVMotion::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
	// TODO: Add your message handler code here

	if (bShow == TRUE)
	{
		// Mosaic Info Set[S]
		memcpy (&m_MOSAIC_bufTmpBSet[0], &glPCurGInfo->MOSAIC_bufBSet[0], sizeof(BOOL) *NUM_LIVE_DISP_WND);
		memcpy (&m_MOSAIC_bufTmpRc[0], &glPCurGInfo->MOSAIC_bufRect[0], sizeof(RECT) *NUM_LIVE_DISP_WND);

		SetDlgItemState_Mosaic ();
		// Mosaic Info Set[E]

		SetDlgItemState_VM ();	// Video Motion Info Set

		SetDlgItemState_ADVMD ();	// Advanced Motion Detection

		EnableDlgItemFromMode ();
		
		glPWndLiveDisp->Invalidate ();
	}
}

void CDlgSettingMosaicVMotion::SetDlgItemState_Mosaic ()
{
	int i;
	for (i=IDX_DSMVM_BUT_MO_CH_0; i<=IDX_DSMVM_BUT_MO_CH_15; i++)
	{
		m_butAll[i].SetCheck (glPCurGInfo->MOSAIC_bufBSet[i -IDX_DSMVM_BUT_MO_CH_0]);
	}
}

void CDlgSettingMosaicVMotion::SetDlgItemState_VM ()
{
	int i;

	UpdateData (TRUE);
	
	for (i=IDX_DSMVM_BUT_VM_CH_0; i<=IDX_DSMVM_BUT_VM_CH_15; i++)
	{
		m_butAll[i].SetCheck (glPCurSelS6010->VMOTION_GetOnOff (i -IDX_DSMVM_BUT_VM_CH_0));
		m_butAll[IDX_DSMVM_BUT_AMD_CH_0 +i -IDX_DSMVM_BUT_VM_CH_0].EnableWindow (glPCurSelS6010->VMOTION_GetOnOff (i -IDX_DSMVM_BUT_VM_CH_0));
	}

	if (glPCurSelS6010->VMOTION_GetGridOnOff () == TRUE)
	{
		m_VM_cbutBShowGrid.SetCheck (TRUE);
	}
	else
	{
		m_VM_cbutBShowGrid.SetCheck (FALSE);
	}

	BYTE idxCModeY, idxCModeCb, idxCModeCr, valY, valCb, valCr;

	glPCurSelS6010->VMOTION_GetBorderColor (idxCModeY, valY, idxCModeCb, valCb, idxCModeCr, valCr);
	
	m_VM_bufComClr[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_Y].SetCurSel (idxCModeY);
	m_VM_bufEdClr[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_Y] = valY;
	m_VM_bufComClr[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CB].SetCurSel (idxCModeCb);
	m_VM_bufEdClr[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CB] = valCb;
	m_VM_bufComClr[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CR].SetCurSel (idxCModeCr);
	m_VM_bufEdClr[IDX_VM_GRID_ITEM_BORDER][IDX_VM_CLR_CR] = valCr;
	
	m_VM_bufComClr[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_Y].SetCurSel (idxCModeY);
	m_VM_bufEdClr[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_Y] = valY;
	m_VM_bufComClr[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CB].SetCurSel (idxCModeCb);
	m_VM_bufEdClr[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CB] = valCb;
	m_VM_bufComClr[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CR].SetCurSel (idxCModeCr);
	m_VM_bufEdClr[IDX_VM_GRID_ITEM_BAR][IDX_VM_CLR_CR] = valCr;

	if (glPCurGInfo->VM_bExecThSetting == TRUE)
	{
		((CButton *)GetDlgItem (IDC_VM_CH_B_EXEC_SET_TH))->SetCheck (TRUE);
	}
	else
	{
		((CButton *)GetDlgItem (IDC_VM_CH_B_EXEC_SET_TH))->SetCheck (FALSE);
	}

	for (i=0; i<NUM_VM_TH; i++)
	{
		m_VM_bufRadTh[i].SetCheck (FALSE);
	}
	m_VM_bufRadTh[GetVMThIdxFromRealTh (m_VM_idxCurSelTh)].SetCheck (TRUE);

	m_VM_edCntMinDet = glPCurSelS6010->VMOTION_GetMinDetectThreshold ();

	for (i=IDX_DSMVM_BUT_CBD_CH_0; i<=IDX_DSMVM_BUT_CBD_CH_15; i++)
	{
		m_butAll[i].SetCheck (glPCurSelS6010->CAM_BLK_DET_GetOnOff (i -IDX_DSMVM_BUT_CBD_CH_0));
	}

	UpdateData (FALSE);
}

void CDlgSettingMosaicVMotion::SetDlgItemState_ADVMD ()
{
	int i;

	UpdateData (TRUE);

	for (i=IDX_DSMVM_BUT_AMD_CH_0; i<=IDX_DSMVM_BUT_AMD_CH_15; i++)
	{
		m_butAll[i].SetCheck (glPCurGInfo->ADVMD_bufBUse[i -IDX_DSMVM_BUT_AMD_CH_0]);
	}

	if (glPCurGInfo->ADVMD_bExecThSetting == TRUE)
	{
		((CButton *)GetDlgItem (IDC_ADVMD_CH_B_EXEC_SET_TH))->SetCheck (TRUE);
	}
	else
	{
		((CButton *)GetDlgItem (IDC_ADVMD_CH_B_EXEC_SET_TH))->SetCheck (FALSE);
	}

	for (i=0; i<NUM_VM_TH; i++)
	{
		m_ADVMD_bufRadTh[i].SetCheck (FALSE);
	}
	m_ADVMD_bufRadTh[GetVMThIdxFromRealTh (m_ADVMD_idxCurSelTh)].SetCheck (TRUE);

	for (i=0; i<MAX_ADV_MOT_CHECK_ITEM; i++)
	{
		m_ADVMD_bufRadNumChkItem[i].SetCheck (FALSE);
		m_ADVMD_bufRadCurChkItem[i].SetCheck (FALSE);
	}
	m_ADVMD_bufRadNumChkItem[glPCurGInfo->ADVMD_bufNumMotionCheck[m_ADVMD_idxCurSelCh] -1].SetCheck (TRUE);
	m_ADVMD_bufRadCurChkItem[glPCurGInfo->ADVMD_idxCurSelCheckItem].SetCheck (TRUE);

	m_ADVMD_edCntMinDet = glPCurGInfo->ADVMD_bufCntMinDet[m_ADVMD_idxCurSelCh];

	m_ADVMD_comChkInterval.SetCurSel (glPCurGInfo->ADVMD_bbufMotDataDist[m_ADVMD_idxCurSelCh][glPCurGInfo->ADVMD_idxCurSelCheckItem] -1);

	if (glPCurGInfo->ADVMD_bShowGrid == TRUE)
	{
		m_ADVMD_cbutBShowGrid.SetCheck (TRUE);
	}
	else
	{
		m_ADVMD_cbutBShowGrid.SetCheck (FALSE);
	}

	UpdateData (FALSE);
}

UINT CDlgSettingMosaicVMotion::OnNcHitTest(CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	return HTCAPTION;
	return CDialog::OnNcHitTest(point);
}

LRESULT CDlgSettingMosaicVMotion::OnIndexButtonClick (WPARAM wParam, LPARAM lParam)
{
	if (wParam <= IDX_DSMVM_BUT_MO_CH_15)
	{
		if (glPCurGInfo->idxModeSetMosaic != IDX_SETM_MODE_MOSAIC ||
			wParam -IDX_DSMVM_BUT_MO_CH_0 != glPCurSelS6010->LIVE_GetChFromIdxWnd (0))
		{
			glPCurGInfo->idxModeSetMosaic = IDX_SETM_MODE_MOSAIC;

			glPDlgCtrlLive->SendMessage (WM_DOO_INDEX_BUTTON_CLICK, IDX_DCL_BUT_CH_0 +wParam -IDX_DSMVM_BUT_MO_CH_0, 0);

			EnableDlgItemFromMode ();
			SetDlgItemState_Mosaic ();
			
			Invalidate ();	// for mosaic range display
		}
		else
		{
			int idxWnd = wParam -IDX_DSMVM_BUT_MO_CH_0;

			glPCurGInfo->MOSAIC_bufBSet[idxWnd] = 1 -glPCurGInfo->MOSAIC_bufBSet[idxWnd];

			glPWndLiveDisp->Invalidate ();	// for mosaic range display
			glPCurSelS6010->MOSAIC_SetOnOffAndRect (idxWnd, glPCurGInfo->MOSAIC_bufBSet[idxWnd], &glPCurGInfo->MOSAIC_bufRect[idxWnd]);
		}
	}
	else if (wParam <= IDX_DSMVM_BUT_VM_CH_15)
	{
		if (glPCurGInfo->idxModeSetMosaic != IDX_SETM_MODE_V_MOTION ||
			wParam -IDX_DSMVM_BUT_VM_CH_0 != glPCurSelS6010->LIVE_GetChFromIdxWnd (0))
		{
			glPCurGInfo->idxModeSetMosaic = IDX_SETM_MODE_V_MOTION;

			glPDlgCtrlLive->SendMessage (WM_DOO_INDEX_BUTTON_CLICK, IDX_DCL_BUT_CH_0 +wParam -IDX_DSMVM_BUT_VM_CH_0, 0);

			EnableDlgItemFromMode ();
			SetDlgItemState_VM ();

			Invalidate ();	// for v-motion range display
		}
		else
		{
			BYTE idxCh = wParam -IDX_DSMVM_BUT_VM_CH_0;
			glPCurGInfo->VM_bufBUse[idxCh] = 1 -glPCurGInfo->VM_bufBUse[idxCh];
			glPCurSelS6010->VMOTION_SetOnOff (idxCh, glPCurGInfo->VM_bufBUse[idxCh]);

			SetDlgItemState_VM ();

			glPWndLiveDisp->Invalidate ();	// for v-motion threshold display
		}
	}
	else if (wParam <= IDX_DSMVM_BUT_CBD_CH_15)
	{
		BYTE idxCh = wParam -IDX_DSMVM_BUT_CBD_CH_0;
		glPCurGInfo->CBD_bufBUseCBD[idxCh] = 1 -glPCurGInfo->CBD_bufBUseCBD[idxCh];

		if (glPCurGInfo->CBD_bufBUseCBD[idxCh] == 1 && glPCurGInfo->VM_bufBUse[idxCh] == 0)
		{
			glPCurGInfo->CBD_bufBUseCBD[idxCh] = 0;
		}
		glPCurSelS6010->CAM_BLK_DET_SetOnOff (idxCh, glPCurGInfo->CBD_bufBUseCBD[idxCh]);

		SetDlgItemState_VM ();
	}
	else if (wParam <= IDX_DSMVM_BUT_AMD_CH_15)
	{
		m_ADVMD_idxCurSelCh = wParam -IDX_DSMVM_BUT_AMD_CH_0;

		if (glPCurGInfo->idxModeSetMosaic != IDX_SETM_MODE_ADV_MOT_DET ||
			m_ADVMD_idxCurSelCh != glPCurSelS6010->LIVE_GetChFromIdxWnd (0))
		{
			glPCurGInfo->idxModeSetMosaic = IDX_SETM_MODE_ADV_MOT_DET;

			glPDlgCtrlLive->SendMessage (WM_DOO_INDEX_BUTTON_CLICK, IDX_DCL_BUT_CH_0 +m_ADVMD_idxCurSelCh, 0);

			m_butAll[wParam].SetCheck (glPCurGInfo->ADVMD_bufBUse[m_ADVMD_idxCurSelCh]);

			EnableDlgItemFromMode ();
			SetDlgItemState_ADVMD ();
			
			Invalidate ();	// for v-motion range display
		}
		else
		{
			glPCurGInfo->ADVMD_bufBUse[m_ADVMD_idxCurSelCh] = 1 -glPCurGInfo->ADVMD_bufBUse[m_ADVMD_idxCurSelCh];
			glPCurSelS6010->ADV_MOT_SetOnOff (m_ADVMD_idxCurSelCh, glPCurGInfo->ADVMD_bufBUse[m_ADVMD_idxCurSelCh]);

			glPWndLiveDisp->Invalidate ();	// for v-motion threshold display
		}
	}
	else if (wParam == IDX_DSMVM_BUT_OK)
	{
		glPDlgApp->SendMessage (WM_DOO_APP_DLG_CTRL, IDX_APP_MSG_ON_BUT_LIVE, 0);
	}
	else if (wParam == IDX_DSMVM_BUT_CANCEL)
	{
		int i;

		memcpy (&glPCurGInfo->MOSAIC_bufBSet[0], &m_MOSAIC_bufTmpBSet[0], sizeof(BOOL) *NUM_LIVE_DISP_WND);
		memcpy (&glPCurGInfo->MOSAIC_bufRect[0], &m_MOSAIC_bufTmpRc[0], sizeof(RECT) *NUM_LIVE_DISP_WND);
		
		for (i=0; i<NUM_LIVE_DISP_WND; i++)
		{
			glPCurSelS6010->MOSAIC_SetOnOffAndRect (i, glPCurGInfo->MOSAIC_bufBSet[i], &glPCurGInfo->MOSAIC_bufRect[i]);
		}

		glPDlgApp->SendMessage (WM_DOO_APP_DLG_CTRL, IDX_APP_MSG_ON_BUT_LIVE, 0);
	}

	return 0;
}

void CDlgSettingMosaicVMotion::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	CRect rcSelBut;
	if (glPCurGInfo->idxModeSetMosaic == IDX_SETM_MODE_MOSAIC)
	{
		m_butAll[IDX_DSMVM_BUT_MO_CH_0 +glPCurSelS6010->LIVE_GetChFromIdxWnd (0)]
			.GetWindowRect (&rcSelBut);
	}
	else if (glPCurGInfo->idxModeSetMosaic == IDX_SETM_MODE_V_MOTION)
	{
		m_butAll[IDX_DSMVM_BUT_VM_CH_0 +glPCurSelS6010->LIVE_GetChFromIdxWnd (0)]
			.GetWindowRect (&rcSelBut);
	}
	else if (glPCurGInfo->idxModeSetMosaic == IDX_SETM_MODE_ADV_MOT_DET)
	{
		m_butAll[IDX_DSMVM_BUT_AMD_CH_0 +glPCurSelS6010->LIVE_GetChFromIdxWnd (0)]
			.GetWindowRect (&rcSelBut);
	}
	ScreenToClient (&rcSelBut);

	rcSelBut.InflateRect (2, 2);
	dc.FillSolidRect (&rcSelBut, RGB(255, 0, 0));

	// Do not call CDialog::OnPaint() for painting messages
}

LRESULT CDlgSettingMosaicVMotion::OnDispWndClick (WPARAM wParam, LPARAM lParam)
{
	if (glPCurGInfo->idxModeSetMosaic == IDX_SETM_MODE_MOSAIC)
	{
		int idxCh = glPCurSelS6010->LIVE_GetChFromIdxWnd (0);

		glPCurSelS6010->MOSAIC_SetOnOffAndRect (idxCh, glPCurGInfo->MOSAIC_bufBSet[idxCh],
			&glPCurGInfo->MOSAIC_bufRect[idxCh]);
	}
	else if (glPCurGInfo->idxModeSetMosaic == IDX_SETM_MODE_V_MOTION)
	{
		int idxCh = glPCurSelS6010->LIVE_GetChFromIdxWnd (0);

		if (glPCurSelS6010->VMOTION_GetOnOff (idxCh) == TRUE)
		{
			int i, j, hStart, vStart, hEnd, vEnd;

			hStart = glPCurGInfo->VM_rcCurSelTh.left /16;
			hEnd = glPCurGInfo->VM_rcCurSelTh.right /16;
			vStart = glPCurGInfo->VM_rcCurSelTh.top /16;
			vEnd = glPCurGInfo->VM_rcCurSelTh.bottom /16;

			for (i=vStart; i<=vEnd; i++)
			{
				for (j=hStart; j<=hEnd; j++)
				{
					glPCurGInfo->VM_bbufTh[idxCh][i *(1024 /16) +j] = m_VM_idxCurSelTh;
				}
			}
			glPCurSelS6010->VMOTION_SetThreshold (idxCh, glPCurGInfo->VM_bbufTh[idxCh]);

			glPCurGInfo->VM_rcCurSelTh.left = 0;
			glPCurGInfo->VM_rcCurSelTh.right = 0;
			glPCurGInfo->VM_rcCurSelTh.top = 0;
			glPCurGInfo->VM_rcCurSelTh.bottom = 0;

			glPWndLiveDisp->Invalidate ();	// for v-motion threshold display
		}
	}
	else if (glPCurGInfo->idxModeSetMosaic == IDX_SETM_MODE_ADV_MOT_DET)
	{
		int idxCh = glPCurSelS6010->LIVE_GetChFromIdxWnd (0);

		if (glPCurGInfo->ADVMD_bufBUse[idxCh] == TRUE)
		{
			int i, j, hStart, vStart, hEnd, vEnd;

			hStart = glPCurGInfo->ADVMD_rcCurSelTh.left /16;
			hEnd = glPCurGInfo->ADVMD_rcCurSelTh.right /16;
			vStart = glPCurGInfo->ADVMD_rcCurSelTh.top /16;
			vEnd = glPCurGInfo->ADVMD_rcCurSelTh.bottom /16;

			for (i=vStart; i<=vEnd; i++)
			{
				for (j=hStart; j<=hEnd; j++)
				{
					glPCurGInfo->ADVMD_bbbufMotionTh[idxCh][glPCurGInfo->ADVMD_idxCurSelCheckItem][i *(1024 /16) +j] = m_ADVMD_idxCurSelTh;
				}
			}
			glPCurSelS6010->ADV_MOT_SetThreshold (idxCh, glPCurGInfo->ADVMD_idxCurSelCheckItem,
				glPCurGInfo->ADVMD_bbbufMotionTh[idxCh][glPCurGInfo->ADVMD_idxCurSelCheckItem]);

			glPCurGInfo->ADVMD_rcCurSelTh.left = 0;
			glPCurGInfo->ADVMD_rcCurSelTh.right = 0;
			glPCurGInfo->ADVMD_rcCurSelTh.top = 0;
			glPCurGInfo->ADVMD_rcCurSelTh.bottom = 0;

			glPWndLiveDisp->Invalidate ();
		}
	}
	
	return 0;
}

void CDlgSettingMosaicVMotion::OnVmChBShowGrid() 
{
	// TODO: Add your control notification handler code here
	
	glPCurGInfo->VM_bShowGrid = 1 -glPCurGInfo->VM_bShowGrid;
	glPCurSelS6010->VMOTION_SetGridOnOff (glPCurGInfo->VM_bShowGrid);
}

void CDlgSettingMosaicVMotion::OnVmRadTh0() 
{
	VM_ChangeCurSelTh (IDX_VM_TH_0);
}
void CDlgSettingMosaicVMotion::OnVmRadTh1() 
{
	VM_ChangeCurSelTh (IDX_VM_TH_1);
}
void CDlgSettingMosaicVMotion::OnVmRadTh2() 
{
	VM_ChangeCurSelTh (IDX_VM_TH_2);
}
void CDlgSettingMosaicVMotion::OnVmRadTh3() 
{
	VM_ChangeCurSelTh (IDX_VM_TH_3);
}
void CDlgSettingMosaicVMotion::OnVmRadTh4() 
{
	VM_ChangeCurSelTh (IDX_VM_TH_4);
}
void CDlgSettingMosaicVMotion::OnVmRadTh5() 
{
	VM_ChangeCurSelTh (IDX_VM_TH_5);
}
void CDlgSettingMosaicVMotion::OnVmRadTh6() 
{
	VM_ChangeCurSelTh (IDX_VM_TH_6);
}
void CDlgSettingMosaicVMotion::OnVmRadTh7() 
{
	VM_ChangeCurSelTh (IDX_VM_TH_7);
}
void CDlgSettingMosaicVMotion::OnVmRadTh8() 
{
	VM_ChangeCurSelTh (IDX_VM_TH_8);
}
void CDlgSettingMosaicVMotion::VM_ChangeCurSelTh (int idxMotionTh)
{
	m_VM_idxCurSelTh = GL_VM_TH_IDX_TO_VAL[idxMotionTh];

	SetDlgItemState_VM ();
}

void CDlgSettingMosaicVMotion::OnSelchangeVmComClrBoxY() 
{
	VM_SetColorMode (IDX_VM_GRID_ITEM_BORDER, IDX_VM_CLR_Y);
}
void CDlgSettingMosaicVMotion::OnSelchangeVmComClrBoxCr() 
{
	VM_SetColorMode (IDX_VM_GRID_ITEM_BORDER, IDX_VM_CLR_CR);
}
void CDlgSettingMosaicVMotion::OnSelchangeVmComClrBoxCb() 
{
	VM_SetColorMode (IDX_VM_GRID_ITEM_BORDER, IDX_VM_CLR_CB);
}
void CDlgSettingMosaicVMotion::OnSelchangeVmComClrAreaY() 
{
	VM_SetColorMode (IDX_VM_GRID_ITEM_BAR, IDX_VM_CLR_Y);
}
void CDlgSettingMosaicVMotion::OnSelchangeVmComClrAreaCr() 
{
	VM_SetColorMode (IDX_VM_GRID_ITEM_BAR, IDX_VM_CLR_CR);
}
void CDlgSettingMosaicVMotion::OnSelchangeVmComClrAreaCb() 
{
	VM_SetColorMode (IDX_VM_GRID_ITEM_BAR, IDX_VM_CLR_CB);
}

void CDlgSettingMosaicVMotion::VM_SetColorMode (int idxItem, int idxColor)
{
	if (idxItem == IDX_VM_GRID_ITEM_BORDER)
	{
		switch (idxColor)
		{
		case IDX_VM_CLR_Y:
			glPCurGInfo->VM_clrModeBorderY = m_VM_bufComClr[idxItem][idxColor].GetCurSel ();
			glPCurSelS6010->VMOTION_SetBorderColor (
				glPCurGInfo->VM_clrModeBorderY, VMOTION_NOT_CHANGE_PROP,
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP,
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP);
			break;
		case IDX_VM_CLR_CB:
			glPCurGInfo->VM_clrModeBorderCb = m_VM_bufComClr[idxItem][idxColor].GetCurSel ();
			glPCurSelS6010->VMOTION_SetBorderColor (
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP,
				glPCurGInfo->VM_clrModeBorderCb, VMOTION_NOT_CHANGE_PROP,
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP);
			break;
		case IDX_VM_CLR_CR:
			glPCurGInfo->VM_clrModeBorderCr = m_VM_bufComClr[idxItem][idxColor].GetCurSel ();
			glPCurSelS6010->VMOTION_SetBorderColor (
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP,
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP,
				glPCurGInfo->VM_clrModeBorderCr, VMOTION_NOT_CHANGE_PROP);
			break;
		}
	}
	else	// IDX_VM_GRID_ITEM_BAR
	{
		switch (idxColor)
		{
		case IDX_VM_CLR_Y:
			glPCurGInfo->VM_clrModeBarY = m_VM_bufComClr[idxItem][idxColor].GetCurSel ();
			glPCurSelS6010->VMOTION_SetBarColor (
				glPCurGInfo->VM_clrModeBarY, VMOTION_NOT_CHANGE_PROP,
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP,
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP);
			break;
		case IDX_VM_CLR_CB:
			glPCurGInfo->VM_clrModeBarCb = m_VM_bufComClr[idxItem][idxColor].GetCurSel ();
			glPCurSelS6010->VMOTION_SetBarColor (
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP,
				glPCurGInfo->VM_clrModeBarCb, VMOTION_NOT_CHANGE_PROP,
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP);
			break;
		case IDX_VM_CLR_CR:
			glPCurGInfo->VM_clrModeBarCr = m_VM_bufComClr[idxItem][idxColor].GetCurSel ();
			glPCurSelS6010->VMOTION_SetBarColor (
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP,
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP,
				glPCurGInfo->VM_clrModeBarCr, VMOTION_NOT_CHANGE_PROP);
			break;
		}
	}
}

void CDlgSettingMosaicVMotion::OnChangeVmEdClrBoxY() 
{
	VM_SetColorData (IDX_VM_GRID_ITEM_BORDER, IDX_VM_CLR_Y);
}
void CDlgSettingMosaicVMotion::OnChangeVmEdClrBoxCr() 
{
	VM_SetColorData (IDX_VM_GRID_ITEM_BORDER, IDX_VM_CLR_CR);
}
void CDlgSettingMosaicVMotion::OnChangeVmEdClrBoxCb() 
{
	VM_SetColorData (IDX_VM_GRID_ITEM_BORDER, IDX_VM_CLR_CB);
}
void CDlgSettingMosaicVMotion::OnChangeVmEdClrAreaY() 
{
	VM_SetColorData (IDX_VM_GRID_ITEM_BAR, IDX_VM_CLR_Y);
}
void CDlgSettingMosaicVMotion::OnChangeVmEdClrAreaCr() 
{
	VM_SetColorData (IDX_VM_GRID_ITEM_BAR, IDX_VM_CLR_CR);
}
void CDlgSettingMosaicVMotion::OnChangeVmEdClrAreaCb() 
{
	VM_SetColorData (IDX_VM_GRID_ITEM_BAR, IDX_VM_CLR_CB);
}

void CDlgSettingMosaicVMotion::VM_SetColorData (int idxItem, int idxColor)
{
	UpdateData (TRUE);

	if (m_VM_bufEdClr[idxItem][idxColor] < 0)
	{
		m_VM_bufEdClr[idxItem][idxColor] = 0;
	}
	if (m_VM_bufEdClr[idxItem][idxColor] > 255)
	{
		m_VM_bufEdClr[idxItem][idxColor] = 255;
	}

	if (idxItem == IDX_VM_GRID_ITEM_BORDER)
	{
		switch (idxColor)
		{
		case IDX_VM_CLR_Y:
			glPCurGInfo->VM_clrValBorderY = m_VM_bufEdClr[idxItem][idxColor];
			glPCurSelS6010->VMOTION_SetBorderColor (
				VMOTION_NOT_CHANGE_PROP, glPCurGInfo->VM_clrValBorderY,
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP,
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP);
			break;
		case IDX_VM_CLR_CB:
			glPCurGInfo->VM_clrValBorderCb = m_VM_bufEdClr[idxItem][idxColor];
			glPCurSelS6010->VMOTION_SetBorderColor (
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP,
				VMOTION_NOT_CHANGE_PROP, glPCurGInfo->VM_clrValBorderCb,
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP);
			break;
		case IDX_VM_CLR_CR:
			glPCurGInfo->VM_clrValBorderCr = m_VM_bufEdClr[idxItem][idxColor];
			glPCurSelS6010->VMOTION_SetBorderColor (
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP,
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP,
				VMOTION_NOT_CHANGE_PROP, glPCurGInfo->VM_clrValBorderCr);
			break;
		}
	}
	else	// IDX_VM_GRID_ITEM_BAR
	{
		switch (idxColor)
		{
		case IDX_VM_CLR_Y:
			glPCurGInfo->VM_clrValBarY = m_VM_bufEdClr[idxItem][idxColor];
			glPCurSelS6010->VMOTION_SetBarColor (
				VMOTION_NOT_CHANGE_PROP, glPCurGInfo->VM_clrValBarY,
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP,
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP);
			break;
		case IDX_VM_CLR_CB:
			glPCurGInfo->VM_clrValBarCb = m_VM_bufEdClr[idxItem][idxColor];
			glPCurSelS6010->VMOTION_SetBarColor (
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP,
				VMOTION_NOT_CHANGE_PROP, glPCurGInfo->VM_clrValBarCb,
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP);
			break;
		case IDX_VM_CLR_CR:
			glPCurGInfo->VM_clrValBarCr = m_VM_bufEdClr[idxItem][idxColor];
			glPCurSelS6010->VMOTION_SetBarColor (
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP,
				VMOTION_NOT_CHANGE_PROP, VMOTION_NOT_CHANGE_PROP,
				VMOTION_NOT_CHANGE_PROP, glPCurGInfo->VM_clrValBarCr);
			break;
		}
	}

	UpdateData (FALSE);
}

void CDlgSettingMosaicVMotion::EnableDlgItemFromMode ()
{
	EnableItem_Mosaic (FALSE);
	EnableItem_VM (FALSE);
	EnableItem_ADVMD (FALSE);

	switch (glPCurGInfo->idxModeSetMosaic)
	{
	case IDX_SETM_MODE_MOSAIC:
		EnableItem_Mosaic (TRUE);
		break;
	case IDX_SETM_MODE_V_MOTION:
		EnableItem_VM (TRUE, glPCurGInfo->VM_bExecThSetting);
		break;
	case IDX_SETM_MODE_ADV_MOT_DET:
		EnableItem_ADVMD (TRUE, glPCurGInfo->ADVMD_bExecThSetting);
		break;
	}
}

void CDlgSettingMosaicVMotion::EnableItem_Mosaic (BOOL bEnable)
{
	int i;
	for (i=IDC_MOSAIC_COM_STRENGTH; i<=IDC_MOSAIC_COM_STRENGTH; i++)
	{
		GetDlgItem (i)->EnableWindow (bEnable);
	}
}

void CDlgSettingMosaicVMotion::EnableItem_VM (BOOL bEnable, BOOL bExecThSetting)
{
	int i;
	for (i=IDC_VM_CH_B_SHOW_GRID; i<=IDC_VM_STA_AREA_CR; i++)
	{
		if (i >= IDC_VM_STA_SENSITIVITY_LEVEL && i <= IDC_VM_RAD_TH_8)
		{
			GetDlgItem (i)->EnableWindow (bExecThSetting);
		}
		else
		{
			GetDlgItem (i)->EnableWindow (bEnable);
		}
	}
	for (i=IDC_VM_CBD_CH_B_0; i<=IDC_VM_CBD_STA_ON_OFF; i++)
	{
		GetDlgItem (i)->EnableWindow (bEnable);
	}
}

void CDlgSettingMosaicVMotion::EnableItem_ADVMD (BOOL bEnable, BOOL bExecThSetting)
{
	int i;
	for (i=IDC_ADVMD_CH_B_EXEC_SET_TH; i<=IDC_ADVMD_CH_B_SHOW_GRID; i++)
	{
		if (i >= IDC_ADVMD_STA_SENSITIVITY_LEVEL && i <= IDC_ADVMD_RAD_TH_8)
		{
			GetDlgItem (i)->EnableWindow (bExecThSetting);
		}
		else if (i >= IDC_ADVMD_RAD_CUR_CHK_ITEM_0 && i <= IDC_ADVMD_RAD_CUR_CHK_ITEM_3)
		{
			GetDlgItem (IDC_ADVMD_RAD_CUR_CHK_ITEM_0)->EnableWindow (FALSE);
			GetDlgItem (IDC_ADVMD_RAD_CUR_CHK_ITEM_1)->EnableWindow (FALSE);
			GetDlgItem (IDC_ADVMD_RAD_CUR_CHK_ITEM_2)->EnableWindow (FALSE);
			GetDlgItem (IDC_ADVMD_RAD_CUR_CHK_ITEM_3)->EnableWindow (FALSE);

			if (bEnable == TRUE)
			{
				switch (glPCurGInfo->ADVMD_bufNumMotionCheck[m_ADVMD_idxCurSelCh])
				{
				case 4:
					GetDlgItem (IDC_ADVMD_RAD_CUR_CHK_ITEM_3)->EnableWindow (TRUE);
				case 3:
					GetDlgItem (IDC_ADVMD_RAD_CUR_CHK_ITEM_2)->EnableWindow (TRUE);
				case 2:
					GetDlgItem (IDC_ADVMD_RAD_CUR_CHK_ITEM_1)->EnableWindow (TRUE);
				case 1:
					GetDlgItem (IDC_ADVMD_RAD_CUR_CHK_ITEM_0)->EnableWindow (TRUE);
					break;
				}
			}
		}
		else
		{
			GetDlgItem (i)->EnableWindow (bEnable);
		}
	}
}

int CDlgSettingMosaicVMotion::GetVMThIdxFromRealTh (int nRealTh)
{
	int i, idxTh;

	idxTh = IDX_VM_TH_8;
	for (i=0; i<NUM_VM_TH; i++)
	{
		if (nRealTh <= GL_VM_TH_IDX_TO_VAL[i])
		{
			idxTh = i;
			break;
		}
	}

	return idxTh;
}

void CDlgSettingMosaicVMotion::OnVmChBExecSetTh() 
{
	// TODO: Add your control notification handler code here
	
	glPCurGInfo->VM_bExecThSetting = 1 -glPCurGInfo->VM_bExecThSetting;

	EnableDlgItemFromMode ();

	glPWndLiveDisp->Invalidate ();	// Show or hide v-motion th grid
}

void CDlgSettingMosaicVMotion::OnChangeVmEdCntMinDet() 
{
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);

	glPCurGInfo->VM_minCntTh = m_VM_edCntMinDet;
	glPCurSelS6010->VMOTION_SetMinDetectThreshold (glPCurGInfo->VM_minCntTh);
}

void CDlgSettingMosaicVMotion::OnSelchangeMosaicComStrength() 
{
	// TODO: Add your control notification handler code here
	
	glPCurGInfo->MOSAIC_nStrength = m_MOSAIC_comStrength.GetCurSel ();
	glPCurSelS6010->MOSAIC_SetStrength (glPCurGInfo->MOSAIC_nStrength);
}

void CDlgSettingMosaicVMotion::OnAdvmdChBExecSetTh() 
{
	// TODO: Add your control notification handler code here
	
	glPCurGInfo->ADVMD_bExecThSetting = 1 -glPCurGInfo->ADVMD_bExecThSetting;

	EnableDlgItemFromMode ();

	glPWndLiveDisp->Invalidate ();	// Show or hide v-motion th grid
}

void CDlgSettingMosaicVMotion::OnSelchangeAdvmdComChkInterval() 
{
	// TODO: Add your control notification handler code here
	
	glPCurGInfo->ADVMD_bbufMotDataDist[m_ADVMD_idxCurSelCh][glPCurGInfo->ADVMD_idxCurSelCheckItem] = m_ADVMD_comChkInterval.GetCurSel () +1;
	glPCurSelS6010->ADV_MOT_SetMotChkDist (m_ADVMD_idxCurSelCh, glPCurGInfo->ADVMD_idxCurSelCheckItem,
		glPCurGInfo->ADVMD_bbufMotDataDist[m_ADVMD_idxCurSelCh][glPCurGInfo->ADVMD_idxCurSelCheckItem]);
}

void CDlgSettingMosaicVMotion::OnChangeAdvmdEdCntMinDet() 
{
	// TODO: Add your control notification handler code here
	
	UpdateData (TRUE);

	glPCurGInfo->ADVMD_bufCntMinDet[m_ADVMD_idxCurSelCh] = m_ADVMD_edCntMinDet;
	glPCurSelS6010->ADV_MOT_SetMinDetectThreshold (m_ADVMD_idxCurSelCh, glPCurGInfo->ADVMD_bufCntMinDet[m_ADVMD_idxCurSelCh]);
}

void CDlgSettingMosaicVMotion::OnAdvmdRadTh0() 
{
	ADVMD_ChangeCurSelTh (IDX_VM_TH_0);
}

void CDlgSettingMosaicVMotion::OnAdvmdRadTh1() 
{
	ADVMD_ChangeCurSelTh (IDX_VM_TH_1);
}

void CDlgSettingMosaicVMotion::OnAdvmdRadTh2() 
{
	ADVMD_ChangeCurSelTh (IDX_VM_TH_2);
}

void CDlgSettingMosaicVMotion::OnAdvmdRadTh3() 
{
	ADVMD_ChangeCurSelTh (IDX_VM_TH_3);
}

void CDlgSettingMosaicVMotion::OnAdvmdRadTh4() 
{
	ADVMD_ChangeCurSelTh (IDX_VM_TH_4);
}

void CDlgSettingMosaicVMotion::OnAdvmdRadTh5() 
{
	ADVMD_ChangeCurSelTh (IDX_VM_TH_5);
}

void CDlgSettingMosaicVMotion::OnAdvmdRadTh6() 
{
	ADVMD_ChangeCurSelTh (IDX_VM_TH_6);
}

void CDlgSettingMosaicVMotion::OnAdvmdRadTh7() 
{
	ADVMD_ChangeCurSelTh (IDX_VM_TH_7);
}

void CDlgSettingMosaicVMotion::OnAdvmdRadTh8() 
{
	ADVMD_ChangeCurSelTh (IDX_VM_TH_8);
}

void CDlgSettingMosaicVMotion::ADVMD_ChangeCurSelTh (int idxTh)
{
	m_ADVMD_idxCurSelTh = GL_VM_TH_IDX_TO_VAL[idxTh];

	SetDlgItemState_ADVMD ();
}

void CDlgSettingMosaicVMotion::OnAdvmdRadNumChkItem0()
{
	ADVMD_ChangeNumChkItem (0);
}

void CDlgSettingMosaicVMotion::OnAdvmdRadNumChkItem1()
{
	ADVMD_ChangeNumChkItem (1);
}

void CDlgSettingMosaicVMotion::OnAdvmdRadNumChkItem2()
{
	ADVMD_ChangeNumChkItem (2);
}

void CDlgSettingMosaicVMotion::OnAdvmdRadNumChkItem3()
{
	ADVMD_ChangeNumChkItem (3);
}

void CDlgSettingMosaicVMotion::OnAdvmdRadCurChkItem0()
{
	ADVMD_ChangeCurChkItem (0);
}

void CDlgSettingMosaicVMotion::OnAdvmdRadCurChkItem1()
{
	ADVMD_ChangeCurChkItem (1);
}

void CDlgSettingMosaicVMotion::OnAdvmdRadCurChkItem2()
{
	ADVMD_ChangeCurChkItem (2);
}

void CDlgSettingMosaicVMotion::OnAdvmdRadCurChkItem3()
{
	ADVMD_ChangeCurChkItem (3);
}

void CDlgSettingMosaicVMotion::ADVMD_ChangeNumChkItem (int idxNumChkItem)
{
	glPCurGInfo->ADVMD_bufNumMotionCheck[m_ADVMD_idxCurSelCh] = idxNumChkItem +1;
	glPCurSelS6010->ADV_MOT_SetNumMotChkItem (m_ADVMD_idxCurSelCh, glPCurGInfo->ADVMD_bufNumMotionCheck[m_ADVMD_idxCurSelCh]);

	glPCurGInfo->ADVMD_idxCurSelCheckItem = 0;

	EnableDlgItemFromMode ();
	SetDlgItemState_ADVMD ();
}

void CDlgSettingMosaicVMotion::ADVMD_ChangeCurChkItem (int idxCurChkItem)
{
	glPCurGInfo->ADVMD_idxCurSelCheckItem = idxCurChkItem;

	EnableDlgItemFromMode ();
	SetDlgItemState_ADVMD ();

	glPWndLiveDisp->Invalidate ();
}

void CDlgSettingMosaicVMotion::OnAdvmdChBShowGrid() 
{
	// TODO: Add your control notification handler code here
	
	glPCurGInfo->ADVMD_bShowGrid = 1 -glPCurGInfo->ADVMD_bShowGrid;

	glPWndLiveDisp->Invalidate ();
}

