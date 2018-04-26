#pragma once

#include "ChartCtrl.h"
#include "MyFunctions.h"
// CDataChartWnd
class CsswUAVFlyQuaSysEngDoc;
class CDataChartWnd : public CDockablePane
{
	DECLARE_DYNAMIC(CDataChartWnd)

public:
	CDataChartWnd();
	virtual ~CDataChartWnd();
	CsswUAVFlyQuaSysEngDoc *m_pDoc;
	CComboBox  m_ComboDataExtend;
		SSWQUALITYTYPE m_eQualityType;
	CChartCtrl m_DataChart;
	CChartSerie *m_pLineSerie;
	CChartAxis *m_pAxisL;
	CChartAxis *m_pAxisB;
	void FillDataChart(double *&pValue, int num, double dMax, double dMin, SSWQUALITYTYPE eType);
protected:
	void UpdateFonts();
	void AdjustLayout();
	void FillComboxCtrl();
	void CreateDataChartCtrl();
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT onMsgFillDataChart(WPARAM wParam, LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnComboDatachartExtend();
	afx_msg void OnSelComboDatachartExtendChange();
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};


