#pragma once


// CStripImgWnd
#include "MyListCtrlEx.h"
#include "MyTreeCtrlEx.h"
#include <vector>
using namespace std;
class CStripImgWnd : public CDockablePane
{
	DECLARE_DYNAMIC(CStripImgWnd)

public:
	CStripImgWnd();
	virtual ~CStripImgWnd();

public:
	CMyListCtrlReDraw m_ListStripImg;
	vector<CString> m_vecStripImgName;
	void LoadData(vector<CString> &vecStr);
	int m_nSelectedImgIdx;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMDblclkList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetdispinfoListImginfo(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};


#pragma once


// CPrjViewWnd

class CPrjViewWnd : public CDockablePane
{
	DECLARE_DYNAMIC(CPrjViewWnd)

public:
	CPrjViewWnd();
	virtual ~CPrjViewWnd();
	CMyTreeCtrlEx m_wndPrjTree;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};


