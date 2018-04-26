// PrjNewDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "sswUAVFlyQuaSys.h"
#include "MainFrm.h"
#include "PrjNewDlg.h"
#include "afxdialogex.h"
#include "MyMultiThread.h"
#include "GradientProgressCtrl.h"
#include "MyFunctions.h"
#include <omp.h>
// CPrjNewDlg 对话框

extern CGradientProgressCtrl g_wndProgressCtrl;
extern CMainFrame* g_pMain;


IMPLEMENT_DYNAMIC(CPrjNewDlg, CDialogEx)

CPrjNewDlg::CPrjNewDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPrjNewDlg::IDD, pParent)
	, m_strPrjFilePath(_T(""))
	, m_strCmrFilePath(_T(""))
	, m_strPosFilePath(_T(""))
	, m_strImgFolderPath(_T(""))
// 	, m_bSearchJpg(TRUE)
// 	, m_bSearchTif(TRUE)
// 	, m_bAddData(FALSE)
// 	, m_bDeleteImgNoPos(TRUE)
, m_bAutoManageImage(FALSE)
{
	m_bCmrOk = false;
	m_bPosOk = false;
	m_bImgOk = false;
	m_nCmrIdxSelected = InvalidValue;
	m_eImgPosMatchDirection=IMG2POS;
//	m_nCmrIdxSelected=0;
}

CPrjNewDlg::~CPrjNewDlg()
{
	ClearPosInfo();
	ClearImgInfo();
	ClearCmrInfo();
}

void CPrjNewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PRJ_FILEPATH, m_strPrjFilePath);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_CMR_FILEPATH, m_strCmrFilePath);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_POS_FILEPATH, m_strPosFilePath);
	DDX_Control(pDX, IDC_LIST_CMRINFO, m_ListCmrInfo);
	DDX_Control(pDX, IDC_LIST_IMGINFO, m_ListImgInfo);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_IMGFOLDER, m_strImgFolderPath);
	// 	DDX_Check(pDX, IDC_CHECK_SEARCH_JPG, m_bSearchJpg);
	// 	DDX_Check(pDX, IDC_CHECK_SEARCH_TIF, m_bSearchTif);
	// 	DDX_Check(pDX, IDC_CHECK_ADDDATA, m_bAddData);
	// 	DDX_Check(pDX, IDC_CHECK_DELETEIMG_NOPOS, m_bDeleteImgNoPos);
	DDX_Check(pDX, IDC_CHECK_AUTO_MANAGE_IMAGE, m_bAutoManageImage);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_IMGFOLDER, m_BrowseImgFolder);
}


BEGIN_MESSAGE_MAP(CPrjNewDlg, CDialogEx)
	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_MFCEDITBROWSE_CMR_FILEPATH, &CPrjNewDlg::OnEnChangeMfceditbrowseCmrFilepath)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE_POS_FILEPATH, &CPrjNewDlg::OnEnChangeMfceditbrowsePosFilepath)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_IMGINFO, &CPrjNewDlg::OnGetdispinfoListImginfo)
	ON_NOTIFY(NM_CLICK, IDC_LIST_CMRINFO, &CPrjNewDlg::OnClickListCmrinfo)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE_IMGFOLDER, &CPrjNewDlg::OnEnChangeMfceditbrowseImgfolder)
	ON_MESSAGE(ID_MSG_IMGPOSMATCH, OnMsgImgPosMatch)
//	ON_WM_TIMER()
	ON_BN_CLICKED(IDOK, &CPrjNewDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_POSCOORDINATE, &CPrjNewDlg::OnBnClickedButtonChangePoscoordinate)
END_MESSAGE_MAP()

//自定义消息响应
LRESULT CPrjNewDlg::OnMsgImgPosMatch(WPARAM wParam, LPARAM lParam)
{
	if(m_eImgPosMatchDirection==POS2IMG) MatchPos2Img();
	if(m_eImgPosMatchDirection==IMG2POS) MatchImg2Pos();
	return 1;
}


// CPrjNewDlg 消息处理程序
void CPrjNewDlg::InitItemPos()
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

void CPrjNewDlg::ResetItemPos()
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

BOOL CPrjNewDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	InitItemPos();
	int nWndX = GetSystemMetrics(SM_CXSCREEN);
	int nWndY = GetSystemMetrics(SM_CYSCREEN);
	int nDlgX = min(800, nWndX);
	int nDlgY = nDlgX * 3 / 4;
	MoveWindow(0, 0, nDlgX, nDlgY);
	CenterWindow();

	FunReadSysConfigFile(m_SysCfgData, m_SysCfgPara);
	InitCmrList();
	InitImgList();
// 	GetDlgItem(IDC_CHECK_DELETEIMG_NOPOS)->EnableWindow(FALSE);
// 	GetDlgItem(IDC_CHECK_ADDDATA)->EnableWindow(FALSE);
	m_strPrjFilePath = "New_Prj.xml";
