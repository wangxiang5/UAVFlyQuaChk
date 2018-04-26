// SysConfigDataDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "sswUAVFlyQuaSysEng.h"
#include "SysConfigureDataDlg.h"
#include "afxdialogex.h"

// CSysConfigureDataDlg 对话框

IMPLEMENT_DYNAMIC(CSysConfigureDataDlg, CDialogEx)

CSysConfigureDataDlg::CSysConfigureDataDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSysConfigureDataDlg::IDD, pParent)
{

}

CSysConfigureDataDlg::~CSysConfigureDataDlg()
{
}

void CSysConfigureDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CMRDATA, m_ListCmrData);
	DDX_Control(pDX, IDC_LIST_POSDATA, m_ListPosData);
	DDX_Control(pDX, IDC_COMBO_UNIT_X0Y0, m_ComboUnitX0Y0);
// 	DDX_Control(pDX, IDC_COMBO_UNIT_F, m_ComboUnitF);
// 	DDX_Control(pDX, IDC_COMBO_UNIT_PIXSIZE, m_ComboUnitPixSize);
	DDX_Control(pDX, IDC_COMBO_FORMAT_POSCOORDINATE, m_ComboFormatCoordinate);
	DDX_Control(pDX, IDC_COMBO_FORMAT_POSCOORDINATELBH, m_ComboFormatPosCoordinateLBH);
	DDX_Control(pDX, IDC_COMBO_FORMAT_POSANGLE, m_ComboFormatPosAngle);
	DDX_Control(pDX, IDC_COMBO_METHOD_POSIMGMATCH, m_ComboMethodPosImgMch);
}


BEGIN_MESSAGE_MAP(CSysConfigureDataDlg, CDialogEx)
	ON_WM_SIZE()
	ON_CBN_SELCHANGE(IDC_COMBO_UNIT_X0Y0, &CSysConfigureDataDlg::OnCbnSelchangeComboUnitX0y0)
// 	ON_CBN_SELCHANGE(IDC_COMBO_UNIT_F, &CSysConfigureDataDlg::OnCbnSelchangeComboUnitF)
// 	ON_CBN_SELCHANGE(IDC_COMBO_UNIT_PIXSIZE, &CSysConfigureDataDlg::OnCbnSelchangeComboUnitPixsize)
	ON_CBN_SELCHANGE(IDC_COMBO_FORMAT_POSCOORDINATE, &CSysConfigureDataDlg::OnCbnSelchangeComboFormatPoscoordinate)
	ON_CBN_SELCHANGE(IDC_COMBO_FORMAT_POSCOORDINATELBH, &CSysConfigureDataDlg::OnCbnSelchangeComboFormatPoscoordinatelbh)
	ON_CBN_SELCHANGE(IDC_COMBO_FORMAT_POSANGLE, &CSysConfigureDataDlg::OnCbnSelchangeComboFormatPosangle)
	ON_CBN_SELCHANGE(IDC_COMBO_METHOD_POSIMGMATCH, &CSysConfigureDataDlg::OnCbnSelchangeComboMethodPosimgmatch)
	ON_NOTIFY(NM_CLICK, IDC_LIST_CMRDATA, &CSysConfigureDataDlg::OnClickListCmrdata)
	ON_NOTIFY(NM_CLICK, IDC_LIST_POSDATA, &CSysConfigureDataDlg::OnClickListPosdata)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CSysConfigureDataDlg 消息处理程序
void CSysConfigureDataDlg::InitItemPos()
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

void CSysConfigureDataDlg::ResetItemPos()
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
		if (woc == IDC_COMBO_UNIT_X0Y0 ||
			woc == IDC_COMBO_UNIT_F ||
			woc == IDC_COMBO_UNIT_PIXSIZE||
			woc == IDC_COMBO_FORMAT_POSCOORDINATE||
			woc == IDC_COMBO_FORMAT_POSCOORDINATELBH||
			woc == IDC_COMBO_FORMAT_POSANGLE||
			woc == IDC_COMBO_METHOD_POSIMGMATCH)
			Rect.bottom += 150;
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

