// TSearchServer.h : PROJECT_NAME ���ε{�����D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
	#error �b�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'
#endif

#include "resource.h"		// �D�n�Ÿ�


// CTSearchServerApp:
// �аѾ\��@�����O�� TSearchServer.cpp
//

class CTSearchServerApp : public CWinApp
{
public:
	CTSearchServerApp();

// �мg
	public:
	virtual BOOL InitInstance();

// �{���X��@

	DECLARE_MESSAGE_MAP()
};

extern CTSearchServerApp theApp;
