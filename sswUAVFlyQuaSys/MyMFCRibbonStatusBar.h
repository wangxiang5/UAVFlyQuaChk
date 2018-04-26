#pragma once
#include "afxribbonstatusbar.h"
class CMyMFCRibbonStatusBar :
	public CMFCRibbonStatusBar
{
public:
	CMyMFCRibbonStatusBar(void);
	~CMyMFCRibbonStatusBar(void);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};

