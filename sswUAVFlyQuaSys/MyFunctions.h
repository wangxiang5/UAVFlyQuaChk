#pragma once
#include "sswUAVFlyQua/sswUAVFlyQua.h"
#include "sswPdfLib.h"
class CMainFrame;
class CsswUAVFlyQuaSysDoc;
class CsswUAVFlyQuaSysView;

CMainFrame* GetMainFramHand();
CsswUAVFlyQuaSysDoc* GetDocHand();
CsswUAVFlyQuaSysView* GetViewHand();

//读取相机参数文件
bool FunReadCmrFile(CString strCmrFilePath,SSWstuCmrFileExtend& sswCmrFileExtend, vector<SSWstuCmrInfo>& vecCmrInfo);
//读取POS参数文件
bool FunReadPosFile(CString strPosFilePath,SSWstuPosFileExtend& sswPosFileExtend, vector<SSWstuPosInfo>& vecPosIngo);
//保存系统配置文件
bool FunSaveSysConfigFile(SSWstuSysConfigData& sysCfgData, SSWstuSysConfigPara& sysCfgPara);
//解析系统配置文件
bool FunReadSysConfigFile(SSWstuSysConfigData& sysCfgData, SSWstuSysConfigPara& sysCfgPara);
//输出窗口日志
void FunOutPutWndLog(CMainFrame* pMain, CString strLog, bool bAdd);
//删除目录及文件
bool FunRemoveDirAndFile(CString strFolderPath);

void FunVectorPt2VertexXY(void *pVector, int nDim, double *&pV);
void FunCreateVertexColor(int nPtNum, double r,double g,double b, double *&pC);


//部分回调函数实例
void FunCallback4FlyQuaProcess(const char *pInfoMain, float dPercentMain, const char *pInfoSub, float dPercentSub, bool bAdd2LogWnd = false);
void FunCallback4FlyQuaProcessBundle(const char *pInfoMain, float dPercentMain, const char *pInfoSub, float dPercentSub);



void FunCallBackTest(FlyQuaCallBackFun pFun);

//通过参数文件建立工程
bool FunCreatePrj4ParaFile(CsswUAVFlyQuaPrj &FlyQuaPrj, CString strParaFilePath);