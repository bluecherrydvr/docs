// AbsolutePosition.h : PROJECT_NAME ���ε{�����D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
	#error �b�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'
#endif

#include "resource.h"		// �D�n�Ÿ�


// CAbsolutePositionApp:
// �аѾ\��@�����O�� AbsolutePosition.cpp
//

class CAbsolutePositionApp : public CWinApp
{
public:
	CAbsolutePositionApp();

// �мg
	public:
	virtual BOOL InitInstance();

// �{���X��@

	DECLARE_MESSAGE_MAP()
};

extern CAbsolutePositionApp theApp;
