// DecodeSample.h : PROJECT_NAME ���ε{�����D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
	#error �b�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'
#endif

#include "resource.h"		// �D�n�Ÿ�


// CDecodeSampleApp:
// �аѾ\��@�����O�� DecodeSample.cpp
//

class CDecodeSampleApp : public CWinApp
{
public:
	CDecodeSampleApp();

// �мg
	public:
	virtual BOOL InitInstance();

// �{���X��@

	DECLARE_MESSAGE_MAP()
};

extern CDecodeSampleApp theApp;
