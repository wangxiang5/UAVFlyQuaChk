// DataChartWnd.cpp : 实现文件
//

#include "stdafx.h"
#include "sswUAVFlyQuaSysEng.h"
#include "DataChartWnd.h"
//#include "MyFunctions.h"
#include "MainFrm.h"
#include "sswUAVFlyQuaSysEngDoc.h"
#include <omp.h>
#include "ChartAxisLabel.h"
#include <algorithm>
// CDataChartWnd

IMPLEMENT_DYNAMIC(CDataChartWnd, CDockablePane)

CDataChartWnd::CDataChartWnd()
{
	m_pLineSerie = NULL;
	m_pDoc = NULL;
}

CDataChartWnd::~CDataChartWnd()
{
}


BEGIN_MESSAGE_MAP(CDataChartWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_COMMAND(IDC_COMBO_DATACHART_EXTEND, &CDataChartWnd::OnComboDatachartExtend)
	ON_CBN_SELCHANGE(IDC_COMBO_DATACHART_EXTEND,&CDataChartWnd::OnSelComboDatachartExtendChange)
	ON_MESSAGE(ID_MSG_FILL_DATACHART,onMsgFillDataChart)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

LRESULT CDataChartWnd::onMsgFillDataChart(WPARAM wParam, LPARAM lParam)
{
	OnComboDatachartExtend();
	return 1;
}

// CDataChartWnd 消息处理程序

void CDataChartWnd::FillComboxCtrl()
{
	if(m_ComboDataExtend.GetSafeHwnd()==NULL) return;
	m_ComboDataExtend.AddString("Forward-Overlap");
	m_ComboDataExtend.AddString("Side-Overlap");
	m_ComboDataExtend.AddString("POS Residual");
	m_ComboDataExtend.AddString("Image Pitch");
	m_ComboDataExtend.AddString("Image Yaw");
	m_ComboDataExtend.AddString("Point Number");
}
void CDataChartWnd::CreateDataChartCtrl()
{
	CRect rectDummy;
	rectDummy.SetRectEmpty();
	if(!m_DataChart.Create(this,rectDummy,IDC_DATA_CHART));
	m_DataChart.ShowWindow(true);
	m_pAxisL = m_DataChart.GetLeftAxis();
	m_pAxisL->SetMinMax(0,0);
	//pAxis->SetAxisType(CChartAxis::atStandard);
	m_pAxisL->SetAutomatic(true);
	m_pAxisL->GetLabel()->SetText("Overlap_Y");
	m_pAxisB = m_DataChart.GetBottomAxis();
	m_pAxisB->SetMinMax(0,0);
	//pAxis->SetAxisType(CChartAxis::atStandard);
	m_pAxisB->SetAutomatic(true);
	m_pAxisB->GetLabel()->SetText("Image ID");

	m_pLineSerie = m_DataChart.AddSerie(0);
	m_pLineSerie->SetSeriesOrdering(CChartSerie::soXOrdering);  //设置为无序
}
void CDataChartWnd::UpdateFonts()
{
	m_ComboDataExtend.SetFont(&afxGlobalData.fontRegular);
	m_DataChart.SetFont(&afxGlobalData.fontRegular);
}
void CDataChartWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}
	CRect rectClient,rectCombo;
	GetClientRect(rectClient);
	m_ComboDataExtend.GetWindowRect(&rectCombo);
	int cyCmb = rectCombo.Size().cy; int nGap = 5;
	m_ComboDataExtend.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), 300, SWP_NOACTIVATE | SWP_NOZORDER);
	m_DataChart.SetWindowPos(NULL, rectClient.left, rectClient.top+cyCmb+nGap-nGap, rectClient.Width(), rectClient.Height()-cyCmb, SWP_NOACTIVATE | SWP_NOZORDER);
}
int CDataChartWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码
	if(m_pDoc==NULL) m_pDoc = GetDocHand();
	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 创建质检参数类型Combox:
	const DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER /*| CBS_SORT */| WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	if (!m_ComboDataExtend.Create(dwStyle, rectDummy, this, IDC_COMBO_DATACHART_EXTEND))
	{
		TRACE0("Para combox list error\n");
		return -1;      // 未能创建
	}
	CreateDataChartCtrl();
	FillComboxCtrl();
	m_ComboDataExtend.SetCurSel(0);
	m_eQualityType = Q_OVERLAP_Y;
	AdjustLayout();
	UpdateFonts();

	return 0;
}

void CDataChartWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
	if(m_pDoc==NULL) 
		m_pDoc = GetDocHand();
	// TODO: 在此处添加消息处理程序代码
}

BOOL CDataChartWnd::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
// 
// 	return CDockablePane::OnEraseBkgnd(pDC);
	return TRUE;
}

void CDataChartWnd::FillDataChart(double *&pValue, int num, double dMax, double dMin, SSWQUALITYTYPE eType)
{
// 	m_DataChart.RemoveAllSeries();
// 	m_pLineSerie = NULL;
	if(m_pLineSerie==NULL)
	{
		m_pLineSerie=m_DataChart.AddSerie(0);
		m_pLineSerie->SetSeriesOrdering(CChartSerie::soXOrdering);
	}

	CString strLeftAxis;
	switch(eType)
	{
	case Q_OVERLAP_Y:
		strLeftAxis = "Forward-Overlap";
		break;
	case Q_OVERLAP_X:
		strLeftAxis = "Side-Overlap";
		break;
	case Q_POS_ERR:
		strLeftAxis = "POS_Err-m";
		break;
	case Q_PITCH:
		strLeftAxis = "Pitch-deg";
		break;
	case Q_YAW:
		strLeftAxis = "Yaw-deg";
		break;
	case Q_POINT_NUM:
		strLeftAxis = "PointNum";
		break;
	}
	m_pAxisL->SetMinMax(dMin-fabs(dMin*0.1),dMax+fabs(dMax*0.1));
	m_pAxisL->GetLabel()->SetText(strLeftAxis.GetString());
	m_pAxisB->SetMinMax(0,num);
	CString strBottomAxis = "Image ID";
	m_pAxisB->GetLabel()->SetText(strBottomAxis.GetString());
	m_pLineSerie->ClearSerie();
	
	double *pX = new double[num]; for(int i = 0; i<num; i++) *(pX+i)=i;
	double *pY = pValue;
	m_pLineSerie->SetPoints(pX,pY,num);
	delete[]pX;
	m_DataChart.EnableRefresh(true);
}

