#pragma once
#include "sswUAVFlyQua.h"

#ifdef _DEBUG
#pragma comment(lib,"ZGKeypointLib_64d.lib")
#pragma comment(lib,"ZGSmartATLib_64d.lib")
#else
#pragma comment(lib,"ZGKeypointLib_64r.lib")
#pragma comment(lib,"ZGSmartATLib_64r.lib")
#endif

inline bool ReadLine(const char* mbuf, char*line, int maxlen)
{
	int len = 0;
	while(len<maxlen-1&&*(mbuf+len)!='\r')
	{
		*(line+len)=*(mbuf+len);
		len++;
	}
	*(line+len)='\0';
	if(strlen(line)<=0) return false;
	//	mbuf+=strlen(line)+2;
	return true;
}

class CMemTxtFile
{
public:
	CMemTxtFile();
	~CMemTxtFile();
public:
	bool OpenFile(CString strFilePath);
	void CloseFile();
	DWORD GetFileSize(){return m_dwFileSize;}
	char* GetFileHead(){return m_lpbFile;}
private:
	HANDLE m_hFile;
	HANDLE m_hFileMap;
	char* m_lpbFile;
	DWORD m_dwFileSize;
};


class CsswBundleAdjustment
{
public:
	CsswBundleAdjustment();
	~CsswBundleAdjustment();
	bool LoadData(CString strImgFolder, CString strImgListPath, CString strPosListPath, CString strMatchMatrixPathPath);
	bool RunOneKeyMatchAndAdj(CString strMatchAndAdjFolder, sswUAVMatchAndAdjPara& para, bool bRunAdj = true);
protected:
	void ClearData();
	void UpdateParamDo(sswUAVMatchAndAdjPara &param);
	/// 一键式处理功能
	bool  OneKey( sswUAVMatchAndAdjPara& param );

protected:
	bool m_bDataLoad;
	CString m_strMatchAndAdjFolder;
	CString m_strImgFolder;
	CString m_strImgListFile;
	CString m_strPosListFile;
	CString m_strMatchMatrixFile;
	bool m_bIsEnglishVersion;
};



