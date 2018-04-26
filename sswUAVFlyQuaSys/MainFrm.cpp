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

// MainFrm.cpp : CMainFrame 类的实现
//

#include "stdafx.h"
#include "sswUAVFlyQuaSys.h"

#include "MainFrm.h"
#include "SysConfigureDlg.h"
#include "GradientProgressCtrl.h"
#include "sswUAVFlyQuaSysDoc.h"
#include "sswUAVFlyQuaSysView.h"
#include "MyFunctions.h"
#include "SplashWnd.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

CGradientProgressCtrl g_wndProgressCtrl;  //状态栏进度条
CMainFrame *g_pMain = NULL;
HGLRC g_pRC = NULL;
HGLRC g_pRCSharing = NULL;
IMPLEMENT_DYNCREATE(CMainFrame, CFrameWndEx)

	BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
		ON_WM_CREATE()
		ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
		ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
		ON_COMMAND(ID_FILE_PRINT, &CMainFrame::OnFilePrint)
		ON_COMMAND(ID_FILE_PRINT_DIRECT, &CMainFrame::OnFilePrint)
		ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMainFrame::OnFilePrintPreview)
		ON_UPDATE_COMMAND_UI(ID_FILE_PRINT_PREVIEW, &CMainFrame::OnUpdateFilePrintPreview)
		ON_WM_SETTINGCHANGE()
		ON_COMMAND(ID_BUTTON_SYSTEM_CONFIGURE, &CMainFrame::OnButtonSystemConfigure)
		ON_WM_SIZE()
		ON_WM_TIMER()
		ON_MESSAGE(ID_MSG_UPDATE_STATUSLABEL, OnMsgUpdateStatusLabel)
		ON_MESSAGE(ID_MSG_OPEN_REPORT, OnMsgOpenPdfReport)
		ON_MESSAGE(ID_MSG_FILL_WND_LOG, OnMsgFillLogWnd)
		ON_MESSAGE(ID_MSG_WNDPROGRESSCTRL_POS, OnMsgUpdateWndProgressCtrl)
		ON_MESSAGE(ID_MSG_WNDPROGRESSCTRL_SHOW, OnMsgShowWndProgressCtrl)
		ON_MESSAGE(ID_MSG_SAVE_REPORT, OnMsgSaveReport)

		ON_COMMAND(ID_CHECK_VIEW_THUMBWND, &CMainFrame::OnCheckViewThumbwnd)
		ON_COMMAND(ID_CHECK_VIEW_OUTPUTWND, &CMainFrame::OnCheckViewOutputwnd)
		ON_UPDATE_COMMAND_UI(ID_CHECK_VIEW_OUTPUTWND, &CMainFrame::OnUpdateCheckViewOutputwnd)
		ON_COMMAND(ID_CHECK_VIEW_STRIPIMGWND, &CMainFrame::OnCheckViewStripimgwnd)
		ON_UPDATE_COMMAND_UI(ID_CHECK_VIEW_STRIPIMGWND, &CMainFrame::OnUpdateCheckViewStripimgwnd)
		ON_UPDATE_COMMAND_UI(ID_CHECK_VIEW_THUMBWND, &CMainFrame::OnUpdateCheckViewThumbwnd)
		ON_COMMAND(ID_CHECK_VIEW_DATACHARTWND, &CMainFrame::OnCheckViewDatachartwnd)
		ON_UPDATE_COMMAND_UI(ID_CHECK_VIEW_DATACHARTWND, &CMainFrame::OnUpdateCheckViewDatachartwnd)
		ON_COMMAND(ID_VIEW_RIBBON, &CMainFrame::OnViewRibbon)
		ON_UPDATE_COMMAND_UI(ID_VIEW_RIBBON, &CMainFrame::OnUpdateViewRibbon)
		ON_WM_DESTROY()
		ON_WM_CONTEXTMENU()
	END_MESSAGE_MAP()

	// CMainFrame 构造/析构

	CMainFrame::CMainFrame()
	{
		// TODO: 在此添加成员初始化代码
		theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_BLUE);
		m_bWndReady = false;
		m_pDoc = NULL;
		m_pDlgRepByAdj = NULL;
		m_pDlgRepByMch = NULL;
		m_pDlgRepByPos = NULL;
	}

	CMainFrame::~CMainFrame()
	{
		if(m_pDlgRepByAdj) m_pDlgRepByAdj->DestroyWindow();
		if(m_pDlgRepByMch) m_pDlgRepByMch->DestroyWindow();
		if(m_pDlgRepByPos) m_pDlgRepByPos->DestroyWindow();
	}

	int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
			return -1;

		BOOL bNameValid;
		// 基于持久值设置视觉管理器和样式
		OnApplicationLook(theApp.m_nAppLook);
		CSplashWnd *pSplashWnd;
		pSplashWnd = new CSplashWnd;
		pSplashWnd->CreateBmp();
		pSplashWnd->UpdateWindow();
		pSplashWnd->DestroyWindow();
		m_wndRibbonBar.Create(this);
		m_wndRibbonBar.LoadFromResource(IDR_RIBBON);

		if (!m_wndStatusBar.Create(this))
		{
			TRACE0("未能创建状态栏\n");
			return -1;      // 未能创建
		}

		CString strTitlePane1;
		CString strTitlePane2;
		bNameValid = strTitlePane1.LoadString(IDS_STATUS_PANE1);
		ASSERT(bNameValid);
		bNameValid = strTitlePane2.LoadString(IDS_STATUS_PANE2);
		ASSERT(bNameValid);
		m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE1, "", TRUE), strTitlePane1);
		m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE2, "", TRUE), "x(0), y(0)");	
		m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE2, "", TRUE), "Ready");

		m_wndStatusBar.AddExtendedElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE1, "System Time", TRUE), FunGetSysTimeStr());
		m_pStatusPaneLabel = (CMFCRibbonStatusBarPane*)m_wndStatusBar.GetElement(0);
		m_pStatusPanePosition = (CMFCRibbonStatusBarPane*)m_wndStatusBar.GetElement(1);
		m_pStatusPaneProcessing = (CMFCRibbonStatusBarPane*)m_wndStatusBar.GetElement(2);

		m_pStatusPaneSysTime = (CMFCRibbonStatusBarPane*)m_wndStatusBar.GetExElement(0);


		m_pStatusPaneLabel->SetAlmostLargeText("00000000000000000000000000000000000000000000");
		m_pStatusPaneProcessing->SetAlmostLargeText("00000000000000000000000000000000000");
		m_pStatusPanePosition->SetAlmostLargeText("Position: 10000000.000 3000000000.000");
		m_pStatusPaneSysTime->SetAlmostLargeText("0000-00-00  00-00-00");

		// 启用 Visual Studio 2005 样式停靠窗口行为
		CDockingManager::SetDockingMode(DT_SMART);
		// 启用 Visual Studio 2005 样式停靠窗口自动隐藏行为
		EnableAutoHidePanes(CBRS_ALIGN_ANY);

		// 导航窗格将创建在左侧，因此将暂时禁用左侧的停靠:
		EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM | CBRS_ALIGN_RIGHT);

		// 已创建 Outlook 栏，应允许在左侧停靠。
		EnableDocking(CBRS_ALIGN_LEFT);
		EnableAutoHidePanes(CBRS_ALIGN_RIGHT);

	//	CreatePaneWnd();


		CRect RectStatus, RectSystemTime, RectProgress;
		m_wndStatusBar.GetClientRect(RectStatus);
		RectSystemTime = m_pStatusPaneSysTime->GetRect();
		RectProgress.right = max(RectStatus.left + 5, RectStatus.right - RectSystemTime.Width() - 3);
		RectProgress.left = max(RectProgress.right - 200, RectStatus.left + 1);
		RectProgress.top = RectSystemTime.top;
		RectProgress.bottom = RectSystemTime.bottom;

		if (!g_wndProgressCtrl.Create(WS_VISIBLE | WS_CHILD | PBS_SMOOTH, RectProgress, &m_wndStatusBar, 1))
		{
			TRACE0("Failed to create windows progressctrl\n");
			return -1;
		}
		g_wndProgressCtrl.ShowWindow(false);
		g_wndProgressCtrl.SetStartColor(RGB(255, 0, 0));
		g_wndProgressCtrl.SetEndColor(RGB(30, 144, 255));
		g_wndProgressCtrl.SetTextColor(RGB(255, 0, 0));
		g_wndProgressCtrl.SetRange(0, 100);
		g_wndProgressCtrl.SetPos(0);

		m_bWndReady = TRUE;
		ResetStatusPaneSize();
		SetTimer(TIME_ID_SYSTIME, 1000, NULL);
		m_pStatusPaneLabel->SetText("就绪");

		if(g_pMain==NULL) 	g_pMain = this;

		int ntag = m_wndRibbonBar.GetHideFlags();  //启动时ribbon显示
		if (ntag == 1)
		{
			m_wndRibbonBar.ToggleMimimizeState();
		}
	//	HideOrShowAllWnd(false);

		return 0;
	}
	bool CMainFrame::CreatePaneWnd()
	{
		// 创建停靠窗口
		if (!CreateDockingWindows())
		{
			TRACE0("未能创建停靠窗口\n");
			return false;
		}
		m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
		m_wndImgAndPt.EnableDocking(CBRS_ALIGN_ANY);
		m_wndStripImg.EnableDocking(CBRS_ALIGN_ANY);
		m_wndPrjSource.EnableDocking(CBRS_ALIGN_ANY);
		m_wndDataChart.EnableDocking(CBRS_ALIGN_ANY);
		DockPane(&m_wndPrjSource);
		DockPane(&m_wndDataChart);
		DockPane(&m_wndImgAndPt);
		DockPane(&m_wndStripImg);
		DockPane(&m_wndOutput);
		m_wndStripImg.DockToWindow(&m_wndPrjSource,CBRS_BOTTOM);
		m_wndImgAndPt.DockToWindow(&m_wndDataChart,CBRS_BOTTOM);
		m_wndStripImg.SetControlBarStyle(AFX_CBRS_RESIZE);   //设置窗口不可拖动
		m_wndPrjSource.SetControlBarStyle(AFX_CBRS_RESIZE);
		m_wndImgAndPt.SetControlBarStyle(AFX_CBRS_RESIZE);   //设置窗口不可拖动
		m_wndDataChart.SetControlBarStyle(AFX_CBRS_RESIZE);
		int ntag = m_wndRibbonBar.GetHideFlags();  //启动时ribbon显示
		if (ntag == 1)
		{
			m_wndRibbonBar.ToggleMimimizeState();
		}
		return true;
	}
	void CMainFrame::HideOrShowAllWnd(bool bShow /* = true */)
	{
		m_wndStripImg.ShowPane(bShow,FALSE,TRUE);
		m_wndImgAndPt.ShowPane(bShow,FALSE,TRUE);
		m_wndStripImg.ShowPane(bShow,FALSE,TRUE);
		m_wndPrjSource.ShowPane(bShow,FALSE,TRUE);
		m_wndDataChart.ShowPane(bShow,FALSE,TRUE);
		m_wndImgAndPt.ShowPane(bShow,FALSE,TRUE);
		m_wndOutput.ShowPane(bShow,FALSE,TRUE);
	}
	BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
	{
		if( !CFrameWndEx::PreCreateWindow(cs) )
			return FALSE;
		// TODO: 在此处通过修改
		//  CREATESTRUCT cs 来修改窗口类或样式
		m_strTitle = "SkyPhoto-QC V1.0";
		cs.style &= ~FWS_ADDTOTITLE;
		return TRUE;
	}

	BOOL CMainFrame::CreateDockingWindows()
	{
		BOOL bNameValid;
		// 创建输出窗口
		CString strOutputWnd;
		bNameValid = strOutputWnd.LoadString(IDS_OUTPUT_WND);
		ASSERT(bNameValid);
		DWORD dwNoCloseBarStyle = AFX_DEFAULT_DOCKING_PANE_STYLE & ~AFX_CBRS_CLOSE;  //隐藏关闭按钮
		if (!m_wndOutput.Create(strOutputWnd, this, CRect(0, 0, 100, 100), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI,AFX_CBRS_REGULAR_TABS,dwNoCloseBarStyle))
		{
			TRACE0("未能创建输出窗口\n");
			return FALSE; // 未能创建
		}
		if (!m_wndPrjSource.Create("工程栏", this, CRect(0, 0, 250, 100), TRUE, ID_VIEW_PRJSOURCE_WND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI,AFX_CBRS_REGULAR_TABS,dwNoCloseBarStyle&~AFX_CBRS_RESIZE &~ AFX_CBRS_AUTOHIDE))
		{
			TRACE0("未能创建输出窗口\n");
			return FALSE; // 未能创建
		}
		if (!m_wndStripImg.Create("航带影像", this, CRect(0, 0, 250, 100), TRUE, ID_VIEW_STRIPIMG_WND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI,AFX_CBRS_REGULAR_TABS,dwNoCloseBarStyle&~AFX_CBRS_RESIZE &~ AFX_CBRS_AUTOHIDE))
		{
			TRACE0("未能创建输出窗口\n");
			return FALSE; // 未能创建
		}
		if (!m_wndImgAndPt.Create("报告辅助视图", this, CRect(0, 0, 350, 100), TRUE, ID_VIEW_THUMBWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI,AFX_CBRS_REGULAR_TABS,dwNoCloseBarStyle&~AFX_CBRS_RESIZE &~ AFX_CBRS_AUTOHIDE))
		{
			TRACE0("未能创建输出窗口\n");
			return FALSE; // 未能创建
		}
		if (!m_wndDataChart.Create("数据统计图", this, CRect(0, 0, 350, 70), TRUE, ID_VIEW_DATACHART_WND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI,AFX_CBRS_REGULAR_TABS,dwNoCloseBarStyle&~AFX_CBRS_RESIZE &~ AFX_CBRS_AUTOHIDE))
		{
			TRACE0("未能创建输出窗口\n");
			return FALSE; // 未能创建
		}
		// 	if (!m_wndPdfView.Create("质检报告", this, CRect(0, 0, 250, 70), TRUE, ID_VIEW_PDFVIEWER_WND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI,AFX_CBRS_REGULAR_TABS,dwNoCloseBarStyle))
		// 	{
		// 		TRACE0("未能创建输出窗口\n");
		// 		return FALSE; // 未能创建
		// 	}
		SetDockingWindowIcons(theApp.m_bHiColorIcons);
		return TRUE;
	}

	void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
	{
		HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
		m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

	}

	BOOL CMainFrame::CreateOutlookBar(CMFCOutlookBar& bar, UINT uiID, CTreeCtrl& tree, int nInitialWidth)
	{
		bar.SetMode2003();

		BOOL bNameValid;
		CString strTemp;
		bNameValid = strTemp.LoadString(IDS_SHORTCUTS);
		ASSERT(bNameValid);
		if (!bar.Create(strTemp, this, CRect(0, 0, nInitialWidth, 32000), uiID, WS_CHILD | WS_VISIBLE | CBRS_LEFT))
		{
			return FALSE; // 未能创建
		}

		CMFCOutlookBarTabCtrl* pOutlookBar = (CMFCOutlookBarTabCtrl*)bar.GetUnderlyingWindow();

		if (pOutlookBar == NULL)
		{
			ASSERT(FALSE);
			return FALSE;
		}

		pOutlookBar->EnableInPlaceEdit(TRUE);

		static UINT uiPageID = 1;

		// 可浮动，可自动隐藏，可调整大小，但不能关闭
		DWORD dwStyle = AFX_CBRS_FLOAT | AFX_CBRS_AUTOHIDE | AFX_CBRS_RESIZE;

		CRect rectDummy(0, 0, 0, 0);
		const DWORD dwTreeStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;
		tree.Create(dwTreeStyle, rectDummy, &bar, 1200);
		bNameValid = strTemp.LoadString(IDS_FOLDERS);
		ASSERT(bNameValid);
		pOutlookBar->AddControl(&tree, "工程栏", 2, TRUE, dwStyle);

		bar.SetPaneStyle(bar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

		pOutlookBar->SetImageList(theApp.m_bHiColorIcons ? IDB_PAGES_HC : IDB_PAGES, 24);
		pOutlookBar->SetToolbarImageList(theApp.m_bHiColorIcons ? IDB_PAGES_SMALL_HC : IDB_PAGES_SMALL, 16);
		pOutlookBar->RecalcLayout();

		BOOL bAnimation = theApp.GetInt(_T("OutlookAnimation"), TRUE);
		CMFCOutlookBarTabCtrl::EnableAnimation(bAnimation);

		bar.SetButtonsFont(&afxGlobalData.fontBold);

		return TRUE;
	}
	BOOL CMainFrame::CreateCaptionBar()
	{
		if (!m_wndCaptionBar.Create(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, this, ID_VIEW_CAPTION_BAR, -1, TRUE))
		{
			TRACE0("未能创建标题栏\n");
			return FALSE;
		}

		BOOL bNameValid;

		CString strTemp, strTemp2;
		bNameValid = strTemp.LoadString(IDS_CAPTION_BUTTON);
		ASSERT(bNameValid);
		m_wndCaptionBar.SetButton(strTemp, ID_TOOLS_OPTIONS, CMFCCaptionBar::ALIGN_LEFT, FALSE);
		bNameValid = strTemp.LoadString(IDS_CAPTION_BUTTON_TIP);
		ASSERT(bNameValid);
		m_wndCaptionBar.SetButtonToolTip(strTemp);

		bNameValid = strTemp.LoadString(IDS_CAPTION_TEXT);
		ASSERT(bNameValid);
		m_wndCaptionBar.SetText(strTemp, CMFCCaptionBar::ALIGN_LEFT);

		//m_wndCaptionBar.SetBitmap(IDB_INFO, RGB(255, 255, 255), FALSE, CMFCCaptionBar::ALIGN_LEFT);
		bNameValid = strTemp.LoadString(IDS_CAPTION_IMAGE_TIP);
		ASSERT(bNameValid);
		bNameValid = strTemp2.LoadString(IDS_CAPTION_IMAGE_TEXT);
		ASSERT(bNameValid);
		m_wndCaptionBar.SetImageToolTip(strTemp, strTemp2);

		return TRUE;
	}
	// CMainFrame 诊断

#ifdef _DEBUG
	void CMainFrame::AssertValid() const
	{
		CFrameWndEx::AssertValid();
	}

	void CMainFrame::Dump(CDumpContext& dc) const
	{
		CFrameWndEx::Dump(dc);
	}
#endif //_DEBUG


	// CMainFrame 消息处理程序

	void CMainFrame::OnApplicationLook(UINT id)
	{
		CWaitCursor wait;

		theApp.m_nAppLook = id;

		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_WIN_2000:
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
			m_wndRibbonBar.SetWindows7Look(FALSE);
			break;

		case ID_VIEW_APPLOOK_OFF_XP:
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
			m_wndRibbonBar.SetWindows7Look(FALSE);
			break;

		case ID_VIEW_APPLOOK_WIN_XP:
			CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
			m_wndRibbonBar.SetWindows7Look(FALSE);
			break;

		case ID_VIEW_APPLOOK_OFF_2003:
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
			CDockingManager::SetDockingMode(DT_SMART);
			m_wndRibbonBar.SetWindows7Look(FALSE);
			break;

		case ID_VIEW_APPLOOK_VS_2005:
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
			CDockingManager::SetDockingMode(DT_SMART);
			m_wndRibbonBar.SetWindows7Look(FALSE);
			break;

		case ID_VIEW_APPLOOK_VS_2008:
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
			CDockingManager::SetDockingMode(DT_SMART);
			m_wndRibbonBar.SetWindows7Look(FALSE);
			break;

		case ID_VIEW_APPLOOK_WINDOWS_7:
			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
			CDockingManager::SetDockingMode(DT_SMART);
			m_wndRibbonBar.SetWindows7Look(TRUE);
			break;

		default:
			switch (theApp.m_nAppLook)
			{
			case ID_VIEW_APPLOOK_OFF_2007_BLUE:
				CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
				break;

			case ID_VIEW_APPLOOK_OFF_2007_BLACK:
				CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
				break;

			case ID_VIEW_APPLOOK_OFF_2007_SILVER:
				CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
				break;

			case ID_VIEW_APPLOOK_OFF_2007_AQUA:
				CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
				break;
			}

			CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
			CDockingManager::SetDockingMode(DT_SMART);
			m_wndRibbonBar.SetWindows7Look(FALSE);
		}

		RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

		theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
	}

	void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
	{
		pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
	}

	void CMainFrame::OnFilePrint()
	{
		if (IsPrintPreview())
		{
			PostMessage(WM_COMMAND, AFX_ID_PREVIEW_PRINT);
		}
	}

	void CMainFrame::OnFilePrintPreview()
	{
		if (IsPrintPreview())
		{
			PostMessage(WM_COMMAND, AFX_ID_PREVIEW_CLOSE);  // 强制关闭“打印预览”模式
		}
	}

	void CMainFrame::OnUpdateFilePrintPreview(CCmdUI* pCmdUI)
	{
		pCmdUI->SetCheck(IsPrintPreview());
	}

	void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
	{
		CFrameWndEx::OnSettingChange(uFlags, lpszSection);
		m_wndOutput.UpdateFonts();
	}


	void CMainFrame::OnButtonSystemConfigure()
	{
		// TODO: 在此添加命令处理程序代码
		CSysConfigureDlg dlg;
		dlg.DoModal();
//		m_pDoc->m_nMatchPointNumThresold = m
	}


	void CMainFrame::ResetStatusPaneSize()
	{
		CRect RectStatus, RectSystemTime, RectProgress;
		m_wndStatusBar.GetClientRect(RectStatus);
		RectSystemTime = m_pStatusPaneSysTime->GetRect();
		RectProgress.right = max(RectStatus.left + 5, RectSystemTime.left - 10);
		RectProgress.left = max(RectProgress.right - 250, RectStatus.left + 1);
		RectProgress.top = RectSystemTime.top;
		RectProgress.bottom = RectSystemTime.bottom;
		g_wndProgressCtrl.MoveWindow(RectProgress);
		CRect RectProcess = RectProgress;
		RectProcess.left = max(RectProgress.left - 250, RectStatus.left + 1);
		RectProcess.right = (RectProcess.left + 20, RectProgress.left - 4);
		m_pStatusPaneProcessing->SetRect(RectProcess);
		m_wndStatusBar.SetRedraw();
	}

	void CMainFrame::ShowWndTree(vector<CString> vecItemInfo)
	{
		m_wndPrjSource.m_wndPrjTree.DeleteAllItems();
		m_wndStripImg.m_ListStripImg.DeleteAllItems();
		if(vecItemInfo.size()==0) return;
		HTREEITEM hRoot = m_wndPrjSource.m_wndPrjTree.InsertItem(vecItemInfo[0]);
		m_wndPrjSource.m_wndPrjTree.SetItemData(hRoot,-1);
		for (int i = 1; i<vecItemInfo.size(); i++)
		{
			HTREEITEM hChild = m_wndPrjSource.m_wndPrjTree.InsertItem(vecItemInfo[i],hRoot);
			m_wndPrjSource.m_wndPrjTree.SetItemData(hChild,i-1);
		}
		m_wndPrjSource.m_wndPrjTree.Expand(hRoot,TVE_EXPAND);
	}

	void CMainFrame::ShowStripList(vector<CString> vecItemInfo)
	{
		m_wndStripImg.m_ListStripImg.DeleteAllItems();
		for (int i = 0; i<vecItemInfo.size(); i++)
		{
			CString strItem; strItem.Format("%d",i+1);
			m_wndStripImg.m_ListStripImg.InsertItem(i,strItem);
			m_wndStripImg.m_ListStripImg.SetItemText(i,1,vecItemInfo[i]);
		}
	}

	void CMainFrame::ShowRibbonValue()
	{
		CsswUAVFlyQuaSysDoc* pDoc = GetDocHand();
		CMFCRibbonEdit* pEdit = NULL; CString str;
		pEdit = DYNAMIC_DOWNCAST(CMFCRibbonEdit,m_wndRibbonBar.FindByID(ID_EDIT_IMG_NUM));
		str.Format("%d", pDoc->m_nImgNum);
		if(pEdit!=NULL) pEdit->SetEditText(str); pEdit=NULL;
		pEdit = DYNAMIC_DOWNCAST(CMFCRibbonEdit,m_wndRibbonBar.FindByID(ID_EDIT_MEAN_HEIGHT));
		str.Format("%.2lf", pDoc->m_dMeanHei);
		if(pEdit!=NULL) pEdit->SetEditText(str); pEdit = NULL;
// 		pEdit = DYNAMIC_DOWNCAST(CMFCRibbonEdit,m_wndRibbonBar.FindByID(ID_EDIT_MATCHPOINT_NUM_THRESOLD));
// 		str.Format("%.2lf", pDoc->m_nMatchPointNumThresold);
// 		if(pEdit!=NULL) pEdit->SetEditText(str); pEdit = NULL;

// 		CMFCRibbonComboBox *pComb = NULL;
// 		pComb = DYNAMIC_DOWNCAST(CMFCRibbonComboBox,m_wndRibbonBar.FindByID(ID_COMBO_CHECK_METHOD));
// 		if(pComb!=NULL) pComb->SelectItem(pDoc->m_eFlyQuaChkMethod);
	}

	void CMainFrame::FillLogWnd(CString strLog, bool bAdd)
	{
		m_wndOutput.FillLogWindow(FunGetSysTimeStr()+" "+strLog,bAdd);
	}
	void CMainFrame::UpdateStatusLabel(CString strLabel)
	{

		m_pStatusPaneLabel->SetText(strLabel);
		m_pStatusPaneLabel->Redraw();
	}


	void CMainFrame::OnSize(UINT nType, int cx, int cy)
	{
		CFrameWndEx::OnSize(nType, cx, cy);
		if(m_bWndReady) ResetStatusPaneSize();

		// TODO: 在此处添加消息处理程序代码
	}


	void CMainFrame::OnTimer(UINT_PTR nIDEvent)
	{
		// TODO: 在此添加消息处理程序代码和/或调用默认值
		switch(nIDEvent)
		{
		case TIME_ID_SYSTIME:
			m_pStatusPaneSysTime->SetText(FunGetSysTimeStr());
			m_pStatusPaneSysTime->Redraw();
			break;

		case TIME_ID_IMGPOSMATCH:
			{
				CString str = m_pStatusPaneProcessing->GetText();
				if(str.GetLength()<1)
				{
					str= "Image and POS file is matching...";
				}
				else
				{
					str="";
				}
				m_pStatusPaneProcessing->SetText(str);
				m_pStatusPaneProcessing->Redraw();
			}
			break;

		case TIME_ID_IMGMATCHADJ:
			{
				static int nPos=0;
				g_wndProgressCtrl.SetPos(nPos);
				nPos+=5;
				if(nPos>=100) nPos=0;
			}
			break;
		}
		CFrameWndEx::OnTimer(nIDEvent);
	}

	LRESULT CMainFrame::OnMsgUpdateStatusLabel(WPARAM wParam, LPARAM lParam)
	{
		char* strLabel = (char*)(lParam);
		m_pStatusPaneLabel->SetText(strLabel);
		m_pStatusPaneLabel->Redraw();
		return 1;
	}
	LRESULT CMainFrame::OnMsgFillLogWnd(WPARAM wParam, LPARAM lParam)
	{
		bool bAdd = *(bool*)wParam;
		char *pStr = (char*)lParam;
		CString strLogInfo;strLogInfo.Format("%s  %s",FunGetSysTimeStr(),pStr);
		m_wndOutput.FillLogWindow(strLogInfo,bAdd);
		return 1;
	}
	LRESULT CMainFrame::OnMsgUpdateWndProgressCtrl(WPARAM wParam, LPARAM lParam)
	{
		float nPos = *(float*)lParam;
		g_wndProgressCtrl.SetPos(nPos);
		return 1;
	}
	LRESULT CMainFrame::OnMsgShowWndProgressCtrl(WPARAM wParam, LPARAM lParam)
	{
		bool bShow = *(bool*)lParam;
		g_wndProgressCtrl.ShowWindow(bShow);
		return 1;
	}
	LRESULT CMainFrame::OnMsgSaveReport(WPARAM wParam, LPARAM lParam)
	{
	//	FunOutPutLogInfo("开始写入PDF...");
		char *strCqViewImgPath  = (char*)wParam;
		vector<CString>vecViewImagePath = *((vector<CString>*)wParam);
		SSWFLYQUAMETHOD eMethod = *(SSWFLYQUAMETHOD*)lParam;
		bool bSave = false;
		switch(eMethod)
		{
		case BY_ADJUSTMENT:
			if(m_pDlgRepByAdj!=NULL)
			{
				m_pDlgRepByAdj->SendMessage(WM_CLOSE);
				m_pDlgRepByAdj->DestroyWindow();
				m_pDlgRepByAdj = NULL;
			}
			break;
		case BY_MATCH:
			if(m_pDlgRepByMch!=NULL)
			{
				m_pDlgRepByMch->SendMessage(WM_CLOSE);
				m_pDlgRepByMch->DestroyWindow();
				m_pDlgRepByMch = NULL;
			}
			break;
		case BY_POS:
			if(m_pDlgRepByPos!=NULL)
			{
				m_pDlgRepByPos->SendMessage(WM_CLOSE);
				m_pDlgRepByPos->DestroyWindow();
				m_pDlgRepByPos = NULL;
			}
			break;
		default:
			break;
		}
		Sleep(500);
		bSave = m_pDoc->m_FlyQuaPrj.SaveReport2File20170108_(vecViewImagePath[0],vecViewImagePath[1],vecViewImagePath[2],
			vecViewImagePath[3],vecViewImagePath[4],vecViewImagePath[5],vecViewImagePath[6]);

// 		if(!bSave)
// 		{
// 			AfxMessageBox("质检报告生成失败！");
// 		}
// 		else OnMsgOpenPdfReport(0,0);
		return 1;
	}
	LRESULT CMainFrame::OnMsgOpenPdfReport(WPARAM wParam, LPARAM lParam)
	{
		SSWFLYQUAMETHOD eMethd = m_pDoc->m_eFlyQuaChkMethod;
		CString strRepPath = m_pDoc->m_FlyQuaPrj.GetCheckRepPath(eMethd);
		if(!PathFileExists(strRepPath))
		{
			CString strInfo;
			if(eMethd==BY_ADJUSTMENT) strInfo = "基于平差的质检报告不存在，请重新执行质检操作！";else
				if(eMethd==BY_MATCH) strInfo = "基于匹配的质检报告不存在，请重新执行质检操作！";else
					if(eMethd==BY_POS) strInfo = "基于POS的质检报告不存在，请重新执行质检操作！";
			AfxMessageBox(strInfo);
		}
		else
		{
			if(eMethd==BY_ADJUSTMENT) 
			{
				if(m_pDlgRepByAdj!=NULL)
				{
					m_pDlgRepByAdj->SendMessage(WM_CLOSE);
					m_pDlgRepByAdj->DestroyWindow();
					m_pDlgRepByAdj = NULL;
				}
				m_pDlgRepByAdj = new CPdfViewDlg;
				m_pDlgRepByAdj->Create(IDD_DIALOG_PDF_VIEW);
				m_pDlgRepByAdj->SetWindowText("质检报告-基于平差的质检");
				m_pDlgRepByAdj->ShowWindow(TRUE);
				m_pDlgRepByAdj->SendMessage(ID_MSG_SHOW_PDF,0,(LPARAM)&strRepPath);
			}
			else if(eMethd==BY_MATCH) 
			{
				if(m_pDlgRepByMch!=NULL)
				{
					m_pDlgRepByMch->SendMessage(WM_CLOSE);
					m_pDlgRepByMch->DestroyWindow();
					m_pDlgRepByMch = NULL;
				}
				m_pDlgRepByMch = new CPdfViewDlg;
				m_pDlgRepByMch->Create(IDD_DIALOG_PDF_VIEW);
				m_pDlgRepByMch->SetWindowText("质检报告-基于匹配的质检");
				m_pDlgRepByMch->ShowWindow(TRUE);
				m_pDlgRepByMch->ShowWindow(TRUE);
				m_pDlgRepByMch->SendMessage(ID_MSG_SHOW_PDF,0,(LPARAM)&strRepPath);
			}
			else if(eMethd==BY_POS) 
			{
				if(m_pDlgRepByPos!=NULL)
				{
					m_pDlgRepByPos->SendMessage(WM_CLOSE);
					m_pDlgRepByPos->DestroyWindow();
					m_pDlgRepByPos = NULL;
				}
				m_pDlgRepByPos = new CPdfViewDlg;
				m_pDlgRepByPos->Create(IDD_DIALOG_PDF_VIEW);
				m_pDlgRepByPos->SetWindowText("质检报告-基于POS的质检");
				m_pDlgRepByPos->ShowWindow(TRUE);
				m_pDlgRepByPos->ShowWindow(TRUE);
				m_pDlgRepByPos->SendMessage(ID_MSG_SHOW_PDF,0,(LPARAM)&strRepPath);

			}
		}
		return 1;
	}





	void CMainFrame::ActivateFrame(int nCmdShow)
	{
		// TODO: 在此添加专用代码和/或调用基类
		nCmdShow = SW_SHOWMAXIMIZED;
		CFrameWndEx::ActivateFrame(nCmdShow);
	}


	//void CMainFrame::OnButtonApp()
	//{
	//	// TODO: 在此添加命令处理程序代码
	//}

	void CMainFrame::OnCheckViewOutputwnd()
	{
		// TODO: 在此添加命令处理程序代码
		if(m_wndOutput.IsVisible()) 
			m_wndOutput.ShowPane(FALSE,FALSE,TRUE);
		else m_wndOutput.ShowPane(TRUE,FALSE,TRUE);
	}


	void CMainFrame::OnUpdateCheckViewOutputwnd(CCmdUI *pCmdUI)
	{
		// TODO: 在此添加命令更新用户界面处理程序代码
		if(GetDocHand()==NULL) return;
		pCmdUI->Enable(GetDocHand()->m_bPrjLoaded);
		pCmdUI->SetCheck(m_wndOutput.IsVisible());
	}


	void CMainFrame::OnCheckViewStripimgwnd()
	{
		// TODO: 在此添加命令处理程序代码
		if(m_wndStripImg.IsVisible()) 
			m_wndStripImg.ShowPane(FALSE,FALSE,TRUE);
		else m_wndStripImg.ShowPane(TRUE,FALSE,TRUE);
	}


	void CMainFrame::OnUpdateCheckViewStripimgwnd(CCmdUI *pCmdUI)
	{
		// TODO: 在此添加命令更新用户界面处理程序代码
		if(GetDocHand()==NULL) return;
		pCmdUI->Enable(GetDocHand()->m_bPrjLoaded);
		pCmdUI->SetCheck(m_wndStripImg.IsVisible());
	}


	void CMainFrame::OnCheckViewThumbwnd()
	{
		// TODO: 在此添加命令处理程序代码
		if(m_wndImgAndPt.IsVisible()) 
			m_wndImgAndPt.ShowPane(FALSE,FALSE,TRUE);
		else m_wndImgAndPt.ShowPane(TRUE,FALSE,TRUE);
	}

	void CMainFrame::OnUpdateCheckViewThumbwnd(CCmdUI *pCmdUI)
	{
		// TODO: 在此添加命令更新用户界面处理程序代码
		if(GetDocHand()==NULL) return;
		pCmdUI->Enable(GetDocHand()->m_bPrjLoaded);
		pCmdUI->SetCheck(m_wndImgAndPt.IsVisible());
	}


	void CMainFrame::OnCheckViewDatachartwnd()
	{
		// TODO: 在此添加命令处理程序代码
		if(m_wndDataChart.IsVisible()) 
			m_wndDataChart.ShowPane(FALSE,FALSE,TRUE);
		else m_wndDataChart.ShowPane(TRUE,FALSE,TRUE);
	}


	void CMainFrame::OnUpdateCheckViewDatachartwnd(CCmdUI *pCmdUI)
	{
		// TODO: 在此添加命令更新用户界面处理程序代码
		if(GetDocHand()==NULL) return;
		pCmdUI->Enable(GetDocHand()->m_bPrjLoaded);
		pCmdUI->SetCheck(m_wndDataChart.IsVisible());
	}


	void CMainFrame::OnViewRibbon()
	{
		// TODO: 在此添加命令处理程序代码
		m_wndRibbonBar.ToggleMimimizeState();
		m_wndRibbonBar.RecalcLayout();
	}


	void CMainFrame::OnUpdateViewRibbon(CCmdUI *pCmdUI)
	{
		// TODO: 在此添加命令更新用户界面处理程序代码	
		int ntag = m_wndRibbonBar.GetHideFlags();
		if (ntag == 0) pCmdUI->SetCheck(TRUE);
		else pCmdUI->SetCheck(FALSE);
	}


	void CMainFrame::OnDestroy()
	{
		CFrameWndEx::OnDestroy();
		HANDLE hself = GetCurrentProcess();
		TerminateProcess(hself,0);
		// TODO: 在此处添加消息处理程序代码
	}


	void CMainFrame::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
	{
		// TODO: 在此处添加消息处理程序代码
	}
