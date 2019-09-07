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

// sswUAVFlyQuaSysView.cpp : CsswUAVFlyQuaSysView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "sswUAVFlyQuaSys.h"
#endif
#include "MainFrm.h"
#include "sswUAVFlyQuaSysDoc.h"
#include "sswUAVFlyQuaSysView.h"
#include "MyFunctions.h"
#include "MyMultiThread.h"
#include <omp.h>
#include "GradientProgressCtrl.h"
#include "conio.h"
#include "NodeManager.h"

//#include "StripSorting.h"
//#pragma comment(lib,"StripSorting.lib") stripsorting测试

//#include "sswViewerRender.h";
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
extern HGLRC g_pRC;
extern HGLRC g_pRCSharing;
extern PFNGLGENBUFFERSARBPROC glGenBuffersARB;   // VBO Name Generation Procedure
extern PFNGLBINDBUFFERARBPROC glBindBufferARB;    // VBO Bind Procedure
extern PFNGLBUFFERDATAARBPROC glBufferDataARB;    // VBO Data Loading Procedure
extern PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;  // VBO Deletion Procedure
//extern CGradientProgressCtrl g_wndProgressCtrl;
// CsswUAVFlyQuaSysView

IMPLEMENT_DYNCREATE(CsswUAVFlyQuaSysView, CView)

BEGIN_MESSAGE_MAP(CsswUAVFlyQuaSysView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CsswUAVFlyQuaSysView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_COMMAND(ID_CHECK_VIEW_IMG_NAME, &CsswUAVFlyQuaSysView::OnCheckViewImgName)
	ON_UPDATE_COMMAND_UI(ID_CHECK_VIEW_IMG_NAME, &CsswUAVFlyQuaSysView::OnUpdateCheckViewImgName)
	ON_COMMAND(ID_CHECK_VIEW_IMG_POS, &CsswUAVFlyQuaSysView::OnCheckViewImgPos)
	ON_UPDATE_COMMAND_UI(ID_CHECK_VIEW_IMG_POS, &CsswUAVFlyQuaSysView::OnUpdateCheckViewImgPos)
	ON_COMMAND(ID_BUTTON_SORT_STRIP, &CsswUAVFlyQuaSysView::OnButtonSortStrip)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SORT_STRIP, &CsswUAVFlyQuaSysView::OnUpdateButtonSortStrip)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_CHECK_VIEW_STRIP_LABEL, &CsswUAVFlyQuaSysView::OnCheckViewStripLabel)
	ON_UPDATE_COMMAND_UI(ID_CHECK_VIEW_STRIP_LABEL, &CsswUAVFlyQuaSysView::OnUpdateCheckViewStripLabel)
	ON_WM_RBUTTONDOWN()
	//ON_COMMAND(ID_BUTTON_SAVE_VIEW, &CsswUAVFlyQuaSysView::OnButtonSaveView)
	ON_COMMAND(ID_BUTTON_BYHAND_RESORT_STRIP, &CsswUAVFlyQuaSysView::OnButtonByhandResortStrip)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_BYHAND_RESORT_STRIP, &CsswUAVFlyQuaSysView::OnUpdateButtonByhandResortStrip)
	ON_COMMAND(ID_BUTTON_SEMIAUTO_RESORT_STRIP, &CsswUAVFlyQuaSysView::OnButtonSemiautoResortStrip)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SEMIAUTO_RESORT_STRIP, &CsswUAVFlyQuaSysView::OnUpdateButtonSemiautoResortStrip)
	ON_MESSAGE(ID_MSG_RESORT_STRIPS, OnMsgReSortStrips)
	ON_MESSAGE(ID_MSG_SET_IMGVALID_STATE, OnMsgSetImgInvalid)
	ON_MESSAGE(ID_MSG_CREATE_MISSRENDER, onMsgCreateMissAreaRender)
	ON_COMMAND(ID_CHECK_VIEW_CQ_AREA, &CsswUAVFlyQuaSysView::OnCheckViewCqArea)
	ON_UPDATE_COMMAND_UI(ID_CHECK_VIEW_CQ_AREA, &CsswUAVFlyQuaSysView::OnUpdateCheckViewCqArea)
	ON_COMMAND(ID_CHECK_VIEW_IMG_AREA, &CsswUAVFlyQuaSysView::OnCheckViewImgArea)
	ON_UPDATE_COMMAND_UI(ID_CHECK_VIEW_IMG_AREA, &CsswUAVFlyQuaSysView::OnUpdateCheckViewImgArea)
	ON_COMMAND(ID_CHECK_VIEW_MISS_AREA, &CsswUAVFlyQuaSysView::OnCheckViewMissArea)
	ON_UPDATE_COMMAND_UI(ID_CHECK_VIEW_MISS_AREA, &CsswUAVFlyQuaSysView::OnUpdateCheckViewMissArea)
	ON_COMMAND(ID_BUTTON_SET_IMAGE_VALID_STATE, &CsswUAVFlyQuaSysView::OnButtonSetImgValidState)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SET_IMAGE_VALID_STATE, &CsswUAVFlyQuaSysView::OnUpdateButtonSetInvalidImage)
	ON_COMMAND(ID_BUTTON_SET_INVALID, &CsswUAVFlyQuaSysView::OnButtonSetInvalid)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SET_INVALID, &CsswUAVFlyQuaSysView::OnUpdateButtonSetInvalid)
	ON_COMMAND(ID_BUTTON_SET_VALID, &CsswUAVFlyQuaSysView::OnButtonSetValid)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_SET_VALID, &CsswUAVFlyQuaSysView::OnUpdateButtonSetValid)
	ON_WM_PAINT()
	ON_MESSAGE(ID_MSG_CREATE_SELECTRENDER,&CsswUAVFlyQuaSysView::onMseCreateSelectedRender)
END_MESSAGE_MAP()

// CsswUAVFlyQuaSysView 构造/析构

CsswUAVFlyQuaSysView::CsswUAVFlyQuaSysView()
{
	// TODO: 在此处添加构造代码
	m_pRC=NULL;
	m_pDoc = NULL;
	m_bViewImgPos = true;
	m_bViewImgName=false;
	m_bViewCqArea = false;
	m_bViewImgArea = false;
	m_bViewMissArea = false;
	m_bDisplayCurLine = false;
	m_bActStripResort=false;
	m_bResortStripByHand = false;
	m_bResortStripSemiAuto = true;
	m_bStripResorted=false;
	m_bViewStripLabel=true;
	m_bSaveView2Img=false;
	m_bActSetInvalidImg = false;
	m_bSetImg2Invalid = false;
	m_bSetImg2Valid = true;
	m_pData=NULL;
	m_nStripIdxSelected = InvalidValue;
	m_nNodeLevel = 4;
	m_nKeyNodeNum = m_nNodeLevel * m_nNodeLevel - m_nNodeLevel + 1;
}

CsswUAVFlyQuaSysView::~CsswUAVFlyQuaSysView()
{
	vector<Point2D>().swap(m_vecCurPoint);
}

BOOL CsswUAVFlyQuaSysView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CsswUAVFlyQuaSysView 绘制

void CsswUAVFlyQuaSysView::OnDraw(CDC* /*pDC*/)
{
	CsswUAVFlyQuaSysDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}


// CsswUAVFlyQuaSysView 打印


void CsswUAVFlyQuaSysView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CsswUAVFlyQuaSysView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CsswUAVFlyQuaSysView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CsswUAVFlyQuaSysView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CsswUAVFlyQuaSysView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
// 	ClientToScreen(&point);
// 	OnContextMenu(this, point);
}

void CsswUAVFlyQuaSysView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CsswUAVFlyQuaSysView 诊断

#ifdef _DEBUG
void CsswUAVFlyQuaSysView::AssertValid() const
{
	CView::AssertValid();
}

void CsswUAVFlyQuaSysView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CsswUAVFlyQuaSysDoc* CsswUAVFlyQuaSysView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CsswUAVFlyQuaSysDoc)));
	return (CsswUAVFlyQuaSysDoc*)m_pDocument;
}
#endif //_DEBUG