// 	m_strPrjFilePath = "C:\\Users\\Administrator\\Desktop\\wx\\test\\ssw_prj\\prj01\\FlyQuaPrj.xml";
// 	m_strCmrFilePath = "C:\\Users\\Administrator\\Desktop\\wx\\test\\data\\DY-地址资料馆数据-193P\\cam.scbacmr";
// 	m_strPosFilePath = "C:\\Users\\Administrator\\Desktop\\wx\\test\\data\\DY-地址资料馆数据-193P\\pos.pos";
// 	m_bPosOk = FunReadPosFile(m_strPosFilePath.GetBuffer(), m_SysCfgData.PosFileExtend, m_vecNewPosInfo);
// 	m_bCmrOk = FunReadCmrFile(m_strCmrFilePath.GetBuffer(),m_SysCfgData.CmrFileExtend,m_vecNewCmrInfo);
// 	if(m_bPosOk)
// 	{
// 		UnionPosNew2AllNoRepeat();	
// 	}
// 	if(m_bCmrOk)
// 	{
// 		UnionCmrNew2AllNoRepeat();
// 		FillCmrList();
// 		vector<SSWstuCmrInfo>().swap(m_vecNewCmrInfo);
// 		m_ListImgInfo.Invalidate();
// 	}
// 	m_strImgFolderPath = "C:\\Users\\Administrator\\Desktop\\wx\\test\\data";
	(CButton*)GetDlgItem(IDC_BUTTON_CHANGE_POSCOORDINATE)->EnableWindow(FALSE);
	(CButton*)GetDlgItem(IDC_BUTTON_CHANGE_POSCOORDINATE)->ShowWindow(FALSE);
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CPrjNewDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	ResetItemPos();
	// TODO: 在此处添加消息处理程序代码
}

