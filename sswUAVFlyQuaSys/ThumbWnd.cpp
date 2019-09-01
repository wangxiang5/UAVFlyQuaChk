// ThumbWnd.cpp : 实现文件
//

#include "stdafx.h"
#include "sswUAVFlyQuaSys.h"
#include "ThumbWnd.h"
#include "MainFrm.h"
#include "sswUAVFlyQuaSysDoc.h"
#include "sswUAVFlyQuaSysView.h"
#include "MyFunctions.h"
#include "GradientProgressCtrl.h"
#include <omp.h>
// CThumbWnd
extern HGLRC g_pRC;
extern HGLRC g_pRCSharing;
extern CGradientProgressCtrl g_wndProgressCtrl;
IMPLEMENT_DYNAMIC(CThumbWnd, CDockablePane)

CThumbWnd::CThumbWnd()
{
	m_pDoc = NULL;
	m_bCreateTextures = false;
	m_bCreateAdjPtVBO = false;
	m_bDisplayAdjPtVBO = false;
	m_bSaveView2Img = false;
}

CThumbWnd::~CThumbWnd()
{
}


BEGIN_MESSAGE_MAP(CThumbWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(ID_MSG_CREATE_TEXTURES, OnMsgCreateImgTextures)
	ON_MESSAGE(ID_MSG_CREATE_ADJPTVBO, OnMsgCreateAdjPtsVBO)
	ON_MESSAGE(ID_MSG_FILL_REPRENDER, OnMsgFillRepVBO)
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_WM_NCCALCSIZE()
END_MESSAGE_MAP()



// CThumbWnd 消息处理程序


LRESULT CThumbWnd::OnMsgCreateImgTextures(WPARAM wParam, LPARAM lParam)
{
	vector<CsswUAVImage> vecImg = *(vector<CsswUAVImage>*)lParam;
	CMainFrame *pMain = (CMainFrame *)wParam;
	g_wndProgressCtrl.ShowWindow(true);
	bool bMakeCurrent = false;
	do { bMakeCurrent = wglMakeCurrent(m_pDC->GetSafeHdc(), g_pRC); } while (!bMakeCurrent);
	vector<Point3D> vPt3d(4);
	for (int i = 0; i<vecImg.size(); i++)
	{
		////////////////待修改
		if(!vecImg[i].m_bValid) continue;
		CsswImgRender imgRender;
		imgRender.CreateTexture(m_pDoc->m_vecImgInfo[i].m_strImgPath);
		imgRender.FillTexture();
		m_vecImgRender.push_back(imgRender);
		Point3D *pPt3d = vecImg[i].GetImgVertexPt3d4(0,false,false);

		for (int j = 0; j<4; j++)
		{
			vPt3d[i]=(*(pPt3d+i));
		}
		m_vecImgVertex4.push_back(vPt3d);

		//	g_wndProgressCtrl.SetPos(i*100/m_pDoc->m_nImgNum);
	}

	vector<Point3D>().swap(vPt3d);
	m_bCreateTextures = true;
	Invalidate();
	g_wndProgressCtrl.ShowWindow(false);
	return 1;
}
LRESULT CThumbWnd::OnMsgCreateAdjPtsVBO(WPARAM wParam, LPARAM lParam)
{
	CsswUAVFlyQuaSysDoc *pDoc = (CsswUAVFlyQuaSysDoc *)lParam;
	vector<SSWstuAdjPtInfo> vecAdjPts = pDoc->m_FlyQuaPrj.GetAllAdjPts();
	double *pColor = new double[vecAdjPts.size()*4];
	double *pVertex = new double[vecAdjPts.size()*2];
#pragma omp parallel for
	for (int i = 0; i<vecAdjPts.size(); i++)
	{
		*(pColor+4*i+0)=vecAdjPts[i].r*1.0/255;
		*(pColor+4*i+1)=vecAdjPts[i].g*1.0/255;
		*(pColor+4*i+2)=vecAdjPts[i].b*1.0/255;
		*(pColor+4*i+3)=1.0;

		*(pVertex+2*i+0)=vecAdjPts[i].X;
		*(pVertex+2*i+1)=vecAdjPts[i].Y;	
	}
	bool bMakeCurrent = false;
	do { bMakeCurrent = wglMakeCurrent(m_pDC->GetSafeHdc(), g_pRC); } while (!bMakeCurrent);
	m_AdjPtRender.CreateVectorVBO(vecAdjPts.size(),vPoint,true);
	m_AdjPtRender.FillVectorVBO(pVertex,pColor,vecAdjPts.size());

	double l,r,t,b;
	r=t=-1e10;
	l=b=1e10;
	for (int i = 0; i<vecAdjPts.size(); i++)
	{
		r=max(r,vecAdjPts[i].X);
		l=min(l,vecAdjPts[i].X);
		t=max(t,vecAdjPts[i].Y);
		b=min(b,vecAdjPts[i].Y);
	}
	m_RectAdjPtGeo = CRectD(l-100,t+100,r+100,b-100);
	m_RectCln2Geo = CRectD(l,t,r,b);
	m_ptClnCenter2Geo = m_RectCln2Geo.CenterPoint();
	m_fZoomRateCln2Geo = min(m_RectCln.Width()*1.0/m_RectAdjPtGeo.Width(),
		m_RectCln.Height()*1.0/m_RectAdjPtGeo.Height())*0.9;
	ResetRectCln2Geo();
	m_fZoomRateCln2GeoMin = m_fZoomRateCln2Geo/3;
	m_fZoomRateCln2GeoMax = m_fZoomRateCln2Geo*3;
	vector<SSWstuAdjPtInfo>().swap(vecAdjPts);
	delete[]pColor;
	delete[]pVertex;
	m_bCreateAdjPtVBO = true;
	wglMakeCurrent(NULL, NULL);
	return 1;
}
LRESULT CThumbWnd::OnMsgFillRepVBO(WPARAM wParam, LPARAM lParam)
{
	CsswUAVFlyQuaSysDoc* pDoc = (CsswUAVFlyQuaSysDoc*)lParam;
	CMainFrame *pMain = pDoc->m_pMain;
	SSWQUALITYTYPE eType = pMain->m_wndDataChart.m_eQualityType;
	int nPosNum = pDoc->m_nImgNum;
	double *pV = new double[2*nPosNum];
	double *pC = new double[4*nPosNum];
	for (int i = 0; i<nPosNum; i++)
	{
		*(pV+2*i+0)=pDoc->m_vecImgInfo[i].m_ImgPosInfo.Xs;
		*(pV+2*i+1)=pDoc->m_vecImgInfo[i].m_ImgPosInfo.Ys;
		if(!pDoc->m_vecImgInfo[i].m_bValid)  //无效影像
		{
			*(pC+4*i+0)=100;
			*(pC+4*i+1)=0;
			*(pC+4*i+2)=0;
			*(pC+4*i+3)=255.0;
		}
		else{
			*(pC+4*i+0)=0;
			*(pC+4*i+1)=100;
			*(pC+4*i+2)=0;
			*(pC+4*i+3)=255.0;
		}

	}
	switch(eType)
	{
	case Q_OVERLAP_Y:
		{
			vector<int> vImgIdx = pDoc->m_FlyQuaPrj.GetImgIdxLowOverlapInsStrips();
			for (int i = 0; i<vImgIdx.size(); i++)
			{
				int nImgIdx = vImgIdx[i];
				*(pC+4*nImgIdx+0)=0;
				*(pC+4*nImgIdx+1)=0;
				*(pC+4*nImgIdx+2)=100;
				*(pC+4*nImgIdx+3)=255;
			}
			vector<int>().swap(vImgIdx);
		}
		break;
	case Q_OVERLAP_X:
		{
			vector<int> vImgIdx = pDoc->m_FlyQuaPrj.GetImgIdxLowOverlapBetStrips();
			for (int i = 0; i<vImgIdx.size(); i++)
			{
				int nImgIdx = vImgIdx[i];
				*(pC+4*nImgIdx+0)=0;
				*(pC+4*nImgIdx+1)=0;
				*(pC+4*nImgIdx+2)=100;
				*(pC+4*nImgIdx+3)=255;
			}
			vector<int>().swap(vImgIdx);
		}
		break;
	case Q_POS_ERR:
		{
			vector<int> vImgIdx = pDoc->m_FlyQuaPrj.GetImgIdxHeiPosErr();
			for (int i = 0; i<vImgIdx.size(); i++)
			{
				int nImgIdx = vImgIdx[i];
				*(pC+4*nImgIdx+0)=0;
				*(pC+4*nImgIdx+1)=0;
				*(pC+4*nImgIdx+2)=100;
				*(pC+4*nImgIdx+3)=255;
			}
			vector<int>().swap(vImgIdx);
		}
		break;
	case Q_PITCH:
		{
			vector<int> vImgIdx = pDoc->m_FlyQuaPrj.GetImgIdxHeiPitch();
			for (int i = 0; i<vImgIdx.size(); i++)
			{
				int nImgIdx = vImgIdx[i];
				*(pC+4*nImgIdx+0)=0;
				*(pC+4*nImgIdx+1)=0;
				*(pC+4*nImgIdx+2)=100;
				*(pC+4*nImgIdx+3)=255;
			}
			vector<int>().swap(vImgIdx);
		}
		break;
	case Q_YAW:
		{
			vector<int> vImgIdx = pDoc->m_FlyQuaPrj.GetImgIdxHeiYaw();
			for (int i = 0; i<vImgIdx.size(); i++)
			{
				int nImgIdx = vImgIdx[i];
				*(pC+4*nImgIdx+0)=0;
				*(pC+4*nImgIdx+1)=0;
				*(pC+4*nImgIdx+2)=100;
				*(pC+4*nImgIdx+3)=255;
			}
			vector<int>().swap(vImgIdx);
		}
		break;
	case Q_POINT_NUM:
		{
			vector<int> vImgIdx = pDoc->m_FlyQuaPrj.GetImgIdxLowAdjPoint();
			for (int i = 0; i<vImgIdx.size(); i++)
			{
				int nImgIdx = vImgIdx[i];
				*(pC+4*nImgIdx+0)=0;
				*(pC+4*nImgIdx+1)=0;
				*(pC+4*nImgIdx+2)=100;
				*(pC+4*nImgIdx+3)=255;
			}
			vector<int>().swap(vImgIdx);
		}
		break;
	default:
		break;
	}
 	bool bMakeCurrent = false;
 	do { bMakeCurrent = wglMakeCurrent(m_pDC->GetSafeHdc(), g_pRC); }
	while (!bMakeCurrent);
	InitRepView();
	m_RepRender.FillVectorVBO(pV,pC,nPosNum);
	delete[]pV;
	delete[]pC;
	m_bCreateAdjPtVBO = true;
//	wglMakeCurrent(NULL, NULL);
	Invalidate();
	return 1;
}

void CThumbWnd::InitRepView()
{
	GetClientRect(m_RectCln);
	m_RectAdjPtGeo = GetViewHand()->m_RectPosGeo;
	m_ptClnCenter2Geo = m_RectAdjPtGeo.CenterPoint();
	m_fZoomRateCln2Geo = min(m_RectCln.Width()*1.0/m_RectAdjPtGeo.Width(),
		m_RectCln.Height()*1.0/m_RectAdjPtGeo.Height())*0.9;
	ResetRectCln2Geo();
	m_fZoomRateCln2GeoMin = m_fZoomRateCln2Geo/10;
	m_fZoomRateCln2GeoMax = m_fZoomRateCln2Geo*10;
	m_RepRender.CreateVectorVBO(GetDocHand()->m_nImgNum,vPoint,true);
}

int CThumbWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	m_pDC = new CClientDC(this);//初始化OPENGL设置定时器
	SetupPixelFormat(m_pDC, m_hPalette);
	InitOpenGL(g_pRC, m_pDC, m_hPalette);
//	wglMakeCurrent(m_pDC->GetSafeHdc(), m_pRC);
	CRect rectDummy;
	rectDummy.SetRectEmpty();
	DWORD dStyle;
// 	if (!m_ColorBtn1.Create("hhh",WS_CHILD | WS_VISIBLE ,rectDummy,this,1));
// 	{
// 		TRACE0("未能创建colorbutton1\n");
// 		return -1;      // 未能创建
// 	}
	return 0;
}