void CSysConfigureDataDlg::InitItems()
{
	CRect rect; DWORD dwStyle; int nListWidth, nListHeight;
	dwStyle = (m_ListCmrData).GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;     //可以选择整行
	dwStyle |= LVS_EX_GRIDLINES;         //格网形式 
	dwStyle |= LVS_OWNERDRAWFIXED;       //可重绘
	dwStyle |= LVS_EX_DOUBLEBUFFER;
//	dwStyle |= LVS_EX_HEADERDRAGDROP;
	m_ListCmrData.SetExtendedStyle(dwStyle);

	m_ListCmrData.GetClientRect(rect);
	nListWidth = rect.Width(); nListHeight = rect.Height();
	m_ListCmrData.InsertColumn(0,"", LVCFMT_LEFT, 30, -1);
	m_ListCmrData.InsertColumn(1, "Label", LVCFMT_LEFT, 45, -1);
	m_ListCmrData.InsertColumn(2, "x0", LVCFMT_LEFT, 45 - 1);
	m_ListCmrData.InsertColumn(3, "y0", LVCFMT_LEFT, 45 - 1);
	m_ListCmrData.InsertColumn(4, "f", LVCFMT_LEFT,45 - 1);
	m_ListCmrData.InsertColumn(5, "fx", LVCFMT_LEFT, 45 - 1);
	m_ListCmrData.InsertColumn(6, "fy", LVCFMT_LEFT, 45 - 1);
	m_ListCmrData.InsertColumn(7, "pixsize", LVCFMT_LEFT, 45 - 1);
	m_ListCmrData.InsertColumn(8, "k1", LVCFMT_LEFT, 40 - 1);
	m_ListCmrData.InsertColumn(9, "k2", LVCFMT_LEFT, 40 - 1);
	m_ListCmrData.InsertColumn(10, "p1", LVCFMT_LEFT, 40 - 1);
	m_ListCmrData.InsertColumn(11, "p2", LVCFMT_LEFT, 40 - 1);
	m_ListCmrData.InsertColumn(12, "a", LVCFMT_LEFT, 40 - 1);
	m_ListCmrData.InsertColumn(13, "b", LVCFMT_LEFT, 40 - 1);
	m_ListCmrData.GetHeaderCtrl()->SetRemovable(0, FALSE);
	m_ListCmrData.InsertItem(0,"nCol");
	m_ListCmrData.SetItemText(0,1,"12");
	for(int i = 2; i<SIZE_CMR_INFO_EXTEND+1; i++)
	{
		CString strInfo; strInfo.Format("%d",i);
		m_ListCmrData.SetItemText(0,i,strInfo);
	}
	m_ListCmrData.SetRegistryKey("My ListCtrlEx");
	m_ListCmrData.RestoreState();


	m_ListPosData.SetExtendedStyle(dwStyle);
	m_ListPosData.GetClientRect(rect);
	nListWidth = rect.Width(); nListHeight = rect.Height();
	m_ListPosData.InsertColumn(0,"", LVCFMT_LEFT, 30, -1);
	m_ListPosData.InsertColumn(1, "Label", LVCFMT_LEFT, 50, -1);
	m_ListPosData.InsertColumn(2, "Lon", LVCFMT_LEFT, 60 - 1);
	m_ListPosData.InsertColumn(3, "Lat", LVCFMT_LEFT, 60 - 1);
	m_ListPosData.InsertColumn(4, "Alt", LVCFMT_LEFT, 60 - 1);
	m_ListPosData.InsertColumn(5, "Omega", LVCFMT_LEFT, 60 - 1);
	m_ListPosData.InsertColumn(6, "Phi", LVCFMT_LEFT, 60 - 1);
	m_ListPosData.InsertColumn(7, "Kappa", LVCFMT_LEFT, 60 - 1);

	m_ListPosData.GetHeaderCtrl()->SetRemovable(0, FALSE);
	m_ListPosData.InsertItem(0,"nCol");
	for(int i = 1; i<SIZE_POS_INFO_EXTEND+1; i++)
	{
		CString strInfo; strInfo.Format("%d",i);
		m_ListPosData.SetItemText(0,i,strInfo);
	}
	m_ListPosData.SetRegistryKey("My ListCtrlEx");
	m_ListPosData.RestoreState();


	m_ComboUnitX0Y0.SetCurSel(2);
// 	m_ComboUnitF.SetCurSel(2);
// 	m_ComboUnitPixSize.SetCurSel(2);
	m_ComboFormatCoordinate.SetCurSel(LBH);
	m_ComboFormatPosCoordinateLBH.SetCurSel(DEG);
	m_ComboFormatPosAngle.SetCurSel(DEG);
	m_ComboMethodPosImgMch.SetCurSel(BY_NAME);

	m_ComboFormatPosCoordinateLBH.EnableWindow(TRUE);
}
void CSysConfigureDataDlg::UpdateItems()
{
	switch (m_SysCfgData.CmrFileExtend.Unitx0y0)
	{
	case PIX:m_ComboUnitX0Y0.SetCurSel(0);break;
	case UM: m_ComboUnitX0Y0.SetCurSel(1);break;
	case MM: m_ComboUnitX0Y0.SetCurSel(2);break;
	case CM: m_ComboUnitX0Y0.SetCurSel(3);break;
	case M:  m_ComboUnitX0Y0.SetCurSel(4);break;
	}
// 	switch (m_SysCfgData.CmrFileExtend.Unitf)
// 	{
// 	case PIX:m_ComboUnitF.SetCurSel(0);break;
// 	case UM: m_ComboUnitF.SetCurSel(1);break;
// 	case MM: m_ComboUnitF.SetCurSel(2);break;
// 	case CM: m_ComboUnitF.SetCurSel(3);break;
// 	case M:  m_ComboUnitF.SetCurSel(4);break;
// 	}
// 	switch (m_SysCfgData.CmrFileExtend.Unitpixsize)
// 	{
// 	case PIX:m_ComboUnitPixSize.SetCurSel(0);break;
// 	case UM: m_ComboUnitPixSize.SetCurSel(1);break;
// 	case MM: m_ComboUnitPixSize.SetCurSel(2);break;
// 	case CM: m_ComboUnitPixSize.SetCurSel(3);break;
// 	case M:  m_ComboUnitPixSize.SetCurSel(4);break;
// 	}
	switch (m_SysCfgData.PosFileExtend.FormatCoor)
	{
	case LBH: m_ComboFormatCoordinate.SetCurSel(0);break;
	case XYZ: m_ComboFormatCoordinate.SetCurSel(1);break;
	}
	switch (m_SysCfgData.PosFileExtend.FormatAngleLBH)
	{
	case DEG: m_ComboFormatPosCoordinateLBH.SetCurSel(0);break;
	case RAD: m_ComboFormatPosCoordinateLBH.SetCurSel(1);break;
	}	
	switch (m_SysCfgData.PosFileExtend.FormatAnglePOK)
	{
	case DEG: m_ComboFormatPosAngle.SetCurSel(0);break;
	case RAD: m_ComboFormatPosAngle.SetCurSel(1);break;
	}
	switch (m_SysCfgData.PosFileExtend.MethodPosImgMch)
	{
	case BY_NAME:     m_ComboMethodPosImgMch.SetCurSel(0);break;
	case BY_ORDER:    m_ComboMethodPosImgMch.SetCurSel(1);break;
	}
	for (int i = 1; i<SIZE_CMR_INFO_EXTEND+1; i++)
	{
		char strInfo[10]; sprintf(strInfo,"%d",m_SysCfgData.CmrFileExtend.pCfeColMap[i-1]);
		m_ListCmrData.SetItemText(0,i,strInfo);
	}
	for (int i = 1; i<SIZE_POS_INFO_EXTEND+1; i++)
	{
		char strInfo[10]; sprintf(strInfo,"%d",m_SysCfgData.PosFileExtend.pPfeColMap[i-1]);
		m_ListPosData.SetItemText(0,i,strInfo);
	}
}
void CSysConfigureDataDlg::UpdateDatas()
{
	for (int i = 1; i<SIZE_CMR_INFO_EXTEND+1; i++)
	{
		char strInfo[100]; strcpy(strInfo,m_ListCmrData.GetItemText(0,i));
		*(m_SysCfgData.CmrFileExtend.pCfeColMap+i-1) = atoi(strInfo);
	}
	for (int i = 1; i<SIZE_POS_INFO_EXTEND+1; i++)
	{
		char strInfo[100]; strcpy(strInfo,m_ListPosData.GetItemText(0,i));
		*(m_SysCfgData.PosFileExtend.pPfeColMap+i-1) = atoi(strInfo);
	}
}

