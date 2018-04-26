// MyMFCRibbonBar.cpp : 实现文件
//

#include "stdafx.h"
#include "sswUAVFlyQuaSysEng.h"
#include "MyMFCRibbonBar.h"


// CMyMFCRibbonBar

IMPLEMENT_DYNAMIC(CMyMFCRibbonBar, CMFCRibbonBar)

CMyMFCRibbonBar::CMyMFCRibbonBar()
{

}

CMyMFCRibbonBar::~CMyMFCRibbonBar()
{
}


BEGIN_MESSAGE_MAP(CMyMFCRibbonBar, CMFCRibbonBar)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()



// CMyMFCRibbonBar 消息处理程序




void CMyMFCRibbonBar::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	// TODO: 在此处添加消息处理程序代码
}
