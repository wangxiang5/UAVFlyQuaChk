#ifndef my_create_flyquaprj_from_parafile
#define my_create_flyquaprj_from_parafile
#pragma once
//#include "sswUAVFlyQua/sswUAVFlyQua.h"
#include "MyFunctions.h"
struct stuPrjParas
{
	CString strCmrFilePath;
	SSWstuSysConfigData stuDataCfg;
	CString strPosFilePath;
	CString strImgFolder;
	CString strPrjFilePath;
};

bool ReadParaFile(CString strParaFilePath,stuPrjParas &Paras);
bool SraeChFilePath(CString strFolder, vector<CString> &vecImgPath);
bool CreateFlyQuaPrj(CsswUAVFlyQuaPrj &Prj, CString strParasFilePath); 
bool MatchImgAndPos(vector<SSWstuPosInfo> &vecPosInfo, vector<CString> &vecImgPath,vector<int> &ImgIdx2PosIdx);

#endif