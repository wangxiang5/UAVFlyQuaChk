#ifndef ssw_uav_flyqua_h
#define ssw_uav_flyqua_h
#pragma once

#include "MyDataBaseType.h"

enum SSWMatchMatrixMethod
{//匹配矩阵计算方法
	SSW_POS_DIS = 0,
	SSW_AREA_LINK = 1,
	SSW_STRIP_IDX = 2
};


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


class CsswImage
{
public:
	CsswImage();
	virtual ~CsswImage();
public:
	bool OpenImg(CString strImgPath, bool bReadOnly=true);
	void CloseImg();
	int  GetRows();
	int  GetCols();
	int  GetBandCount();
	int  GetBitSize();
	float GetFocalLength(DYUNITTYPE eUnit=PIX);
	bool ReadImg(int nStartCol, int nStartRow, int nCols, int nRows, int nMemWidth, int nMemHeight, void*pMem);
	bool SaveImg(CString strSavePath, int nCols, int nRows, int nBands, BYTE*pMem,const char *pszFormat = "JPEG");
protected:
	CString findImageTypeGDAL(CString strDetImgName);
private:
	void* m_pDataset;
};

class CsswUAVImage:public CsswImage
{
public:
public:
	CsswUAVImage();
	virtual ~CsswUAVImage();

public:
	void CalRotMatrixByPOK(DYstuPosInfo& sswPosInfo);
	Point2D XYZ2Img(int nCols, int nRows, double f, Point3D &ptXYZ, bool bAdjed);
	Point3D Img2XYZ(int nCols, int nRows,double f, Point2D &ptImg, double Z, bool bAdjed);
	Point2D* GetImgVertexPt2d4();
	Point3D* GetImgVertexPt3d4(double* pZ, bool bAdjed);
protected:
	void CalImg4VertexPt3d(double* pZ, bool bAdjed, Point3D *&pVertex3d);
public:									        
	Point3D m_PrincipalPoint;                     //像主点
	int m_nImgIdx;                                //影像索引
	int m_nStripIdx;						        //航带索引
	int m_nIdxInStrip;						      						
	CString m_strImgPath;                         //影像路径
	CString m_strImgName;
	CString m_strStripLabel;                  
	DYstuCmrInfo m_ImgCmrInfo;                   //相机参数
	DYstuPosInfo m_ImgPosInfo;                   //影像POS信息
	DYstuPosInfo m_AdjPosInfo;                   //空三POS
	DYstuAdjPt4Img  m_ImgAdjPts;                 //空三点
	bool m_bValid;                             //是否有效
	bool m_bAdjed;                             //是否参与平差

	Point2D m_pImgVertexPt2d4[4];
	Point3D m_pImgVertexPt3d4[4];


};