void CPrjNewDlg::InitCmrList()
{
	int nCount = m_ListCmrInfo.GetHeaderCtrl()->GetItemCount();
	for (int i = 0; i<nCount; i++)
	{
		m_ListCmrInfo.DeleteColumn(0);
	}
	CRect rect; DWORD dwStyle; int nListWidth, nListHeight;
	dwStyle = (m_ListCmrInfo).GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;     //可以选择整行
	dwStyle |= LVS_EX_GRIDLINES;         //格网形式 
	dwStyle |= LVS_OWNERDRAWFIXED;       //可重绘
	dwStyle |= LVS_EX_CHECKBOXES;       //可重绘
	dwStyle |= LVS_EX_DOUBLEBUFFER;
	//	dwStyle |= LVS_EX_HEADERDRAGDROP;
	m_ListCmrInfo.SetExtendedStyle(dwStyle);

	m_ListCmrInfo.GetClientRect(rect);
	nListWidth = rect.Width(); nListHeight = rect.Height();
	m_ListCmrInfo.InsertColumn(0, "Id", LVCFMT_LEFT, 30, -1);
	m_ListCmrInfo.InsertColumn(1, "Label", LVCFMT_LEFT, 80, -1);
	CString strHead, strTemp;
	strTemp = sswUnitType2String(m_SysCfgData.CmrFileExtend.Unitx0y0);
	strHead.Format("x0(%s)",strTemp);
	m_ListCmrInfo.InsertColumn(2, strHead, LVCFMT_LEFT, 70 - 1);
	strHead.Format("y0(%s)",strTemp);
	m_ListCmrInfo.InsertColumn(3, strHead, LVCFMT_LEFT, 70 - 1);
	strTemp = sswUnitType2String(m_SysCfgData.CmrFileExtend.Unitf);
	strHead.Format("f(%s)",strTemp);
	m_ListCmrInfo.InsertColumn(4, strHead, LVCFMT_LEFT,70 - 1);
	strHead.Format("fx(%s)",strTemp);
	m_ListCmrInfo.InsertColumn(5, strHead, LVCFMT_LEFT, 70 - 1);
	strHead.Format("fy(%s)",strTemp);
	m_ListCmrInfo.InsertColumn(6, strHead, LVCFMT_LEFT, 70 - 1);
	strTemp = sswUnitType2String(m_SysCfgData.CmrFileExtend.Unitpixsize);
	strHead.Format("pixsize(%s)",strTemp);
	m_ListCmrInfo.InsertColumn(7, strHead, LVCFMT_LEFT, 70 - 1);
	m_ListCmrInfo.InsertColumn(8, "k1", LVCFMT_LEFT, 70 - 1);
	m_ListCmrInfo.InsertColumn(9, "k2", LVCFMT_LEFT, 70 - 1);
	m_ListCmrInfo.InsertColumn(10, "p1", LVCFMT_LEFT, 70 - 1);
	m_ListCmrInfo.InsertColumn(11, "p2", LVCFMT_LEFT, 70 - 1);
	m_ListCmrInfo.InsertColumn(12, "a", LVCFMT_LEFT, 70 - 1);
	m_ListCmrInfo.InsertColumn(13, "b", LVCFMT_LEFT, 70 - 1);
}
void CPrjNewDlg::InitImgList()
{
	CRect rect; DWORD dwStyle; int nListWidth, nListHeight;
	dwStyle = (m_ListImgInfo).GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;     //可以选择整行
	dwStyle |= LVS_EX_GRIDLINES;         //格网形式 
	dwStyle |= LVS_OWNERDRAWFIXED;       //可重绘
	dwStyle |= LVS_EX_DOUBLEBUFFER;
	m_ListImgInfo.SetExtendedStyle(dwStyle);
	m_ListImgInfo.GetClientRect(rect);
	nListWidth = rect.Width(); nListHeight = rect.Height();
	CString strTitleX="Lon(deg)",strTitleY="Lat(deg)",strTitleZ="Alt(m)";
	CString strTitleP="Phi(deg)",strTitleO="Omege(deg)",strTitleK="Kappa(deg)";
	if(m_SysCfgData.PosFileExtend.FormatCoor==XYZ)
	{
		strTitleX="X(m)";
		strTitleY="Y(m)";
	}
	else if(m_SysCfgData.PosFileExtend.FormatAngleLBH==RAD)
	{
		strTitleX="Lon(rad)";
		strTitleY="Lat(rad)";
	}

	if(m_SysCfgData.PosFileExtend.FormatAnglePOK==RAD)
	{
		strTitleP="Phi(rad)";
		strTitleO="Omege(rad)";
		strTitleK="Kappa(rad)";
	}
	m_ListImgInfo.InsertColumn(0, "Id",		LVCFMT_LEFT, 30, -1);
	m_ListImgInfo.InsertColumn(1, "FullPath", LVCFMT_LEFT, 250, -1);
	m_ListImgInfo.InsertColumn(2, strTitleX, LVCFMT_LEFT, 75 - 1);
	m_ListImgInfo.InsertColumn(3, strTitleY, LVCFMT_LEFT, 75 - 1);
	m_ListImgInfo.InsertColumn(4, strTitleZ, LVCFMT_LEFT, 75 - 1);
	m_ListImgInfo.InsertColumn(5, strTitleO, LVCFMT_LEFT, 75 - 1);
	m_ListImgInfo.InsertColumn(6, strTitleP, LVCFMT_LEFT, 75 - 1);
	m_ListImgInfo.InsertColumn(7, strTitleK, LVCFMT_LEFT, 75 - 1);
	m_ListImgInfo.InsertColumn(8, "CameraLabel", LVCFMT_LEFT, 75 - 1);
}
void CPrjNewDlg::UnionImgNew2AllNoRepeat()
{	
	sort(m_vecNewImgPath.begin(), m_vecNewImgPath.end());
	int nStartIdx=0;
	for (int i = 0; i<m_vecNewImgPath.size(); i++)
	{
		for (int j = nStartIdx; j<m_vecAllImgPath.size(); j++)
		{
			if(m_vecNewImgPath[i]==m_vecAllImgPath[j])
			{
				m_vecNewImgPath.erase(m_vecNewImgPath.begin()+i);
				nStartIdx=++j;
				i--;
				break;
			}
		}

	}
	if(m_vecNewImgPath.size()==0) return;
	vector<int> vNewImgPosIdx(m_vecNewImgPath.size(), InvalidValue);
	m_vecImgIdxToPosIdx.insert(m_vecImgIdxToPosIdx.end(),vNewImgPosIdx.begin(),vNewImgPosIdx.end());
	m_vecAllImgPath.insert(m_vecAllImgPath.end(), m_vecNewImgPath.begin(), m_vecNewImgPath.end());
	vector<int>().swap(vNewImgPosIdx);
	sort(m_vecAllImgPath.begin(),m_vecAllImgPath.end());
	vector<CString>().swap(m_vecAllImgName);
	m_vecAllImgName.resize(m_vecAllImgPath.size());
#pragma omp parallel for
	for (int i = 0; i<m_vecAllImgPath.size(); i++)
	{
		m_vecAllImgName[i]=FunGetFileName(m_vecAllImgPath[i],false).MakeLower();
	}
}
void CPrjNewDlg::UnionCmrNew2AllNoRepeat()
{
	sort(m_vecNewCmrInfo.begin(), m_vecNewCmrInfo.end());
	int nStartIdx=0;
	for (int i = 0; i<m_vecNewCmrInfo.size(); i++)
	{
		for (int j = nStartIdx; j<m_vecAllCmrInfo.size(); j++)
		{
			if(m_vecNewCmrInfo[i]==m_vecAllCmrInfo[j])
			{
				m_vecNewCmrInfo.erase(m_vecNewCmrInfo.begin()+i);
				nStartIdx=++j;
				i--;
				break;
			}
		}

	}
	if(m_vecNewCmrInfo.size()==0) return;
	m_vecAllCmrInfo.insert(m_vecAllCmrInfo.end(), m_vecNewCmrInfo.begin(), m_vecNewCmrInfo.end());
}
void CPrjNewDlg::UnionPosNew2AllNoRepeat()
{
	sort(m_vecNewPosInfo.begin(), m_vecNewPosInfo.end());
	int nStartIdx=0;
	for (int i = 0; i<m_vecNewPosInfo.size(); i++)
	{
		for (int j = nStartIdx; j<m_vecAllPosInfo.size(); j++)
		{
			if(m_vecNewPosInfo[i]==m_vecAllPosInfo[j])
			{
				m_vecNewPosInfo.erase(m_vecNewPosInfo.begin()+i);
				nStartIdx=++j;
				i--;
				break;
			}
		}
	
	}
	if(m_vecNewPosInfo.size()==0) return;
	m_vecAllPosInfo.insert(m_vecAllPosInfo.end(), m_vecNewPosInfo.begin(), m_vecNewPosInfo.end());
	sort(m_vecAllPosInfo.begin(),m_vecAllPosInfo.end());
}

