// ArchivePlayer.h : PROJECT_NAME ���ε{�����D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
	#error �b�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'
#endif

#include "resource.h"		// �D�n�Ÿ�


// CArchivePlayerApp:
// �аѾ\��@�����O�� ArchivePlayer.cpp
//

class CArchivePlayerApp : public CWinApp
{
public:
	CArchivePlayerApp();

// �мg
	public:
	virtual BOOL InitInstance();

// �{���X��@

	DECLARE_MESSAGE_MAP()
};

extern CArchivePlayerApp theApp;
