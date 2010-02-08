// TSearchServerDlg.h : 標頭檔 
//

#pragma once
#include "afxwin.h"


// CTSearchServerDlg 對話方塊
class CTSearchServerDlg : public CDialog
{
// 建構
public:
	CTSearchServerDlg(CWnd* pParent = NULL);	// 標準建構函式

// 對話方塊資料
	enum { IDD = IDD_TSEARCHSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支援


// 程式碼實作
protected:
	HICON m_hIcon;

	// 產生的訊息對應函式
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListBox m_list;
	afx_msg void OnBnClickedOk();
	CString m_hostname;
	CString m_pid;
	CString m_wip;
	CString m_lip;
	CString m_mip;
	CString m_mac;
	afx_msg void OnLbnSelchangeList1();
	int m_vt;
	int m_sv;
	CString m_version;
	CString m_mname;
	CString m_sn;
	int m_ph;
	int m_pc2s;
	int m_ps2c;
	int m_pc;
	int m_pv;
	int m_pm;
	int m_pr;
	CString m_system;
	CString m_type;
	int m_channel;
	CString m_mp;
	CString m_aw;
	CString m_at;
	CString m_mt;
	CString m_pt;
	CString m_av;
	CString m_rs;
	CString m_br;
};
