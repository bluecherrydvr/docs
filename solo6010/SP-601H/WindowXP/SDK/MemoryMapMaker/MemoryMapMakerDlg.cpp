// MemoryMapMakerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MemoryMapMaker.h"
#include "MemoryMapMakerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define VMEM_DISP_WR_BASE		0x00000000
DWORD nMEM_DISP_SIZE;

#define VMEM_OSG_BASE			(VMEM_DISP_WR_BASE +nMEM_DISP_SIZE)
DWORD nMEM_OSG_SIZE;

#define VMEM_COMP_OSD_BASE		(VMEM_OSG_BASE +nMEM_OSG_SIZE)
DWORD nMEM_COMP_OSD_SIZE;

#define VMEM_MOTION_BASE		(VMEM_COMP_OSD_BASE +nMEM_COMP_OSD_SIZE)
DWORD nMEM_MOTION_SIZE;

#define	VMEM_AUDIO_BASE			(VMEM_MOTION_BASE +nMEM_MOTION_SIZE)
DWORD nMEM_AUDIO_SIZE;

#define VMEM_COMP_WR_BASE			(VMEM_AUDIO_BASE +nMEM_AUDIO_SIZE)
DWORD nMEM_COMP_WR_SIZE;
DWORD nMEM_COMP_WR_TOTAL;

#define MEM_ENC_REF_BASE		(VMEM_COMP_WR_BASE +nMEM_COMP_WR_TOTAL)
DWORD nMEM_ENC_REF_SIZE;
DWORD nMEM_ENC_REF_TOTAL;

#define MEM_EXT_ENC_REF_BASE	(MEM_ENC_REF_BASE +nMEM_ENC_REF_TOTAL)
DWORD nMEM_EXT_ENC_REF_SIZE;
DWORD nMEM_EXT_ENC_REF_TOTAL;

#define MEM_DEC_REF_BASE		(MEM_EXT_ENC_REF_BASE +nMEM_EXT_ENC_REF_TOTAL)
DWORD nMEM_DEC_REF_SIZE;
DWORD nMEM_DEC_REF_TOTAL;

#define MEM_DEC_TO_LIVE			(MEM_DEC_REF_BASE +nMEM_DEC_REF_TOTAL)
DWORD nMEM_DEC_TO_LIVE_SIZE;

#define VMEM_MPEG_CODE_BASE		(MEM_DEC_TO_LIVE +nMEM_DEC_TO_LIVE_SIZE)
DWORD nVMEM_MPEG_CODE_SIZE;
#define VMEM_JPEG_CODE_BASE		(VMEM_MPEG_CODE_BASE +nVMEM_MPEG_CODE_SIZE)
DWORD nVMEM_JPEG_CODE_SIZE;
#define VMEM_DECODE_BASE		(VMEM_JPEG_CODE_BASE +nVMEM_JPEG_CODE_SIZE)
DWORD nVMEM_DECODE_CODE_SIZE;
#define VMEM_TOTAL_USED			(VMEM_DECODE_BASE +nVMEM_DECODE_CODE_SIZE)


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMemoryMapMakerDlg dialog

