#pragma once


// CMyDialogEx 对话框

class CMyDialogEx : public CDialogEx
{
	DECLARE_DYNAMIC(CMyDialogEx)

public:
	CMyDialogEx(CWnd* pParent = NULL);   // 标准构造函数
	CMyDialogEx(UINT nIDTemplate, CWnd *pParent = NULL);
	virtual ~CMyDialogEx();

// 对话框数据
	enum { IDD = IDD_MYDIALOGEX };
protected:
	void InitItemPos();
	void ResetItemPos();
	POINT old;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
};
