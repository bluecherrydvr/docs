// TSearchServer.cpp : �w�q���ε{�������O�欰�C
//

#include "stdafx.h"
#include "TSearchServer.h"
#include "TSearchServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTSearchServerApp

BEGIN_MESSAGE_MAP(CTSearchServerApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CTSearchServerApp �غc

CTSearchServerApp::CTSearchServerApp()
{
	// TODO: �b���[�J�غc�{���X�A
	// �N�Ҧ����n����l�]�w�[�J InitInstance ��
}


// �Ȧ����@�� CTSearchServerApp ����

CTSearchServerApp theApp;


// CTSearchServerApp ��l�]�w

BOOL CTSearchServerApp::InitInstance()
{
	// ���p���ε{����T�M����w�ϥ� ComCtl32.dll 6.0 (�t) �H�᪩��
	// �H�ҥε�ı�Ƽ˦��A�h Windows XP �ݭn InitCommonControls()�C�_�h���ܡA
	// ����������إ߱N���ѡC
	InitCommonControls();

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// �зǪ�l�]�w
	// �p�G�z���ϥγo�ǥ\��åB�Q��ֳ̫᧹�����i�����ɤj�p�A�z�i�H�q�U�C
	// �{���X�������ݭn����l�Ʊ`���A�ܧ��x�s�]�w�Ȫ��n�����X
	// TODO: �z���ӾA�׭ק惡�r�� (�Ҧp�A���q�W�٩β�´�W��)
	SetRegistryKey(_T("���� AppWizard �Ҳ��ͪ����ε{��"));

	CTSearchServerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �b����m��ϥ� [�T�w] �Ӱ���ϥι�ܤ����
		// �B�z���{���X
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �b����m��ϥ� [����] �Ӱ���ϥι�ܤ����
		// �B�z���{���X
	}

	// �]���w�g������ܤ���A�Ǧ^ FALSE�A�ҥH�ڭ̷|�������ε{���A
	// �ӫD���ܶ}�l���ε{�����T���C
	return FALSE;
}