// DecodeSampleDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "DecodeSample.h"
#include "DecodeSampleDlg.h"
#include <Winsock2.h>
#include ".\decodesampledlg.h"
#include "SDK10000.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 對話方塊資料
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

// 程式碼實作
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CDecodeSampleDlg 對話方塊



CDecodeSampleDlg::CDecodeSampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDecodeSampleDlg::IDD, pParent)
 	
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDecodeSampleDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDOK, m_cBtnStartStop);
    DDX_Control(pDX, IDC_STATIC_VW, m_cCtrlVW);
//    DDX_Control(pDX, IDC_EDIT4, m_ctrlChlNo);
    DDX_Control(pDX, IDC_STATIC_VW2, m_cCtrlVW2);
    DDX_Control(pDX, IDC_STATIC_VW3, m_cCtrlVW3);
    DDX_Control(pDX, IDC_STATIC_VW4, m_cCtrlVW4);
    DDX_Control(pDX, IDOK5, m_cBtnStartStreamStartStop);
    DDX_Control(pDX, IDOK3, m_cBtnStartRecordStartStop);
    DDX_Control(pDX, IDOK6, m_cBtnStartStop2);
    DDX_Control(pDX, IDOK9, m_cBtnStartStop3);
    DDX_Control(pDX, IDOK12, m_cBtnStartStop4);
    DDX_Control(pDX, IDOK8, m_cBtnStartStreamStartStop2);
    DDX_Control(pDX, IDOK11, m_cBtnStartStreamStartStop3);
    DDX_Control(pDX, IDOK14, m_cBtnStartStreamStartStop4);
    DDX_Control(pDX, IDOK7, m_cBtnStartRecordStartStop2);
    DDX_Control(pDX, IDOK10, m_cBtnStartRecordStartStop3);
    DDX_Control(pDX, IDOK13, m_cBtnStartRecordStartStop4);
}

