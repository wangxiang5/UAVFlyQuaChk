/************************************************************************/
/* 目的: 无人机航飞影像质检计算算法库                                   */
/* 时间: 2016-12-08                                                     */
/* 单位: 武汉点云科技有限公司                                           */
/************************************************************************/
#pragma once

#ifdef  DYUAVFLYQUACHKLIB_EXPORTS
#define DYUAVFLY __declspec(dllexport)
#else
#define DYUAVFLY __declspec(dllimport)
#endif 

/************************************************************************/
/* 基本数据结构                                                         */
/************************************************************************/
//长度单位
enum DYUAVFLY DYUNITTYPE
{
	PIX = 0,
	UM  = 1,
	MM  = 1000,
	CM  = 10000,
	M   = 1000000
};
//坐标类型
enum DYUAVFLY DYCOORDINATEFORMAT
{
	LBH = 0,
	XYZ = 1
};
//角度类型
enum DYUAVFLY DYANGLEFORMAT
{
	DEG = 0,
	RAD = 1
};
//相机、POS设备信息解析
struct DYUAVFLY DYstuDeviceFileExtend
{
	int pCfeColMap[13];              //相机文件各参数所在列 cmr_label, x0, y0, f, fx, fy, pixsize, k1, k2, pa, p2, a, b
	DYUNITTYPE UnitIntrinsicParas;   //相机内参数单位x0, y0, f, fx, fy, pixsize

	DYCOORDINATEFORMAT FormatCoor;  //POS坐标类型
	DYANGLEFORMAT FormatAngleLBH;   //POS坐标为经纬高时，经纬度角度类型
	DYANGLEFORMAT FormatAnglePOK;   //POS姿态角类型
	DYstuDeviceFileExtend()
	{
		*pCfeColMap = 12;
		for (int i = 1; i<13; i++)
		{
			*(pCfeColMap+i) = i-1;
		}
		UnitIntrinsicParas = MM;
	
		FormatCoor      = LBH;
		FormatAngleLBH  = DEG;
		FormatAnglePOK  = DEG;
	}
};
//航飞质检参数阈值
struct DYUAVFLY DYstuSysConfigPara
{
	float fMinOverlapY;               //航向重叠度最小阈值
	float fMinOverlapX;               //旁向重叠度最小阈值
	float fMaxHeiDifInArea;           //测区最大航高差阈值
	float fMaxHeiDifLinkInStrip;      //航带相邻影像最大航高差阈值
	float fMaxStripBlend;             //航带最大弯曲度阈值
	float fMaxPitch;                  //测区影像最大倾角阈值
	float fMaxPitch10Per;             //测区影像倾角超过10度最大比例阈值   -暂未使用
	float fMaxYaw;                    //测区影像最大旋角阈值
	float fMaxYaw15Per;				  //测区旋角超过15度影像最大占比阈值   -暂未使用
	int   nMaxYaw20Num;               //测区旋角超过20度影像最大数量       -暂未使用
	float fValidJudgeTimesOfRms;      //测区影像有效性判断阈值
	int   nMatchPtNumThresold;        //测区空三匹配点数最小阈值
	float fMaxErrStationPane;         //测区平整度摄站最大偏离阈值
	float fRmsErrStationPane;         //测区平直度摄站偏离中误差最大阈值
	float fMissAreaRatio;
	float fAngleErrRatio;
	DYstuSysConfigPara()
	{
		fMinOverlapY = 60.0f;
		fMinOverlapX = 30.0f;
		fMaxHeiDifInArea = 50.0f;
		fMaxHeiDifLinkInStrip = 30.0f;
		fMaxStripBlend = 3.0f;
		fMaxPitch = 15.0f;
		fMaxPitch10Per = 10.0f;
		fMaxYaw = 5.0f;
		fMaxYaw15Per = 10.0f;
		nMaxYaw20Num = 3;
		fValidJudgeTimesOfRms=2.0f;
		nMatchPtNumThresold = 8;
		fMaxErrStationPane = 15;
		fRmsErrStationPane = 10;
		fMissAreaRatio = 2.0f;
		fAngleErrRatio = 5.0f;
	}
	bool operator==(DYstuSysConfigPara para){
		return (fMinOverlapY == para.fMinOverlapY && 
			fMinOverlapX == para.fMinOverlapX && 
			fMaxHeiDifInArea == para.fMaxHeiDifInArea && 
			fMaxHeiDifLinkInStrip == para.fMaxHeiDifLinkInStrip && 
			fMaxStripBlend == para.fMaxStripBlend && 
			fMaxPitch == para.fMaxPitch && 
			fMaxPitch10Per == para.fMaxPitch10Per && 
			fMaxYaw == para.fMaxYaw && 
			fMaxYaw15Per == para.fMaxYaw15Per && 
			nMaxYaw20Num == para.nMaxYaw20Num &&
			nMatchPtNumThresold == para.nMatchPtNumThresold &&
			fMissAreaRatio == para.fMissAreaRatio &&
			fAngleErrRatio == para.fAngleErrRatio );}
};

/************************************************************************/
/* 外部接口函数                                                         */
/* 回调函数示例：void* pCallBackFun(const char* strInfo, float fPercent)*/
/************************************************************************/
#ifndef _dll_functions
#define _dll_functions
//修改配置信息-有默认值，可按需设置
/*******参数1：质检参数阈值*/
/*******参数2：回调函数指针*/
void DYUAVFLY DYUpdateCfgFile(DYstuSysConfigPara *cfgPara = NULL, void *pCallBackFun = NULL);
//执行质检计算
/*******参数1：质检数据信息文件路径*/
/*******参数2：质检报告保存路径*/
/*******参数3：回调函数指针*/
/*******返回值：ture-合格；false-不合格*/
bool DYUAVFLY DYUAVFlyQuaChkPro(char* strImpFilePath, char* strReportSavePath, void *pCallBackFun = NULL);
#endif