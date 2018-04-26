// 这段 MFC 示例源代码演示如何使用 MFC Microsoft Office Fluent 用户界面 
// (“Fluent UI”)。该示例仅供参考，
// 用以补充《Microsoft 基础类参考》和 
// MFC C++ 库软件随附的相关电子文档。
// 复制、使用或分发 Fluent UI 的许可条款是单独提供的。
// 若要了解有关 Fluent UI 许可计划的详细信息，请访问  
// http://msdn.microsoft.com/officeui。
//
// 版权所有(C) Microsoft Corporation
// 保留所有权利。

#include "stdafx.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

COutputWnd::COutputWnd()
{
}

COutputWnd::~COutputWnd()
{
}

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

// 	// 创建选项卡窗口:
// 	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1))
// 	{
// 		TRACE0("未能创建输出选项卡窗口\n");
// 		return -1;      // 未能创建
// 	}

	// 创建输出窗格:
	const DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;

// 	if (!m_wndOutputBuild.Create(dwStyle, rectDummy, &m_wndTabs, 2) ||
// 		!m_wndOutputDebug.Create(dwStyle, rectDummy, &m_wndTabs, 3) ||
// 		!m_wndOutputFind.Create(dwStyle, rectDummy, &m_wndTabs, 4))
// 	{
// 		TRACE0("未能创建输出窗口\n");
// 		return -1;      // 未能创建
// 	}
	if (!m_wndOutputLog.Create(dwStyle, rectDummy, this, 2))
	{
		TRACE0("未能创建输出窗口\n");
		return -1;      // 未能创建
	}
	UpdateFonts();

	CString strTabName;
	BOOL bNameValid;

	// 将列表窗口附加到选项卡:
// 	bNameValid = strTabName.LoadString(IDS_BUILD_TAB);
// 	ASSERT(bNameValid);
//	m_wndTabs.AddTab(&m_wndOultputBuild, strTabName, (UINT)0);
// 	bNameValid = strTabName.LoadString(IDS_DEBUG_TAB);
// 	ASSERT(bNameValid);
// 	m_wndTabs.AddTab(&m_wndOutputDebug, strTabName, (UINT)1);
// 	bNameValid = strTabName.LoadString(IDS_FIND_TAB);
// 	ASSERT(bNameValid);
// 	m_wndTabs.AddTab(&m_wndOutputFind, strTabName, (UINT)2);

	// 使用一些虚拟文本填写输出选项卡(无需复杂数据)
//	FillLogWindow();
// 	FillDebugWindow();
// 	FillFindWindow();

	return 0;
}

void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// 选项卡控件应覆盖整个工作区:
	m_wndOutputLog.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void COutputWnd::AdjustHorzScroll(CListBox& wndListBox)
{
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

	int cxExtentMax = 0;

	for (int i = 0; i < wndListBox.GetCount(); i ++)
	{
		CString strItem;
		wndListBox.GetText(i, strItem);

		cxExtentMax = max(cxExtentMax, dc.GetTextExtent(strItem).cx);
	}

	wndListBox.SetHorizontalExtent(cxExtentMax);
	dc.SelectObject(pOldFont);
}

void COutputWnd::FillLogWindow(CString strLog, bool bAdd)
{
	if(!bAdd&&m_wndOutputLog.GetCount()>0)
	{
		m_wndOutputLog.DeleteString(m_wndOutputLog.GetCount()-1);
	}
	m_wndOutputLog.AddString(strLog);
	if(m_wndOutputLog.GetCount()>1)
		m_wndOutputLog.SetCurSel(m_wndOutputLog.GetCount()-1);
	m_wndOutputLog.SetRedraw(TRUE);
}
void COutputWnd::UpdateFonts()
{
	m_wndOutputLog.SetFont(&afxGlobalData.fontRegular);
// 	m_wndOutputDebug.SetFont(&afxGlobalData.fontRegular);
// 	m_wndOutputFind.SetFont(&afxGlobalData.fontRegular);
}

/////////////////////////////////////////////////////////////////////////////
// COutputList1

COutputList::COutputList()
{
	m_bScroll = false;
}

COutputList::~COutputList()
{
}

BEGIN_MESSAGE_MAP(COutputList, CListBox)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEWHEEL()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// COutputList 消息处理程序

void COutputList::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_OUTPUT_POPUP);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}

	SetFocus();
}

void COutputList::OnEditCopy()
{
	MessageBox(_T("复制输出"));
}

void COutputList::OnEditClear()
{
	MessageBox(_T("清除输出"));
}

void COutputList::OnViewOutput()
{
	CDockablePane* pParentBar = DYNAMIC_DOWNCAST(CDockablePane, GetOwner());
	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame());

	if (pMainFrame != NULL && pParentBar != NULL)
	{
		pMainFrame->SetFocus();
		pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
		pMainFrame->RecalcLayout();

	}
}


BOOL COutputList::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(!m_bScroll) return TRUE;
	else return CListBox::OnEraseBkgnd(pDC);
	m_bScroll = false;
}


void COutputList::OnPaint()
{
//	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CListBox::OnPaint()
	CPaintDC dc(this); // device context for painting

	CRect   rect; 
	GetClientRect(&rect); 

	CDC   MemDC;//内存DC 
	CBitmap   MemBmp; 
	MemDC.CreateCompatibleDC(&dc); 
	MemBmp.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	MemDC.SelectObject(&MemBmp); 
	//这时创建的位图只是一张黑色的纸
	//先画上背景    
	MemDC.FillSolidRect(rect, RGB(255, 255, 255));
	//再画上图形
	//这一句是调用默认的OnPaint(),把图形画在内存DC表上 
	DefWindowProc(WM_PAINT, (WPARAM)MemDC.m_hDC, (LPARAM)0); 

	//输出 
	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &MemDC, 0, 0, SRCCOPY); 

	MemDC.DeleteDC(); 
	MemBmp.DeleteObject(); 
}


void COutputList::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_bScroll = true;
	CListBox::OnVScroll(nSBCode, nPos, pScrollBar);
}


void COutputList::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_bScroll =true;
	CListBox::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL COutputList::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	m_bScroll = true;
	return CListBox::OnMouseWheel(nFlags,zDelta,pt);
}

void COutputWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	// TODO: 在此处添加消息处理程序代码
}