BEGIN_MESSAGE_MAP(CDecodeSampleDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
    ON_STN_CLICKED(IDC_STATIC_VW, OnStnClickedStaticVw)
    ON_BN_CLICKED(IDOK2, OnBnClickedOk2)
    ON_BN_CLICKED(IDOK3, OnBnClickedOk3)
    ON_BN_CLICKED(IDOK5, OnBnClickedOk5)
    ON_BN_CLICKED(IDOK6, OnBnClickedOk6)
    ON_BN_CLICKED(IDOK8, OnBnClickedOk8)
    ON_BN_CLICKED(IDOK7, OnBnClickedOk7)
    ON_BN_CLICKED(IDOK9, OnBnClickedOk9)
    ON_BN_CLICKED(IDOK11, OnBnClickedOk11)
    ON_BN_CLICKED(IDOK10, OnBnClickedOk10)
    ON_BN_CLICKED(IDOK12, OnBnClickedOk12)
    ON_BN_CLICKED(IDOK14, OnBnClickedOk14)
    ON_BN_CLICKED(IDOK13, OnBnClickedOk13)
END_MESSAGE_MAP()


// CDecodeSampleDlg 訊息處理常式

BOOL CDecodeSampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 將 "關於..." 功能表加入系統功能表。

	// IDM_ABOUTBOX 必須在系統命令範圍之中。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示


    // Control variable
    for(int i = 0 ; i < MAX_CONTROL_CHANNEL; i ++)
    {
        m_bStartPreview[i] = false;
        m_bStartRecord[i] = false;
        m_bStartStream[i] = false;
    }

	// Update data
	UpdateData(false);

	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

void CDecodeSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，以便繪製圖示。
// 對於使用文件/檢視模式的 MFC 應用程式，框架會自動完成此作業。

void CDecodeSampleDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 繪製的裝置內容

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 將圖示置中於用戶端矩形
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 描繪圖示
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//當使用者拖曳最小化視窗時，系統呼叫這個功能取得游標顯示。
HCURSOR CDecodeSampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// Channel 1 : preview
void CDecodeSampleDlg::OnBnClickedOk()
{
	SetCursor(LoadCursor(NULL, IDC_WAIT));
	UpdateData();

    if(m_bStartStream[0])
        return;

	if(!m_bStartPreview[0])
	{
        // Set channel
        m_oCapChannel[0].SetChannel(1);
        m_oCapChannel[0].SetWindowHandle(m_cCtrlVW.m_hWnd);

        if(m_oCapChannel[0].Start())
        {
            m_cBtnStartStop.SetWindowText("Stop");
            m_bStartPreview[0] = true;
        }
	}
	else
	{
        m_oCapChannel[0].Stop();
		
		m_cBtnStartStop.SetWindowText("Preview");
		m_bStartPreview[0] = false;
	}

	UpdateData(false);
	SetCursor(LoadCursor(NULL, IDC_ARROW));
}



// Channel 1 : Record
void CDecodeSampleDlg::OnBnClickedOk3()
{
    int iChl = 1;
    int iIndex = iChl-1;
    // if preview or start streaming
    if(m_bStartPreview[iIndex] || m_bStartStream[iIndex])
    {
        if(!m_bStartRecord[iIndex])
        {
            // Set channel
            if(m_oCapChannel[iIndex].StartRecord("c:\\rectest\\Precord1.raw"))
            {
                m_cBtnStartRecordStartStop.SetWindowText("Stop");
                m_bStartRecord[iIndex] = true;
            }
        }
        else
        {
            m_oCapChannel[iIndex].StopRecord();

            m_cBtnStartRecordStartStop.SetWindowText("Record");
            m_bStartRecord[iIndex] = false;
        }
    }
}
// channel 1 : start stream without preview
void CDecodeSampleDlg::OnBnClickedOk5()
{
    int iChl = 1;
    int iIndex = iChl-1;

    if(m_bStartPreview[iIndex])
        return;

	if(!m_bStartStream[iIndex])
	{
        // Set channel
        m_oCapChannel[iIndex].SetChannel(iChl);
        if(m_oCapChannel[iIndex].StartWithoutPreview())
        {
            m_cBtnStartStreamStartStop.SetWindowText("Stop");
            m_bStartStream[iIndex] = true;
        }
	}
	else
	{
        m_oCapChannel[iIndex].Stop();
		
		m_cBtnStartStreamStartStop.SetWindowText("Stream");
		m_bStartStream[iIndex] = false;
	}
}





//////////////////////////////////////////////////////////////////////////
// channel 2 : preview
void CDecodeSampleDlg::OnBnClickedOk6()
{
    SetCursor(LoadCursor(NULL, IDC_WAIT));
    UpdateData();
    int iChl = 2;
    int iIndex = iChl-1;

    if(m_bStartStream[iIndex])
        return;

    if(!m_bStartPreview[iIndex])
    {
        // Set channel
        m_oCapChannel[iIndex].SetChannel(iChl);
        m_oCapChannel[iIndex].SetWindowHandle(m_cCtrlVW2.m_hWnd);

        if(m_oCapChannel[iIndex].Start())
        {
            m_cBtnStartStop2.SetWindowText("Stop");
            m_bStartPreview[iIndex] = true;
        }
    }
    else
    {
        m_oCapChannel[iIndex].Stop();

        m_cBtnStartStop2.SetWindowText("Preview");
        m_bStartPreview[iIndex] = false;
    }

    UpdateData(false);
    SetCursor(LoadCursor(NULL, IDC_ARROW));

}
// channel 2 : stream
void CDecodeSampleDlg::OnBnClickedOk8()
{
    int iChl = 2;
    int iIndex = iChl-1;

    if(m_bStartPreview[iIndex])
        return;

	if(!m_bStartStream[iIndex])
	{
        // Set channel
        m_oCapChannel[iIndex].SetChannel(iChl);
        if(m_oCapChannel[iIndex].StartWithoutPreview())
        {
            m_cBtnStartStreamStartStop2.SetWindowText("Stop");
            m_bStartStream[iIndex] = true;
        }
	}
	else
	{
        m_oCapChannel[iIndex].Stop();
		
		m_cBtnStartStreamStartStop2.SetWindowText("Stream");
		m_bStartStream[iIndex] = false;
	}
}
// channel 2 : Record
void CDecodeSampleDlg::OnBnClickedOk7()
{
    int iChl = 2;
    int iIndex = iChl-1;
    // if preview or start streaming
    if(m_bStartPreview[iIndex] || m_bStartStream[iIndex])
    {
        if(!m_bStartRecord[iIndex])
        {
            // Set channel
            if(m_oCapChannel[iIndex].StartRecord("c:\\rectest\\Precord2.raw"))
            {
                m_cBtnStartRecordStartStop2.SetWindowText("Stop");
                m_bStartRecord[iIndex] = true;
            }
        }
        else
        {
            m_oCapChannel[iIndex].StopRecord();

            m_cBtnStartRecordStartStop2.SetWindowText("Record");
            m_bStartRecord[iIndex] = false;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
// channel 3 : preview
void CDecodeSampleDlg::OnBnClickedOk9()
{
      SetCursor(LoadCursor(NULL, IDC_WAIT));
    UpdateData();
    int iChl = 3;
    int iIndex = iChl-1;

    if(m_bStartStream[iIndex])
        return;

    if(!m_bStartPreview[iIndex])
    {
        // Set channel
        m_oCapChannel[iIndex].SetChannel(iChl);
        m_oCapChannel[iIndex].SetWindowHandle(m_cCtrlVW3.m_hWnd);

        if(m_oCapChannel[iIndex].Start())
        {
            m_cBtnStartStop3.SetWindowText("Stop");
            m_bStartPreview[iIndex] = true;
        }
    }
    else
    {
        m_oCapChannel[iIndex].Stop();

        m_cBtnStartStop3.SetWindowText("Preview");
        m_bStartPreview[iIndex] = false;
    }

    UpdateData(false);
    SetCursor(LoadCursor(NULL, IDC_ARROW));
}
// channel 3 : stream
void CDecodeSampleDlg::OnBnClickedOk11()
{
    int iChl = 3;
    int iIndex = iChl-1;

    if(m_bStartPreview[iIndex])
        return;

    if(!m_bStartStream[iIndex])
    {
        // Set channel
        m_oCapChannel[iIndex].SetChannel(iChl);
        if(m_oCapChannel[iIndex].StartWithoutPreview())
        {
            m_cBtnStartStreamStartStop3.SetWindowText("Stop");
            m_bStartStream[iIndex] = true;
        }
    }
    else
    {
        m_oCapChannel[iIndex].Stop();

        m_cBtnStartStreamStartStop3.SetWindowText("Stream");
        m_bStartStream[iIndex] = false;
    }
}
// channel 3 : Record
void CDecodeSampleDlg::OnBnClickedOk10()
{
    int iChl = 3;
    int iIndex = iChl-1;
    // if preview or start streaming
    if(m_bStartPreview[iIndex] || m_bStartStream[iIndex])
    {
        if(!m_bStartRecord[iIndex])
        {
            // Set channel
            if(m_oCapChannel[iIndex].StartRecord("c:\\rectest\\Precord3.raw"))
            {
                m_cBtnStartRecordStartStop3.SetWindowText("Stop");
                m_bStartRecord[iIndex] = true;
            }
        }
        else
        {
            m_oCapChannel[iIndex].StopRecord();

            m_cBtnStartRecordStartStop3.SetWindowText("Record");
            m_bStartRecord[iIndex] = false;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// channel 4 : preview
void CDecodeSampleDlg::OnBnClickedOk12()
{
        SetCursor(LoadCursor(NULL, IDC_WAIT));
    UpdateData();
    int iChl = 4;
    int iIndex = iChl-1;

    if(m_bStartStream[iIndex])
        return;

    if(!m_bStartPreview[iIndex])
    {
        // Set channel
        m_oCapChannel[iIndex].SetChannel(iChl);
        m_oCapChannel[iIndex].SetWindowHandle(m_cCtrlVW4.m_hWnd);

        if(m_oCapChannel[iIndex].Start())
        {
            m_cBtnStartStop4.SetWindowText("Stop");
            m_bStartPreview[iIndex] = true;
        }
    }
    else
    {
        m_oCapChannel[iIndex].Stop();

        m_cBtnStartStop4.SetWindowText("Preview");
        m_bStartPreview[iIndex] = false;
    }

    UpdateData(false);
    SetCursor(LoadCursor(NULL, IDC_ARROW));
}
// channel 4 : stream
void CDecodeSampleDlg::OnBnClickedOk14()
{
    int iChl = 4;
    int iIndex = iChl-1;

    if(m_bStartPreview[iIndex])
        return;

    if(!m_bStartStream[iIndex])
    {
        // Set channel
        m_oCapChannel[iIndex].SetChannel(iChl);
        if(m_oCapChannel[iIndex].StartWithoutPreview())
        {
            m_cBtnStartStreamStartStop4.SetWindowText("Stop");
            m_bStartStream[iIndex] = true;
        }
    }
    else
    {
        m_oCapChannel[iIndex].Stop();

        m_cBtnStartStreamStartStop4.SetWindowText("Stream");
        m_bStartStream[iIndex] = false;
    }
}
// channel 4 : Record
void CDecodeSampleDlg::OnBnClickedOk13()
{
    int iChl = 4;
    int iIndex = iChl-1;
    // if preview or start streaming
    if(m_bStartPreview[iIndex] || m_bStartStream[iIndex])
    {
        if(!m_bStartRecord[iIndex])
        {
            // Set channel
            if(m_oCapChannel[iIndex].StartRecord("c:\\rectest\\Precord4.raw"))
            {
                m_cBtnStartRecordStartStop4.SetWindowText("Stop");
                m_bStartRecord[iIndex] = true;
            }
        }
        else
        {
            m_oCapChannel[iIndex].StopRecord();

            m_cBtnStartRecordStartStop4.SetWindowText("Record");
            m_bStartRecord[iIndex] = false;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
// Other
void CDecodeSampleDlg::OnStnClickedStaticVw()
{
    // TODO: Add your control notification handler code here
}

void CDecodeSampleDlg::OnBnClickedOk2()
{
// m_cCtrlVW2


}

// Exit
void CDecodeSampleDlg::OnBnClickedCancel()
{
	
    for(int i = 0 ; i < MAX_CONTROL_CHANNEL; i ++)
    {
        if(m_bStartRecord[i])
        {
            m_oCapChannel[i].StopRecord();
        }

        if(m_bStartPreview[i] || m_bStartStream[i])
        {
            m_oCapChannel[i].Stop();
        }
    }

	OnCancel();
}