// CsswUAVFlyQuaSysView 消息处理程序
Point2D CsswUAVFlyQuaSysView::ptCln2Geo(CPoint& ptCln)
{
	Point2D ptGeo;
	ptGeo.x = m_ptClnCenter2Geo.x + 1.0*(ptCln.x - m_RectCln.CenterPoint().x) / m_fZoomRateCln2Geo;
	ptGeo.y = m_ptClnCenter2Geo.y - 1.0*(ptCln.y - m_RectCln.CenterPoint().y) / m_fZoomRateCln2Geo;
	return ptGeo;
}
CPoint CsswUAVFlyQuaSysView::ptGeo2Cln(Point2D& ptGeo)
{
	CPoint ptCln;
	ptCln.x = (ptGeo.x - m_ptClnCenter2Geo.x) * m_fZoomRateCln2Geo + m_RectCln.CenterPoint().x;
	ptCln.y = (m_ptClnCenter2Geo.y - ptGeo.y) * m_fZoomRateCln2Geo + m_RectCln.CenterPoint().y;
	return ptCln;
}
Point2D CsswUAVFlyQuaSysView::ClnCenter2Geo(CPoint ptCln, Point2D ptGeo)
{
	Point2D ptClnCenter2Geo;
	ptClnCenter2Geo.x = ptGeo.x - 1.0*(ptCln.x - m_RectCln.CenterPoint().x) / m_fZoomRateCln2Geo;
	ptClnCenter2Geo.y = ptGeo.y + 1.0*(ptCln.y - m_RectCln.CenterPoint().y) / m_fZoomRateCln2Geo;
	return ptClnCenter2Geo;
}
void CsswUAVFlyQuaSysView::ResetRectCln2Geo()
{
	m_RectCln2Geo = CRectD(ptCln2Geo(m_RectCln.TopLeft()),ptCln2Geo(m_RectCln.BottomRight()));
}
void CsswUAVFlyQuaSysView::ReInitCqView()
{
// 	m_ptClnCenter2Geo = m_RectPosGeo.CenterPoint();
// 	m_fZoomRateCln2Geo = min(m_RectCln.Width()*1.0/m_RectPosGeo.Width(),
// 		m_RectCln.Height()*1.0/m_RectPosGeo.Height())*0.7;
	GetClientRect(m_RectCln);
 	m_ptClnCenter2Geo = m_RectCqViewGeo.CenterPoint();
	m_fZoomRateCln2Geo = min(m_RectCln.Width()*1.0/m_RectCqViewGeo.Width(),
		m_RectCln.Height()*1.0/m_RectCqViewGeo.Height())*0.7;
	m_fZoomRateCln2GeoMin = m_fZoomRateCln2Geo/20;
	m_fZoomRateCln2GeoMax = m_fZoomRateCln2Geo*20;
}
void CsswUAVFlyQuaSysView::SaveView2Img(CString& strSavePath)
{
	CString strImage=FunGetFileFolder(m_pDoc->m_strPrjFilePath)+"\\source";
	CreateDirectory(strImage,NULL);
	m_strCqViewImagePath=strImage+"\\CqView.jpg";	
	CPoint pt1 = ptGeo2Cln(m_RectCqViewGeo.LeftTop());
	CPoint pt2 = ptGeo2Cln(m_RectCqViewGeo.RightBottom());
	CRect RectSaveView = CRect(pt1,pt2);
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
// struct stuPosInfo
// {
// 	double Xs,Ys,Zs;
// 	double phi,omg,kap;
// 	int nStripIdx;   //航带编号
// 	int nIdxInStrip; //航带内
// };
//void strip_sorting(double line_a, double line_b, double line_c,vector<stuPosInfo>& vecPosInfo)
//{
//	double sinStripDirKap = line_b/sqrt(pow(line_a,2)+pow(line_b,2));
//	double cosStripDirKap = line_a/sqrt(pow(line_a,2)+pow(line_b,2));
//
//	vector<double> vecLocXs(vecPosInfo.size(),0);
//	vector<double> vecLocYs(vecPosInfo.size(),0);                   
//	vector<int> vecLocXYIdx2ImgIdx(vecPosInfo.size(),0);
//	vector<int> vecImgStripIdx(vecPosInfo.size(),InvalidValue);
//
//	//1. 计算局部坐标（坐标轴与航带平行-垂直）
//	int nCount = 0;
//	for (int i = 0; i<vecPosInfo.size(); i++)
//	{
//		vecLocXs[nCount] = cosStripDirKap*vecPosInfo[i].Xs+sinStripDirKap*vecPosInfo[i].Ys;
//		vecLocYs[nCount] = cosStripDirKap*vecPosInfo[i].Ys-sinStripDirKap*vecPosInfo[i].Xs;
//		vecLocXYIdx2ImgIdx[nCount]=i;
//		nCount++;
//	}
//	//2. 按照局部坐标Xs排序-降序
//	for (int i= 0; i<vecPosInfo.size()-1; i++)
//	{
//		for (int j = i+1; j<vecPosInfo.size(); j++)
//		{
//			if(vecLocXs[i]>vecLocXs[j])
//			{
//				swap(vecLocXYIdx2ImgIdx[i],vecLocXYIdx2ImgIdx[j]);
//				swap(vecLocXs[i],vecLocXs[j]);
//				swap(vecLocYs[i],vecLocYs[j]);
//			}
//		}	
//	}
//
//	//3.计算排列航带距离阈值-dMinDis
//	double dMinDis = 1e10;
//	int n1=-1,n2=-1;
//	for (int i = 0; i<vecPosInfo.size(); i++)
//	{
//		for (int j = 0; j<vecPosInfo.size(); j++)
//		{
//			if(j==i) continue;
//			if(dMinDis>sqrt(pow(vecLocXs[i]-vecLocXs[j],2)+pow(vecLocYs[i]-vecLocYs[j],2))){
//				n1=i;n2=j;
//				dMinDis=min(dMinDis,sqrt(pow(vecLocXs[i]-vecLocXs[j],2)+pow(vecLocYs[i]-vecLocYs[j],2)));
//			}
//		}
//	}
//	//4.排列航带
//	int nStripIdx = 0, nImgNumInStrip = 0, nStripNum; 
//	double dStripStartX = InvalidValue; 
//	vector<int> vecImgNumInEachStrip, vecStripIdx;
//	vector<vector<int>> vvImgIdxInStrips;
//	for (int i = 0; i<vecPosInfo.size(); i++)
//	{
//		if(dStripStartX==InvalidValue)
//		{
//			dStripStartX=vecLocXs[i];
//			vecImgStripIdx[vecLocXYIdx2ImgIdx[i]]=nStripIdx;
//			nImgNumInStrip++;
//			continue;
//		}
//		double dis = fabs(dStripStartX-vecLocXs[i]);
//		if(dis<=dMinDis)
//		{
//			dStripStartX=vecLocXs[i];
//			vecImgStripIdx[vecLocXYIdx2ImgIdx[i]]=nStripIdx;
//			nImgNumInStrip++;
//		}
//		else
//		{
//			vecImgNumInEachStrip.push_back(nImgNumInStrip);
//			vecStripIdx.push_back(nStripIdx);
//			nStripIdx++;
//			nImgNumInStrip=0;
//			dStripStartX=vecLocXs[i];
//			vecImgStripIdx[vecLocXYIdx2ImgIdx[i]]=nStripIdx;
//			nImgNumInStrip++;
//		}
//	}
//	if(nImgNumInStrip>0) 
//	{
//		vecImgNumInEachStrip.push_back(nImgNumInStrip);
//		vecStripIdx.push_back(nStripIdx);
//	}
//	//5.航带内排序
//	for (int i = 0; i<vecPosInfo.size()-1; i++)
//	{
//		for (int j = i+1; j<vecPosInfo.size(); j++)
//		{
//			if(vecImgStripIdx[vecLocXYIdx2ImgIdx[i]]>vecImgStripIdx[vecLocXYIdx2ImgIdx[j]])
//			{
//				swap(vecLocXYIdx2ImgIdx[i],vecLocXYIdx2ImgIdx[j]);
//				swap(vecLocXs[i],vecLocXs[j]);
//				swap(vecLocYs[i],vecLocYs[j]);
//			}
//		}	
//	}
//	nCount = 0;
//	for (int i = 0; i<vecImgNumInEachStrip.size(); i++)
//	{
//		for (int j = 0; j<vecImgNumInEachStrip[i]-1; j++)
//		{
//			for (int k = j+1; k<vecImgNumInEachStrip[i]; k++)
//			{
//				int nIdx1 = j+nCount;
//				int nIdx2 = k+nCount;
//				if(vecLocYs[nIdx1]>vecLocYs[nIdx2])
//				{
//					swap(vecLocXYIdx2ImgIdx[nIdx1],vecLocXYIdx2ImgIdx[nIdx2]);
//					swap(vecLocXs[nIdx1],vecLocXs[nIdx2]);
//					swap(vecLocYs[nIdx1],vecLocYs[nIdx2]);
//				}
//			}
//		}
//		nCount+=vecImgNumInEachStrip[i];
//	}	
//	nCount = 0;
//	nStripNum = vecImgNumInEachStrip.size();
//	vvImgIdxInStrips.resize(nStripNum);
//	nCount = 0;
//	for (int i = 0; i<nStripNum; i++)
//	{		vector<int> vecImgIdx(vecImgNumInEachStrip[i]);
//
//	for (int j = 0; j<vecImgNumInEachStrip[i]; j++)
//	{
//		int nIdx = vecLocXYIdx2ImgIdx[nCount+j];
//		vecPosInfo[nIdx].nIdxInStrip=j;
//		vecPosInfo[nIdx].nStripIdx=i;
//		vecImgIdx[j]=nIdx;
//	}
//	nCount+=vecImgNumInEachStrip[i];
//	vvImgIdxInStrips[i]=vecImgIdx;	
//	vector<int>().swap(vecImgIdx);
//	}
//	vector<double>().swap(vecLocXs);
//	vector<double>().swap(vecLocYs);
//	vector<int>().swap(vecImgStripIdx);
//	vector<int>().swap(vecLocXYIdx2ImgIdx);
//	vector<int>().swap(vecImgNumInEachStrip);
//	vector<int>().swap(vecStripIdx);
//	for (int i = 0; i<vvImgIdxInStrips.size(); i++)
//	{
//		vector<int>().swap(vvImgIdxInStrips[i]);
//	}
//	vector<vector<int>>().swap(vvImgIdxInStrips);
//
//}
LRESULT CsswUAVFlyQuaSysView::OnMsgReSortStrips(WPARAM wParam, LPARAM lParam)
{
	if(m_bResortStripSemiAuto)
	{
		m_pDoc->m_FlyQuaPrj.ReOrderStrips(m_LineParaA,m_LineParaB,m_LineParaC,FunCallback4FlyQuaProcess);	
		m_pDoc->m_pMain->UpdateStatusLabel("设置航带方向...");
	}
	else
	{
		if(!m_pDoc->m_FlyQuaPrj.ReOrderStripByHand(m_vvCurPoint,FunCallback4FlyQuaProcess))
		{
			AfxMessageBox("航带排列异常，请重新操作！");
		}
		else 
			m_pDoc->m_pMain->UpdateStatusLabel("就绪");	
		DeleteCurLineInfo();	
	}
	m_nStripIdxSelected = InvalidValue;
	m_pDoc->m_FlyQuaPrj.SavePrj2File(m_pDoc->m_strPrjFilePath); 
	m_pDoc->FillPosRender();
	m_bSaveView2Img = true;
	CString str; str.Format("航带排列完成，航带数%d",m_pDoc->m_FlyQuaPrj.GetStripNum());
	FunOutPutWndLog(m_pDoc->m_pMain,str,true);
	FunOutPutLogInfo(str);
	m_pDoc->UpdatePrjTree();
	vector<int> vecErrStripIdx;
	for (int i = 0; i < m_pDoc->m_FlyQuaPrj.GetStripNum(); i++)
	{
		if(m_pDoc->m_FlyQuaPrj.GetImgNumInStrip(i)<3)
			vecErrStripIdx.push_back(i);
	}
	CString strNotice = str+"。";
	if(vecErrStripIdx.size()>0)
	{
		str.Format("以下航带影像数异常（<3）:");
		strNotice+=str;
		for (int i = 0; i<vecErrStripIdx.size(); i++)
		{
			str.Format("%s\nStrip_%d",strNotice,vecErrStripIdx[i]);
			strNotice = str;
		}
		AfxMessageBox(str);
	}
		m_bStripResorted = true;
		m_pDoc->UpdatePrjTree();
	return 1;
}
LRESULT CsswUAVFlyQuaSysView::OnMsgSetImgInvalid(WPARAM wParam, LPARAM lParam)
{
	if(m_bSetImg2Invalid)
	{
		m_pDoc->m_FlyQuaPrj.SetImgValidState(m_vvCurPoint,false,FunCallback4FlyQuaProcess);
//		m_pDoc->m_pMain->UpdateStatusLabel("设置航带方向...");
	}
	else
	{
		m_pDoc->m_FlyQuaPrj.SetImgValidState(m_vvCurPoint,true,FunCallback4FlyQuaProcess);
		
//		DeleteCurLineInfo();		
	}
	m_nStripIdxSelected = InvalidValue;
	m_pDoc->m_FlyQuaPrj.SavePrj2File(m_pDoc->m_strPrjFilePath); 
	m_pDoc->FillPosRender();
	DeleteCurLineInfo();	
	m_RectCqViewGeo = m_RectPosGeo;
	ReInitCqView();
	m_pDoc->UpdatePrjTree();
	CString strInfo;
	strInfo.Format("无效影像：%d张", m_pDoc->m_FlyQuaPrj.GetInvalidImgNum());
	FunOutPutWndLog(m_pDoc->m_pMain,strInfo,true);
	MessageBox("影像有效信息变更，请重新排列航带！");
//	m_pDoc->m_pMain->UpdatShowStripListeStatusLabel("就绪");	
	return 1;
}
LRESULT CsswUAVFlyQuaSysView::onMsgCreateMissAreaRender(WPARAM wParam, LPARAM lParam)
{
	ShowMissArea();
	Invalidate();
	return 1;
}
LRESULT CsswUAVFlyQuaSysView::onMseCreateSelectedRender(WPARAM wParam, LPARAM lParam)
{
	for (int i = 0; i<m_vecSelectedImgRender.size(); i++)
	{
		m_vecSelectedImgRender[i].DeleteVBO();
	}
	vector<CsswViewerRender>().swap(m_vecSelectedImgRender);
	vector<int> vecSelectedImgIdx = *(vector<int>*)wParam;
	if(vecSelectedImgIdx.size()==0) return 1;
	m_vecSelectedImgRender.resize(vecSelectedImgIdx.size());
	vector<vector<Point3D>> vvImgVertex = m_pDoc->m_FlyQuaPrj.GetImgVertex3d();
	if(vvImgVertex.size()==0)
	{
		vvImgVertex.resize(m_pDoc->m_FlyQuaPrj.GetImgNum());
		for (int i = 0; i<vecSelectedImgIdx.size(); i++)
		{
			CsswUAVImage image=m_pDoc->m_FlyQuaPrj.GetImgInfo()[vecSelectedImgIdx[i]];
	//		image.OpenImg(image.m_strImgPath);
			double pZ[4]={0,0,0,0};
			Point3D* pVerPt4=image.GetImgVertexPt3d4(pZ,false,false);
			for (int j = 0; j<4; j++)
			{
				vvImgVertex[vecSelectedImgIdx[i]].push_back(pVerPt4[j]);
			}
		//	image.CloseImg();
		}
	}
	for (int i = 0; i<vecSelectedImgIdx.size(); i++)
	{
		m_vecSelectedImgRender[i].CreateVectorVBO(vvImgVertex[vecSelectedImgIdx[i]].size(),vLine);
		double *pV = new double[vvImgVertex[vecSelectedImgIdx[i]].size()*2];
		double *pC = new double[vvImgVertex[vecSelectedImgIdx[i]].size()*4];
		FunVectorPt2VertexXY((void*)&vvImgVertex[vecSelectedImgIdx[i]],3,pV);
		FunCreateVertexColor(vvImgVertex[vecSelectedImgIdx[i]].size(),255,0,0,pC);//255 69 0
		m_vecSelectedImgRender[i].FillVectorVBO(pV,pC,vvImgVertex[vecSelectedImgIdx[i]].size());
		delete[]pV;pV=NULL;
		delete[]pC;pC=NULL;
	}
	vector<int>().swap(vecSelectedImgIdx);
	vector<vector<Point3D>>().swap(vvImgVertex); 
	Invalidate();
}
void CsswUAVFlyQuaSysView::ShowSeletctedStrip()
{
	for (int i = 0; i<m_vecSelectedImgRender.size(); i++)
	{
		m_vecSelectedImgRender[i].DeleteVBO();
	}
	vector<CsswViewerRender>().swap(m_vecSelectedImgRender);
	if(m_nStripIdxSelected < 0) return;
	double pV[4], pColor[8];
	int nImgIdx1,nImgIdx2;
	nImgIdx1 = m_pDoc->m_FlyQuaPrj.GetImgIdx(m_nStripIdxSelected,0);
	if(m_pDoc->m_FlyQuaPrj.GetImgNumInStrip(m_nStripIdxSelected)<2) 
		nImgIdx2 = nImgIdx1;else
	nImgIdx2 = m_pDoc->m_FlyQuaPrj.GetImgIdx(m_nStripIdxSelected,m_pDoc->m_FlyQuaPrj.GetImgNumInStrip(m_nStripIdxSelected)-1);
	pV[0]= m_pDoc->m_vecImgInfo[nImgIdx1].m_ImgPosInfo.Xs;
	pV[1]= m_pDoc->m_vecImgInfo[nImgIdx1].m_ImgPosInfo.Ys;
	pV[2]= m_pDoc->m_vecImgInfo[nImgIdx2].m_ImgPosInfo.Xs;
	pV[3]= m_pDoc->m_vecImgInfo[nImgIdx2].m_ImgPosInfo.Ys;
	if(pV[0]==pV[2]&&pV[1]==pV[3])
	{
		pV[0]-=12; pV[2]+=12;
		pV[1]-=12; pV[3]+=12;
	}
	for(int i = 0; i<2; i++)
	{
		*(pColor+4*i+0)=200;
		*(pColor+4*i+1)=200;
		*(pColor+4*i+2)=200;
		*(pColor+4*i+3)=255;
	}
	m_SelectedStripRender.FillVectorVBO(pV,pColor,2);

}
void CsswUAVFlyQuaSysView::ShowMissArea()
{
	bool bMakeCurrent = false;
	do { bMakeCurrent = wglMakeCurrent(m_pDC->GetSafeHdc(), g_pRC); } while (!bMakeCurrent);
	vector<vector<Point3D>> vvAreaVertex = m_pDoc->m_FlyQuaPrj.GetAreaVertex();
	for(int i = 0; i<m_vecCqExtRingRender.size(); i++) m_vecCqExtRingRender[i].DeleteVBO();
	m_vecCqExtRingRender.resize(vvAreaVertex.size());
	double l = 1e10,r = -1e10,t = -1e10,b = 1e10;
	for (int i = 0; i<vvAreaVertex.size(); i++)
	{
		CreateCqAreaRenderVBO(vvAreaVertex[i],m_vecCqExtRingRender[i]);
		for (int j = 0; j<vvAreaVertex[i].size(); j++)
		{
			l = min(l,vvAreaVertex[i][j].X);
			r = max(r,vvAreaVertex[i][j].X);
			t = max(t,vvAreaVertex[i][j].Y);
			b = min(b,vvAreaVertex[i][j].Y);
		}
	}
	//测区范围从POS分布范围更新为影像覆盖范围
	m_RectCqViewGeo = CRectD(l,t,r,b);
	ReInitCqView();
	CreateMissAreaRenderVBO(m_pDoc->m_FlyQuaPrj.GetMissAreaVertex(),m_vecMissAreaRender);
	vector<Point3D> vecAllImgVertex;
	vector<vector<Point3D>> vvImgVertex = m_pDoc->m_FlyQuaPrj.GetImgVertex3d();
	if(m_vecImgVertexRender.size()==0)
	{
		m_vecImgVertexRender.resize(m_pDoc->m_FlyQuaPrj.GetValidImgNum());
		for (int i = 0; i<m_vecImgVertexRender.size(); i++)
		{
			m_vecImgVertexRender[i].CreateVectorVBO(4,vPane);
		}
	}
	int nCount = 0;
	for (int i = 0; i<vvImgVertex.size(); i++)
	{
		if(vvImgVertex[i].size()==0) continue;
		CreateImgAreaRenderVBO(vvImgVertex[i],m_vecImgVertexRender[nCount]);
		nCount++;
	}
	vector<vector<Point3D>> vvvv(vvImgVertex.begin(),vvImgVertex.end());
	vector<vector<Point3D>> vvTmp; 
	for (int i = 0; i<1; i++)
	{
		vvTmp.push_back(vvImgVertex[i]); 
	}
	CreateOverlapRenderTex(vvImgVertex,m_ImgOverlapRender, m_pDoc->m_fOverlapRatio4);
	vector<vector<Point3D>>().swap(vvImgVertex);
	wglMakeCurrent(NULL, NULL);
}
void CsswUAVFlyQuaSysView::CreateCqAreaRenderVBO(vector<Point3D> &vecCqAreaVertex, CsswViewerRender &render)
{
	if(vecCqAreaVertex.size()==0) return;
	if(vecCqAreaVertex[vecCqAreaVertex.size()-1]!=vecCqAreaVertex[0])
		vecCqAreaVertex.push_back(vecCqAreaVertex[0]);  //闭合
	double *pV = new double[vecCqAreaVertex.size()*2];
	double *pC = new double[vecCqAreaVertex.size()*4];
	FunVectorPt2VertexXY((void*)&vecCqAreaVertex,3,pV);
	FunCreateVertexColor(vecCqAreaVertex.size(),0,0,0,pC);
	render.DeleteVBO();
	render.CreateVectorVBO(vecCqAreaVertex.size());
	render.FillVectorVBO(pV,pC,vecCqAreaVertex.size());
	delete[]pV;pV=NULL;
	delete[]pC;pC=NULL;
}
void CsswUAVFlyQuaSysView::CreateImgAreaRenderVBO(vector<Point3D> &vecImgAreaVertex,CsswViewerRender &render)
{
	double *pV = new double[vecImgAreaVertex.size()*2];
	double *pC = new double[vecImgAreaVertex.size()*4];
	FunVectorPt2VertexXY((void*)&vecImgAreaVertex,3,pV);
	FunCreateVertexColor(vecImgAreaVertex.size(),0,0.2*69/255,0.2,pC);//255 69 0
//	FunCreateVertexColor(vecImgAreaVertex.size(),0.2,0.1,0.1,pC);//255 69 0
	render.FillVectorVBO(pV,pC,vecImgAreaVertex.size());
	delete[]pV;pV=NULL;
	delete[]pC;pC=NULL;
}
void CsswUAVFlyQuaSysView::CreateMissAreaRenderVBO(vector<vector<Point3D>> &vvMissAreaVertex,vector<CsswViewerRender> &vRender)
{
	for (int i = 0; i<m_vecMissAreaRender.size(); i++)
	{
		m_vecMissAreaRender[i].DeleteVBO();
	}
	vector<CsswViewerRender>().swap(m_vecMissAreaRender);
	for (int i = 0; i<vvMissAreaVertex.size(); i++)
	{
		if(vvMissAreaVertex[i][vvMissAreaVertex[i].size()-1]!=vvMissAreaVertex[i][0])
			vvMissAreaVertex[i].push_back(vvMissAreaVertex[i][0]);  //闭合
		double *pV = new double[vvMissAreaVertex[i].size()*2];
		double *pC = new double[vvMissAreaVertex[i].size()*4];
		FunVectorPt2VertexXY((void*)&vvMissAreaVertex[i],3,pV);
		FunCreateVertexColor(vvMissAreaVertex[i].size(),1,0,0,pC);
		CsswViewerRender Render;
		Render.CreateVectorVBO(vvMissAreaVertex[i].size(),vLine);
		Render.FillVectorVBO(pV,pC,vvMissAreaVertex[i].size());
		vRender.push_back(Render);
		delete[]pV;pV=NULL;
		delete[]pC;pC=NULL;
	}
}
void CsswUAVFlyQuaSysView::CreateOverlapRenderTex(vector<vector<Point3D>> &vvImgArea, CsswImgRender &render,float &fOverlap4Ratio)
{
	fOverlap4Ratio = 0;
	CRectD RectImgGeo = m_RectCqViewGeo;
	vector<vector<CPoint>> vvImgArea2Cln(vvImgArea.size());
	for (int i = 0; i<vvImgArea.size(); i++)
	{
		if(vvImgArea[i].size()==0) continue;
		for (int j = 0; j<vvImgArea[i].size(); j++)
		{
			Point2D pt2d = Point2D(vvImgArea[i][j].X,vvImgArea[i][j].Y);
			vvImgArea2Cln[i].push_back(ptGeo2Cln(pt2d));
		}
	}
	CRect RectOverlapImg2Cln = CRect(ptGeo2Cln(RectImgGeo.LeftTop()),ptGeo2Cln(RectImgGeo.RightBottom()));
	float fScale = RectOverlapImg2Cln.Height()*1.0/RectOverlapImg2Cln.Width();
  	RectOverlapImg2Cln.left = max(0,RectOverlapImg2Cln.left-20);
  	RectOverlapImg2Cln.right = min(m_RectCln.right,RectOverlapImg2Cln.right+20);
  	RectOverlapImg2Cln.top = max(0,RectOverlapImg2Cln.top-20*fScale);
  	RectOverlapImg2Cln.bottom = min(m_RectCln.bottom,RectOverlapImg2Cln.bottom+20*fScale);
	int nWidth = RectOverlapImg2Cln.Width();
	int nHeight = RectOverlapImg2Cln.Height();
	int *pOverlapCount = new int[nWidth*nHeight];
	int nCountOverlap4 = 0;
	memset(pOverlapCount,0,nWidth*nHeight*sizeof(int));
	for (int i = 0; i<vvImgArea2Cln.size(); i++)
	{
		if(vvImgArea2Cln[i].size()==0) continue;
		float pA[4],pB[4],pC[4];
		int pLineYMax[4],pLineYMin[4];
		int pLineXMax[4],pLineXMin[4];
		int nLeft = 1e10, nRight = -1e10;
		for (int j = 0; j<4; j++)
		{
			int nIdx1 = j;
			int nIdx2 = j+1; 
			if(nIdx2==4)nIdx2=0;
			CalLineParas(&vvImgArea2Cln[i][nIdx1],&vvImgArea2Cln[i][nIdx2],2,pA[j],pB[j],pC[j]);
			pLineYMax[j] = max(vvImgArea2Cln[i][nIdx1].y,vvImgArea2Cln[i][nIdx2].y);
			pLineYMin[j] = min(vvImgArea2Cln[i][nIdx1].y,vvImgArea2Cln[i][nIdx2].y);
			pLineXMax[j] = max(vvImgArea2Cln[i][nIdx1].x,vvImgArea2Cln[i][nIdx2].x);
			pLineXMin[j] = min(vvImgArea2Cln[i][nIdx1].x,vvImgArea2Cln[i][nIdx2].x);
			nLeft = min(nLeft,min(vvImgArea2Cln[i][nIdx1].x,vvImgArea2Cln[i][nIdx2].x));
			nRight = max(nRight, max(vvImgArea2Cln[i][nIdx1].x,vvImgArea2Cln[i][nIdx2].x));
		}
		nLeft = max(nLeft,RectOverlapImg2Cln.left);
		nRight = min(nRight,RectOverlapImg2Cln.right);
		int yTop = 1e10, yBot = -1e10;
		for (int j = 0; j<vvImgArea2Cln[i].size(); j++)
		{
			yTop = min(yTop,vvImgArea2Cln[i][j].y);
			yBot = max(yBot,vvImgArea2Cln[i][j].y);
		}
		for (int j = yTop; j<=yBot; j++)
		{
			int line_y = j; 
			if(line_y>=RectOverlapImg2Cln.bottom||line_y<=RectOverlapImg2Cln.top) continue;
			vector<int> vLineIdx2Inter;
			for (int k = 0; k<4; k++)
			{
				if(line_y>=pLineYMin[k]&&line_y<=pLineYMax[k])
					vLineIdx2Inter.push_back(k);
			}
			int xLeft = 10000, xRight = -10000;
			for (int k = 0; k<vLineIdx2Inter.size(); k++)
			{
				int x = LineY2X(line_y,pA[vLineIdx2Inter[k]],pB[vLineIdx2Inter[k]],pC[vLineIdx2Inter[k]]);
				xLeft = min(xLeft,x); if(xLeft<nLeft) xLeft = nLeft;
				xRight = max(xRight,x); if(xRight==0||xRight> nRight) xRight = nRight;
			}
			if(vLineIdx2Inter.size()==0) continue;
			for (int ii = xLeft-RectOverlapImg2Cln.left; ii<xRight-RectOverlapImg2Cln.left; ii++)
			{
				int jj = line_y-RectOverlapImg2Cln.top;
				if(jj<0||ii<0) continue;
				(*(pOverlapCount+jj*nWidth+ii))++;
			}
			vector<int>().swap(vLineIdx2Inter);
		}
	}
	BYTE *pMem = new BYTE[nWidth*(nHeight+15)*3];
	memset(pMem,255,nWidth*(nHeight+15)*3);
	for (int i = 0; i<nWidth*nHeight; i++)
	{
		*(pMem+i*3+0) = 250;
		*(pMem+i*3+1) = 240;
		*(pMem+i*3+2) = 230;
		if(pOverlapCount[i]==1)
		{
			*(pMem+i*3+0) = 205;
			*(pMem+i*3+1) = 198;
			*(pMem+i*3+2) = 115;
		}else
			if(pOverlapCount[i]==2)
			{
				*(pMem+i*3+0) = 255;
				*(pMem+i*3+1) = 255;
				*(pMem+i*3+2) = 0;
			}else
				if(pOverlapCount[i]==3)
				{
					*(pMem+i*3+0) = 255;
					*(pMem+i*3+1) = 140;
					*(pMem+i*3+2) = 0;
				}else
				{
					nCountOverlap4++;
					if(pOverlapCount[i]==4)
					{
						*(pMem+i*3+0) = 50;
						*(pMem+i*3+1) = 205;
						*(pMem+i*3+2) = 50;
					}else
						if(pOverlapCount[i]>4&&pOverlapCount[i]<=10)
						{
							*(pMem+i*3+0) = 0;
							*(pMem+i*3+1) = 100;
							*(pMem+i*3+2) = 0;
						}else
							if(pOverlapCount[i]>10)
							{
								*(pMem+i*3+0) = 0;
								*(pMem+i*3+1) = 0;
								*(pMem+i*3+2) = 205;
							}
				}
			
	}
	fOverlap4Ratio = nCountOverlap4*1.0/(nWidth*nHeight);
	int nGapWidth = nWidth*2/3/7;
	for (int i = nHeight; i<nHeight+15; i++)
	{
		for (int j = 0; j<nGapWidth; j++)
		{
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+0) = 250;
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+1) = 240;
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+2) = 230;
		}
		for (int j = nGapWidth; j<nGapWidth*2; j++)
		{
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+0) = 205;
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+1) = 198;
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+2) = 115;
		}
		for (int j = nGapWidth*2; j<nGapWidth*3; j++)
		{
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+0) =255;
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+1) =255;
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+2) =0;
		}
		for (int j = nGapWidth*3; j<nGapWidth*4; j++)
		{
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+0) =255;
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+1) =140;
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+2) =0;
		}
		for (int j =nGapWidth*4; j<nGapWidth*5; j++)
		{
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+0) = 50;
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+1) = 205;
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+2) = 50;
		}
		for (int j = nGapWidth*5; j<nGapWidth*6; j++)
		{
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+0) = 0;
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+1) = 100;
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+2) = 0;
		}
		for (int j =  nGapWidth*6; j<nGapWidth*7; j++)
		{
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+0) = 0;
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+1) = 0;
			*(pMem+i*nWidth*3+(j+nWidth/3)*3+2) = 205;
		}
	}
	int nPicWidth = 1024;
	int nPicHeight = 512;
	CString strOverlapImg = FunGetFileFolder(m_strCqViewImagePath);
	if(m_pDoc->m_eFlyQuaChkMethod==BY_ADJUSTMENT) strOverlapImg+="\\OverlapImg_ByBundle.jpg";else
	if(m_pDoc->m_eFlyQuaChkMethod==BY_MATCH) strOverlapImg+="\\OverlapImg_ByMatch.jpg";else
		if(m_pDoc->m_eFlyQuaChkMethod==BY_POS) strOverlapImg+="\\OverlapImg_ByPos.jpg";
 	CsswUAVImage img; img.SaveImg(strOverlapImg,nWidth,nHeight+15,3,pMem);
	/*****************************************************/
	FILE* fff = fopen(FunGetThisExePath()+"_adj.txt","w");
	for (int i = 0; i<vvImgArea.size(); i++)
	{
		fprintf(fff,"%d  ",i);
		for (int j = 0; j<vvImgArea[i].size(); j++)
		{
			fprintf(fff,"%lf %lf  ", vvImgArea[i][j].X,vvImgArea[i][j].Y);
		}
		fprintf(fff,"\n");
	}
	fclose(fff);
	/****************************************************/