void CThumbWnd::OnSize(UINT nType, int cx, int cy)
{
	// TODO: 在此处添加消息处理程序代码
	CDockablePane::OnSize(nType, cx, cy);
	m_ColorBtn1.SetWindowPos (NULL, -1, -1, 50, 50, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	GetClientRect(m_RectCln);
	m_RectCln.bottom-=30;
	ResetRectCln2Geo();
}

Point2D CThumbWnd::ptCln2Geo(CPoint& ptCln)
{
	Point2D ptGeo;
	ptGeo.x = m_ptClnCenter2Geo.x + 1.0*(ptCln.x - m_RectCln.CenterPoint().x) / m_fZoomRateCln2Geo;
	ptGeo.y = m_ptClnCenter2Geo.y - 1.0*(ptCln.y - m_RectCln.CenterPoint().y) / m_fZoomRateCln2Geo;
	return ptGeo;
}
CPoint CThumbWnd::ptGeo2Cln(Point2D& ptGeo)
{
	CPoint ptCln;
	ptCln.x = (ptGeo.x - m_ptClnCenter2Geo.x) * m_fZoomRateCln2Geo + m_RectCln.CenterPoint().x;
	ptCln.y = (m_ptClnCenter2Geo.y - ptGeo.y) * m_fZoomRateCln2Geo + m_RectCln.CenterPoint().y;
	return ptCln;
}
Point2D CThumbWnd::ClnCenter2Geo(CPoint ptCln, Point2D ptGeo)
{
	Point2D ptClnCenter2Geo;
	ptClnCenter2Geo.x = ptGeo.x - 1.0*(ptCln.x - m_RectCln.CenterPoint().x) / m_fZoomRateCln2Geo;
	ptClnCenter2Geo.y = ptGeo.y + 1.0*(ptCln.y - m_RectCln.CenterPoint().y) / m_fZoomRateCln2Geo;
	return ptClnCenter2Geo;
}
void CThumbWnd::ResetRectCln2Geo()
{
	m_RectCln2Geo = CRectD(ptCln2Geo(m_RectCln.TopLeft()),ptCln2Geo(m_RectCln.BottomRight()));
}


BOOL CThumbWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

//	return CDockablePane::OnEraseBkgnd(pDC);
 	if (GetDocHand() == NULL) return CDockablePane::OnEraseBkgnd(pDC);
// 	if (!GetDocHand()->m_bPrjLoaded) return CDockablePane::OnEraseBkgnd(pDC);
 	bool bMakeCurrent = false;
 	do { bMakeCurrent = wglMakeCurrent(m_pDC->GetSafeHdc(), g_pRC); } while (!bMakeCurrent);
 	PrepareDraw(1,1,1, 1);
	glViewport(0, 0, m_RectCln.Width(), m_RectCln.Height());
 	if(!m_bCreateAdjPtVBO/*||!m_bDisplayAdjPtVBO*/)
 	{
// 		glColor3f(1,0,0);
// 		glLoadIdentity();    // 重置当前指定的矩阵为单位矩阵
// 		glOrtho(m_RectCln.left, m_RectCln.right, m_RectCln.bottom, m_RectCln.top, -1, 1);
// 		glRasterPos2f(m_RectCln.left+10,m_RectCln.top+20);
// 		DisplayString("无有效信息!");
 	}
// 	else if(m_bDisplayAdjPtVBO)
// 	{
// 		glLoadIdentity();    // 重置当前指定的矩阵为单位矩阵
// 		glOrtho(m_RectCln2Geo.left, m_RectCln2Geo.right, m_RectCln2Geo.bottom, m_RectCln2Geo.top, -1, 1);
// 		m_AdjPtRender.Display(2);
// 		if(m_bCreateTextures)  //显示影像纹理
// 		{
// 		}
// 	}
	else
	{
		glLoadIdentity();    // 重置当前指定的矩阵为单位矩阵
		glOrtho(m_RectCln2Geo.left, m_RectCln2Geo.right, m_RectCln2Geo.bottom, m_RectCln2Geo.top, -1, 1);
		m_RepRender.Display(5);
	}
 	EndDraw(m_pDC);
	if(m_bSaveView2Img)
	{
		CString strImage=FunGetFileFolder(m_pDoc->m_strPrjFilePath)+"\\source";
		CreateDirectory(strImage,NULL);
		int nCur = GetMainFramHand()->m_wndDataChart.m_ComboDataExtend.GetCurSel();
		switch(nCur)
		{
		case 0:
			strImage+="\\OverlapYErrView.jpg";
			GetViewHand()->m_strOverlapYErrViewImagePath = strImage;
			break;
		case 1:
			strImage+="\\OverlapXErrView.jpg";
			GetViewHand()->m_strOverlapXErrViewImagePath = strImage;
			break;
		case 2:
			strImage+="\\PosErrView.jpg";
			GetViewHand()->m_strPosErrViewImagePath = strImage;
			break;
		case 3:
			strImage+="\\PitchErrView.jpg";
			GetViewHand()->m_strPitchErrViewImagePath = strImage;
			break;
		case 4:
			strImage+="\\YawErrView.jpg";
			GetViewHand()->m_strYawErrViewImagePath = strImage;
			break;
		case 5:
			strImage+="\\PtNumErrView.jpg";
			GetViewHand()->m_strPtNumErrViewImagePath = strImage;
			break;
		default:
			break;
		}
		CPoint pt1 = ptGeo2Cln(GetViewHand()->m_RectPosGeo.LeftTop());
		CPoint pt2 = ptGeo2Cln(GetViewHand()->m_RectPosGeo.RightBottom());
		DeleteFile(strImage);
		SaveView2Img(CRect(pt1,pt2),strImage);
		while(!PathFileExists(strImage))
			SaveView2Img(CRect(pt1,pt2),strImage);
		m_bSaveView2Img = false;
	}
	return TRUE;
}
void CThumbWnd::SaveView2Img(CRect RectSaveView, CString& strSavePath)
{
	int l,t,w,h;
	l = max(RectSaveView.left-10,0);
	t = max(m_RectCln.Height()-RectSaveView.bottom-10,0);   //opengl窗口左下角为（0，0）
	w = min(RectSaveView.Width()+20,m_RectCln.Width()-l);
	h = min(RectSaveView.Height()+20,m_RectCln.Height()-t);	
	BYTE* pMem = new BYTE[w*h*3]; memset(pMem,0,w*h*3);
	BYTE* pData = new BYTE[w*h*3];memset(pData,0,w*h*3);
	glReadBuffer(GL_FRONT);
	glPixelStorei(GL_PACK_ALIGNMENT,1);
	glReadPixels(l,t,w,h, GL_RGB, GL_UNSIGNED_BYTE, pMem); //l,t相对窗口左下角
#pragma omp parallel for
	for (int i = 0; i<h; i++)
	{
		for (int j = 0; j<w; j++)
		{
			for (int b = 0; b<3; b++)
			{
				*(pData+(h-i-1)*w*3+j*3+b)=*(pMem+i*w*3+j*3+b);
			}
		}
	}
	CsswImage sswImg;
	sswImg.SaveImg(strSavePath,w,h,3,pData);
	if(pMem) delete[] pMem;
	if(pData) delete[] pData;
	pMem=NULL;
	pData=NULL;


}

