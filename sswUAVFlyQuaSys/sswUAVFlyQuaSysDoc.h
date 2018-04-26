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

// sswUAVFlyQuaSysDoc.h : CsswUAVFlyQuaSysDoc 类的接口
//


#pragma once
#include "MyFunctions.h"
class CMainFrame;
class CsswUAVFlyQuaSysView;

// class CsswUAVFlyQua;
// class CsswUAVImage;

struct stuOverlap
{
	int nIdx1;
	int nIdx2;
	double dOverlapX;
	double dOverlapY;
};
class CsswUAVFlyQuaSysDoc : public CDocument
{
protected: // 仅从序列化创建
	CsswUAVFlyQuaSysDoc();
	DECLARE_DYNCREATE(CsswUAVFlyQuaSysDoc)

protected:
	void ClearData();
	void UnionParaFormat();
public:
	bool PrjNew(CString strPath,vector<CsswUAVImage> &vecImgInfo, SSWstuSysConfigPara para, bool bWithCmr, bool bAutoManageImages);
// 特性
public:
	/**2016-12-28**/
	bool m_bCmrFile;
	float m_fOverlapRatio4;

	CsswUAVFlyQuaPrj m_FlyQuaPrj;
	CString m_strPrjFilePath;
	vector<CsswUAVImage> m_vecImgInfo;
	vector<CString> m_vecInvalidImgPath;
	CString m_strMatchAdjFolder;
	CString m_strImgListFilePath;
	CString m_strPosListFilePath;
	CString m_strMatchMatrixFilePath;
	CString m_strAdjReportFilePath;
	CString m_strFlyQuaRepPath;
	bool m_bTransXYInView;
	bool m_bPrjLoaded;
	bool m_bAutoManageImages;
	bool m_bInRunning;
	int m_nImgNum;
	double m_dMeanHei;
//	int m_nMatchPointNumThresold;
	SSWFLYQUAMETHOD m_eFlyQuaChkMethod;
	bool GetConfigInfo(SSWstuSysConfigData &data, SSWstuSysConfigPara &para);
	void InitViewRect();
	void FillPosRender();
	void UpdatePrjTree();
	void ReSortStrips();
	void ManageImgFile5();
	void ManageImgFile3();
// 操作
public:
	CsswUAVFlyQuaSysView* m_pView;
	CMainFrame* m_pMain;
// 重写
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// 实现
public:
	virtual ~CsswUAVFlyQuaSysDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 用于为搜索处理程序设置搜索内容的 Helper 函数
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	afx_msg void OnButtonPrjNew();
	afx_msg void OnButtonRunFlyquaCheck();
	afx_msg void OnUpdateButtonRunFlyquaCheck(CCmdUI *pCmdUI);
	afx_msg void OnButtonFlyquaReport();
	afx_msg void OnUpdateButtonFlyquaReport(CCmdUI *pCmdUI);
	afx_msg void OnButtonPrjOpen();
// 	afx_msg void OnComboCheckMethod();
// 	afx_msg void OnUpdateComboCheckMethod(CCmdUI *pCmdUI);
	afx_msg void OnUpdateButtonPrjNew(CCmdUI *pCmdUI);
	afx_msg void OnUpdateButtonPrjOpen(CCmdUI *pCmdUI);
	afx_msg void OnButtonPrjSave();
	afx_msg void OnUpdateButtonPrjSave(CCmdUI *pCmdUI);
	afx_msg void OnEditMeanHeight();
	afx_msg void OnUpdateEditMeanHeight(CCmdUI *pCmdUI);
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnButtonDataOrder();
	afx_msg void OnUpdateButtonDataOrder(CCmdUI *pCmdUI);
// 	afx_msg void OnEditMatchpointNumThresold();
// 	afx_msg void OnUpdateEditMatchpointNumThresold(CCmdUI *pCmdUI);
};