// 	img.OpenImg(strOverlapImg);
// 	BYTE*pData = new BYTE[nPicWidth*nPicHeight*3];
// 	img.ReadImg(0,0,img.GetCols(),img.GetRows(),nPicWidth,nPicHeight,pData);
// 	img.CloseImg();
// 	 img.SaveImg(strOverlapImg,nPicWidth,nPicHeight,3,pData);
 	render.CreateTexture(strOverlapImg);
 	render.FillTexture();
	delete[]pOverlapCount;
	delete[]pMem;
//	delete[]pData;


}

void CsswUAVFlyQuaSysView::CreateKeyNodeRenterVBO()
{
	m_KeyNodePointRender.CreateVectorVBO(m_nKeyNodeNum, vPoint, true);
	m_vecKeyNodeLineRender.resize(m_nNodeLevel + m_nNodeLevel - 1);
	for (int i = 0; i<m_nNodeLevel + m_nNodeLevel -1; i++)
	{
		m_vecKeyNodeLineRender[i].CreateVectorVBO(m_nNodeLevel, vLine, true);
	}
}

void CsswUAVFlyQuaSysView::FillKeyNodeRenderVBO()
{
	GLdouble* pv = new GLdouble[m_nKeyNodeNum * 2];
	GLdouble* pc = new GLdouble[m_nKeyNodeNum * 4];
	GLdouble cx = m_RectCln2Geo.CenterPoint().x;
	GLdouble cy = m_RectCln2Geo.CenterPoint().y;
	GLdouble sw = m_RectCln2Geo.Width()*0.8/2;
	GLdouble sh = m_RectCln2Geo.Height()*0.8/2;
	GLdouble r = sqrt(sw*sw + sh*sh);
	int nNodeLevel = m_nNodeLevel; double pi = 3.1415926;
	int count = 0;
	//for line
	GLdouble *pvLine = new GLdouble[m_nNodeLevel * 2];
	GLdouble *pcLine = new GLdouble[m_nNodeLevel * 4];
	for (int i = 0; i < nNodeLevel; i++)
	{
		double angle = i * pi * 2 / m_nNodeLevel;
		for (int j = 0; j<m_nNodeLevel-1; j++)
		{
			GLdouble x = cx + r*(j + 1) / nNodeLevel*cos(angle);
			GLdouble y = cy - r*(j + 1) / nNodeLevel*sin(angle);
			*(pv + 2 * count + 0) = x;
			*(pv + 2 * count + 1) = y;
			*(pc + 4 * count + 0) = 150;
			*(pc + 4 * count + 1) = 150;
			*(pc + 4 * count + 2) = 150;
			*(pc + 4 * count + 3) = 255;
			count++;
		}
		// for line
		for (int j = 0; j<m_nNodeLevel; j++)
		{
			GLdouble x = cx + r*j / nNodeLevel*cos(angle);
			GLdouble y = cy - r*j / nNodeLevel*sin(angle);
			*(pvLine + 2 * j + 0) = x;
			*(pvLine + 2 * j + 1) = y;
			*(pcLine + 4 * j + 0) = 0.2;
			*(pcLine + 4 * j + 1) = 0.2;
			*(pcLine + 4 * j + 2) = 0.2;
			*(pcLine + 4 * j + 3) = 255;
		}
		m_vecKeyNodeLineRender[i].FillVectorVBO(pvLine, pcLine, m_nNodeLevel);
	}
	*(pv + 2 * count + 0) = cx;
	*(pv + 2 * count + 1) = cy;
	*(pc + 4 * count + 0) = 150;
	*(pc + 4 * count + 1) = 150;
	*(pc + 4 * count + 2) = 150;
	*(pc + 4 * count + 3) = 255;
	m_KeyNodePointRender.FillVectorVBO(pv, pc, m_nKeyNodeNum);


	// for line-cirle
	for (int i = 1; i<m_nNodeLevel; i++)
	{
		count = 0;
		for (int j = 0; j < m_nNodeLevel; j++)
		{
			double angle = j * pi * 2 / m_nNodeLevel;
			GLdouble x = cx + r*i / nNodeLevel*cos(angle);
			GLdouble y = cy - r*i / nNodeLevel*sin(angle);
			*(pvLine + 2 * count + 0) = x;
			*(pvLine + 2 * count + 1) = y;
			*(pcLine + 4 * count + 0) = 0.2;
			*(pcLine + 4 * count + 1) = 0.2;
			*(pcLine + 4 * count + 2) = 0.2;
			*(pcLine + 4 * count + 3) = 255;
			count++;
		}
		m_vecKeyNodeLineRender[m_nNodeLevel + i - 1].FillVectorVBO(pvLine, pcLine, m_nNodeLevel);
	}


	
	delete[]pv; pv = NULL;
	delete[]pc; pc = NULL;
	delete[]pvLine; pvLine = NULL;
	delete[]pcLine; pcLine = NULL;
}

