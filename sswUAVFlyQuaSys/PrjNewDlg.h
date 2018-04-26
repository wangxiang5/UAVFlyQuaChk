#pragma once
#include "afxcmn.h"
#include "MyListCtrlEx.h"
#include "MyFunctions.h"
#include "afxeditbrowsectrl.h"
#include "MyMFCEditBrowseCtrl.h"
// CPrjNewDlg 对话框

static enum ImgPosMatchDirectiong
{
	IMG2POS=0,
	POS2IMG=1
};
class CPrjNewDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPrjNewDlg)

public:
	CPrjNewDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPrjNewDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_PRJ_NEW };
public:
	void ImgAndPosMatch();

	vector<CString> GetImgFilePath(){return m_vecAllImgPath;}
	vector<SSWstuPosInfo> GetPosInfo(){return m_vecAllPosInfo;}
	vector<int> GetImgIdx2PosIdx(){return m_vecImgIdxToPosIdx;}
	SSWstuCmrInfo GetCmrInfo(){return m_vecAllCmrInfo[m_nCmrIdxSelected];}
	SSWstuSysConfigPara GetFlyQuaPara(){return m_SysCfgPara;}
	CString GetPrjFilePath(){return m_strPrjFilePath;}
	CString GetCmrFilePath(){return m_strCmrFilePath;}
	int GetNumImgPosed(){return m_nImgPosed;}
	bool GetCmrInfo(SSWstuCmrInfo &cmr,bool &bCmrFile)
	{
		bCmrFile=m_bCmrOk;
		if(m_nCmrIdxSelected>=0)
		{
			cmr = m_vecAllCmrInfo[m_nCmrIdxSelected];
			return true;
		}
		return false;
	}

	vector<CsswUAVImage> LoadImg2Prj();
protected:
	bool FunLoadImgFile();
	void MatchImg2Pos();
	void MatchPos2Img();

	void InitItemPos();
	void ResetItemPos();
	POINT old;
	void InitCmrList();
	void InitImgList();
	void FillCmrList();
	void UnionImgNew2AllNoRepeat();
	void UnionCmrNew2AllNoRepeat();
	void UnionPosNew2AllNoRepeat();
	void DeleteInvalidImgAndPos();
	void ClearPosInfo()
	{
		vector<SSWstuPosInfo>().swap(m_vecAllPosInfo);
		vector<SSWstuPosInfo>().swap(m_vecNewPosInfo);
		m_vecImgIdxToPosIdx.assign(m_vecImgIdxToPosIdx.size(),InvalidValue);
		m_bPosOk=false;
		m_nImgPosed=0;
	}
	void ClearCmrInfo()
	{
		vector<SSWstuCmrInfo>().swap(m_vecAllCmrInfo);
		vector<SSWstuCmrInfo>().swap(m_vecNewCmrInfo);
		m_nCmrIdxSelected=InvalidValue;
		m_bCmrOk=false;
	}
	void ClearImgInfo()
	{
		vector<CString>().swap(m_vecNewImgPath);
		vector<CString>().swap(m_vecAllImgPath);
		vector<CString>().swap(m_vecAllImgName);
		vector<int>().swap(m_vecImgIdxToPosIdx);
		m_bImgOk=false;
		m_nImgPosed=0;
	}
protected:
	SSWstuSysConfigData m_SysCfgData;
	SSWstuSysConfigPara m_SysCfgPara;
	vector<SSWstuCmrInfo> m_vecAllCmrInfo;
	vector<SSWstuCmrInfo> m_vecNewCmrInfo;
	vector<SSWstuPosInfo> m_vecAllPosInfo;
	vector<SSWstuPosInfo> m_vecNewPosInfo;
	vector<CString> m_vecNewImgPath;
	vector<CString> m_vecAllImgPath;
	vector<CString> m_vecAllImgName;
	vector<int> m_vecImgIdxToPosIdx;
	int m_nImgPosed;
	bool m_bCmrOk;
	bool m_bPosOk;
	bool m_bImgOk;
	int m_nCmrIdxSelected;

	ImgPosMatchDirectiong m_eImgPosMatchDirection;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_strPrjFilePath;
	CString m_strCmrFilePath;
	CString m_strPosFilePath;
	CString m_strImgFolderPath;
	CListCtrl m_ListCmrInfo;
	CMyListCtrlEx m_ListImgInfo;
// 	BOOL m_bAddData;
// 	BOOL m_bSearchJpg;
// 	BOOL m_bSearchTif;
// 	BOOL m_bDeleteImgNoPos;
	virtual BOOL OnInitDialog();
	LRESULT OnMsgImgPosMatch(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEnChangeMfceditbrowseCmrFilepath();
	afx_msg void OnEnChangeMfceditbrowsePosFilepath();
	afx_msg void OnEnChangeMfceditbrowseImgfolder();
	afx_msg void OnGetdispinfoListImginfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClickListCmrinfo(NMHDR *pNMHDR, LRESULT *pResult);
	
//	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonChangePoscoordinate();
	BOOL m_bAutoManageImage;
	CMyMFCEditBrowseCtrl m_BrowseImgFolder;
};



