// RTPSample.h : PROJECT_NAME ���ε{�����D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
	#error �b�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'
#endif

#include "resource.h"		// �D�n�Ÿ�


// CRTPSampleApp:
// �аѾ\��@�����O�� RTPSample.cpp
//

class CRTPSampleApp : public CWinApp
{
public:
	CRTPSampleApp();

// �мg
	public:
	virtual BOOL InitInstance();

// �{���X��@

	DECLARE_MESSAGE_MAP()
};

extern CRTPSampleApp theApp;