void CsswUAVFlyQuaSysView::ShowBackGround()
{
	
	CRect  rect; GetClientRect(&rect);
	int width, height;
	width = rect.Width();
	height = rect.Height();
	CDC dcMem; CBitmap bmp, *pbmpOld;
	int w, h, w0, h0;
	
	bmp.LoadBitmap(IDB_BITMAP_BACKFROUND);
	BITMAP bitMap;
	bmp.GetBitmap(&bitMap);

// 	CString strViewBmp = FunGetFileFolder(FunGetThisExePath())+"\\resource\\DlgBk.bmp";
// 	HBITMAP hBmp = (HBITMAP)::LoadImage(AfxGetInstanceHandle(),strViewBmp,IMAGE_BITMAP, width, height, LR_LOADFROMFILE);
// 	bmp.Attach(hBmp);	
// 	BITMAP   bitMap;
// 	bmp.GetBitmap(&bitMap);

// 	float fScaleH2W = height*1.0/width;
// 	w0 = bitMap.bmWidth;
// 	h0 = w0*fScaleH2W;
// 	if(h0>bitMap.bmHeight)
// 	{
// 		h0=bitMap.bmHeight;
// 		w0 = h0/fScaleH2W;
// 	}
	dcMem.CreateCompatibleDC(GetDC());
	pbmpOld = dcMem.SelectObject(&bmp);
	dcMem.SelectObject(bmp);
	GetDC()->StretchBlt(0, 0, width, height, &dcMem, 0, 0,bitMap.bmWidth,bitMap.bmHeight, SRCCOPY);
	bmp.Detach();
	dcMem.DeleteDC();
	bmp.DeleteObject();
}
int CsswUAVFlyQuaSysView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{

	int a = 11.9 / 2;
	CString tr; tr.Format("11.9/2=%d", a);
	AfxMessageBox(tr); 
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	m_pDC = new CClientDC(this);//初始化OPENGL设置定时器
	SetupPixelFormat(m_pDC, m_hPalette);
	//InitOpenGL(g_pRCSharing, m_pDC, m_hPalette);
	InitOpenGL(g_pRC, m_pDC, m_hPalette);
	//wglShareLists(g_pRCSharing, g_pRC);
	wglMakeCurrent(m_pDC->GetSafeHdc(), g_pRC);
	glewInit();

	bool g_bSupportVBO = isGLExtensionSupported("GL_ARB_vertex_buffer_object");
	if (g_bSupportVBO)
	{
		glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
		glBindBufferARB = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
		glBufferDataARB = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
		glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");

		m_SelectedStripRender.CreateVectorVBO(2,vLine,true);
		m_BkgrdRender.CreateVectorVBO(4,vPane,true);
	}

	return 0;
}
void CsswUAVFlyQuaSysView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	GetClientRect(m_RectCln);
	//ResetRectCln2Geo();
	// TODO: 在此处添加消息处理程序代码
}
void CsswUAVFlyQuaSysView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CView::OnPaint()
}

