// PdfViewDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "sswUAVFlyQuaSysEng.h"
#include "PdfViewDlg.h"
#include "afxdialogex.h"

#include "MyFunctions.h"
#include "sswUAVFlyQuaSysEngDoc.h"
// CPdfViewDlg 对话框

IMPLEMENT_DYNAMIC(CPdfViewDlg, CDialogEx)

CPdfViewDlg::CPdfViewDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPdfViewDlg::IDD, pParent)
{
//	m_pDoc = NULL;
}

CPdfViewDlg::~CPdfViewDlg()
{
}

void CPdfViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPdfViewDlg, CDialogEx)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_MESSAGE(ID_MSG_SHOW_PDF, OnMsgShowPdf)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CPdfViewDlg 消息处理程序
LRESULT CPdfViewDlg::OnMsgShowPdf(WPARAM wParam,LPARAM lParam)
{
	CString str = *(CString*)lParam;
	ShowPdf(str);
	return 1;
}

int CPdfViewDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	CRect rectDummy;
	rectDummy.SetRectEmpty();
	if (!m_PdfWeb.Create(NULL,WS_CHILD|WS_VISIBLE,rectDummy,this, 2))
	{
		TRACE0("PDF window error\n");
		return -1;      // 未能创建
	}
	return 0;
}

void CPdfViewDlg::ShowPdf(CString strPath)
{
	m_PdfWeb.Navigate(strPath,NULL,NULL,NULL,NULL);
//	m_PdfWeb.UpdateData();
}
BOOL CPdfViewDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(hIcon, TRUE);
	SetIcon(hIcon, FALSE);
	CenterWindow();
	int nWndX = GetSystemMetrics(SM_CXSCREEN);
	int nWndY = GetSystemMetrics(SM_CYSCREEN);
	int nDlgX = min(800, nWndX);
	int nDlgY = nDlgX * 3 / 4;
	CRect rectWnd; GetWindowRect(rectWnd);
    SetWindowPos (NULL, -1, -1, nDlgX, nDlgY, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	CenterWindow();
// 	if(m_pDoc->m_eFlyQuaChkMethod==BY_POS) SetWindowText("质检报告-基于POS质检");else
// 		if(m_pDoc->m_eFlyQuaChkMethod==BY_MATCH) SetWindowText("质检报告-基于匹配的质检");else
// 			if(m_pDoc->m_eFlyQuaChkMethod==BY_ADJUSTMENT) SetWindowText("质检报告-基于平差的质检");
// 	m_PdfWeb.Navigate(m_pDoc->m_strFlyQuaRepPath,NULL,NULL,NULL,NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CPdfViewDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	m_PdfWeb.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	// TODO: 在此处添加消息处理程序代码
}

void CPdfViewDlg::ReleasePdf()
{
	m_PdfWeb.ExternalRelease();
	m_PdfWeb.ExternalDisconnect();
}
void CPdfViewDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ReleasePdf();
	CDialogEx::OnClose();
//	ShowWindow(FALSE);
}
