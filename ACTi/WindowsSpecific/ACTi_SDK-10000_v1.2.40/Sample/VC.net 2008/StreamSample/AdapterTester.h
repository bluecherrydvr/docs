// AdapterTester.h : PROJECT_NAME ���ε{�����D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
	#error �b�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'
#endif

#include "resource.h"		// �D�n�Ÿ�


// CAdapterTesterApp:
// �аѾ\��@�����O�� AdapterTester.cpp
//

class CAdapterTesterApp : public CWinApp
{
public:
	CAdapterTesterApp();

// �мg
	public:
	virtual BOOL InitInstance();

// �{���X��@

	DECLARE_MESSAGE_MAP()
};

extern CAdapterTesterApp theApp;
