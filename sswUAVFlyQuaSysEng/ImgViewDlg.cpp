// ImgViewDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "sswUAVFlyQuaSysEng.h"
#include "ImgViewDlg.h"
#include "afxdialogex.h"
#include "MyFunctions.h"
#include "MainFrm.h"
#include "sswUAVFlyQuaSysEngDoc.h"
#include "sswUAVFlyQuaSysEngView.h"

// CImgViewDlg 对话框
extern HGLRC g_pRC;
IMPLEMENT_DYNAMIC(CImgViewDlg, CDialogEx)

	CImgViewDlg::CImgViewDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CImgViewDlg::IDD, pParent)
{
	m_nImgIdx2Show = InvalidValue;
	m_nShowImgStripIdx = InvalidValue;
	m_nShowImgIdxInStrip = InvalidValue;
	m_bTextureReady = false;
}

CImgViewDlg::~CImgViewDlg()
{
	m_ImgRender.DeleteTexture();
}

void CImgViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CImgViewDlg, CDialogEx)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEWHEEL()
	ON_WM_MBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MBUTTONUP()
END_MESSAGE_MAP()


// CImgViewDlg 消息处理程序
Point2D CImgViewDlg::ptCln2Geo(CPoint& ptCln)
{
	Point2D ptGeo;
	ptGeo.x = m_ptClnCenter2Geo.x + 1.0*(ptCln.x - m_RectCln.CenterPoint().x) / m_fZoomRateCln2Geo;
	ptGeo.y = m_ptClnCenter2Geo.y - 1.0*(ptCln.y - m_RectCln.CenterPoint().y) / m_fZoomRateCln2Geo;
	return ptGeo;
}
CPoint CImgViewDlg::ptGeo2Cln(Point2D& ptGeo)
{
	CPoint ptCln;
	ptCln.x = (ptGeo.x - m_ptClnCenter2Geo.x) * m_fZoomRateCln2Geo + m_RectCln.CenterPoint().x;
	ptCln.y = (m_ptClnCenter2Geo.y - ptGeo.y) * m_fZoomRateCln2Geo + m_RectCln.CenterPoint().y;
	return ptCln;
}
Point2D CImgViewDlg::ClnCenter2Geo(CPoint ptCln, Point2D ptGeo)
{
	Point2D ptClnCenter2Geo;
	ptClnCenter2Geo.x = ptGeo.x - 1.0*(ptCln.x - m_RectCln.CenterPoint().x) / m_fZoomRateCln2Geo;
	ptClnCenter2Geo.y = ptGeo.y + 1.0*(ptCln.y - m_RectCln.CenterPoint().y) / m_fZoomRateCln2Geo;
	return ptClnCenter2Geo;
}
void CImgViewDlg::ResetRectCln2Geo()
{
	m_RectCln2Geo = CRectD(ptCln2Geo(m_RectCln.TopLeft()),ptCln2Geo(m_RectCln.BottomRight()));
}

void CImgViewDlg::InitImgViewInfo()
{
	if(m_nImgIdx2Show==InvalidValue) return;
	CsswUAVFlyQuaSysEngDoc *pDoc = GetDocHand();
	CsswUAVImage uImg; 
	uImg.OpenImg(pDoc->m_vecImgInfo[m_nImgIdx2Show].m_strImgPath);
	int nCols = uImg.GetCols();
	int nRows = uImg.GetRows();
	m_RectImg = CRectD(0,nRows,nCols,0);

	m_ptClnCenter2Geo = m_RectImg.CenterPoint();
	m_fZoomRateCln2Geo = min(m_RectCln.Width()*1.0/m_RectImg.Width(),
		m_RectCln.Height()*1.0/m_RectImg.Height())*0.9;
	ResetRectCln2Geo();
	m_fZoomRateCln2GeoMin = m_fZoomRateCln2Geo/10;
	m_fZoomRateCln2GeoMax = m_fZoomRateCln2Geo*10;
	uImg.CloseImg();
}
BOOL CImgViewDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CsswUAVFlyQuaSysEngDoc *pDoc = GetDocHand();
	m_nImgIdx2Show = pDoc->m_pMain->m_wndStripImg.m_nSelectedImgIdx;
	if(m_nImgIdx2Show==InvalidValue) return TRUE;
	m_nShowImgStripIdx = pDoc->m_vecImgInfo[m_nImgIdx2Show].m_nStripIdx;
	m_nShowImgIdxInStrip = pDoc->m_vecImgInfo[m_nImgIdx2Show].m_nIdxInStrip;
	SetWindowText("Image..."+pDoc->m_vecImgInfo[m_nImgIdx2Show].m_strImgName);
	CenterWindow();
	GetClientRect(m_RectCln);
	InitImgViewInfo();
	m_ImgRender.CreateTexture(GetDocHand()->m_vecImgInfo[m_nImgIdx2Show].m_strImgPath);
	m_ImgRender.FillTexture();
	m_bTextureReady = true;
	Invalidate();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


int CImgViewDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_pDC = new CClientDC(this);//初始化OPENGL设置定时器
	SetupPixelFormat(m_pDC, m_hPalette);
	InitOpenGL(g_pRC, m_pDC, m_hPalette);
	// TODO:  在此添加您专用的创建代码
	return 0;
}


BOOL CImgViewDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if(!m_bTextureReady) return CDialogEx::OnEraseBkgnd(pDC);
	else
	{
		bool bMakeCurrent = false;
		do { bMakeCurrent = wglMakeCurrent(m_pDC->GetSafeHdc(), g_pRC); } while (!bMakeCurrent);
		PrepareDraw(0,0,0, 0);
		glViewport(0, 0, m_RectCln.Width(), m_RectCln.Height());
		glLoadIdentity(); 
		glOrtho(m_RectCln.left, m_RectCln.right, m_RectCln.bottom, m_RectCln.top, -1, 1);
		CRect RectImg2Cln = CRect(ptGeo2Cln(m_RectImg.LeftTop()),ptGeo2Cln(m_RectImg.RightBottom()));
		m_ImgRender.DisplayTexture(&RectImg2Cln);
		EndDraw(m_pDC);
	//	wglMakeCurrent(NULL,NULL);
	//	glColor3f(1,1,1);
		return TRUE;
	}
}


BOOL CImgViewDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ScreenToClient(&pt);
	Point2D ptGeo =ptCln2Geo(pt);
	m_fZoomRateCln2Geo *= pow(1.15, 1.0*zDelta / 120);
	m_fZoomRateCln2Geo = min(m_fZoomRateCln2Geo, m_fZoomRateCln2GeoMax);
	m_fZoomRateCln2Geo = max(m_fZoomRateCln2Geo, m_fZoomRateCln2GeoMin);
	m_ptClnCenter2Geo = ClnCenter2Geo(pt, ptGeo);
	ResetRectCln2Geo();
	Invalidate();
	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}

void CImgViewDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if((nFlags&MK_MBUTTON)==MK_MBUTTON)
	{
		m_ptClnCenter2Geo = ClnCenter2Geo( point, m_ptMBtnDnGeo);
		ResetRectCln2Geo();
		Invalidate();
	}
	CDialogEx::OnMouseMove(nFlags, point);
}

void CImgViewDlg::OnMButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	SetCapture();
	m_ptMBtnDnGeo = ptCln2Geo(point);
	CDialogEx::OnMButtonDown(nFlags, point);
}




void CImgViewDlg::OnMButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ReleaseCapture();
	CDialogEx::OnMButtonUp(nFlags, point);
}


BOOL CImgViewDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	CsswUAVFlyQuaSysEngDoc *pDoc = GetDocHand();
	switch(pMsg->message)
	{
	case WM_KEYDOWN:{
		switch(pMsg->wParam)
		{
		case VK_LEFT:
			if(m_nShowImgStripIdx>=0)
			{
				m_nShowImgIdxInStrip--;
				m_nShowImgIdxInStrip = max(0,m_nShowImgIdxInStrip);
				m_nImgIdx2Show = pDoc->m_FlyQuaPrj.GetImgIdx(m_nShowImgStripIdx,m_nShowImgIdxInStrip);
			}else
			{
				for (int i = 0; i<pDoc->m_FlyQuaPrj.GetInvalidImgNum(); i++)
				{
					int nIdx  = pDoc->m_FlyQuaPrj.GetInvalidImgIdx(i);
					if(nIdx==m_nImgIdx2Show)
					{
						i--;
						i=max(0,i);
						m_nImgIdx2Show = pDoc->m_FlyQuaPrj.GetInvalidImgIdx(i);
						break;
					}
				}

			}
			break;
		case VK_RIGHT:
			if(m_nShowImgStripIdx>=0)
			{
				m_nShowImgIdxInStrip++;
				m_nShowImgIdxInStrip = min(m_nShowImgIdxInStrip,pDoc->m_FlyQuaPrj.GetImgNumInStrip(m_nShowImgStripIdx)-1);
				m_nImgIdx2Show = pDoc->m_FlyQuaPrj.GetImgIdx(m_nShowImgStripIdx,m_nShowImgIdxInStrip);
			}else
			{
				for (int i = 0; i<pDoc->m_FlyQuaPrj.GetInvalidImgNum(); i++)
				{
					int nIdx  = pDoc->m_FlyQuaPrj.GetInvalidImgIdx(i);
					if(nIdx==m_nImgIdx2Show)
					{
						i++;
						i=min(i,pDoc->m_FlyQuaPrj.GetInvalidImgNum());
						m_nImgIdx2Show = pDoc->m_FlyQuaPrj.GetInvalidImgIdx(i);
						break;
					}
				}
			}
			break;
		}
		SetWindowText("Image..."+pDoc->m_vecImgInfo[m_nImgIdx2Show].m_strImgName);
//		InitImgViewInfo();
		m_ImgRender.DeleteTexture();
		m_ImgRender.CreateTexture(pDoc->m_vecImgInfo[m_nImgIdx2Show].m_strImgPath);
		m_ImgRender.FillTexture();
		m_bTextureReady = true;
		Invalidate();
					}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