BOOL CSysConfigureDataDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	InitItems();
//	InitItemPos();
	UpdateItems();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CSysConfigureDataDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	//ResetItemPos();
	// TODO: 在此处添加消息处理程序代码
}


void CSysConfigureDataDlg::OnCbnSelchangeComboUnitX0y0()
{
	// TODO: 在此添加控件通知处理程序代码
	m_SysCfgData.CmrFileExtend.Unitx0y0=ComSel2Unit(m_ComboUnitX0Y0.GetCurSel());
	m_SysCfgData.CmrFileExtend.Unitf=ComSel2Unit(m_ComboUnitX0Y0.GetCurSel());
	m_SysCfgData.CmrFileExtend.Unitpixsize=ComSel2Unit(m_ComboUnitX0Y0.GetCurSel());
}


// void CSysConfigureDataDlg::OnCbnSelchangeComboUnitF()
// {
// 	// TODO: 在此添加控件通知处理程序代码
// 	m_SysCfgData.CmrFileExtend.Unitf=ComSel2Unit(m_ComboUnitF.GetCurSel());
// }
// 
// 
// void CSysConfigureDataDlg::OnCbnSelchangeComboUnitPixsize()
// {
// 	// TODO: 在此添加控件通知处理程序代码
// 	m_SysCfgData.CmrFileExtend.Unitpixsize=ComSel2Unit(m_ComboUnitPixSize.GetCurSel());
// }