void CPrjNewDlg::FillCmrList()
{
	if(/*!m_bAddData*/1)
	{
		m_ListCmrInfo.DeleteAllItems();
		for(int i = 0; i<m_vecAllCmrInfo.size(); i++)
		{
			CString strInfo; strInfo.Format("%d",i);
			m_ListCmrInfo.InsertItem(i,strInfo);
			m_ListCmrInfo.SetItemText(i,1,m_vecAllCmrInfo[i].strLabel);
			strInfo.Format("%.8lf",m_vecAllCmrInfo[i].x0);m_ListCmrInfo.SetItemText(i,2,strInfo);
			strInfo.Format("%.8lf",m_vecAllCmrInfo[i].y0);m_ListCmrInfo.SetItemText(i,3,strInfo);
			strInfo.Format("%.8lf",m_vecAllCmrInfo[i].f);m_ListCmrInfo.SetItemText(i,4,strInfo);
			strInfo.Format("%.8lf",m_vecAllCmrInfo[i].fx);m_ListCmrInfo.SetItemText(i,5,strInfo);
			strInfo.Format("%.8lf",m_vecAllCmrInfo[i].fy);m_ListCmrInfo.SetItemText(i,6,strInfo);
			strInfo.Format("%.8lf",m_vecAllCmrInfo[i].pixsize);m_ListCmrInfo.SetItemText(i,7,strInfo);
			strInfo.Format("%.8lf",m_vecAllCmrInfo[i].k1);m_ListCmrInfo.SetItemText(i,8,strInfo);
			strInfo.Format("%.8lf",m_vecAllCmrInfo[i].k2);m_ListCmrInfo.SetItemText(i,9,strInfo);
			strInfo.Format("%.8lf",m_vecAllCmrInfo[i].p1);m_ListCmrInfo.SetItemText(i,10,strInfo);
			strInfo.Format("%.8lf",m_vecAllCmrInfo[i].p2);m_ListCmrInfo.SetItemText(i,11,strInfo);
			strInfo.Format("%.8lf",m_vecAllCmrInfo[i].a);m_ListCmrInfo.SetItemText(i,12,strInfo);
			strInfo.Format("%.8lf",m_vecAllCmrInfo[i].b);m_ListCmrInfo.SetItemText(i,13,strInfo);
		}
		m_ListCmrInfo.SetCheck(0);
		if(m_vecAllCmrInfo.size()>0) m_nCmrIdxSelected=0;
	}
	else
	{
		int nItemCount=m_ListCmrInfo.GetItemCount();
		for(int i = 0; i<m_vecNewCmrInfo.size(); i++)
		{
			CString strInfo; strInfo.Format("%d",i);
			m_ListCmrInfo.InsertItem(nItemCount+i,strInfo);
			m_ListCmrInfo.SetItemText(nItemCount+i,1,m_vecAllCmrInfo[i].strLabel);
			strInfo.Format("%.8lf",m_vecNewCmrInfo[i].x0);m_ListCmrInfo.SetItemText(nItemCount+i,2,strInfo);
			strInfo.Format("%.8lf",m_vecNewCmrInfo[i].y0);m_ListCmrInfo.SetItemText(nItemCount+i,3,strInfo);
			strInfo.Format("%.8lf",m_vecNewCmrInfo[i].f);m_ListCmrInfo.SetItemText(nItemCount+i,4,strInfo);
			strInfo.Format("%.8lf",m_vecNewCmrInfo[i].fx);m_ListCmrInfo.SetItemText(nItemCount+i,5,strInfo);
			strInfo.Format("%.8lf",m_vecNewCmrInfo[i].fy);m_ListCmrInfo.SetItemText(nItemCount+i,6,strInfo);
			strInfo.Format("%.8lf",m_vecNewCmrInfo[i].pixsize);m_ListCmrInfo.SetItemText(nItemCount+i,7,strInfo);
			strInfo.Format("%.8lf",m_vecNewCmrInfo[i].k1);m_ListCmrInfo.SetItemText(nItemCount+i,8,strInfo);
			strInfo.Format("%.8lf",m_vecNewCmrInfo[i].k2);m_ListCmrInfo.SetItemText(nItemCount+i,9,strInfo);
			strInfo.Format("%.8lf",m_vecNewCmrInfo[i].p1);m_ListCmrInfo.SetItemText(nItemCount+i,10,strInfo);
			strInfo.Format("%.8lf",m_vecNewCmrInfo[i].p2);m_ListCmrInfo.SetItemText(nItemCount+i,11,strInfo);
			strInfo.Format("%.8lf",m_vecNewCmrInfo[i].a);m_ListCmrInfo.SetItemText(nItemCount+i,12,strInfo);
			strInfo.Format("%.8lf",m_vecNewCmrInfo[i].b);m_ListCmrInfo.SetItemText(nItemCount+i,13,strInfo);
		}
		if(m_nCmrIdxSelected==InvalidValue)
		{
			m_ListCmrInfo.SetCheck(0);
			if(m_vecAllCmrInfo.size()>0) 
				m_nCmrIdxSelected=0;
		}
	}
}