class CsswUAVFlyQuaPrj
{
public:
	CsswUAVFlyQuaPrj();
	~CsswUAVFlyQuaPrj();

public:
	//载入质检工程
	bool LoadPrj4File(CString strFilePath);
	//清除工程数据
	void ClearPrjData();
	//设置配置参数
	void SetPara2Prj(DYstuSysConfigPara para);
    //计算各项质检参数
	bool RunFlyQuaChk(double Z = 0, /*int nMatchPtNumThresold = 8,*/ DYFLYQUAMETHOD eMethod = BY_ADJUSTMENT,FlyQuaCallBackFun pFun=NULL);
	//保存质检报告
	bool SaveReport2File(CString strSavePath,CString strOverlapImgPath, float fOverlap4Ratio);
	//保存重叠度影像
	void SaveOverlapImage(CString strSavePath,float &fOverlap4Ratio);
	//获取配置参数信息
	DYstuSysConfigPara GetPara()
	{
		return m_FlyQuaParaSet;
	}
// 	//获取匹配文件路径
// 	CString GetMatchPointPath(){return m_strMatchPointPath;}
// 	//获取平差结果路径
// 	CString GetAdjRepPath(){return m_strBundleAdjRepPath;}
	vector<CsswUAVImage> GetImgInfo(){return m_vecImgInfo;}
	int GetImgNum(){return m_nImgNum;}
	int GetValidImgNum(){return m_nImgNum-m_nInvalidNum;}
	int GetInvalidImgNum(){return m_nInvalidNum;}
	int GetStripNum(){return m_nStripNum;}
	int GetImgNumInStrip(int nIdx){
		if(nIdx<0) return 0;
		else if(nIdx>=m_nStripNum) return 0;
		else return m_vecImgNumInEachStrip[nIdx];
	}
	int GetImgIdx(int nStripIdx, int nIdxInStrip)
	{
		return m_vvImgIdxInEachStrip[nStripIdx][nIdxInStrip];
	}
	int GetInvalidImgIdx(int nIdxInInvalid)
	{
		if(m_nInvalidNum<0) return 0;
		if(nIdxInInvalid<0) return 0;
		if(nIdxInInvalid>=m_nInvalidNum) return 0;
		return m_vecImgIdxInvalid[nIdxInInvalid];
	}
	//获取各航带影像索引
	vector<vector<int>> GetStripImgIdx(){return m_vvImgIdxInEachStrip;}
	//获取所有空三点
	vector<DYstuAdjPtInfo> GetAllAdjPts(){return m_vecAdjPts;}
	//获取各影像POS误差
	vector<DYstuPosInfo> GetImgPosErr(){return m_vecAdjImgPosErr;}
	//获取各影像POS距离误差
	vector<double> GetImgPosDisErr(){return m_vecAdjImgPosDisErr;}
	//获取航向重叠度
	vector<vector<double>> GetImgOverlapInsStrip(){return m_vvOverlapsInsStrips;}
	//获取旁向重叠度
	vector<vector<double>> GetImgOverlapBetStrip(){return m_vvOverlapsBetStrips;}
	//获取影像倾角
	vector<double> GetImgPitch(){return m_vecImagePitch;}
	//获取各航带影像旋角
	vector<vector<double>> GetStripImgYaw(){return m_vvStripImageYaw;}
	//获取漏飞区域节点
	vector<vector<Point3D>> GetMissAreaVertex(){return m_vvMissAreaVertex;}
	//获取测区多边形节点
	vector<vector<Point3D>> GetAreaVertex(){return m_vvAreaVertex;}
	vector<vector<Point3D>> GetImgVertex3d(){return m_vvImgVertex3d;}
	//获取航向重叠度不合格影像索引
	vector<int> GetImgIdxLowOverlapInsStrips(){return m_vecImgIdxLowOverlapInsStrips;}
	//获取旁向重叠度不合格影像索引
	vector<int> GetImgIdxLowOverlapBetStrips(){return m_vecImgIdxLowOverlapBetStrips;}
	//获取POS不合格影像索引
	vector<int> GetImgIdxHeiPosErr(){return m_vecImgIdxHeiPosErr;}
	//获取倾角不合格影像索引
	vector<int> GetImgIdxHeiPitch(){return m_vecImgIdxHeiPitch;}
	//获取旋角不合格影像索引
	vector<int> GetImgIdxHeiYaw(){return m_vecImgIdxHeiYaw;}
	//获取空三困难影像索引
	vector<int> GetImgIdxLowAdjPoint(){return m_vecImgIdxLowMatchPointNum;}
	//获取质检通过影像索引
	vector<int> GetImgIdxPassCheck(){return m_vecImgIdxPassCheck;}
protected:
	void ClearAdjInfo();
	void ClearStripInfo();
	void ClearReportInfo(DYFLYQUAMETHOD eMethod);

	bool ReadImgList();
	bool ReadAdjRep(FlyQuaCallBackFun pFun=NULL);
	bool ReadMatchPoint(FlyQuaCallBackFun pFun = NULL);
	void CalBundleImgList2ImgInfo();

	void CalImagePitch();
	void CalImageYaw(double Z, DYFLYQUAMETHOD eMethod);
	void CalImageFlyHeiErr(DYFLYQUAMETHOD eMethod);
	void CalImageFlyDisErr(DYFLYQUAMETHOD eMethod);
	bool CalImagePosErr(DYFLYQUAMETHOD eMethod = BY_ADJUSTMENT);
	void CalStripCqPaneErr(DYFLYQUAMETHOD eMethod = BY_ADJUSTMENT);
	void CalStripOverlaps(double Z, DYFLYQUAMETHOD eMethod,FlyQuaCallBackFun pFun = NULL);
	
	double CalOverlapsByPos(int nIdx1, int nIdx2, double Z, DYOVERLAPDIRECTION eDirection);
	double CalOverlapsByMatch(int nIdx1, int nIdx2, DYOVERLAPDIRECTION eDirection);
	double CalOverlapsByAdjustment(int nIdx1, int nIdx2, DYOVERLAPDIRECTION eDirection);
	double CalOverlapByInterVertex(Point2D *pVertexPts, int nPtNum, double dFullLen, DYOVERLAPDIRECTION eDirection);
	void AnalysisFlyQuaChkParas(int nMatchPtNumThresold);
	void CheckMissArea(double Z, DYFLYQUAMETHOD eMethod);
	///////////////////报告数据类型///////////////////////
public:
	//void CalCqVertexByProject(float Z);
	
protected:
	CString m_strMatchPointPath;
	CString m_strBundleAdjRepPath;
	CString m_strImgListPath;
	CString m_strFlyQuaChkRepPath;
	vector<CsswUAVImage> m_vecImgInfo;
	vector<CsswUAVImage> m_vecInvalidImgInfo;
	vector<int> m_vecImgIdxInvalid;
	vector<int> m_vecBundleImgList2ImgInfo;
	vector<int> m_vecBundleSuccess2ImgInfo;
	vector<CString> m_vecBundleImgName;
	bool m_bCmrFile;
	int m_nImgNum;
	int m_nInvalidNum;
	int m_nInAdjedNum;
	int m_nStripNum;
	int m_nMaxStripIdx;
	vector<int> m_vecStripIdx;
	DYstuCmrInfo m_ImgCmrInfo;       //影像相机参数

