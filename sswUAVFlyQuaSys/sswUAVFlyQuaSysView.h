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

// sswUAVFlyQuaSysView.h : CsswUAVFlyQuaSysView 类的接口
//

#pragma once
#include "sswViewerRender.h"
#include "MyFunctions.h"
class CsswUAVFlyQuaSysView : public CView
{
protected: // 仅从序列化创建
	CsswUAVFlyQuaSysView();
	DECLARE_DYNCREATE(CsswUAVFlyQuaSysView)

// 特性
public:
	CsswUAVFlyQuaSysDoc* GetDocument() const;
	double m_dLineTheta;
	CsswUAVFlyQuaSysDoc *m_pDoc;
protected:
	inline void CalLineParas(Point2D pt1, Point2D pt2)
	{
		if (pt1.x == pt2.x)
		{
			m_LineParaB = 0;
			m_LineParaA = 1;
			m_LineParaC = -pt1.x;
		}
		else
		{
			m_LineParaB = 1;
			m_LineParaA = -(pt1.y -pt2.y) / (pt1.x - pt2.x);
			m_LineParaC = -m_LineParaA*pt1.x - pt1.y;
		}
	}
	inline void CalLineParas(void* p1, void* p2, int nDims, float &a, float &b, float &c)
	{
		if(nDims<2||nDims>3) return;
		CPoint pt1,pt2;
		if(nDims==2) 
			{
				pt1 = *(CPoint*)p1;
				pt2 = *(CPoint*)p2;
		}else
		{
			if(nDims==3)
			{
				pt1 = CPoint(((Point3D*)p1)->X, ((Point3D*)p1)->Y);
				pt2 = CPoint(((Point3D*)p2)->X, ((Point3D*)p2)->Y);
			}
		}
		if (pt1.x == pt2.x)
		{
			b = 0;
			a = 1;
			c = -pt1.x;
		}
		else
		{
			b = 1;
			a = -1.0*(pt1.y -pt2.y) / (pt1.x - pt2.x);
			c = -1.0*a*pt1.x - pt1.y;
		}
	}
	inline double LineX2Y(double x, float a,float b, float c)
	{
		if(b==0) return 0;
		return -(a*x+c)/b;
	}
	inline double LineY2X(double y, float a, float b, float c)
	{
		if(a==0) return 0;
		return -(b*y+c)/a;
	}
// 操作
public:
	CRect  m_RectCln;
	CRectD m_RectCln2Geo;
	CRectD m_RectPosGeo;
	CRectD m_RectCqViewGeo;
	Point2D m_ptClnCenter2Geo;
	Point2D m_ptMBtnDnGeo;
	float m_fZoomRateCln2Geo;
	float m_fZoomRateCln2GeoMin;
	float m_fZoomRateCln2GeoMax;
	HPALETTE      m_hPalette; //opengl调色表
	CClientDC*    m_pDC;      //opengl绘图设备
	HGLRC         m_pRC;
	//手绘航带方向
	Point2D m_LinePt1Geo;
	Point2D m_LinePt2Geo;
	double  m_LineParaA;
	double  m_LineParaB;
	double  m_LineParaC;
	CsswViewerRender m_KeyNodePointRender;
	vector<CsswViewerRender> m_vecKeyNodeLineRender;
	int m_nNodeLevel;
	int m_nKeyNodeNum;
	CsswViewerRender m_BkgrdRender;   //背景
	CsswViewerRender m_PosRender;
	CsswViewerRender m_PosRectRender;
	CsswViewerRender m_SelectedStripRender;
	CsswViewerRender m_CurLineRender;
	CsswViewerRender m_StripDirectLineRender;
	vector<CsswViewerRender> m_vecCurLineRender;
	vector<CsswViewerRender> m_vecMissAreaRender;
	CsswViewerRender m_CqAreaRender;
	CsswViewerRender m_ImgAreaRender;
	CsswImgRender    m_ImgOverlapRender;
	vector<CsswViewerRender> m_vecImgVertexRender;
	vector<CsswViewerRender> m_vecCqExtRingRender;
	vector<CsswViewerRender> m_vecSelectedImgRender;
	vector<Point2D> m_vecCurPoint;
	vector<vector<Point2D>> m_vvCurPoint;
	bool m_bViewImgPos;
	bool m_bViewImgName;
	bool m_bViewCqArea;
	bool m_bViewImgArea;
	bool m_bViewMissArea;
	bool m_bDisplayCurLine;
	bool m_bStripResorted;   //航带已排列
	bool m_bViewStripLabel;
	bool m_bActStripResort;    //激活排列航带
	bool m_bResortStripByHand; //手动排列航带
	bool m_bResortStripSemiAuto;//半自动排列航带
	bool m_bActSetInvalidImg;  //激活标记无效影像
	bool m_bSetImg2Valid;      //设置为无效
	bool m_bSetImg2Invalid;    //设置为有效

