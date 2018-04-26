#pragma once

// CSysConfigureParaDlg 对话框
#include "MyFunctions.h"
class CSysConfigureParaDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSysConfigureParaDlg)

public:
	CSysConfigureParaDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSysConfigureParaDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_SYSTEM_CONFIGURE_PARA };
protected:
	void InitItemPos();
	void ResetItemPos();
	POINT old;
public:
	void UpdateItems();
	void UpdateParas();
	SSWstuSysConfigPara m_SysCfgPara;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	float m_fMinOverlapY;
	float m_fMinOverlapX;
	float m_fMaxHeiDifInArea;
	float m_fMaxHeiDifLinkInStrip;
	float m_fMaxStripBlend;
	float m_fMaxPitch;
	float m_fMaxPitch10Per;
	float m_fMaxYaw;
	float m_fMaxYaw15Per;
	int   m_nMaxYaw20Num;
	float m_fImgValidJudgeTimesOfRms;
	int   m_nMatchPtNumThresold;
	float m_fMaxErrOfStationPane;
	float m_fRmsErrOfStationPane;
	float m_fMaxAngleErrRatio;
	float m_fMaxMissAreaRatio;
};