void CPrjNewDlg::OnEnChangeMfceditbrowseCmrFilepath()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if(/*!m_bAddData*/1) ClearCmrInfo();
	m_bCmrOk = FunReadCmrFile(m_strCmrFilePath,m_SysCfgData.CmrFileExtend,m_vecNewCmrInfo);

	if(m_bCmrOk)
	{
		if(m_SysCfgData.bAutoJudgeUnion)
		{
			if(m_vecNewCmrInfo[0].pixsize==1)
			{
				m_SysCfgData.CmrFileExtend.Unitf=PIX;
				m_SysCfgData.CmrFileExtend.Unitx0y0=PIX;
				m_SysCfgData.CmrFileExtend.Unitpixsize=PIX;
				for (int i = 0; i<m_vecNewCmrInfo.size(); i++)
				{
					m_vecNewCmrInfo[i].eUnitf = PIX;
					m_vecNewCmrInfo[i].eUnitx0y0 = PIX;
					m_vecNewCmrInfo[i].eUnitpixsize = PIX;
				}
			}
			else
			{
				m_SysCfgData.CmrFileExtend.Unitf=MM;
				m_SysCfgData.CmrFileExtend.Unitx0y0=MM;
				m_SysCfgData.CmrFileExtend.Unitpixsize=MM;
				for (int i = 0; i<m_vecNewCmrInfo.size(); i++)
				{
					m_vecNewCmrInfo[i].eUnitf = MM;
					m_vecNewCmrInfo[i].eUnitx0y0 = MM;
					m_vecNewCmrInfo[i].eUnitpixsize = MM;
				}
			}
			InitCmrList();
		}
		UnionCmrNew2AllNoRepeat();
		FillCmrList();
		vector<SSWstuCmrInfo>().swap(m_vecNewCmrInfo);
		m_ListImgInfo.Invalidate();
	}
}
void CPrjNewDlg::OnEnChangeMfceditbrowsePosFilepath()
{
	UpdateData(TRUE);
	/*if(!m_bAddData) */ClearPosInfo();
	m_bPosOk = FunReadPosFile(m_strPosFilePath, m_SysCfgData.PosFileExtend, m_vecNewPosInfo);
	if(m_bPosOk)
	{
		UnionPosNew2AllNoRepeat();	
		m_eImgPosMatchDirection=POS2IMG;
		HANDLE hThread;DWORD IdThread;
		hThread = CreateThread(NULL, 0,
			(LPTHREAD_START_ROUTINE)MultiThreadImgPosMatch,
			(LPVOID*)this, 0, &IdThread);
		//自动检测XYZ-LBH
		if(m_SysCfgData.bAutoJudgeUnion)
			OnBnClickedButtonChangePoscoordinate();
	}

}
void CPrjNewDlg::OnEnChangeMfceditbrowseImgfolder()
{
	UpdateData(TRUE);
	if(/*!m_bAddData*/1) ClearImgInfo();
	CString strFolder(m_strImgFolderPath);
	CString strExt;
	if(/*m_bSearchJpg*/1)
	{
		FunSearchFile(strFolder,"jpg",m_vecNewImgPath);
		FunSearchFile(strFolder,"JPG",m_vecNewImgPath);
		FunSearchFile(strFolder,"jpeg",m_vecNewImgPath);
		FunSearchFile(strFolder,"JPEG",m_vecNewImgPath);
	}
	if(/*m_bSearchTif*/1)
	{
		FunSearchFile(strFolder,"tif",m_vecNewImgPath);
		FunSearchFile(strFolder,"TIF",m_vecNewImgPath);
		FunSearchFile(strFolder,"tiff",m_vecNewImgPath);
		FunSearchFile(strFolder,"TIFF",m_vecNewImgPath);
	}
	if(m_vecNewImgPath.size()==0) 
	{
		m_ListImgInfo.SetItemCountEx(0);
		m_ListImgInfo.Invalidate();
		return;
	}
	FunOutPutLogInfo("文件检索成功。");
	UnionImgNew2AllNoRepeat();
	m_eImgPosMatchDirection=IMG2POS;
	HANDLE hThread;DWORD IdThread;
	hThread = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)MultiThreadImgPosMatch,
		(LPVOID*)this, 0, &IdThread);
}