	int m_nStripIdxSelected;
public:
	void ShowBackGround();
	void ShowMissArea();
	void ShowSeletctedStrip();
	void DeleteCurLineInfo()
	{
		vector<Point2D>().swap(m_vecCurPoint);
		m_CurLineRender.DeleteVBO();
		for (int i = 0; i<m_vecCurLineRender.size(); i++)
		{
			m_vecCurLineRender[i].DeleteVBO();
		}
		vector<CsswViewerRender>().swap(m_vecCurLineRender);
		vector<vector<Point2D>>().swap(m_vvCurPoint);
	}
	void DeleteViewVBO()
	{
		for(int i = 0; i<m_vecCqExtRingRender.size(); i++) 
		{		
			m_vecCqExtRingRender[i].DeleteVBO();
		}
		vector<CsswViewerRender>().swap(m_vecCqExtRingRender);
		for (int i = 0; i<m_vecMissAreaRender.size(); i++)
		{
			m_vecMissAreaRender[i].DeleteVBO();
		}
		vector<CsswViewerRender>().swap(m_vecMissAreaRender);
		for (int i = 0; i<m_vecImgVertexRender.size(); i++)
		{
			m_vecImgVertexRender[i].DeleteVBO();
		}
		vector<CsswViewerRender>().swap(m_vecImgVertexRender);
		m_PosRender.DeleteVBO();
		m_PosRectRender.DeleteVBO();
		m_ImgAreaRender.DeleteVBO();
		m_KeyNodePointRender.DeleteVBO();
	}
	CPoint ptGeo2Cln(Point2D& ptGeo);
	Point2D ptCln2Geo(CPoint& ptCln);
	Point2D ClnCenter2Geo(CPoint ptCln, Point2D ptGeo);	
	void ResetRectCln2Geo();
	void ReInitCqView();
	BYTE* m_pData;
	bool m_bSaveView2Img;
	CString m_strCqViewImagePath;
	CString m_strPosErrViewImagePath;
	CString m_strOverlapYErrViewImagePath;
	CString m_strOverlapXErrViewImagePath;
	CString m_strPitchErrViewImagePath;
	CString m_strYawErrViewImagePath;
	CString m_strPtNumErrViewImagePath;
	void SaveView2Img(CString& strSavePath);
protected:
	void CreateCqAreaRenderVBO(vector<Point3D> &vecCqAreaVertex, CsswViewerRender &render);
	void CreateImgAreaRenderVBO(vector<Point3D> &vecImgAreaVertex,CsswViewerRender &render);
	void CreateMissAreaRenderVBO(vector<vector<Point3D>> &vvMissAreaVertex,vector<CsswViewerRender> &vRender);
	void CreateOverlapRenderTex(vector<vector<Point3D>> &vvImgArea, CsswImgRender &render,float &fOverlap4Ratio);
	void CreateKeyNodeRenterVBO();
	void FillKeyNodeRenderVBO();
	// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CsswUAVFlyQuaSysView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
public:
	afx_msg LRESULT onMseCreateSelectedRender(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT onMsgCreateMissAreaRender(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMsgReSortStrips(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMsgSetImgInvalid(WPARAM wParam, LPARAM lParam);
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCheckViewImgName();
	afx_msg void OnUpdateCheckViewImgName(CCmdUI *pCmdUI);
	afx_msg void OnCheckViewImgPos();
	afx_msg void OnUpdateCheckViewImgPos(CCmdUI *pCmdUI);
	afx_msg void OnButtonSortStrip();
	afx_msg void OnUpdateButtonSortStrip(CCmdUI *pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCheckViewStripLabel();
	afx_msg void OnUpdateCheckViewStripLabel(CCmdUI *pCmdUI);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//afx_msg void OnButtonSaveView();
	afx_msg void OnButtonByhandResortStrip();
	afx_msg void OnUpdateButtonByhandResortStrip(CCmdUI *pCmdUI);
	afx_msg void OnButtonSemiautoResortStrip();
	afx_msg void OnUpdateButtonSemiautoResortStrip(CCmdUI *pCmdUI);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnInitialUpdate();
	afx_msg void OnCheckViewCqArea();
	afx_msg void OnUpdateCheckViewCqArea(CCmdUI *pCmdUI);
	afx_msg void OnCheckViewImgArea();
	afx_msg void OnUpdateCheckViewImgArea(CCmdUI *pCmdUI);
	afx_msg void OnCheckViewMissArea();
	afx_msg void OnUpdateCheckViewMissArea(CCmdUI *pCmdUI);
	afx_msg void OnButtonSetImgValidState();
	afx_msg void OnUpdateButtonSetInvalidImage(CCmdUI *pCmdUI);
	afx_msg void OnButtonSetInvalid();
	afx_msg void OnUpdateButtonSetInvalid(CCmdUI *pCmdUI);
	afx_msg void OnButtonSetValid();
	afx_msg void OnUpdateButtonSetValid(CCmdUI *pCmdUI);
	afx_msg void OnPaint();
};

#ifndef _DEBUG  // sswUAVFlyQuaSysView.cpp 中的调试版本
inline CsswUAVFlyQuaSysDoc* CsswUAVFlyQuaSysView::GetDocument() const
   { return reinterpret_cast<CsswUAVFlyQuaSysDoc*>(m_pDocument); }
#endif

