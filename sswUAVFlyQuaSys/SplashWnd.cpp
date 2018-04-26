// SplashWnd.cpp : 实现文件
//

#include "stdafx.h"
#include "sswUAVFlyQuaSys.h"
#include "SplashWnd.h"
#include "MyFunctions.h"

// CSplashWnd

IMPLEMENT_DYNAMIC(CSplashWnd, CWnd)

CSplashWnd::CSplashWnd()
{

}

CSplashWnd::~CSplashWnd()
{
}


BEGIN_MESSAGE_MAP(CSplashWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// CSplashWnd 消息处理程序




bool CSplashWnd::CreateBmp()
{

	// TODO:  在此添加您专用的创建代码
	m_bitmap_bak.LoadBitmap(IDB_BITMAP_SPLASHBACK);
	BITMAP bitmap;
	m_bitmap_bak.GetBitmap(&bitmap);
	CreateEx(0,	AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
		NULL, WS_POPUP | WS_VISIBLE, 0, 0, bitmap.bmWidth, bitmap.bmHeight, NULL, NULL);

// 	CString strBakBmp = FunGetFileFolder(FunGetThisExePath())+"\\resource\\MainFrame.bmp";
// 	m_bitmap_bak.Detach(); 
// 	HBITMAP hBitmapBak = (HBITMAP)::LoadImage(NULL,strBakBmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION|LR_LOADFROMFILE|LR_DEFAULTSIZE);
// 	if(!m_bitmap_bak.Attach(hBitmapBak)) return false;
// 	BITMAP bitmap;
// 	m_bitmap_bak.GetBitmap(&bitmap);
// 	CreateEx(0,	AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
// 		NULL, WS_POPUP | WS_VISIBLE, 0, 0, bitmap.bmWidth, bitmap.bmHeight, NULL, NULL);
	CenterWindow();
	GetClientRect(m_WndRect);
	Sleep(2000);
	return true;
}


void CSplashWnd::OnPaint()
{
	CPaintDC dc(this); // device context forpainting
// 	BITMAP bitmap;
// 	m_bitmap_fly.GetBitmap(&bitmap);
// 	CDC dcComp;
// 	dcComp.CreateCompatibleDC(&dc);
// 	dcComp.SelectObject(&m_bitmap_bak);
// 	dc.BitBlt(m_nPosX_BmpFly, m_nPosX_BmpFly, bitmap.bmWidth, bitmap.bmHeight, &dcComp, 0, 0, SRCCOPY);
}


void CSplashWnd::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CWnd::OnTimer(nIDEvent);
}


BOOL CSplashWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CPaintDC dc(this);
	BITMAP bitmap;
	m_bitmap_bak.GetBitmap(&bitmap);
	CDC dcComp;
	dcComp.CreateCompatibleDC(&dc);
	dcComp.SelectObject(&m_bitmap_bak);
	dc.BitBlt(0, 0, bitmap.bmWidth, bitmap.bmHeight, &dcComp, 0, 0, SRCCOPY);
	return true;
}
