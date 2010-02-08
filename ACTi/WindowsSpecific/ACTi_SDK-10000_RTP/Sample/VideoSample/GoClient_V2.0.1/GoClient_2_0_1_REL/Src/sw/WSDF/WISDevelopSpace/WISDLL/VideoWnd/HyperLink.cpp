// HyperLink.cpp : implementation file
//
// HyperLink static control. Will open the default browser with the given URL
// when the user clicks on the link.
//
// Can also be used to set a email link.
//


#include "stdafx.h"
#include "resource.h"
#include "HyperLink.h"

#define TOOLTIP_ID 1

/////////////////////////////////////////////////////////////////////////////
// CHyperLink

CHyperLink::CHyperLink()
{
	m_crLinkColor      = RGB(0, 0, 238);     // Blue
	m_crVisitedColor   = RGB(85, 26, 139);   // Purple
	m_bVisited          = FALSE;              // Hasn't been visited yet.
	m_bUnderline        = TRUE;               // Underline the link?
	m_bAdjustToFit      = TRUE;               // Resize the window to fit the text?
	m_strURL.Empty();
}

CHyperLink::~CHyperLink()
{
	if ((HFONT)m_Font)
		m_Font.DeleteObject();
}


BEGIN_MESSAGE_MAP(CHyperLink, CStatic)
	//{{AFX_MSG_MAP(CHyperLink)
	ON_CONTROL_REFLECT(STN_CLICKED, OnClicked)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHyperLink message handlers

BOOL CHyperLink::PreTranslateMessage(MSG* pMsg) 
{
	m_ToolTip.RelayEvent(pMsg);
	return CStatic::PreTranslateMessage(pMsg);
}

HBRUSH CHyperLink::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	ASSERT(nCtlColor == CTLCOLOR_STATIC);

	// Set the approriate color
	if (m_bVisited)
		pDC->SetTextColor(m_crVisitedColor);
	else
		pDC->SetTextColor(m_crLinkColor);

	// Set underline font if required
	if (m_bUnderline) {

		if (!(HFONT)m_Font) {			// Create font only once.
			LOGFONT lf;
			GetFont()->GetLogFont(&lf);
			lf.lfUnderline = TRUE;
			m_Font.CreateFontIndirect(&lf);
		}

		pDC->SelectObject(&m_Font);
	}

	// transparent text.
	pDC->SetBkMode(TRANSPARENT);
	return (HBRUSH)GetStockObject(NULL_BRUSH);
}

BOOL CHyperLink::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/) 
{
	::SetCursor(m_hLinkCursor);
	return TRUE;
}

void CHyperLink::PreSubclassWindow() 
{
	// We want to get mouse clicks via STN_CLICKED
	DWORD dwStyle = GetStyle();
	::SetWindowLong(m_hWnd, GWL_STYLE, dwStyle | SS_NOTIFY);
	
	// Set the URL as the window text
	if (m_strURL.IsEmpty())
		GetWindowText(m_strURL);

	// Check that the window text isn't empty. If it is, set it as the URL.
	CString strWndText;
	GetWindowText(strWndText);
	if (strWndText.IsEmpty()) {
		ASSERT(!m_strURL.IsEmpty());
		SetWindowText(m_strURL);
	}

	// Adjust size of window to fit URL if necessary
	PositionWindow();		

	// Load up the hyperlink cursor
	m_hLinkCursor = AfxGetApp()->LoadCursor(IDC_HLINK);

	// Create the tooltip
	CRect rect; 
	GetClientRect(rect);
	m_ToolTip.Create(this);
	m_ToolTip.AddTool(this, m_strURL, rect, TOOLTIP_ID);

	CStatic::PreSubclassWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CHyperLink operations

void CHyperLink::SetURL(CString strURL)
{
	m_strURL = strURL;
	PositionWindow();
	m_ToolTip.UpdateTipText(strURL, this, TOOLTIP_ID);
}

void CHyperLink::SetColors(COLORREF crLinkColor, COLORREF crVisitedColor) 
{ 
	m_crLinkColor    = crLinkColor; 
	m_crVisitedColor = crVisitedColor; 
	Invalidate(); 
}

void CHyperLink::SetVisited(BOOL bVisited /* = TRUE */) 
{ 
	m_bVisited = bVisited; 
	Invalidate(); 
}

void CHyperLink::SetUnderline(BOOL bUnderline /* = TRUE */)
{
	m_bUnderline = bUnderline; 
	Invalidate(); 
}

void CHyperLink::SetAutoSize(BOOL bAutoSize /* = TRUE */)
{
	m_bAdjustToFit = bAutoSize;
	PositionWindow();
}


void CHyperLink::PositionWindow()
{
	CRect rect;

	if (!m_bAdjustToFit) 
		return;

	// Get the current window position
	GetWindowRect(rect);

	CWnd* pParent = GetParent();
	if (pParent)
		pParent->ScreenToClient(rect);

	// Get the size of the window text
	CString strWndText;
	GetWindowText(strWndText);

	CDC* pDC = GetDC();
	CSize Extent = pDC->GetTextExtent(strWndText);
	ReleaseDC(pDC);

	// Get the text justification via the window style
	DWORD dwStyle = GetStyle();

	// Recalc the window size and position based on the text justification
	if (dwStyle & SS_CENTERIMAGE)
		rect.DeflateRect(0, (rect.Height() - Extent.cy) / 2);
	else
		rect.bottom = rect.top + Extent.cy;

	if (dwStyle & SS_CENTER)   
		rect.DeflateRect((rect.Width() - Extent.cx) / 2, 0);
	else if (dwStyle & SS_RIGHT) 
		rect.left  = rect.right - Extent.cx;
	else // SS_LEFT = 0, so we can't test for it explicitly 
		rect.right = rect.left + Extent.cx;

	// Move the window
	SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);
}