BOOL CsswUAVFlyQuaSysView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_pDoc == NULL) return CView::OnEraseBkgnd(pDC);
	if (!m_pDoc->m_bPrjLoaded)
	{
		ShowBackGround();
		return TRUE;
	}
	bool bMakeCurrent = false;
	do { bMakeCurrent = wglMakeCurrent(m_pDC->GetSafeHdc(), g_pRC); } while (!bMakeCurrent);

	PrepareDraw(0, 0, 0, 1);
	glViewport(0, 0, m_RectCln.Width(), m_RectCln.Height());
	glLoadIdentity();    // 重置当前指定的矩阵为单位矩阵
	ResetRectCln2Geo();
	glOrtho(m_RectCln2Geo.left, m_RectCln2Geo.right, m_RectCln2Geo.bottom, m_RectCln2Geo.top, -1, 1);
	float A[4]={0.5,0.5, 0.5,0.5 };
	float R[4] = {30.0/255,0,160.0/255,217.0/255 };
	float G[4] = { 138.0/255,130.0 / 255,61.0/255,64.0/255 };
	float B[4] = { 0,156.0/255,1,0};
	SetBackColor(m_BkgrdRender,m_RectCln2Geo,R,G,B,A,true);
	if (!m_KeyNodePointRender.IsVBOFilled())
	{
		CreateKeyNodeRenterVBO();
		FillKeyNodeRenderVBO();
	}
	if (m_KeyNodePointRender.IsVBOFilled())
	{
		for (int i = 0; i<m_nNodeLevel+m_nNodeLevel-1; i++)
		{
			m_vecKeyNodeLineRender[i].Display(2);
		}
		m_KeyNodePointRender.Display(6);
	}
	EndDraw(m_pDC);
	return TRUE;


	if(m_bViewImgArea)
	{		
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE,GL_ONE);
		for (int i = 0; i<m_vecImgVertexRender.size(); i++)
		{	
			m_vecImgVertexRender[i].Display(1);
		}
		glDisable(GL_BLEND);
