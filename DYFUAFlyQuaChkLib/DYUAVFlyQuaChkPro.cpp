// DYUAVFlyQuaChkLib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "MyFunctions.h"
#include "sswUAVFlyQua.h"
void DYUpdateCfgFile(DYstuSysConfigPara *cfgPara /*= NULL*/, void *pCallBackFun/* = NULL*/)
{
	DYstuSysConfigPara para;
	if(!FunReadSysConfigFile(para))
	{
		para = DYstuSysConfigPara();
	};
	if(cfgPara!=NULL) para = *cfgPara;
	FunSaveSysConfigFile(para);
} 

bool DYUAVFlyQuaChkPro(
	char* strImpFilePath, 
	char* strReportSavePath, 
	void *pCallBackFun /* = NULL */)
{
	CsswUAVFlyQuaPrj prj;
	CString strlog;
	strlog.Format("开始解析输入数据文件%s",strImpFilePath);
	FunOutPutLogInfo(strlog);
	DYstuSysConfigPara para;
	if(!FunReadSysConfigFile(para)) para = DYstuSysConfigPara();
	prj.SetPara2Prj(para);
	if(!prj.LoadPrj4File(strImpFilePath)) return false;
	FunOutPutLogInfo("开始计算质检参数...");
	if(!prj.RunFlyQuaChk(0,BY_ADJUSTMENT,(FlyQuaCallBackFun)pCallBackFun)) return false;

	float fOverlap4Ratio = 0;
	CString strOverlapImgPath; strOverlapImgPath.Format("%s_overlap.jpg",strReportSavePath);
	prj.SaveOverlapImage(strOverlapImgPath, fOverlap4Ratio);
	strlog.Format("保存报告%s",strReportSavePath);
	FunOutPutLogInfo(strlog);
	if(!prj.SaveReport2File(strReportSavePath,strOverlapImgPath,4)) return false;
	return true;
}