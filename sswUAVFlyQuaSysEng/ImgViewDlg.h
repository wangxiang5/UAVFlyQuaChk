#pragma once


// CImgViewDlg 对话框
#include "sswViewerRender.h"
class CImgViewDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CImgViewDlg)

public:
	CImgViewDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CImgViewDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_IMG_VIEW };
public:
	int m_nImgIdx2Show;
	int m_nShowImgStripIdx;
	int m_nShowImgIdxInStrip;
	bool m_bTextureReady;
	HPALETTE      m_hPalette; //opengl调色表
	CClientDC*    m_pDC;      //opengl绘图设备
	CRect  m_RectCln;
	CRectD m_RectImg;
	CRectD m_RectCln2Geo;
	Point2D m_ptClnCenter2Geo;
	Point2D m_ptMBtnDnGeo;
	float m_fZoomRateCln2Geo;
	float m_fZoomRateCln2GeoMin;
	float m_fZoomRateCln2GeoMax;
	CsswImgRender m_ImgRender;

	CPoint ptGeo2Cln(Point2D& ptGeo);
	Point2D ptCln2Geo(CPoint& ptCln);
	Point2D ClnCenter2Geo(CPoint ptCln, Point2D ptGeo);	
	void ResetRectCln2Geo();
	void InitImgViewInfo();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
