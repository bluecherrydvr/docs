// MediaReceiverDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MediaReceiver.h"
#include "MediaReceiverDlg.h"
#include ".\mediareceiverdlg.h"
#include ".\tcpconnection.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMediaReceiverDlg dialog



CMediaReceiverDlg::CMediaReceiverDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMediaReceiverDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMediaReceiverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMediaReceiverDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CMediaReceiverDlg message handlers

BOOL CMediaReceiverDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMediaReceiverDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMediaReceiverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

typedef struct 
{
	char szUserName[32];
	char rsvd[28];
	unsigned short ChannelNumber;
	unsigned short nEncryptionType;
	char szPassword[64];
}TCP_STREAM_AUTHEN_REQ_V2;

void CMediaReceiverDlg::OnBnClickedOk()
{
	WSADATA wsaData;
	WSAStartup( MAKEWORD( 2, 2 ), &wsaData );

	CTCPConnection conn;

	// To connect to the device ( video server or IP camera ) by IP and streamint port
	if( conn.Connect( "172.16.1.123", 6002, 10 ) )
	{
		// If connection ready, we send Ip and password to the device
		// The ID and password combined in a 128 bytes array as TCP_STREAM_AUTHEN_REQ_V2
		TCP_STREAM_AUTHEN_REQ_V2 tsar;

		memset( &tsar, 0, sizeof(TCP_STREAM_AUTHEN_REQ_V2) );
		memcpy( tsar.szUserName, "Admin\0", 6 );
		memcpy( tsar.szPassword, "123456\0", 7 );
		tsar.ChannelNumber = 0; // Set it to 0, in the future, this flag will use to support dual stream devices;
		// 0 mean the ID and password is not encrypted
		// 1 mean the ID and password encrypted by MD5
		tsar.nEncryptionType = 0; 

		if( conn.SendData( (char*)&tsar, 128 ) )
		{
			int nget = 0;
			int nRecv = 0;
			char tmpBuffer[128];
			// here we hunt for 128 bytes of response
			while( nget != 128 )
			{
				nRecv = conn.RecvData( &tmpBuffer[nget], 128 - nget );
				if( nRecv > 0 )
					nget += nRecv;

				else
				{
					// some thing wrong on socket conection
					conn.Disconnect();
					return;
				}
			}

			if( tmpBuffer[0] == 0 )
			{
				int nSessionID = *(int*)(tmpBuffer + 4);
				
				char *pBuffer = new char[1024000];

				int RawDataLength;
				int VideoOrAudio;
				int nGet;
				int nTmpLength;

				try
				{
					int nLikeToGetPackets = 30;

					while ( nLikeToGetPackets -- > 0 ) 
					{
						nGet = 0;
						nTmpLength = 0;
						while( nGet != 12 && conn.IsConnected() )
						{
							nTmpLength = conn.RecvData( &pBuffer[nGet], 12 - nTmpLength );
							if( nTmpLength >= 0 )
								nGet += nTmpLength;

							else
							{
								// some thing wrong on socket conection
								conn.Disconnect();
								delete [] pBuffer;
								WSACleanup();
								return;
							}
						}

						if( *((DWORD*)pBuffer) == 0xB2010000 )
						{
							memcpy( (BYTE*)&VideoOrAudio, &pBuffer[4], 4 );
							memcpy( (BYTE*)&RawDataLength, &pBuffer[8], 4 );

							nTmpLength = 0;
							while( nGet != ( RawDataLength + 12 ) && conn.IsConnected() )
							{
								nTmpLength = conn.RecvData( &pBuffer[nGet], RawDataLength + 12 - nGet );

								if( nTmpLength >= 0 )
									nGet += nTmpLength;

								else
								{
									// some thing wrong on socket conection
									conn.Disconnect();
									delete [] pBuffer;
									WSACleanup();
									return;
								}			
							}

							if( pBuffer[4] == 1 )
							{
								// Mpeg4 video frame with length of nGet
								// Start at pBuffer[12(header)+32(private data)], there are nGet - 44 bytes of mpeg4 video data
								// AfxMessageBox( "Get Mpeg4 Video !" );
							}

							else if( pBuffer[4] == 4 )
							{
								// Mjpeg video frame with length of nGet
								// Start at pBuffer[12(header)+32(private data)], there are nGet - 44 bytes of mjpeg video data
								// AfxMessageBox( "Get MJpeg Video !" );
							}

							else if( pBuffer[4] == 2 )
							{
								// Audio frame with length of nGet
								//AfxMessageBox( "Get Audio !" );
								// Start at pBuffer[12(header)+16(time stamp)], there are nGet - 28 bytes of PCM (8K,16bit) audio data
							}
						}
					}
				}
				catch(...)
				{
					conn.Disconnect();
					delete [] pBuffer;
				}
			}

			else
			{
				// The ID and password is invalid
				AfxMessageBox( "The ID and password is invalid!" );
			}
		}
		else
		{
			AfxMessageBox( "Bad IP or Port!" );
		}
	}
	WSACleanup();
}