BOOL CThumbWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if(!m_bCreateAdjPtVBO) return CDockablePane::OnMouseWheel(nFlags, zDelta, pt);
	ScreenToClient(&pt);
	Point2D ptGeo =ptCln2Geo(pt);
	m_fZoomRateCln2Geo *= pow(1.15, 1.0*zDelta / 120);
	m_fZoomRateCln2Geo = min(m_fZoomRateCln2Geo, m_fZoomRateCln2GeoMax);
	m_fZoomRateCln2Geo = max(m_fZoomRateCln2Geo, m_fZoomRateCln2GeoMin);
	m_ptClnCenter2Geo = ClnCenter2Geo(pt, ptGeo);
	ResetRectCln2Geo();
	Invalidate();
	return CDockablePane::OnMouseWheel(nFlags, zDelta, pt);
}


void CThumbWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(!m_bCreateAdjPtVBO) return;
	if((nFlags&MK_MBUTTON)==MK_MBUTTON)
	{
		m_ptClnCenter2Geo = ClnCenter2Geo( point, m_ptMBtnDnGeo);
		ResetRectCln2Geo();
		Invalidate();
	}
	CDockablePane::OnMouseMove(nFlags, point);
}


void CThumbWnd::OnMButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(!m_bCreateAdjPtVBO) return;
	SetCapture();
	m_ptMBtnDnGeo = ptCln2Geo(point);
	CDockablePane::OnMButtonDown(nFlags, point);
}


void CThumbWnd::OnMButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ReleaseCapture();
	CDockablePane::OnMButtonUp(nFlags, point);
}


void CThumbWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	// TODO: 在此处添加消息处理程序代码
}


void CThumbWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	// TODO: Add your message handler code here and/or call default

	CDockablePane::OnNcCalcSize(bCalcValidRects, lpncsp);
	lpncsp->rgrc[0].bottom += 30;
}
