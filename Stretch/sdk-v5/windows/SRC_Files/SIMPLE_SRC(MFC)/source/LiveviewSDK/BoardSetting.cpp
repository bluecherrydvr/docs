// BoardSetting.cpp : 實作檔
//

#include "stdafx.h"
#include "DvrFCD.h"
#include "BoardSetting.h"


// CBoardSetting 對話方塊

IMPLEMENT_DYNAMIC(CBoardSetting, CDialog)

CBoardSetting::CBoardSetting(CWnd* pParent /*=NULL*/)
	: CDialog(CBoardSetting::IDD, pParent)
{

}

CBoardSetting::~CBoardSetting()
{
}

void CBoardSetting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CBoardSetting, CDialog)
END_MESSAGE_MAP()


// CBoardSetting 訊息處理常式