void CPrjNewDlg::ImgAndPosMatch()
{
//	g_pMain->m_pStatusPaneLabel->SetText("影像及POS匹配...");
//	g_pMain->m_pStatusPaneLabel->Redraw();
	switch(m_eImgPosMatchDirection)
	{
	case IMG2POS:

		MatchImg2Pos();
		break;
	case POS2IMG:
		MatchPos2Img();
		break;
	default:
		MatchImg2Pos();
		break;
	}
// 	g_pMain->m_pStatusPaneLabel->SetText("");
// 	g_pMain->m_pStatusPaneLabel->Redraw();
}
void CPrjNewDlg::MatchImg2Pos()
{
	g_wndProgressCtrl.ShowWindow(true);
	g_wndProgressCtrl.SetPos(0);

	CRITICAL_SECTION g_cs;
	if(m_SysCfgData.PosFileExtend.MethodPosImgMch==BY_ORDER)
	{
		int num = min(m_vecAllImgPath.size(), m_vecAllPosInfo.size());
		for (int i = 0; i<num; i++)
		{
			if(m_vecAllPosInfo.size()<num||m_vecAllImgPath.size()<num) return;
			g_wndProgressCtrl.SetPos((i+1)*100.0/num);
			m_vecImgIdxToPosIdx[i]=i;
			m_nImgPosed++;
		}
	}
	else if(m_SysCfgData.PosFileExtend.MethodPosImgMch==BY_NAME)
	{
		for (int i = 0; i<m_vecAllImgPath.size(); i++)
		{
			for (int j = 0; j<m_vecAllPosInfo.size(); j++)
			{
				if(m_vecAllPosInfo.size()==0||m_vecAllImgPath.size()==0) return;
				if(m_vecAllImgName[i]==m_vecAllPosInfo[j].strLabel)
				{
					m_vecImgIdxToPosIdx[i]=j;
					g_wndProgressCtrl.SetPos((i*m_vecAllPosInfo.size()+j)*100.0/(m_vecAllImgPath.size()*m_vecAllPosInfo.size()));
				m_nImgPosed++;
					break;
				}
			}
			
		}
// 		int jStart = 0;
// 		for (int i = 0; i<m_vecAllImgPath.size(); i++)
// 		{
// 			CString strPosName,strImgName,strPosNameExt,strImgNameExt;
// 			strImgName = FunGetFileName(m_vecAllImgPath[i],false).MakeLower();
// 			strImgNameExt = FunGetFileName(m_vecAllImgPath[i],true).MakeLower();
// 			for (int j = jStart; j<m_vecAllPosInfo.size(); j++)
// 			{
// 				g_wndProgressCtrl.SetPos((i*m_vecAllPosInfo.size()+j)*100.0/(m_vecAllImgPath.size()*m_vecAllPosInfo.size()));
// 				strPosName = FunGetFileName(m_vecAllPosInfo[j].strLabel, false).MakeLower();
// 				strPosNameExt = FunGetFileName(m_vecAllPosInfo[j].strLabel, true).MakeLower();
// 				if(strImgName==strPosName||strImgNameExt==strPosNameExt)
// 				{
// 					m_vecImgIdxToPosIdx[i]=j;
// 					m_nImgPosed++;
// 					break;
// 					jStart=j;
// 				}
// 			}
// 		}

	}

	vector<CString>().swap(m_vecNewImgPath);
	m_ListImgInfo.SetItemCountEx(m_vecAllImgPath.size());
	m_ListImgInfo.Invalidate();
	g_wndProgressCtrl.ShowWindow(false);
}
void CPrjNewDlg::MatchPos2Img()
{
//	SetTimer(TIME_ID_IMGPOSMATCH,600,NULL);
	g_wndProgressCtrl.ShowWindow(true);
	g_wndProgressCtrl.SetPos(0);
	if(m_SysCfgData.PosFileExtend.MethodPosImgMch==BY_ORDER)
	{
		int num = min(m_vecAllImgPath.size(), m_vecAllPosInfo.size());
		for (int i = 0; i<num; i++)
		{
			if(m_vecAllPosInfo.size()<num||m_vecAllImgPath.size()<num) return;
			g_wndProgressCtrl.SetPos((i+1)*100.0/num);
			m_vecImgIdxToPosIdx[i]=i;
			m_nImgPosed++;
		}
	}
	else if(m_SysCfgData.PosFileExtend.MethodPosImgMch==BY_NAME)
 	{
		for (int i = 0; i<m_vecAllImgPath.size(); i++)
		{
			for (int j = 0; j<m_vecAllPosInfo.size(); j++)
			{
				if(m_vecAllPosInfo.size()==0||m_vecAllImgPath.size()==0) return;
				if(m_vecAllImgName[i]==m_vecAllPosInfo[j].strLabel)
				{
					m_vecImgIdxToPosIdx[i]=j;
					g_wndProgressCtrl.SetPos((i*m_vecAllPosInfo.size()+j)*100.0/(m_vecAllImgPath.size()*m_vecAllPosInfo.size()));
					m_nImgPosed++;
					break;
				}
			}

		}
// 		CString strPosName,strImgName,strPosNameExt,strImgNameExt;
// 		//#pragma omp parallel for
// 		int jStart = 0;
// 		for (int i = 0; i<m_vecAllImgPath.size(); i++)
// 		{
// 			strImgName = FunGetFileName(m_vecAllImgPath[i],false).MakeLower();
// 			strImgNameExt = FunGetFileName(m_vecAllImgPath[i],true).MakeLower();
// 			for (int j = jStart; j<m_vecAllPosInfo.size(); j++)
// 			{
// 				g_wndProgressCtrl.SetPos((i*m_vecAllPosInfo.size()+j)*100/(m_vecAllImgPath.size()*m_vecAllPosInfo.size()));
// 				strPosName = FunGetFileName(m_vecAllPosInfo[j].strLabel, false).MakeLower();
// 				strPosNameExt = FunGetFileName(m_vecAllPosInfo[j].strLabel, true).MakeLower();
// 				if(strImgName==strPosName||strImgNameExt==strPosNameExt)
// 				{
// 					m_vecImgIdxToPosIdx[i]=j;
// 					m_nImgPosed++;
// 					jStart=j;
// 					break;
// 				}
// 			}
// 		}
	}

	vector<SSWstuPosInfo>().swap(m_vecNewPosInfo);
	m_ListImgInfo.Invalidate();
	g_wndProgressCtrl.ShowWindow(false);
}

