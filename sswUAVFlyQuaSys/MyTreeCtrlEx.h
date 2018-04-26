#pragma once


// CMyTreeCtrlEx

class CMyTreeCtrlEx : public CTreeCtrl
{
	DECLARE_DYNAMIC(CMyTreeCtrlEx)

public:
	CMyTreeCtrlEx();
	virtual ~CMyTreeCtrlEx();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
};


