#pragma once


// CThumbWnd
#include "sswViewerRender.h"

class CThumbWnd : public CDockablePane
{
	DECLARE_DYNAMIC(CThumbWnd)

public:
	CThumbWnd();
	virtual ~CThumbWnd();
public:
	void DeleteAllRender()
	{
		m_RepRender.DeleteVBO();
		m_AdjPtRender.DeleteVBO();
		for (int i = 0; i<m_vecImgRender.size(); i++)
		{
			m_vecImgRender[i].DeleteTexture();
		}
		vector<CsswImgRender>().swap(m_vecImgRender);
		m_bCreateTextures = false;
		m_bCreateAdjPtVBO = false;
		m_bDisplayAdjPtVBO = false;
		Invalidate();
	}
	CPoint ptGeo2Cln(Point2D& ptGeo);
	Point2D ptCln2Geo(CPoint& ptCln);
	Point2D ClnCenter2Geo(CPoint ptCln, Point2D ptGeo);	
	void ResetRectCln2Geo();

	void InitRepView();
	void SaveView2Img(CRect RectSaveView, CString& strSavePath);
	bool m_bSaveView2Img;
public:
	HPALETTE      m_hPalette; //opengl调色表
	CClientDC*    m_pDC;      //opengl绘图设备
	CsswUAVFlyQuaSysEngDoc* m_pDoc;
	CRect  m_RectCln;
	CRectD m_RectCln2Geo;
	CRectD m_RectAdjPtGeo;
	Point2D m_ptClnCenter2Geo;
	Point2D m_ptMBtnDnGeo;
	float m_fZoomRateCln2Geo;
	float m_fZoomRateCln2GeoMin;
	float m_fZoomRateCln2GeoMax;
	CsswViewerRender m_AdjPtRender;
	vector<CsswImgRender> m_vecImgRender;
	vector<vector<Point3D>> m_vecImgVertex4;
	bool m_bCreateTextures;
	bool m_bCreateAdjPtVBO;
	bool m_bDisplayAdjPtVBO;

	CsswViewerRender m_RepRender;


public:
	CButton m_ColorBtn1;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnMsgCreateImgTextures(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMsgCreateAdjPtsVBO(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMsgFillRepVBO(WPARAM wParam, LPARAM lParam);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
};