	vector<int> m_vecImgNumInEachStrip;
	vector<vector<int>> m_vvImgIdxInEachStrip; //航带中的影像索引信息，排列航带时不改变m_vecImgInfo中的影像顺序
	int *m_pMatchMatrix;
	vector<DYstuAdjPtInfo> m_vecAdjPts;
	double m_dMeanHeiCq;  //测区空三点高程均值
	/************质检时间统计*************/
	double m_dTimeSecsWholeProcess;
	double m_dTimeSecsCmrDistortion;
	double m_dTimeSecsBundleAdjustment;
	double m_dTimeSecsTimeReadAdjRep;
	double m_dTimeSecsRunFlyQuaChk;
	double m_dTimeSecsWriteChkRep;
	///////////////////////////////////////////////////////////////////////////
	/************质检物理量*************/
	DYstuSysConfigPara m_FlyQuaParaSet;
	vector<double> m_vecImagePitch;                 //影像倾角
	vector<vector<double>> m_vvStripImageYaw;       //航带影像旋角
	vector<vector<double>> m_vvStripImageFlyHeiErr; //航带相邻影像航高差
	vector<DYstuPosInfo>  m_vecAdjImgPosErr;        //影像POS误差
	vector<double> m_vecAdjImgPosDisErr;            //影像POS距离误差
	vector<vector<double>> m_vvStripImageFlyDisErr; //航带航偏距
	vector<double> m_vecStripBlendPara;             //航带弯曲度
	vector<vector<double>> m_vvOverlapsInsStrips;   //航向重叠度
	vector<vector<double>> m_vvOverlapsBetStrips;   //旁向重叠度
	vector<double> m_vecStripPanePara;           //航带平整度-平差后POS-Z偏离度
	vector<int> m_vecImgIdxLowOverlapInsStrips;  //航向重叠度低于阈值的影像索引
	vector<int> m_vecImgIdxLowOverlapBetStrips;  //旁向重叠度低于阈值的影像索引
	vector<int> m_vecImgIdxHeiPitch;             //倾角高于阈值的影像索引
	vector<int> m_vecImgIdxHeiYaw;               //旋角高于阈值的影像索引
	vector<int> m_vecImgIdxHeiHeiErr;            //航高差高于阈值的影像索引
	vector<int> m_vecImgIdxHeiPosErr;            //Pos误差高于3倍中误差的影像索引
	vector<int> m_vecBundleIdxHeiPosErr;         //Pos误差高于3倍中误差的Bundle索引
	vector<int> m_vecStripIdxHeiBlend;           //弯曲度高于阈值的航带索引
	vector<double> m_vecDisImgAdjPos2PosPane;    //平差后POS到POS平面的距离
	vector<int> m_vecImgIdxPassCheck;
	double m_dRmsPosErr;
	double m_dMeanPosErr;
	double m_dMaxHeiErr;                         //测区最大航高差
	double m_dMaxPaneDifErr;                     //测区最大凸起值-平差后POS-偏离度
	double m_dRmsPaneDifErr;
	double m_dMissAreaRatio;
	int m_nImgNumPitch10;                        //倾角>10度影像数
	int m_nImgNumYaw15;                          //旋角>15度影像数
	vector<int> m_vecImgNumInEachStripYaw20;     //每条航带旋角>20度影像数

	int m_nMatchPointNumThresold;
	vector<int> m_vecImgIdxLowMatchPointNum;     //匹配困难影像索引

	vector<vector<Point3D>> m_vvMissAreaVertex;  //航飞漏洞角点坐标
	vector<vector<int>> m_vvMissAreaImgIdx;      //航飞漏洞相关影像索引
	vector<vector<Point3D>> m_vvAreaVertex;      //航飞测区范围-多边形求并外边界
	vector<vector<Point3D>> m_vvImgVertex3d;     //影像四角点
};
#endif