//		m_ImgOverlapRender.DisplayTexture(&m_RectCln2Geo);
	}
	if(m_bViewMissArea)
	{
		for (int i = 0; i<m_vecMissAreaRender.size(); i++)
		{
			m_vecMissAreaRender[i].Display(3);
		}
	}
	if(m_bViewCqArea) 
	{
		for (int i = 0; i<m_vecCqExtRingRender.size(); i++)
		{
			m_vecCqExtRingRender[i].Display(2);
		}
	}

	if(m_bViewImgName)
	{	
		glColor3f(1.0,1.0,0);
		for (int i = 0; i<m_pDoc->m_vecImgInfo.size(); i++)
		{
			glRasterPos2f(m_pDoc->m_vecImgInfo[i].m_ImgPosInfo.Xs,m_pDoc->m_vecImgInfo[i].m_ImgPosInfo.Ys);
			if(m_bViewStripLabel&&m_bStripResorted)
			{
				int nStripIdx=m_pDoc->m_vecImgInfo[i].m_nStripIdx+1;
				int nIdxInStrip=m_pDoc->m_vecImgInfo[i].m_nIdxInStrip+1;
				CString strLabel = m_pDoc->m_vecImgInfo[i].m_strStripLabel;
				DisplayString(strLabel);
			}
			else 
			{
				DisplayString(FunGetFileName(m_pDoc->m_vecImgInfo[i].m_strImgPath,true));
			}
		}
	}
	if(m_bViewImgPos) m_PosRender.Display(5);
	if(m_bDisplayCurLine) 
	{
		for (int i = 0; i<m_vecCurLineRender.size(); i++)
		{
			m_vecCurLineRender[i].Display(1);
		}
		m_CurLineRender.Display(1);
	}
	if(m_nStripIdxSelected>=0) m_SelectedStripRender.Display(5);	
	for (int i = 0; i<m_vecSelectedImgRender.size(); i++)
	{
		m_vecSelectedImgRender[i].Display(3);
	}
	EndDraw(m_pDC);
	if(m_bSaveView2Img)
	{
		SaveView2Img(m_strCqViewImagePath);
		m_bSaveView2Img=false;
	}
	return TRUE;
}


BOOL CsswUAVFlyQuaSysView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(m_pDoc==NULL) return CView::OnMouseWheel(nFlags, zDelta, pt);
	if(!m_pDoc->m_bPrjLoaded) return CView::OnMouseWheel(nFlags, zDelta, pt);
	ScreenToClient(&pt);
	Point2D ptGeo =ptCln2Geo(pt);
	m_fZoomRateCln2Geo *= pow(1.15, 1.0*zDelta / 120);
	m_fZoomRateCln2Geo = min(m_fZoomRateCln2Geo, m_fZoomRateCln2GeoMax);
	m_fZoomRateCln2Geo = max(m_fZoomRateCln2Geo, m_fZoomRateCln2GeoMin);
	m_ptClnCenter2Geo = ClnCenter2Geo(pt, ptGeo);
	//ResetRectCln2Geo();
	Invalidate();
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}
void CsswUAVFlyQuaSysView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(m_pDoc==NULL) return;
	if(!m_pDoc->m_bPrjLoaded) return;
	if((nFlags&MK_MBUTTON)==MK_MBUTTON)
	{
		m_ptClnCenter2Geo = ClnCenter2Geo( point, m_ptMBtnDnGeo);
		//ResetRectCln2Geo();
		Invalidate();
	}
	if(m_bActStripResort)
	{
		if(m_bResortStripSemiAuto&&m_LinePt1Geo!=Point2D()&&m_LinePt2Geo==Point2D())
		{
			m_LinePt2Geo = ptCln2Geo(point);
			double *pVertex = new double[2*2];
			double *pColor = new double[2*4];
			for(int i = 0; i<2; i++) 
			{
				*(pColor+i*4+0)=200;
				*(pColor+i*4+1)=200;
				*(pColor+i*4+2)=200;
				*(pColor+i*4+3)=255;
			}
				
			pVertex[0]=m_LinePt1Geo.x;
			pVertex[1]=m_LinePt1Geo.y;
			pVertex[2]=m_LinePt2Geo.x;
			pVertex[3]=m_LinePt2Geo.y;
			m_CurLineRender.FillVectorVBO(pVertex,pColor,2);
			m_bDisplayCurLine = true;
			Invalidate();
			delete[]pVertex;
			delete[]pColor;
			m_LinePt2Geo = Point2D();
		}
		else if(m_bResortStripByHand)
		{
			if(m_vecCurPoint.size()==0) return;
			Point2D PtGeo = ptCln2Geo(point);
			m_vecCurPoint[m_vecCurPoint.size()-1]=PtGeo;
			double *pVertex = new double[m_vecCurPoint.size()*2];
			double *pColor = new double[m_vecCurPoint.size()*4];
			for(int i = 0; i<m_vecCurPoint.size(); i++) 
			{
				*(pColor+i*4+0)=200;
				*(pColor+i*4+1)=200;
				*(pColor+i*4+2)=200;
				*(pColor+i*4+3)=255;
			}
			for (int i = 0; i<m_vecCurPoint.size(); i++)
			{
				*(pVertex+2*i+0)=m_vecCurPoint[i].x;
				*(pVertex+2*i+1)=m_vecCurPoint[i].y;
			}
			m_CurLineRender.DeleteVBO();
			m_CurLineRender.CreateVectorVBO(m_vecCurPoint.size(),vLine,true);
			m_CurLineRender.FillVectorVBO(pVertex,pColor,m_vecCurPoint.size());
			m_bDisplayCurLine = true;
			Invalidate();
			delete[]pVertex;
			delete[]pColor;
		}
	}
	if(m_bActSetInvalidImg)
	{
		if(m_vecCurPoint.size()==0) return;
		Point2D PtGeo = ptCln2Geo(point);
		m_vecCurPoint[m_vecCurPoint.size()-1]=PtGeo;
		double *pVertex = new double[m_vecCurPoint.size()*2];
		double *pColor = new double[m_vecCurPoint.size()*4];
		for(int i = 0; i<m_vecCurPoint.size(); i++) 
		{
			*(pColor+i*4+0)=200;
			*(pColor+i*4+1)=200;
			*(pColor+i*4+2)=200;
			*(pColor+i*4+3)=255;
		}
		for (int i = 0; i<m_vecCurPoint.size(); i++)
		{
			*(pVertex+2*i+0)=m_vecCurPoint[i].x;
			*(pVertex+2*i+1)=m_vecCurPoint[i].y;
		}
		m_CurLineRender.DeleteVBO();
		m_CurLineRender.CreateVectorVBO(m_vecCurPoint.size(),vLine,true);
		m_CurLineRender.FillVectorVBO(pVertex,pColor,m_vecCurPoint.size());
		m_bDisplayCurLine = true;
		Invalidate();
		delete[]pVertex;
		delete[]pColor;
	}
	CView::OnMouseMove(nFlags, point);
}

void CsswUAVFlyQuaSysView::OnMButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
		if(m_pDoc==NULL) return;
	if(!m_pDoc->m_bPrjLoaded) return;
	SetCapture();
	m_ptMBtnDnGeo = ptCln2Geo(point);
	CView::OnMButtonDown(nFlags, point);
}

void CsswUAVFlyQuaSysView::OnMButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	ReleaseCapture();
	CView::OnMButtonUp(nFlags, point);
}