void CPrjNewDlg::OnGetdispinfoListImginfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pGetInfoTip = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	LV_ITEM* pItem = &(pGetInfoTip)->item;
	if(m_vecAllImgPath.size()==0) return;
	if (pItem->mask & LVIF_TEXT)
	{
		SSWstuPosInfo sswPos;SSWstuCmrInfo sswCmr;
		int nPosIdx = m_vecImgIdxToPosIdx[pItem->iItem];
		int nCmrIdx=m_nCmrIdxSelected;
		if(nPosIdx==InvalidValue) sswPos = SSWstuPosInfo();
		else sswPos=m_vecAllPosInfo[nPosIdx];
		if(nCmrIdx==InvalidValue||m_vecAllCmrInfo.size()==0) sswCmr = SSWstuCmrInfo();
		else sswCmr=m_vecAllCmrInfo[nCmrIdx];
		CString strImgPath = m_vecAllImgPath[pItem->iItem];
		switch (pItem->iSubItem)
		{
		case 0:{
			char strId[10]; sprintf(strId, "%d", pItem->iItem);
			lstrcpyn(pItem->pszText, strId, pItem->cchTextMax);
			break;
			   }
		case 1:{
			lstrcpyn(pItem->pszText, strImgPath, pItem->cchTextMax);
			break;
			   }
		case 2:{
			char strInfo[50]; sprintf(strInfo, "%lf", sswPos.Xs);
			lstrcpyn(pItem->pszText, strInfo, pItem->cchTextMax);
			break;
			   }
		case 3:{
			char strInfo[50]; sprintf(strInfo, "%lf", sswPos.Ys);
			lstrcpyn(pItem->pszText, strInfo, pItem->cchTextMax);
			break;
			   }
		case 4:{
			char strInfo[50]; sprintf(strInfo, "%lf", sswPos.Zs);
			lstrcpyn(pItem->pszText, strInfo, pItem->cchTextMax);
			break;
			   }
		case 5:{
			char strInfo[50]; sprintf(strInfo, "%lf", sswPos.omg);
			lstrcpyn(pItem->pszText, strInfo, pItem->cchTextMax);
			break;
			   }
		case 6:{
			char strInfo[50]; sprintf(strInfo, "%lf", sswPos.phi);
			lstrcpyn(pItem->pszText, strInfo, pItem->cchTextMax);
			break;
			   }
		case 7:{
			char strInfo[50]; sprintf(strInfo, "%lf", sswPos.kap);
			lstrcpyn(pItem->pszText, strInfo, pItem->cchTextMax);
			break;
			   }
		case 8:{
			lstrcpyn(pItem->pszText, sswCmr.strLabel, pItem->cchTextMax);
			break;
			   }

		default:
			break;
		}
	}
	*pResult = 0;
}


void CPrjNewDlg::OnClickListCmrinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	NMLISTVIEW* pNMListView=(NM_LISTVIEW*)pNMHDR;
	LVHITTESTINFO hit;
	*pResult = 0;
	bool bChecked = false;
	hit.pt=pNMListView->ptAction;
	int nItem=m_ListCmrInfo.HitTest(&hit);
// 	if(hit.flags!=LVHT_ONITEMSTATEICON) return;
// 	if(m_ListCmrInfo.GetCheck(nItem)) return;
	for (int i = 0; i<m_ListCmrInfo.GetItemCount(); i++)
	{
		if(i!=nItem) m_ListCmrInfo.SetItemState(i,LVIS_SELECTED,0);
		else m_ListCmrInfo.SetItemState(i,LVIS_SELECTED,LVIS_SELECTED);
//		m_ListCmrInfo.SetCheck(i,false);
	}
	m_nCmrIdxSelected=nItem;
//	m_ListCmrInfo.SetCheck(nItem,true);
}

void CPrjNewDlg::DeleteInvalidImgAndPos()
{
	for (int i = 0; i<m_vecAllImgPath.size(); i++)
	{
		if(m_vecImgIdxToPosIdx[i]==InvalidValue)
		{
			m_vecAllImgPath.erase(m_vecAllImgPath.begin()+i);    //删除多余影像
			m_vecImgIdxToPosIdx.erase(m_vecImgIdxToPosIdx.begin()+i);
			i--;
		}
	}

	for (int i = 0; i<m_vecAllPosInfo.size(); i++)
	{
		bool bImged=false;
		for (int j = 0; j<m_vecImgIdxToPosIdx.size(); j++)
		{
			if(m_vecImgIdxToPosIdx[j]==i)
			{
				bImged=true;
				break;
			}
		}
		if(!bImged)
		{
			for (int j = 0; j<m_vecImgIdxToPosIdx.size();j++)
			{
				if(m_vecImgIdxToPosIdx[j]>=i)
					m_vecImgIdxToPosIdx[j]--;
			}
			m_vecAllPosInfo.erase(m_vecAllPosInfo.begin()+i);
			i--;
	
		}
	}
}

