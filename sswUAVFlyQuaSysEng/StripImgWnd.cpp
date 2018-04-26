// StripImgWnd.cpp : 实现文件
//

#include "stdafx.h"
#include "sswUAVFlyQuaSysEng.h"
#include "StripImgWnd.h"
#include "ImgViewDlg.h"
#include "MyFunctions.h"
#include "MainFrm.h"
#include "sswUAVFlyQuaSysEngDoc.h"
#include "sswUAVFlyQuaSysEngView.h"
// CStripImgWnd

IMPLEMENT_DYNAMIC(CStripImgWnd, CDockablePane)

CStripImgWnd::CStripImgWnd()
{
	m_nSelectedImgIdx = InvalidValue;
}

CStripImgWnd::~CStripImgWnd()
{

}


BEGIN_MESSAGE_MAP(CStripImgWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_STRIPIMG, &CStripImgWnd::OnGetdispinfoListImginfo)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_STRIPIMG,&CStripImgWnd::OnNMDblclkList)
		ON_NOTIFY(NM_CLICK, IDC_LIST_STRIPIMG,&CStripImgWnd::OnNMClickList)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()



// CStripImgWnd 消息处理程序
void CStripImgWnd::LoadData(vector<CString> &vecStr)
{
	vector<CString>().swap(m_vecStripImgName);
	m_vecStripImgName = vecStr;
	m_ListStripImg.SetItemCountEx(m_vecStripImgName.size());
	m_ListStripImg.Invalidate();
}
void CStripImgWnd::OnGetdispinfoListImginfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pGetInfoTip = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	LV_ITEM* pItem = &(pGetInfoTip)->item;
	if(m_vecStripImgName.size()==0) return;
	if (pItem->mask & LVIF_TEXT)
	{
		CString strImgName = m_vecStripImgName[pItem->iItem];
		switch (pItem->iSubItem)
		{
		case 0:{
			char strId[10]; sprintf(strId, "%d", pItem->iItem);
			lstrcpyn(pItem->pszText, strId, pItem->cchTextMax);
			break;
			   }
		case 1:{
			lstrcpyn(pItem->pszText, strImgName, pItem->cchTextMax);
			break;
			   }
		default:
			break;
		}
	}
	*pResult = 0;
}

void CStripImgWnd::OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_LISTVIEW*   pNMListView = (NM_LISTVIEW*)pNMHDR;   
	int nItem = pNMListView -> iItem; 
	int nStripIdx = GetViewHand()->m_nStripIdxSelected;
	if(nItem >= 0)  
	{ 
		if(nStripIdx>=0)
		{
			int nIdxInStrip = nItem;
			m_nSelectedImgIdx = GetDocHand()->m_FlyQuaPrj.GetImgIdx(nStripIdx,nIdxInStrip);

		}
		else
		{
			m_nSelectedImgIdx = GetDocHand()->m_FlyQuaPrj.GetInvalidImgIdx(nItem);
		}
		CImgViewDlg dlg;
		dlg.DoModal();
	}
}
void CStripImgWnd::OnNMClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	vector<int> vecSelectedIdx;
	CString str;
	POSITION pos=m_ListStripImg.GetFirstSelectedItemPosition(); //pos选中的首行位置
	if(pos!=NULL)
	{
		while(pos)   //如果你选择多行
		{
			int nIdx=-1;
			nIdx= m_ListStripImg.GetNextSelectedItem(pos);

			if(nIdx >=0&&nIdx<m_ListStripImg.GetItemCount())
			{
				vecSelectedIdx.push_back(nIdx);
			}
		}
	}
	if(vecSelectedIdx.size() == 0) return;
	int nStripIdx = GetViewHand()->m_nStripIdxSelected;
	if(nStripIdx>=0)
	{
		for (int i = 0; i<vecSelectedIdx.size(); i++)
		{
			vecSelectedIdx[i] = GetDocHand()->m_FlyQuaPrj.GetImgIdx(nStripIdx,vecSelectedIdx[i]);
		}
	}
	else
	{
		for (int i = 0; i<vecSelectedIdx.size(); i++)
		{
			vecSelectedIdx[i] = GetDocHand()->m_FlyQuaPrj.GetInvalidImgIdx(vecSelectedIdx[i]);
		}
	}
	GetViewHand()->SendMessage(ID_MSG_CREATE_SELECTRENDER,(WPARAM)&vecSelectedIdx,0);
	vector<int>().swap(vecSelectedIdx);
}


int CStripImgWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	CRect rectDummy; rectDummy.SetRectEmpty();
 	/*const*/ DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;
 	m_ListStripImg.Create(dwStyle,rectDummy,this,IDC_LIST_STRIPIMG);
	m_ListStripImg.ModifyStyle(0, LVS_REPORT);
	m_ListStripImg.ModifyStyle(0, LVS_OWNERDRAWFIXED);

// 	DWORD dwStyle = (m_ListStripImg).GetExtendedStyle();
// 	dwStyle |= LVS_EX_FULLROWSELECT;     //可以选择整行
// 	dwStyle |= LVS_EX_GRIDLINES;         //格网形式 
// 	dwStyle |= LVS_OWNERDRAWFIXED;       //可重绘
// 	dwStyle |= LVS_EX_DOUBLEBUFFER;
// 	m_ListStripImg.SetExtendedStyle(dwStyle);

	CRect RECT;
	m_ListStripImg.GetClientRect(RECT);
	int nListWidth = RECT.Width();
	int nListHeight = RECT.Height();
	m_ListStripImg.InsertColumn(0, "Id", LVCFMT_CENTER, (float)nListWidth / 8, -1);
	m_ListStripImg.InsertColumn(1, "Image", LVCFMT_LEFT, (float)nListWidth * 9 / 10, -1);
	return 0;
}


void CStripImgWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	m_ListStripImg.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	CRect rect; m_ListStripImg.GetClientRect(rect);
	m_ListStripImg.SetColumnWidth(0,40);
	m_ListStripImg.SetColumnWidth(1,rect.Width()-40);
	// TODO: 在此处添加消息处理程序代码
}

// CPrjViewWnd

IMPLEMENT_DYNAMIC(CPrjViewWnd, CDockablePane)

CPrjViewWnd::CPrjViewWnd()
{

}

CPrjViewWnd::~CPrjViewWnd()
{
}


BEGIN_MESSAGE_MAP(CPrjViewWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()



// CPrjViewWnd 消息处理程序




int CPrjViewWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	CRect rectDummy(0, 0, 0, 0);
	const DWORD dwTreeStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;
	m_wndPrjTree.Create(dwTreeStyle, rectDummy, this, IDC_PRJTREE_CTRL);
	return 0;
}


void CPrjViewWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	m_wndPrjTree.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	// TODO: 在此处添加消息处理程序代码
}


void CPrjViewWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	// TODO: 在此处添加消息处理程序代码
}


void CStripImgWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	// TODO: 在此处添加消息处理程序代码
}