/////////////////////////////////////////////////////////////////////////////
// CHyperLink implementation

void CHyperLink::OnClicked()
{
	int result = (int)GotoURL(m_strURL, SW_SHOW);
	m_bVisited = (result > HINSTANCE_ERROR);
	if (!m_bVisited)
		ShowError(result);
	else 
		SetVisited();						// Repaint to show visited color
}

LONG CHyperLink::GetRegKey(HKEY key, LPCTSTR subkey, LPTSTR retdata)
{
	HKEY hkey;
    LONG retval = RegOpenKeyEx(key, subkey, 0, KEY_QUERY_VALUE, &hkey);

    if (retval == ERROR_SUCCESS) {
		long datasize = MAX_PATH;
		TCHAR data[MAX_PATH];
		RegQueryValue(hkey, NULL, data, &datasize);
		lstrcpy(retdata,data);
		RegCloseKey(hkey);
	}

    return retval;
}

void CHyperLink::ShowError(int nError)
{
	CString str;
	switch (nError) {
		case 0:                       str = "The operating system is out\nof memory or resources."; break;
		case SE_ERR_PNF:              str = "The specified path was not found."; break;
		case SE_ERR_FNF:              str = "The specified file was not found."; break;
		case ERROR_BAD_FORMAT:        str = "The .EXE file is invalid\n(non-Win32 .EXE or error in .EXE image)."; break;
		case SE_ERR_ACCESSDENIED:     str = "The operating system denied\naccess to the specified file."; break;
		case SE_ERR_ASSOCINCOMPLETE:  str = "The filename association is\nincomplete or invalid."; break;
		case SE_ERR_DDEBUSY:          str = "The DDE transaction could not\nbe completed because other DDE transactions\nwere being processed."; break;
		case SE_ERR_DDEFAIL:          str = "The DDE transaction failed."; break;
		case SE_ERR_DDETIMEOUT:       str = "The DDE transaction could not\nbe completed because the request timed out."; break;
		case SE_ERR_DLLNOTFOUND:      str = "The specified dynamic-link library was not found."; break;
		case SE_ERR_NOASSOC:          str = "There is no application associated\nwith the given filename extension."; break;
		case SE_ERR_OOM:              str = "There was not enough memory to complete the operation."; break;
		case SE_ERR_SHARE:            str = "A sharing violation occurred. ";
		default:                      str.Format("Unknown Error (%d) occurred.", nError); break;
	}
	str = "Unable to open hyperlink:\n\n" + str;
	AfxMessageBox(str, MB_ICONEXCLAMATION | MB_OK);
}

HINSTANCE CHyperLink::GotoURL(LPCTSTR url, int showcmd)
{
	TCHAR key[MAX_PATH + MAX_PATH];

	// First try ShellExecute()
	HINSTANCE result = ShellExecute(NULL, _T("open"), url, NULL,NULL, showcmd);

	// If it failed, get the .htm regkey and lookup the program
	if ((UINT)result <= HINSTANCE_ERROR) {

		if (GetRegKey(HKEY_CLASSES_ROOT, _T(".htm"), key) == ERROR_SUCCESS) {
			lstrcat(key, _T("\\shell\\open\\command"));

			if (GetRegKey(HKEY_CLASSES_ROOT,key,key) == ERROR_SUCCESS) {
				TCHAR *pos;
				pos = _tcsstr(key, _T("\"%1\""));
				if (pos == NULL) {                       // No quotes found
					pos = strstr(key, _T("%1"));       // Check for %1, without quotes 
					if (pos == NULL)                   // No parameter at all...
						pos = key+lstrlen(key)-1;
					else
						*pos = '\0';                 // Remove the parameter
				}
				else
					*pos = '\0';                       // Remove the parameter

				lstrcat(pos, _T(" "));
				lstrcat(pos, url);
				result = (HINSTANCE) WinExec(key,showcmd);
			}
		}
	}

	return result;
}