CMemoryMapMakerDlg::CMemoryMapMakerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMemoryMapMakerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMemoryMapMakerDlg)
	m_chEncOSD_bUseVirtualCh = FALSE;
	m_chMP4Enc_bUseVirtualCh = FALSE;
	m_chMP4Enc_bUseCIFHD1 = FALSE;
	m_chMP4Enc_bUseFullD1 = FALSE;
	m_chUseEncOSD = FALSE;
	m_chUseG723Enc = FALSE;
	m_chUseJPEGEnc = FALSE;
	m_chUseMP4Dec = FALSE;
	m_chUseVideoMotion = FALSE;
	m_chUseVideoOSG = FALSE;
	m_edStrMemMap = _T("");
	m_staJPEGEnc_szCodeBuf = _T("");
	m_staMP4Dec_szCodeBuf = _T("");
	m_staMP4Enc_szCodeBuf = _T("");
	m_staSzTotalSDRAM = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	CMemoryMapMakerApp *pApp = (CMemoryMapMakerApp *)AfxGetApp ();

	m_chEncOSD_bUseVirtualCh = pApp->GetProfileInt ("SETTINGS", "m_chEncOSD_bUseVirtualCh", TRUE);
	m_chMP4Enc_bUseVirtualCh = pApp->GetProfileInt ("SETTINGS", "m_chMP4Enc_bUseVirtualCh", TRUE);
	m_chMP4Enc_bUseCIFHD1 = pApp->GetProfileInt ("SETTINGS", "m_chMP4Enc_bUseCIFHD1", TRUE);
	m_chMP4Enc_bUseFullD1 = pApp->GetProfileInt ("SETTINGS", "m_chMP4Enc_bUseFullD1", TRUE);
	m_chUseEncOSD = pApp->GetProfileInt ("SETTINGS", "m_chUseEncOSD", TRUE);
	m_chUseG723Enc = pApp->GetProfileInt ("SETTINGS", "m_chUseG723Enc", TRUE);
	m_chUseJPEGEnc = pApp->GetProfileInt ("SETTINGS", "m_chUseJPEGEnc", TRUE);
	m_chUseMP4Dec = pApp->GetProfileInt ("SETTINGS", "m_chUseMP4Dec", TRUE);
	m_chUseVideoMotion = pApp->GetProfileInt ("SETTINGS", "m_chUseVideoMotion", TRUE);
	m_chUseVideoOSG = pApp->GetProfileInt ("SETTINGS", "m_chUseVideoOSG", TRUE);

	m_MP4Enc_numCh = pApp->GetProfileInt ("SETTINGS", "m_MP4Enc_numCh", 15);
	m_MP4Dec_numCh = pApp->GetProfileInt ("SETTINGS", "m_MP4Dec_numCh", 15);
	m_EncOSD_numCh = pApp->GetProfileInt ("SETTINGS", "m_EncOSD_numCh", 15);
	m_MP4Enc_codeBufMul = pApp->GetProfileInt ("SETTINGS", "m_MP4Enc_codeBufMul", 128);
	m_MP4Dec_codeBufMul = pApp->GetProfileInt ("SETTINGS", "m_MP4Dec_codeBufMul", 128);
	m_JPEGEnc_codeBufMul = pApp->GetProfileInt ("SETTINGS", "m_JPEGEnc_codeBufMul", 128);
}

