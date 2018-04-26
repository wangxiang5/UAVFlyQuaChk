#pragma once


// CSplashWnd

class CSplashWnd : public CWnd
{
	DECLARE_DYNAMIC(CSplashWnd)

public:
	CSplashWnd();
	virtual ~CSplashWnd();
	CBitmap m_bitmap_bak;
	CRect m_WndRect;
	bool CreateBmp();
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};