void CSysConfigureDataDlg::OnCbnSelchangeComboFormatPoscoordinate()
{
	// TODO: 在此添加控件通知处理程序代码
	m_SysCfgData.PosFileExtend.FormatCoor=ComSel2CoorFormat(m_ComboFormatCoordinate.GetCurSel());
}


void CSysConfigureDataDlg::OnCbnSelchangeComboFormatPoscoordinatelbh()
{
	// TODO: 在此添加控件通知处理程序代码
	m_SysCfgData.PosFileExtend.FormatAngleLBH=ComSel2AngleFormat(m_ComboFormatPosCoordinateLBH.GetCurSel());
}


void CSysConfigureDataDlg::OnCbnSelchangeComboFormatPosangle()
{
	// TODO: 在此添加控件通知处理程序代码
	m_SysCfgData.PosFileExtend.FormatAnglePOK=ComSel2AngleFormat(m_ComboFormatPosAngle.GetCurSel());
}


void CSysConfigureDataDlg::OnCbnSelchangeComboMethodPosimgmatch()
{
	// TODO: 在此添加控件通知处理程序代码
	m_SysCfgData.PosFileExtend.MethodPosImgMch = ComSel2PosImgMatchMethod(m_ComboMethodPosImgMch.GetCurSel());
}


void CSysConfigureDataDlg::OnClickListCmrdata(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
		// TODO: 在此添加控件通知处理程序代码
	POINT point; GetCursorPos(&point); ScreenToClient(&point);
	LVHITTESTINFO hi; hi.pt = point;
	//if (m_ListCmrData.SubItemHitTest(&hi) == -1) return;
	if(m_ListCmrData.IsEditItem()) 
		m_ListCmrData.UpdateEditItem();
	if(pNMItemActivate==NULL) return;
	int nCol = pNMItemActivate->iSubItem;
	int nRow = pNMItemActivate->iItem;
	if(nRow>=0&&nRow<m_ListCmrData.GetItemCount()&&	nCol> 0&&nCol<m_ListCmrData.GetHeaderCtrl()->GetItemCount())  //第1列不可编辑
	{
		m_ListCmrData.SetEditItem(nRow,nCol);
	}
	*pResult = 0;
}


void CSysConfigureDataDlg::OnClickListPosdata(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	POINT point; GetCursorPos(&point); ScreenToClient(&point);
	LVHITTESTINFO hi; hi.pt = point;
	//if (m_ListCmrData.SubItemHitTest(&hi) == -1) return;
	if(m_ListPosData.IsEditItem()) 	
		m_ListPosData.UpdateEditItem();
	if(pNMItemActivate==NULL) return;
	int nCol = pNMItemActivate->iSubItem;
	int nRow = pNMItemActivate->iItem;
	
	if(nRow>=0&&nRow<m_ListPosData.GetItemCount()&&	nCol> 0&&nCol<m_ListPosData.GetHeaderCtrl()->GetItemCount())  //第1列不可编辑
	{
		m_ListPosData.SetEditItem(nRow,nCol);
	}
//	SetFocus();
	*pResult = 0;
}


void CSysConfigureDataDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	OnClickListCmrdata(0,0);
	OnClickListPosdata(0,0);
	CDialogEx::OnLButtonDown(nFlags, point);
}