void CsswUAVFlyQuaSysView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(m_pDoc==NULL) return;
	if(!m_pDoc->m_bPrjLoaded) return;
	if(m_bActStripResort)
	{
		if(m_bResortStripSemiAuto)
		{
			if(m_LinePt1Geo==Point2D())
			{
				m_LinePt1Geo=ptCln2Geo(point);
				m_CurLineRender.CreateVectorVBO(2,vLine,true);
				m_bDisplayCurLine = true;
			}
			else 
			{
				m_LinePt2Geo = ptCln2Geo(point);
				double *pVertex = new double[2*2];
				double *pColor = new double[2*4];
				for(int i = 0; i<2; i++) 
				{
					*(pColor+i*4+0)=200;
					*(pColor+i*4+1)=200;
					*(pColor+i*4+2)=200;
					*(pColor+i*4+3)=255;
				}
				pVertex[0]=m_LinePt1Geo.x;
				pVertex[1]=m_LinePt1Geo.y;
				pVertex[2]=m_LinePt2Geo.x;
				pVertex[3]=m_LinePt2Geo.y;
				m_CurLineRender.FillVectorVBO(pVertex,pColor,2);
				m_bDisplayCurLine = true;
				Invalidate();
				delete[]pVertex;
				delete[]pColor;
				CalLineParas(m_LinePt1Geo,m_LinePt2Geo);		
				m_pDoc->m_pMain->UpdateStatusLabel("设置航带方向...");
				
				if(MessageBox("是否按照指示方向排列航带？","提示",MB_YESNO)==IDYES)
				{
					HANDLE hThread;DWORD IdThread;
					hThread = CreateThread(NULL, 0,
						(LPTHREAD_START_ROUTINE)MultiThreadReOrderStrips,
						(LPVOID*)this, 0, &IdThread);
					//		m_bViewImgName=true;
					
				}
				m_LinePt1Geo= Point2D();
				m_LinePt2Geo=Point2D();
//				m_CurLineRender.DeleteVBO();
				m_bDisplayCurLine = false;
				Invalidate();
			}
		}
		else if(m_bResortStripByHand)
		{
			Point2D PtGeo=ptCln2Geo(point);
			if(m_vecCurPoint.size()==0)m_vecCurPoint.push_back(PtGeo);
			else m_vecCurPoint[m_vecCurPoint.size()-1]=PtGeo;
			double *pVertex = new double[m_vecCurPoint.size()*2];
			double *pColor = new double[m_vecCurPoint.size()*4];
			for(int i = 0; i<m_vecCurPoint.size(); i++) 
			{
				*(pColor+i*4+0)=200;
				*(pColor+i*4+1)=200;
				*(pColor+i*4+2)=200;
				*(pColor+i*4+3)=255;
			}
			for (int i = 0; i<m_vecCurPoint.size(); i++)
			{
				*(pVertex+2*i+0)=m_vecCurPoint[i].x;
				*(pVertex+2*i+1)=m_vecCurPoint[i].y;
			}
			m_CurLineRender.DeleteVBO();
			m_CurLineRender.CreateVectorVBO(m_vecCurPoint.size(),vLine,true);
			m_CurLineRender.FillVectorVBO(pVertex,pColor,m_vecCurPoint.size());
			m_bDisplayCurLine = true;
			delete[]pVertex;
			delete[]pColor;
			Invalidate();
			m_vecCurPoint.resize(m_vecCurPoint.size()+1);
		}
	}
	if(m_bActSetInvalidImg)
	{
		Point2D PtGeo=ptCln2Geo(point);
		if(m_vecCurPoint.size()==0)m_vecCurPoint.push_back(PtGeo);
		else m_vecCurPoint[m_vecCurPoint.size()-1]=PtGeo;
		double *pVertex = new double[m_vecCurPoint.size()*2];
		double *pColor = new double[m_vecCurPoint.size()*4];
		for(int i = 0; i<m_vecCurPoint.size(); i++) 
		{
			*(pColor+i*4+0)=200;
			*(pColor+i*4+1)=200;
			*(pColor+i*4+2)=200;
			*(pColor+i*4+3)=255;
		}
		for (int i = 0; i<m_vecCurPoint.size(); i++)
		{
			*(pVertex+2*i+0)=m_vecCurPoint[i].x;
			*(pVertex+2*i+1)=m_vecCurPoint[i].y;
		}
		m_CurLineRender.DeleteVBO();
		m_CurLineRender.CreateVectorVBO(m_vecCurPoint.size(),vLine,true);
		m_CurLineRender.FillVectorVBO(pVertex,pColor,m_vecCurPoint.size());
		m_bDisplayCurLine = true;
		delete[]pVertex;
		delete[]pColor;
		Invalidate();
		m_vecCurPoint.resize(m_vecCurPoint.size()+1);
	}
	CView::OnLButtonDown(nFlags, point);
}

void CsswUAVFlyQuaSysView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CView::OnLButtonUp(nFlags, point);
}

void CsswUAVFlyQuaSysView::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(m_pDoc==NULL) return;
	if(!m_pDoc->m_bPrjLoaded) return;
	if ((m_bActStripResort&&m_bResortStripByHand)||m_bActSetInvalidImg)
	{
		if(m_vecCurPoint.size()==0) return;
		m_vecCurPoint[m_vecCurPoint.size()-1]=m_vecCurPoint[0];
		double *pVertex = new double[m_vecCurPoint.size()*2];
		double *pColor = new double[m_vecCurPoint.size()*4];
		for(int i = 0; i<m_vecCurPoint.size(); i++) 
		{
			*(pColor+i*4+0)=200;
			*(pColor+i*4+1)=200;
			*(pColor+i*4+2)=200;
			*(pColor+i*4+3)=255;
		}
		for (int i = 0; i<m_vecCurPoint.size(); i++)
		{
			*(pVertex+2*i+0)=m_vecCurPoint[i].x;
			*(pVertex+2*i+1)=m_vecCurPoint[i].y;
		}
		CsswViewerRender CurLineRender;
		CurLineRender.CreateVectorVBO(m_vecCurPoint.size());
		CurLineRender.FillVectorVBO(pVertex,pColor,m_vecCurPoint.size());
		m_vecCurLineRender.push_back(CurLineRender);
		m_vvCurPoint.push_back(m_vecCurPoint);
		m_CurLineRender.DeleteVBO();
		Invalidate();
		delete[]pVertex;
		delete[]pColor;
		m_LinePt1Geo=Point2D();
		m_LinePt2Geo=Point2D();
		vector<Point2D>().swap(m_vecCurPoint);
	}
	CView::OnRButtonDown(nFlags, point);
}


// void CsswUAVFlyQuaSysView::OnButtonSaveView()
// {
// 	// TODO: 在此添加命令处理程序代码
// 	CString strImage=FunGetFileFolder(m_pDoc->m_strPrjFilePath)+"\\source";
// 	CreateDirectory(strImage,NULL);
// 	m_strCqViewImagePath=strImage+"\\CqView.jpg";	
// 	CPoint pt1 = ptGeo2Cln(m_RectCqViewGeo.LeftTop());
// 	CPoint pt2 = ptGeo2Cln(m_RectCqViewGeo.RightBottom());
// 	SaveView2Img(CRect(pt1,pt2),m_strCqViewImagePath);
// }

void CsswUAVFlyQuaSysView::OnButtonSortStrip()
{
	// TODO: 在此添加命令处理程序代码
	if(!m_bActStripResort)
	{
		DeleteCurLineInfo();
		Invalidate();
		if(m_bResortStripSemiAuto)
		{
			m_pDoc->m_pMain->UpdateStatusLabel("设置航带方向...");
			switch(MessageBox("请在测区范围外选择两点，绘制一条与航向平行的直线，辅助航带排列。","提示",MB_OKCANCEL))
			{
			case IDOK:
				break;
			default:
				m_pDoc->m_pMain->UpdateStatusLabel("就绪");
				return;
			}
		}
		else
		{
			m_pDoc->m_pMain->UpdateStatusLabel("手动选择航带影像...");
			switch(MessageBox("请在测区范围内绘制多边形，手动选择属于同一条航带的影像。","提示",MB_OKCANCEL))
			{
			case IDOK:
				break;
			default:
				m_pDoc->m_pMain->UpdateStatusLabel("就绪");
				return;
			}
		} 
		FunOutPutWndLog(m_pDoc->m_pMain,"航带排列激活！",true);
		
	}
	else
	{
		if(m_bResortStripByHand)  //此前手动排列航带
		{
			m_pDoc->m_pMain->UpdateStatusLabel("手动选择航带影像...");
			if(m_vvCurPoint.size()!=0)
			{
				if(MessageBox("是否按照绘图方式排列航带？","提示",MB_YESNO)==IDYES)
				{
					HANDLE hThread;DWORD IdThread;
					hThread = CreateThread(NULL, 0,
						(LPTHREAD_START_ROUTINE)MultiThreadReOrderStrips,
						(LPVOID*)this, 0, &IdThread);
					m_bStripResorted = true;
				}
				else
					DeleteCurLineInfo();	
			}
			vector<Point2D>().swap(m_vecCurPoint);
			m_CurLineRender.DeleteVBO();
		}else
		{
			m_pDoc->m_pMain->UpdateStatusLabel("就绪");
			DeleteCurLineInfo();
		}
		Invalidate();
	}
	m_bActStripResort =!m_bActStripResort;
	if(m_bActStripResort) m_bActSetInvalidImg = false;
}


void CsswUAVFlyQuaSysView::OnUpdateButtonSortStrip(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	if(!m_pDoc->m_bPrjLoaded) pCmdUI->Enable(m_pDoc->m_bPrjLoaded);
	else pCmdUI->Enable(!m_pDoc->m_bInRunning);
	pCmdUI->SetCheck(m_bActStripResort);
}
void CsswUAVFlyQuaSysView::OnButtonByhandResortStrip()
{
	// TODO: 在此添加命令处理程序代码
	if(m_bResortStripByHand) return;
	m_bResortStripByHand = true;
	m_bResortStripSemiAuto = false;
	m_LinePt1Geo = Point2D();
	m_LinePt2Geo = Point2D();
	m_CurLineRender.DeleteVBO();
	if(m_bActStripResort)
	{
		m_pDoc->m_pMain->UpdateStatusLabel("手动选择航带影像...");
		MessageBox("请在测区范围内绘制多边形，手动选择属于同一条航带的影像。","提示");
	}
	Invalidate();
}


void CsswUAVFlyQuaSysView::OnUpdateButtonByhandResortStrip(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(m_bResortStripByHand);
}


