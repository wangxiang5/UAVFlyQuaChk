// MyDialogEx.cpp : 实现文件
//

#include "stdafx.h"
#include "sswUAVFlyQuaSys.h"
#include "MyDialogEx.h"
#include "afxdialogex.h"


// CMyDialogEx 对话框

IMPLEMENT_DYNAMIC(CMyDialogEx, CDialogEx)

CMyDialogEx::CMyDialogEx(CWnd* pParent /*=NULL*/)
	: CDialogEx()
{
	CDialogEx();
}
CMyDialogEx::CMyDialogEx(UINT nIDTemplate, CWnd *pParent /* = NULL */)
	:CDialogEx(CMyDialogEx::IDD, pParent)
{
	CDialogEx();
}
CMyDialogEx::~CMyDialogEx()
{
}

void CMyDialogEx::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMyDialogEx, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CMyDialogEx 消息处理程序
void CMyDialogEx::InitItemPos()
{
	CRect rect;
	GetClientRect(&rect);     //取客户区大小    
	old.x = rect.right - rect.left;
	old.y = rect.bottom - rect.top;
	int cx = GetSystemMetrics(SM_CXFULLSCREEN);
	int cy = GetSystemMetrics(SM_CYFULLSCREEN);
	CRect rt;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rt, 0);
	cy = rt.bottom;
	MoveWindow(0, 0, cx, cy);
}

void CMyDialogEx::ResetItemPos()
{
	float fsp[2];
	POINT Newp; //获取现在对话框的大小  
	CRect recta;
	GetClientRect(&recta);     //取客户区大小    
	Newp.x = recta.right - recta.left;
	Newp.y = recta.bottom - recta.top;
	fsp[0] = (float)Newp.x / old.x;
	fsp[1] = (float)Newp.y / old.y;
	CRect Rect;
	int woc;
	CPoint OldTLPoint, TLPoint; //左上角  
	CPoint OldBRPoint, BRPoint; //右下角  
	HWND  hwndChild = ::GetWindow(m_hWnd, GW_CHILD);  //列出所有控件    
	while (hwndChild)
	{
		woc = ::GetDlgCtrlID(hwndChild);//取得ID  
		GetDlgItem(woc)->GetWindowRect(Rect);
		ScreenToClient(Rect);
		OldTLPoint = Rect.TopLeft();
		TLPoint.x = long(OldTLPoint.x*fsp[0]);
		TLPoint.y = long(OldTLPoint.y*fsp[1]);
		OldBRPoint = Rect.BottomRight();
		BRPoint.x = long(OldBRPoint.x *fsp[0]);
		BRPoint.y = long(OldBRPoint.y *fsp[1]);
		Rect.SetRect(TLPoint, BRPoint);
		GetDlgItem(woc)->MoveWindow(Rect, TRUE);
		hwndChild = ::GetWindow(hwndChild, GW_HWNDNEXT);
	}
	old = Newp;
}

BOOL CMyDialogEx::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	InitItemPos();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CMyDialogEx::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	ResetItemPos();
	// TODO: 在此处添加消息处理程序代码
}
