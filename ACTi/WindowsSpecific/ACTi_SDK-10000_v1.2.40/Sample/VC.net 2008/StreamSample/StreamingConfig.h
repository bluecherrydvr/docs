#pragma once

#include "SDK10000.h"
// CStreamingConfig dialog

class CStreamingConfig : public CDialog
{
	DECLARE_DYNAMIC(CStreamingConfig)

public:
	CStreamingConfig(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStreamingConfig();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };
//	char		m_szUserID[16];                       // Streaming engine login ID
//	char		m_szUserPwd[16];                    // Streaming engine login Password
//	char		m_szServerIP[16];                     // Streaming engine IP address
//	DWORD		m_dwStreamingPort;                  // Audio/Video port number for streaming engine
//	DWORD		m_dwControlPort;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	DWORD m_IP;
	CString m_ID;
	CString m_PW;
	DWORD m_SP;
	DWORD m_CP;
};