void CsswUAVFlyQuaSysView::OnButtonSemiautoResortStrip()
{
	// TODO: 在此添加命令处理程序代码
	if(m_bResortStripSemiAuto) return;
	m_bResortStripByHand=false;
	m_bResortStripSemiAuto=true;
	vector<Point2D>().swap(m_vecCurPoint);
	vector<vector<Point2D>>().swap(m_vvCurPoint);
	m_CurLineRender.DeleteVBO();
	for (int i = 0; i<m_vecCurLineRender.size(); i++)
	{
		m_vecCurLineRender[i].DeleteVBO();
	}
	if(m_bActStripResort)
	{
		m_pDoc->m_pMain->UpdateStatusLabel("设置航带方向...");
		MessageBox("请在测区范围外选择两点，绘制一条与航向平行的直线，辅助航带排列。","提示");
	}

	Invalidate();
}


void CsswUAVFlyQuaSysView::OnUpdateButtonSemiautoResortStrip(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(m_bResortStripSemiAuto);
}


BOOL CsswUAVFlyQuaSysView::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if(pMsg->hwnd!=this->m_hWnd) return CView::PreTranslateMessage(pMsg);
	if(!m_bActStripResort&&!m_bActSetInvalidImg) return CView::PreTranslateMessage(pMsg);
	switch(pMsg->message)
	{
	case WM_KEYDOWN:{
		switch(pMsg->wParam)
		{
		case VK_DELETE:
			if(m_vecCurPoint.size()>0)
			{
				if(m_vecCurPoint[m_vecCurPoint.size()-1]==Point2D())
				{
					m_vecCurPoint.erase(m_vecCurPoint.end()-1);
					m_vecCurPoint.erase(m_vecCurPoint.end()-1);
				}
				else m_vecCurPoint.erase(m_vecCurPoint.end()-2);
				POINT point; GetCursorPos(&point);
				ScreenToClient(&point);
				OnMouseMove(0,point);
			}
			else if(m_LinePt1Geo!=Point2D())
			{
				m_LinePt1Geo = Point2D();
				m_CurLineRender.DeleteVBO();
				Invalidate();
			}else
			{
				if(m_vecCurLineRender.size()==0) break;
				m_vvCurPoint.erase(m_vvCurPoint.end()-1);
				m_vecCurLineRender[m_vecCurLineRender.size()-1].DeleteVBO();
				m_vecCurLineRender.erase(m_vecCurLineRender.end()-1);
					Invalidate();
			}
			break;
		case VK_ESCAPE:
			if(m_vecCurPoint.size()>0)
			{
				m_CurLineRender.DeleteVBO();
				vector<Point2D>().swap(m_vecCurPoint);
				Invalidate();
			}
			else if(m_LinePt1Geo!=Point2D())
			{
				m_LinePt1Geo = Point2D();
				m_CurLineRender.DeleteVBO();
				Invalidate();
			}
			break;
		default:
			break;
		}
	default:
		break;
					}
	}
	return CView::PreTranslateMessage(pMsg);
}


void CsswUAVFlyQuaSysView::OnInitialUpdate()
{
	CView::OnInitialUpdate();
	m_pDoc=GetDocHand();
	// TODO: 在此添加专用代码和/或调用基类
}

void CsswUAVFlyQuaSysView::OnCheckViewImgName()
{
	// TODO: 在此添加命令处理程序代码
	m_bViewImgName=!m_bViewImgName;
	Invalidate();
}


void CsswUAVFlyQuaSysView::OnUpdateCheckViewImgName(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
pCmdUI->Enable(m_pDoc->m_bPrjLoaded);
	pCmdUI->SetCheck(m_bViewImgName);
}


void CsswUAVFlyQuaSysView::OnCheckViewImgPos()
{
	// TODO: 在此添加命令处理程序代码
	m_bViewImgPos=!m_bViewImgPos;
	Invalidate();
}


void CsswUAVFlyQuaSysView::OnUpdateCheckViewImgPos(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable(m_pDoc->m_bPrjLoaded);
		pCmdUI->SetCheck(m_bViewImgPos);
}

void CsswUAVFlyQuaSysView::OnCheckViewStripLabel()
{
	// TODO: 在此添加命令处理程序代码
	m_bViewStripLabel=!m_bViewStripLabel;
	Invalidate();
}

void CsswUAVFlyQuaSysView::OnUpdateCheckViewStripLabel(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->Enable(m_pDoc->m_bPrjLoaded&&m_bStripResorted&&m_bViewImgName);
	pCmdUI->SetCheck(m_bViewStripLabel);
}


void CsswUAVFlyQuaSysView::OnCheckViewCqArea()
{
	// TODO: 在此添加命令处理程序代码
	m_bViewCqArea = !m_bViewCqArea;
	Invalidate();
}


void CsswUAVFlyQuaSysView::OnUpdateCheckViewCqArea(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	if(!m_pDoc->m_bPrjLoaded) pCmdUI->Enable(false);
	else 
	{
		if(m_vecCqExtRingRender.size()==0) pCmdUI->Enable(false);
		else pCmdUI->Enable(m_vecCqExtRingRender[0].IsVBOFilled());
	}
	pCmdUI->SetCheck(m_bViewCqArea);
}


void CsswUAVFlyQuaSysView::OnCheckViewImgArea()
{
	// TODO: 在此添加命令处理程序代码
	m_bViewImgArea = !m_bViewImgArea;
	Invalidate();
}


void CsswUAVFlyQuaSysView::OnUpdateCheckViewImgArea(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	if(!m_pDoc->m_bPrjLoaded) pCmdUI->Enable(false);
	else 
	{
		if(m_vecImgVertexRender.size()==0) pCmdUI->Enable(false);
		else pCmdUI->Enable(m_vecImgVertexRender[0].IsVBOFilled());
	}
	pCmdUI->SetCheck(m_bViewImgArea);
}


void CsswUAVFlyQuaSysView::OnCheckViewMissArea()
{
	// TODO: 在此添加命令处理程序代码
	m_bViewMissArea = !m_bViewMissArea;
	Invalidate();
}


void CsswUAVFlyQuaSysView::OnUpdateCheckViewMissArea(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	if(!m_pDoc->m_bPrjLoaded) pCmdUI->Enable(false);
	else 
	{
		if(m_vecMissAreaRender.size()==0) pCmdUI->Enable(false);
		else pCmdUI->Enable(m_vecMissAreaRender[0].IsVBOFilled());
	}
	pCmdUI->SetCheck(m_bViewMissArea);
}


void CsswUAVFlyQuaSysView::OnButtonSetImgValidState()
{
	// TODO: 在此添加命令处理程序代码
	if(!m_bActSetInvalidImg)
	{
		DeleteCurLineInfo();
		Invalidate();
		if(m_bSetImg2Invalid)
		{
			m_pDoc->m_pMain->UpdateStatusLabel("设置无效影像...");
				switch(MessageBox("请在测区范围内绘制多边形，手动选择需修改为“无效”的影像。\n选择完毕后，点此按钮完成设置。","提示",MB_OKCANCEL))
				{
				case IDOK:
					break;
				default:
					m_pDoc->m_pMain->UpdateStatusLabel("就绪");
					return;
				}
		}
		else
		{
			m_pDoc->m_pMain->UpdateStatusLabel("设置有效影像...");
			switch(MessageBox("请在测区范围内绘制多边形，手动选择需修改为“有效”的影像。\n选择完毕后，点此按钮完成设置。","提示",MB_OKCANCEL))
			{
			case IDOK:
				break;
			default:
				m_pDoc->m_pMain->UpdateStatusLabel("就绪");
				return;
			}
		} 
		FunOutPutWndLog(m_pDoc->m_pMain,"影像有效性设置激活！",true);

	}
	else
	{
		if(m_vvCurPoint.size()!=0)
		{
			if(MessageBox("是否按照绘图设置影像有效性？","提示",MB_YESNO)==IDYES)
			{
				HANDLE hThread;DWORD IdThread;
				hThread = CreateThread(NULL, 0,
					(LPTHREAD_START_ROUTINE)MultiThreadSetImgValidState,
					(LPVOID*)this, 0, &IdThread);
				m_bStripResorted = true;
			}
		}
		vector<Point2D>().swap(m_vecCurPoint);
		//		DeleteCurLineInfo();		
		m_pDoc->m_pMain->UpdateStatusLabel("就绪");

	}
	m_bActSetInvalidImg =!m_bActSetInvalidImg;
	if(m_bActSetInvalidImg) 
	{
		Invalidate();
		m_bActStripResort = false;
	}
}


void CsswUAVFlyQuaSysView::OnUpdateButtonSetInvalidImage(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	if(!m_pDoc->m_bPrjLoaded) pCmdUI->Enable(m_pDoc->m_bPrjLoaded);
	else pCmdUI->Enable(!m_pDoc->m_bInRunning);
	pCmdUI->SetCheck(m_bActSetInvalidImg);
}


void CsswUAVFlyQuaSysView::OnButtonSetInvalid()
{
	// TODO: 在此添加命令处理程序代码
	m_bSetImg2Invalid = true;
	m_bSetImg2Valid = false;
	if(m_bActSetInvalidImg)
	{
		m_pDoc->m_pMain->UpdateStatusLabel("设置无效影像...");
		MessageBox("请在测区范围内绘制多边形，手动选择需修改为“无效”的影像。\n选择完毕后，点此按钮完成设置。","提示");
	}
}


void CsswUAVFlyQuaSysView::OnUpdateButtonSetInvalid(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(m_bSetImg2Invalid);
}


void CsswUAVFlyQuaSysView::OnButtonSetValid()
{
	// TODO: 在此添加命令处理程序代码
	m_bSetImg2Valid = true;
	m_bSetImg2Invalid=false;
	if(m_bActSetInvalidImg)
	{
		m_pDoc->m_pMain->UpdateStatusLabel("设置有效影像...");
		MessageBox("请在测区范围内绘制多边形，手动选择需修改为“有效”的影像。\n选择完毕后，点此按钮完成设置。","提示");
	}
}


void CsswUAVFlyQuaSysView::OnUpdateButtonSetValid(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	pCmdUI->SetCheck(m_bSetImg2Valid);
}