void CDataChartWnd::OnComboDatachartExtend()
{
	// TODO: 在此添加命令处理程序代码
	if(m_pDoc==NULL) return;
	CsswUAVFlyQuaSysEngDoc *pDoc = m_pDoc;
	double *pValue = NULL, dMax = -1e10, dMin = 1e10; int num = 0;
	switch(m_ComboDataExtend.GetCurSel())
	{
	case 2:{   //PosErr
		m_eQualityType = Q_POS_ERR;
		vector<SSWstuPosInfo> vecPosErr = pDoc->m_FlyQuaPrj.GetImgPosErr();
		vector<double> vecValue = pDoc->m_FlyQuaPrj.GetImgPosDisErr();
		num = vecValue.size();
		if(num>0)
		{
			dMax = *max_element(vecValue.begin(),vecValue.end());
			dMin = *min_element(vecValue.begin(),vecValue.end());
			//	if(num == 0) {num++; vecValue.push_back(0);}
			pValue = new double[num];
			for (int i = 0; i<num; i++)
			{
				*(pValue+i)=vecValue[i];
			}
			dMax = *max_element(vecValue.begin(),vecValue.end());
			dMin = *min_element(vecValue.begin(),vecValue.end());
			vector<SSWstuPosInfo>().swap(vecPosErr);
			vector<double>().swap(vecValue);
		}
		break;
		   }
	case 0:{//OverlapY
		m_eQualityType = Q_OVERLAP_Y;
		vector<vector<double>> vvOverlapY = pDoc->m_FlyQuaPrj.GetImgOverlapInsStrip();
		pValue = new double[pDoc->m_FlyQuaPrj.GetValidImgNum()];
		for (int i = 0; i<vvOverlapY.size(); i++)
		{
			for (int j = 0; j<vvOverlapY[i].size(); j++)
			{
				*(pValue+num) = vvOverlapY[i][j];
				num++;
				dMax = max(vvOverlapY[i][j],dMax);
				dMin = min(vvOverlapY[i][j],dMin);
			}
		}
		vector<vector<double>>().swap(vvOverlapY);
		break;
		   }
	case 1:{//OverlapX
		m_eQualityType = Q_OVERLAP_X;
		vector<vector<double>> vvOverlapX = pDoc->m_FlyQuaPrj.GetImgOverlapBetStrip();
		pValue = new double[pDoc->m_FlyQuaPrj.GetValidImgNum()];
		for (int i = 0; i<vvOverlapX.size(); i++)
		{
			for (int j = 0; j<vvOverlapX[i].size(); j++)
			{
				*(pValue+num) = vvOverlapX[i][j];
				num++;
				dMax = max(vvOverlapX[i][j],dMax);
				dMin = min(vvOverlapX[i][j],dMin);
			}
		}
		vector<vector<double>>().swap(vvOverlapX);
		break;
		   }
	case 3:{//Pitch
		m_eQualityType = Q_PITCH;
		vector<double> vecPitch = pDoc->m_FlyQuaPrj.GetImgPitch();
		num=vecPitch.size();
		pValue = new double[num];
		for (int i = 0; i<vecPitch.size(); i++)
		{
				*(pValue+i) = vecPitch[i];
		}
		dMax = *max_element(vecPitch.begin(),vecPitch.end());
		dMin = *min_element(vecPitch.begin(),vecPitch.end());
		vector<double>().swap(vecPitch);
		break;
		   }
	case 4:{//Yaw
		m_eQualityType = Q_YAW;
		vector<vector<double>> vvImgYaw = pDoc->m_FlyQuaPrj.GetStripImgYaw();
		pValue = new double[pDoc->m_FlyQuaPrj.GetValidImgNum()];
		for (int i = 0; i<vvImgYaw.size(); i++)
		{
			for (int j = 0; j<vvImgYaw[i].size(); j++)
			{
				*(pValue+num) = vvImgYaw[i][j];
				num++;
				dMax = max(vvImgYaw[i][j],dMax);
				dMin = min(vvImgYaw[i][j],dMin);
			}
		}
		vector<vector<double>>().swap(vvImgYaw);
		break;
		   }
	case 5:{//PointNum
			m_eQualityType = Q_POINT_NUM;
			vector<int> vecPtNumInImg = pDoc->m_FlyQuaPrj.GetImgPointNum();
			num=vecPtNumInImg.size();
			pValue = new double[num];
			for (int i = 0; i<vecPtNumInImg.size(); i++)
			{
				*(pValue+i) = vecPtNumInImg[i];
			}
			dMax = *max_element(vecPtNumInImg.begin(),vecPtNumInImg.end());
			dMin = *min_element(vecPtNumInImg.begin(),vecPtNumInImg.end());
			vector<int>().swap(vecPtNumInImg);
			break;
		   }
	default:
		break;
	}
	if(pValue!=NULL)
	{
		FillDataChart(pValue,num,dMax,dMin,m_eQualityType);
		delete[]pValue;
		m_pDoc->m_pMain->m_wndImgAndPt.SendMessage(ID_MSG_FILL_REPRENDER,0,(LPARAM)m_pDoc);
	}


}
void CDataChartWnd::OnSelComboDatachartExtendChange()
{
	OnComboDatachartExtend();
}


void CDataChartWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	// TODO: 在此处添加消息处理程序代码
}
