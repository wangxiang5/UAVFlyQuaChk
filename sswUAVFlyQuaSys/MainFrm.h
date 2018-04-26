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

// MainFrm.h : CMainFrame 类的接口
//

#pragma once
#include "OutputWnd.h"
#include "Resource.h"
#include "ThumbWnd.h"
#include "StripImgWnd.h"
#include "DataChartWnd.h"
#include "PdfViewDlg.h"
#include "MyMFCRibbonBar.h"
#include "MyMFCRibbonStatusBar.h"
class COutlookBar : public CMFCOutlookBar
{
	virtual BOOL AllowShowOnPaneMenu() const { return TRUE; }
	virtual void GetPaneName(CString& strName) const { BOOL bNameValid = strName.LoadString(IDS_OUTLOOKBAR); ASSERT(bNameValid); if (!bNameValid) strName.Empty(); }
};
class CsswUAVFlyQuaSysDoc;
class CMainFrame : public CFrameWndEx
{
	
protected: // 仅从序列化创建
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// 特性
public:
	CPdfViewDlg *m_pDlgRepByAdj;
	CPdfViewDlg *m_pDlgRepByMch;
	CPdfViewDlg *m_pDlgRepByPos;
// 操作
public:
// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	void HideOrShowAllWnd(bool bShow = true);
	bool CreatePaneWnd();
// 实现
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
public:
	void ResetStatusPaneSize();
	void ShowWndTree(vector<CString> vecItemInfo);
	void ShowStripList(vector<CString> vecItemInfo);
	void ShowRibbonValue();
	void FillLogWnd(CString strLog, bool bAdd);
	void UpdateStatusLabel(CString strLabel);
public:  // 控件条嵌入成员
	CsswUAVFlyQuaSysDoc *m_pDoc;
	CMyMFCRibbonBar     m_wndRibbonBar;
	CMFCRibbonApplicationButton m_MainButton;
	CMFCToolBarImages m_PanelImages;
	CMyMFCRibbonStatusBar  m_wndStatusBar;
	COutlookBar       m_wndNavigationBar;
	CMFCCaptionBar    m_wndCaptionBar;
	CTreeCtrl m_wndTree;
	COutputWnd        m_wndOutput;
	CThumbWnd      m_wndImgAndPt;
	CStripImgWnd      m_wndStripImg;
	CPrjViewWnd       m_wndPrjSource;
	CDataChartWnd     m_wndDataChart;
	CMFCRibbonStatusBarPane *m_pStatusPaneLabel;
	CMFCRibbonStatusBarPane *m_pStatusPaneProcessing;
	CMFCRibbonStatusBarPane *m_pStatusPanePosition;
	CMFCRibbonStatusBarPane *m_pStatusPaneSysTime;

	bool m_bWndReady;
// 生成的消息映射函数
public:
	afx_msg LRESULT OnMsgSaveReport(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMsgUpdateWndProgressCtrl(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMsgShowWndProgressCtrl(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMsgFillLogWnd(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMsgOpenPdfReport(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMsgUpdateStatusLabel(WPARAM wParam, LPARAM lParam);
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnFilePrint();
	afx_msg void OnFilePrintPreview();
	afx_msg void OnUpdateFilePrintPreview(CCmdUI* pCmdUI);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);
	BOOL CreateOutlookBar(CMFCOutlookBar& bar, UINT uiID, CTreeCtrl& tree, int nInitialWidth);
	BOOL CreateCaptionBar();

	int FindFocusedOutlookWnd(CMFCOutlookBarTabCtrl** ppOutlookWnd);

	CMFCOutlookBarTabCtrl* FindOutlookParent(CWnd* pWnd);
	CMFCOutlookBarTabCtrl* m_pCurrOutlookWnd;
	CMFCOutlookBarPane*    m_pCurrOutlookPage;
public:
	afx_msg void OnButtonSystemConfigure();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual void ActivateFrame(int nCmdShow = -1);
//	afx_msg void OnStatic();
//	afx_msg void OnButtonApp();
	afx_msg void OnCheckViewThumbwnd();
	afx_msg void OnCheckViewOutputwnd();
	afx_msg void OnUpdateCheckViewOutputwnd(CCmdUI *pCmdUI);
	afx_msg void OnCheckViewStripimgwnd();
	afx_msg void OnUpdateCheckViewStripimgwnd(CCmdUI *pCmdUI);
	afx_msg void OnUpdateCheckViewThumbwnd(CCmdUI *pCmdUI);
	afx_msg void OnCheckViewChartwnd();
	afx_msg void OnUpdateCheckViewChartwnd(CCmdUI *pCmdUI);
	afx_msg void OnCheckViewDatachartwnd();
	afx_msg void OnUpdateCheckViewDatachartwnd(CCmdUI *pCmdUI);
	afx_msg void OnViewRibbon();
	afx_msg void OnUpdateViewRibbon(CCmdUI *pCmdUI);
	afx_msg void OnDestroy();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};


