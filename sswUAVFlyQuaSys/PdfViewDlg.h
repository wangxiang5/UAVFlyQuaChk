#pragma once


// CPdfViewDlg 对话框
#include "WebBrowser2.h"
#include "WebBrowser.h"
//class CsswUAVFlyQuaSysDoc;
class CPdfViewDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPdfViewDlg)

public:
	CPdfViewDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPdfViewDlg();
	//CsswUAVFlyQuaSysDoc *m_pDoc;
	void ShowPdf(CString strPath);
	void ReleasePdf();
// 对话框数据
	enum { IDD = IDD_DIALOG_PDF_VIEW };
	CWebBrowser2 m_PdfWeb;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnMsgShowPdf(WPARAM wParam,LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
};