void CMemoryMapMakerDlg::OnDestroy() 
{
	UpdateData (TRUE);

	m_MP4Enc_numCh = m_comMP4Enc_numCh.GetCurSel ();
	m_MP4Dec_numCh = m_comMP4Dec_numCh.GetCurSel ();
	m_EncOSD_numCh = m_comEncOSD_numCh.GetCurSel ();
	m_MP4Enc_codeBufMul = m_comMP4Enc_codeBufMul.GetCurSel ();
	m_MP4Dec_codeBufMul = m_comMP4Dec_codeBufMul.GetCurSel ();
	m_JPEGEnc_codeBufMul = m_comJPEGEnc_codeBufMul.GetCurSel ();

	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here

	CMemoryMapMakerApp *pApp = (CMemoryMapMakerApp *)AfxGetApp ();

	pApp->WriteProfileInt ("SETTINGS", "m_chEncOSD_bUseVirtualCh", m_chEncOSD_bUseVirtualCh);
	pApp->WriteProfileInt ("SETTINGS", "m_chMP4Enc_bUseVirtualCh", m_chMP4Enc_bUseVirtualCh);
	pApp->WriteProfileInt ("SETTINGS", "m_chMP4Enc_bUseCIFHD1", m_chMP4Enc_bUseCIFHD1);
	pApp->WriteProfileInt ("SETTINGS", "m_chMP4Enc_bUseFullD1", m_chMP4Enc_bUseFullD1);
	pApp->WriteProfileInt ("SETTINGS", "m_chUseEncOSD", m_chUseEncOSD);
	pApp->WriteProfileInt ("SETTINGS", "m_chUseG723Enc", m_chUseG723Enc);
	pApp->WriteProfileInt ("SETTINGS", "m_chUseJPEGEnc", m_chUseJPEGEnc);
	pApp->WriteProfileInt ("SETTINGS", "m_chUseMP4Dec", m_chUseMP4Dec);
	pApp->WriteProfileInt ("SETTINGS", "m_chUseVideoMotion", m_chUseVideoMotion);
	pApp->WriteProfileInt ("SETTINGS", "m_chUseVideoOSG", m_chUseVideoOSG);

	pApp->WriteProfileInt ("SETTINGS", "m_MP4Enc_numCh", m_MP4Enc_numCh);
	pApp->WriteProfileInt ("SETTINGS", "m_MP4Dec_numCh", m_MP4Dec_numCh);
	pApp->WriteProfileInt ("SETTINGS", "m_EncOSD_numCh", m_EncOSD_numCh);
	pApp->WriteProfileInt ("SETTINGS", "m_MP4Enc_codeBufMul", m_MP4Enc_codeBufMul);
	pApp->WriteProfileInt ("SETTINGS", "m_MP4Dec_codeBufMul", m_MP4Dec_codeBufMul);
	pApp->WriteProfileInt ("SETTINGS", "m_JPEGEnc_codeBufMul", m_JPEGEnc_codeBufMul);
}

void CMemoryMapMakerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMemoryMapMakerDlg)
	DDX_Control(pDX, IDC_COM_MPEG4_ENC_NUM_CH, m_comMP4Enc_numCh);
	DDX_Control(pDX, IDC_COM_MPEG4_ENC_CODE_BUF_MUL, m_comMP4Enc_codeBufMul);
	DDX_Control(pDX, IDC_COM_MPEG4_DEC_NUM_CH, m_comMP4Dec_numCh);
	DDX_Control(pDX, IDC_COM_MPEG4_DEC_CODE_BUF_MUL, m_comMP4Dec_codeBufMul);
	DDX_Control(pDX, IDC_COM_JPEG_ENC_CODE_BUF_MUL, m_comJPEGEnc_codeBufMul);
	DDX_Control(pDX, IDC_COM_ENC_OSD_NUM_CH, m_comEncOSD_numCh);
	DDX_Check(pDX, IDC_CH_ENC_OSD_USE_VIRTUAL_CH, m_chEncOSD_bUseVirtualCh);
	DDX_Check(pDX, IDC_CH_MPEG4_USE_VIRTUAL_CH, m_chMP4Enc_bUseVirtualCh);
	DDX_Check(pDX, IDC_CH_USE_CIF_HD1_IMAGE, m_chMP4Enc_bUseCIFHD1);
	DDX_Check(pDX, IDC_CH_USE_FULL_D1_IMAGE, m_chMP4Enc_bUseFullD1);
	DDX_Check(pDX, IDC_CH_USE_ENCODER_OSD, m_chUseEncOSD);
	DDX_Check(pDX, IDC_CH_USE_G723_ENCODING, m_chUseG723Enc);
	DDX_Check(pDX, IDC_CH_USE_JPEG_ENCODING, m_chUseJPEGEnc);
	DDX_Check(pDX, IDC_CH_USE_MPEG4_DECODING, m_chUseMP4Dec);
	DDX_Check(pDX, IDC_CH_USE_VIDEO_MOTION, m_chUseVideoMotion);
	DDX_Check(pDX, IDC_CH_USE_VIDEO_OSG, m_chUseVideoOSG);
	DDX_Text(pDX, IDC_ED_MEMORY_MAP, m_edStrMemMap);
	DDX_Text(pDX, IDC_STA_JPEG_ENC_SZ_CODE_BUF, m_staJPEGEnc_szCodeBuf);
	DDX_Text(pDX, IDC_STA_MPEG4_DEC_SZ_CODE_BUF, m_staMP4Dec_szCodeBuf);
	DDX_Text(pDX, IDC_STA_MPEG4_ENC_SZ_CODE_BUF, m_staMP4Enc_szCodeBuf);
	DDX_Text(pDX, IDC_STA_TOTAL_SDRAM_SIZE, m_staSzTotalSDRAM);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMemoryMapMakerDlg, CDialog)
	//{{AFX_MSG_MAP(CMemoryMapMakerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CH_USE_VIDEO_OSG, OnChangeSettings)
	ON_BN_CLICKED(IDC_CH_USE_ENCODER_OSD, OnChangeSettings)
	ON_BN_CLICKED(IDC_CH_ENC_OSD_USE_VIRTUAL_CH, OnChangeSettings)
	ON_BN_CLICKED(IDC_CH_USE_VIDEO_MOTION, OnChangeSettings)
	ON_BN_CLICKED(IDC_CH_USE_G723_ENCODING, OnChangeSettings)
	ON_BN_CLICKED(IDC_CH_USE_CIF_HD1_IMAGE, OnChangeSettings)
	ON_BN_CLICKED(IDC_CH_USE_FULL_D1_IMAGE, OnChangeSettings)
	ON_BN_CLICKED(IDC_CH_MPEG4_USE_VIRTUAL_CH, OnChangeSettings)
	ON_BN_CLICKED(IDC_CH_USE_JPEG_ENCODING, OnChangeSettings)
	ON_BN_CLICKED(IDC_CH_USE_MPEG4_DECODING, OnChangeSettings)
	ON_CBN_SELCHANGE(IDC_COM_ENC_OSD_NUM_CH, OnChangeSettings)
	ON_CBN_SELCHANGE(IDC_COM_MPEG4_ENC_NUM_CH, OnChangeSettings)
	ON_CBN_SELCHANGE(IDC_COM_MPEG4_ENC_CODE_BUF_MUL, OnChangeSettings)
	ON_CBN_SELCHANGE(IDC_COM_JPEG_ENC_CODE_BUF_MUL, OnChangeSettings)
	ON_CBN_SELCHANGE(IDC_COM_MPEG4_DEC_NUM_CH, OnChangeSettings)
	ON_CBN_SELCHANGE(IDC_COM_MPEG4_DEC_CODE_BUF_MUL, OnChangeSettings)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMemoryMapMakerDlg message handlers

