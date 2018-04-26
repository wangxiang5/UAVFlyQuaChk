// SysConfigureDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "sswUAVFlyQuaSysEng.h"
#include "SysConfigureDlg.h"
#include "afxdialogex.h"

#include "MyFunctions.h"
// CSysConfigureDlg 对话框

IMPLEMENT_DYNAMIC(CSysConfigureDlg, CDialogEx)

CSysConfigureDlg::CSysConfigureDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSysConfigureDlg::IDD, pParent)
{
	m_pDlgSysConfigData = NULL;
	m_pDlgSysConfigPara = NULL;
}

CSysConfigureDlg::~CSysConfigureDlg()
{
}

void CSysConfigureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_SYSTEM_CONFIGURE, m_TabSysConfig);
}


BEGIN_MESSAGE_MAP(CSysConfigureDlg, CDialogEx)
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_SYSTEM_CONFIGURE, &CSysConfigureDlg::OnTcnSelchangeTabSystemConfigure)
	ON_BN_CLICKED(IDOK, &CSysConfigureDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CSysConfigureDlg 消息处理程序


void CSysConfigureDlg::InitItemPos()
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

void CSysConfigureDlg::ResetItemPos()
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

BOOL CSysConfigureDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
//	InitItemPos();
	// TODO:  在此添加额外的初始化
// 	int nWndX = GetSystemMetrics(SM_CXSCREEN);
// 	int nWndY = GetSystemMetrics(SM_CYSCREEN);
// 	int nDlgX = min(800, nWndX);
// 	int nDlgY = nDlgX * 3 / 4;
// 	MoveWindow(0, 0, nDlgX, nDlgY);
	HICON hIcon = AfxGetApp()->LoadIcon(IDI_ICON_SETTING);
	SetIcon(hIcon, TRUE);
	SetIcon(hIcon, FALSE);
	CenterWindow();
	
	m_TabSysConfig.InsertItem(0, "File Settings");
	m_TabSysConfig.InsertItem(1, "Qualiy Control Paras");
	m_pDlgSysConfigData = new CSysConfigureDataDlg;
	m_pDlgSysConfigPara = new CSysConfigureParaDlg;
	m_pDlgSysConfigData->Create(IDD_DIALOG_SYSTEM_CONFIGURE_DATA, &m_TabSysConfig);	
	m_pDlgSysConfigPara->Create(IDD_DIALOG_SYSTEM_CONFIGURE_PARA, &m_TabSysConfig);

	
	CRect rect; GetClientRect(rect);
	rect.bottom -= 60;
	m_TabSysConfig.MoveWindow(rect);
	rect.top += 21;
	rect.bottom -= 20;
	rect.left += 1;
	rect.right -= 2;
	m_pDlgSysConfigData->MoveWindow(&rect);
	m_pDlgSysConfigPara->MoveWindow(&rect);

	m_pDlgSysConfigData->ShowWindow(TRUE);
	m_pDlgSysConfigPara->ShowWindow(FALSE);
	m_TabSysConfig.SetCurSel(0);

	if(!FunReadSysConfigFile(m_pDlgSysConfigData->m_SysCfgData,m_pDlgSysConfigPara->m_SysCfgPara))
	{
		m_pDlgSysConfigData->m_SysCfgData = SSWstuSysConfigData();
		m_pDlgSysConfigPara->m_SysCfgPara = SSWstuSysConfigPara();
	}
	m_pDlgSysConfigData->UpdateItems();
	m_pDlgSysConfigPara->UpdateItems();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CSysConfigureDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
//	ResetItemPos();
	// TODO: 在此处添加消息处理程序代码
}


void CSysConfigureDlg::OnTcnSelchangeTabSystemConfigure(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	switch (m_TabSysConfig.GetCurSel())
	{
	case 0:
		m_pDlgSysConfigData->ShowWindow(TRUE);
		m_pDlgSysConfigPara->ShowWindow(FALSE);
		break;

	case 1:
		m_pDlgSysConfigData->ShowWindow(FALSE);
		m_pDlgSysConfigPara->ShowWindow(TRUE);
		break;
	}
	*pResult = 0;
}


void CSysConfigureDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	m_pDlgSysConfigPara->UpdateParas();
	m_pDlgSysConfigData->UpdateDatas();
	m_sswSysCfgData = m_pDlgSysConfigData->m_SysCfgData;
	m_sswSysCfgPara = m_pDlgSysConfigPara->m_SysCfgPara;
	FunSaveSysConfigFile(m_sswSysCfgData, m_sswSysCfgPara);
	CDialogEx::OnOK();
}
