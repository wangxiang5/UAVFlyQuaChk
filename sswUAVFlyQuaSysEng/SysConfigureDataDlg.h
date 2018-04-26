#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "MyListCtrlEx.h"
#include "MyFunctions.h"
// CSysConfigureDataDlg 对话框

class CSysConfigureDataDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSysConfigureDataDlg)

public:
	CSysConfigureDataDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSysConfigureDataDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_SYSTEM_CONFIGURE_DATA };
protected:
	void InitItemPos();
	void ResetItemPos();
	POINT old;
	void InitItems();
	SSWUNITTYPE ComSel2Unit(int nSel)
	{
		switch (nSel)
		{
		case 0:return PIX;
		case 1:return UM;
		case 2:return MM;
		case 3:return CM;
		case 4:return M;
		default:return MM;
		}
	}
	SSWCOORDINATEFORMAT ComSel2CoorFormat(int nSel)
	{
		switch(nSel)
		{
		case 0:return LBH;
		case 1:return XYZ;
		default:return LBH;
		}
	}
	SSWANGLEFORMAT ComSel2AngleFormat(int nSel)
	{
		switch(nSel)
		{
		case 0:return DEG;
		case 1:return RAD;
		default:return DEG;
		}
	}
	SSWPOSIMGMATCHMETHOD ComSel2PosImgMatchMethod(int nSel)
	{
		switch(nSel)
		{
		case 0:return BY_NAME;
		case 1:return BY_ORDER;
		default:return BY_NAME;
		}
	}
public:
	void UpdateItems();
	void UpdateDatas();
	SSWstuSysConfigData m_SysCfgData;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CMyListCtrlEx m_ListCmrData;
	CMyListCtrlEx m_ListPosData;
	CComboBox m_ComboUnitX0Y0;
// 	CComboBox m_ComboUnitF;
// 	CComboBox m_ComboUnitPixSize;
	CComboBox m_ComboFormatCoordinate;
	CComboBox m_ComboFormatPosCoordinateLBH;
	CComboBox m_ComboFormatPosAngle;
	afx_msg void OnCbnSelchangeComboUnitX0y0();
// 	afx_msg void OnCbnSelchangeComboUnitF();
// 	afx_msg void OnCbnSelchangeComboUnitPixsize();
	afx_msg void OnCbnSelchangeComboFormatPoscoordinate();
	afx_msg void OnCbnSelchangeComboFormatPoscoordinatelbh();
	afx_msg void OnCbnSelchangeComboFormatPosangle();
	CComboBox m_ComboMethodPosImgMch;
	afx_msg void OnCbnSelchangeComboMethodPosimgmatch();
	afx_msg void OnClickListCmrdata(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClickListPosdata(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