BOOL CMemoryMapMakerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	m_fntEdMemMap.CreatePointFont (90, "Courier New");
	GetDlgItem (IDC_ED_MEMORY_MAP)->SetFont (&m_fntEdMemMap);
	((CEdit *)GetDlgItem (IDC_ED_MEMORY_MAP))->SetLimitText (MAX_PATH *256);

	int i;
	char strTmp[MAX_PATH];

	for (i=0; i<16; i++)
	{
		sprintf (strTmp, "%d", i +1);

		m_comEncOSD_numCh.AddString (strTmp);
		m_comMP4Enc_numCh.AddString (strTmp);
		m_comMP4Dec_numCh.AddString (strTmp);
	}

	for (i=0; i<256; i++)
	{
		sprintf (strTmp, "%d", i);

		m_comMP4Enc_codeBufMul.AddString (strTmp);
		m_comMP4Dec_codeBufMul.AddString (strTmp);
		m_comJPEGEnc_codeBufMul.AddString (strTmp);
	}
	
	m_comMP4Enc_numCh.SetCurSel (m_MP4Enc_numCh);
	m_comMP4Dec_numCh.SetCurSel (m_MP4Dec_numCh);
	m_comEncOSD_numCh.SetCurSel (m_EncOSD_numCh);
	m_comMP4Enc_codeBufMul.SetCurSel (m_MP4Enc_codeBufMul);
	m_comMP4Dec_codeBufMul.SetCurSel (m_MP4Dec_codeBufMul);
	m_comJPEGEnc_codeBufMul.SetCurSel (m_JPEGEnc_codeBufMul);

	OnChangeSettings ();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMemoryMapMakerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMemoryMapMakerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMemoryMapMakerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CMemoryMapMakerDlg::OnChangeSettings() 
{
	// TODO: Add your control notification handler code here

	UpdateData (TRUE);

	int numMP4EncCh, numMp4DecCh, numCompChVal;

	numMP4EncCh = m_comMP4Enc_numCh.GetCurSel () +1;
	numMp4DecCh = m_comMP4Dec_numCh.GetCurSel () +1;

	nMEM_DISP_SIZE = 0x00480000;

	if (m_chUseVideoOSG == TRUE)
	{
		nMEM_OSG_SIZE = 0x00120000;
	}
	else
	{
		nMEM_OSG_SIZE = 0;
	}

	if (m_chUseEncOSD == TRUE)
	{
		if (m_chEncOSD_bUseVirtualCh == TRUE)
		{
			nMEM_COMP_OSD_SIZE = 0x10000 *(16 +(m_comEncOSD_numCh.GetCurSel () +1));
		}
		else
		{
			nMEM_COMP_OSD_SIZE = 0x10000 *(m_comEncOSD_numCh.GetCurSel () +1);
		}
	}
	else
	{
		nMEM_COMP_OSD_SIZE = 0;
	}

	if (m_chUseVideoMotion == TRUE)
	{
		nMEM_MOTION_SIZE = 0x00080000;
	}
	else
	{
		nMEM_MOTION_SIZE = 0;
	}

	if (m_chUseG723Enc == TRUE)
	{
		nMEM_AUDIO_SIZE = 0x00010000;
	}
	else
	{
		nMEM_AUDIO_SIZE = 0;
	}

	// MPEG4 Encoding[S]
	if (m_chMP4Enc_bUseFullD1 == TRUE)
	{
		nMEM_COMP_WR_SIZE = 0x00120000;
		if (numMP4EncCh > 8)
		{
			nMEM_COMP_WR_TOTAL = nMEM_COMP_WR_SIZE *(16 +1);

			numCompChVal = 16;
		}
		else
		{
			nMEM_COMP_WR_TOTAL = nMEM_COMP_WR_SIZE *(numMP4EncCh *2 +1);

			numCompChVal = numMP4EncCh *2;
		}
	}
	else
	{
		nMEM_COMP_WR_SIZE = 0x00120000 /2;
		nMEM_COMP_WR_TOTAL = nMEM_COMP_WR_SIZE *(numMP4EncCh +1);

		numCompChVal = numMP4EncCh;
	}

	if (m_chMP4Enc_bUseFullD1 == TRUE)
	{
		nMEM_ENC_REF_SIZE = 0x00140000;
	}
	else
	{
		nMEM_ENC_REF_SIZE = 0x00140000 /2;
	}
	nMEM_ENC_REF_TOTAL = nMEM_ENC_REF_SIZE *numMP4EncCh;

	if (m_chMP4Enc_bUseVirtualCh == TRUE)
	{
		if (m_chMP4Enc_bUseFullD1 == TRUE)
		{
			nMEM_EXT_ENC_REF_SIZE = 0x00140000;
		}
		else
		{
			nMEM_EXT_ENC_REF_SIZE = 0x00140000 /2;
		}
		nMEM_EXT_ENC_REF_TOTAL = nMEM_EXT_ENC_REF_SIZE *numMP4EncCh;
	}
	else
	{
		nMEM_EXT_ENC_REF_SIZE = 0;
		nMEM_EXT_ENC_REF_TOTAL = 0;
	}

	nVMEM_MPEG_CODE_SIZE = 0x10000 *m_comMP4Enc_codeBufMul.GetCurSel ();
	// MPEG4 Encoding[E]

	if (m_chUseJPEGEnc == TRUE)
	{
		nVMEM_JPEG_CODE_SIZE = 0x10000 *m_comJPEGEnc_codeBufMul.GetCurSel ();
	}
	else
	{
		nVMEM_JPEG_CODE_SIZE = 0;
	}

	if (m_chUseMP4Dec == TRUE)
	{
		nMEM_DEC_TO_LIVE_SIZE = 0x00240000;
		
		if (m_chMP4Enc_bUseFullD1 == TRUE)
		{
			nMEM_DEC_REF_SIZE = 0x00140000;
		}
		else
		{
			nMEM_DEC_REF_SIZE = 0x00140000 /2;
		}
		nMEM_DEC_REF_TOTAL = nMEM_DEC_REF_SIZE *numMp4DecCh;

		nVMEM_DECODE_CODE_SIZE = 0x10000 *m_comMP4Dec_codeBufMul.GetCurSel ();
	}
	else
	{
		nMEM_DEC_TO_LIVE_SIZE = 0;
		nMEM_DEC_REF_SIZE = 0;
		nMEM_DEC_REF_TOTAL = 0;
		nVMEM_DECODE_CODE_SIZE = 0;
	}

	m_staMP4Enc_szCodeBuf.Format ("%d KB (%2.2f MB) ", nVMEM_MPEG_CODE_SIZE /1024, float(nVMEM_MPEG_CODE_SIZE) /1024 /1024);
	m_staJPEGEnc_szCodeBuf.Format ("%d KB (%2.2f MB) ", nVMEM_JPEG_CODE_SIZE /1024, float(nVMEM_JPEG_CODE_SIZE) /1024 /1024);
	m_staMP4Dec_szCodeBuf.Format ("%d KB (%2.2f MB) ", nVMEM_DECODE_CODE_SIZE /1024, float(nVMEM_DECODE_CODE_SIZE) /1024 /1024);
	m_staSzTotalSDRAM.Format ("%d KB (%2.2f MB) ", VMEM_TOTAL_USED /1024, float(VMEM_TOTAL_USED) /1024 /1024);

	m_edStrMemMap.Format (
		"// SOLO6010 SDRAM Map[S]\r\n"\
		"#define VMEM_DISP_WR_BASE			0x%08X\r\n"\
		"#define 	MEM_DISP_SIZE			0x%08X						// 0x00120000 *4 page\r\n"\
		"\r\n"\
		"#define VMEM_OSG_BASE				(VMEM_DISP_WR_BASE +MEM_DISP_SIZE)\r\n"\
		"#define		MEM_OSG_SIZE			0x%08X						// PAL size\r\n"\
		"\r\n"\
		"#define VMEM_COMP_OSD_BASE			(VMEM_OSG_BASE +MEM_OSG_SIZE)\r\n"\
		"#define		MEM_COMP_OSD_SIZE		0x%08X						// 64KB *Num of Total Video Channel\r\n"\
		"\r\n"\
		"#define VMEM_MOTION_BASE			(VMEM_COMP_OSD_BASE +MEM_COMP_OSD_SIZE)\r\n"\
		"#define		MEM_MOTION_SIZE			0x%08X						// 512KB\r\n"\
		"\r\n"\
		"#define	VMEM_AUDIO_BASE				(VMEM_MOTION_BASE +MEM_MOTION_SIZE)\r\n"\
		"#define		MEM_AUDIO_SIZE			0x%08X\r\n"\
		"\r\n"\
		"#define VMEM_COMP_WR_BASE			(VMEM_AUDIO_BASE +MEM_AUDIO_SIZE)\r\n"\
		"#define		MEM_COMP_WR_SIZE		0x%08X						// 18 *64KB = 1,152KB\r\n"\
		"#define		MEM_COMP_WR_TOTAL		(MEM_COMP_WR_SIZE *(%d +1))		// D1: (NumCh x2) +1, Others: NumCh +1\r\n"\
		"\r\n"\
		"#define MEM_ENC_REF_BASE			(VMEM_COMP_WR_BASE +MEM_COMP_WR_TOTAL)\r\n"\
		"#define		MEM_ENC_REF_SIZE		0x%08X						// 20 *64KB = 1,280KB\r\n"\
		"#define		MEM_ENC_REF_TOTAL		(MEM_ENC_REF_SIZE *%d)			// Num of Real Video Channel\r\n"\
		"\r\n"\
		"#define MEM_EXT_ENC_REF_BASE		(MEM_ENC_REF_BASE +MEM_ENC_REF_TOTAL)\r\n"\
		"#define		MEM_EXT_ENC_REF_SIZE	0x%08X						// 20 *64KB = 1,280KB\r\n"\
		"#define		MEM_EXT_ENC_REF_TOTAL	(MEM_EXT_ENC_REF_SIZE *%d)		// Num of Real Video Channel\r\n"\
		"\r\n"\
		"#define MEM_DEC_REF_BASE			(MEM_EXT_ENC_REF_BASE +MEM_EXT_ENC_REF_TOTAL)\r\n"\
		"#define		MEM_DEC_REF_SIZE		0x%08X						// 20 *64KB = 1,280KB\r\n"\
		"#define		MEM_DEC_REF_TOTAL		(MEM_DEC_REF_SIZE *%d)			// Num of Real Video Channel\r\n"\
		"\r\n"\
		"#define MEM_DEC_TO_LIVE				(MEM_DEC_REF_BASE +MEM_DEC_REF_TOTAL)\r\n"\
		"#define		MEM_DEC_TO_LIVE_SIZE	0x%08X						// 0x00120000 *2\r\n"\
		"\r\n"\
		"#define VMEM_MPEG_CODE_BASE			(MEM_DEC_TO_LIVE +MEM_DEC_TO_LIVE_SIZE)\r\n"\
		"#define		VMEM_MPEG_CODE_SIZE		0x%08X\r\n"\
		"#define VMEM_JPEG_CODE_BASE			(VMEM_MPEG_CODE_BASE +VMEM_MPEG_CODE_SIZE)\r\n"\
		"#define		VMEM_JPEG_CODE_SIZE		0x%08X\r\n"\
		"#define VMEM_DECODE_BASE			(VMEM_JPEG_CODE_BASE +VMEM_JPEG_CODE_SIZE)\r\n"\
		"#define		VMEM_DECODE_CODE_SIZE	0x%08X\r\n"\
		"#define VMEM_TOTAL_USED				(VMEM_DECODE_BASE +VMEM_DECODE_CODE_SIZE)\r\n"\
		"// SOLO6010 SDRAM Map[E]\r\n\r\n",
		VMEM_DISP_WR_BASE,
		nMEM_DISP_SIZE,
		nMEM_OSG_SIZE,
		nMEM_COMP_OSD_SIZE,
		nMEM_MOTION_SIZE,
		nMEM_AUDIO_SIZE,
		nMEM_COMP_WR_SIZE,
		numCompChVal,
		nMEM_ENC_REF_SIZE,
		numMP4EncCh,
		nMEM_EXT_ENC_REF_SIZE,
		numMP4EncCh,
		nMEM_DEC_REF_SIZE,
		numMp4DecCh,
		nMEM_DEC_TO_LIVE_SIZE,
		nVMEM_MPEG_CODE_SIZE,
		nVMEM_JPEG_CODE_SIZE,
		nVMEM_DECODE_CODE_SIZE
		);

	UpdateData (FALSE);
}