vector<CsswUAVImage> CPrjNewDlg::LoadImg2Prj()
{
	vector<CsswUAVImage> vecImg(m_vecAllImgPath.size());
	for (int i = 0; i<m_vecAllImgPath.size(); i++)
	{
		vecImg[i].m_nImgIdx=i;
		vecImg[i].m_strImgPath=m_vecAllImgPath[i];
		if(m_nCmrIdxSelected>=0) 
			vecImg[i].m_ImgCmrInfo=m_vecAllCmrInfo[m_nCmrIdxSelected];
		vecImg[i].m_ImgPosInfo = m_vecAllPosInfo[m_vecImgIdxToPosIdx[i]];	
		vecImg[i].m_strImgName = FunGetFileName(vecImg[i].m_strImgPath,true).MakeLower();
	}
	return vecImg;
}

void CPrjNewDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if(FunGetFileExt(m_strPrjFilePath)!="xml")
	{
		m_strPrjFilePath+=".xml";
		UpdateData(FALSE);
	}

	if(m_strPrjFilePath.IsEmpty())
	{
		AfxMessageBox("请设置工程路径！");
		return;
	}
	if(m_nCmrIdxSelected==InvalidValue)
	{
		CsswUAVImage image;image.OpenImg(m_vecAllImgPath[0]);
		SSWstuCmrInfo cmrInfo;
		cmrInfo.f = image.GetFocalLength(PIX);
		cmrInfo.eUnitf = PIX;
		cmrInfo.eUnitpixsize=PIX;
		cmrInfo.eUnitx0y0=MM;
		cmrInfo.x0=0;
		cmrInfo.y0=0;
		cmrInfo.a=0;
		cmrInfo.b=0;
		cmrInfo.k1=0;
		cmrInfo.k2=0;
		cmrInfo.p1=0;
		cmrInfo.p2=0;
		cmrInfo.fx=cmrInfo.f;
		cmrInfo.fy=cmrInfo.f;
		cmrInfo.pixsize=1;
		cmrInfo.strLabel = "1";
		if(cmrInfo.f==0)
		{
			AfxMessageBox("影像文件无法识别焦距信息，请设置相机参数！");
			return;
		}
		m_vecAllCmrInfo.push_back(cmrInfo);
		m_nCmrIdxSelected=0;
	}
	if(m_nImgPosed == 0)
	{
		if(AfxMessageBox("无影像具有POS信息，是否继续?",MB_YESNO)==IDNO)
		{
			return;
		}
	}
	if(/*m_bDeleteImgNoPos*/1)
	{
		DeleteInvalidImgAndPos();
	}
	CDialogEx::OnOK();
}


void CPrjNewDlg::OnBnClickedButtonChangePoscoordinate()
{
	// TODO: 在此添加控件通知处理程序代码
	//自动解析经纬度还是XYZ
	if(m_vecAllPosInfo.size()==0) return;
	CString strTitleX,strTitleY;
	if(m_vecAllPosInfo[0].Xs>=-180&&m_vecAllPosInfo[0].Ys<=180&&m_vecAllPosInfo[0].Ys>=-90&&m_vecAllPosInfo[0].Ys<=90)
	{
		if(m_SysCfgData.PosFileExtend.FormatAngleLBH==RAD)
		{
			strTitleX="Lon(rad)";
			strTitleY="Lat(rad)";
		}else
		{
			strTitleX="Lon(deg)";
			strTitleY="Lat(deg)";
		}
		for(int i = 0 ;i<m_vecAllPosInfo.size(); i++)
		{
			m_vecAllPosInfo[i].eFormatCoordinate = LBH;
		}

	}
	else
	{
		strTitleX="X(m)";
     	strTitleY="Y(m)";
		for(int i = 0 ;i<m_vecAllPosInfo.size(); i++)
		{
			m_vecAllPosInfo[i].eFormatCoordinate = XYZ;
		}
	}
	//CString strTitleX,strTitleY;
	//if(m_SysCfgData.PosFileExtend.FormatCoor==LBH)
	//{
	//	m_SysCfgData.PosFileExtend.FormatCoor=XYZ;
	//	strTitleX="X(m)";
	//	strTitleY="Y(m)";
	//}
	//else if(m_SysCfgData.PosFileExtend.FormatAngleLBH==RAD)
	//{
	//	m_SysCfgData.PosFileExtend.FormatCoor=LBH;
	//	strTitleX="Lon(rad)";
	//	strTitleY="Lat(rad)";
	//}
	//else 
	//{
	//	m_SysCfgData.PosFileExtend.FormatCoor=LBH;
	//	strTitleX="Lon(deg)";
	//	strTitleY="Lat(deg)";
	//}
	CHeaderCtrl *pHeader = m_ListImgInfo.GetHeaderCtrl();
	TCHAR szBuf[129] = {0};    
	HDITEM hdItem ={0};    
	hdItem.mask = HDI_TEXT; //取字符掩码     
	hdItem.pszText = szBuf; //字符缓冲区     
	hdItem.cchTextMax = 128; //缓冲区大小      
	hdItem.pszText = strTitleX.GetBuffer();     
	pHeader->SetItem(2,&hdItem);
	hdItem.pszText = strTitleY.GetBuffer();     
	pHeader->SetItem(3,&hdItem);
	strTitleX.ReleaseBuffer();
	strTitleY.ReleaseBuffer();
// 	for (int i = 0; i<m_vecAllPosInfo.size(); i++)
// 	{
// 		m_vecAllPosInfo[i].eFormatCoordinate = m_SysCfgData.PosFileExtend.FormatCoor;
// 	}
}



