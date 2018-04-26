#include "StdAfx.h"
#include "MainFrm.h"
#include "sswUAVFlyQuaSysEngDoc.h"
#include "sswUAVFlyQuaSysEngView.h"
#include "MyFunctions.h"
#include "GradientProgressCtrl.h"
#include <io.h>

extern CGradientProgressCtrl g_wndProgressCtrl;
extern CMainFrame* g_pMain;
CMainFrame* GetMainFramHand()
{
	return (CMainFrame *)(CFrameWndEx *)AfxGetMainWnd();
}
CsswUAVFlyQuaSysEngDoc*GetDocHand()
{
	CMainFrame*pMain = GetMainFramHand();
	if (pMain == NULL) return NULL;
	else return (CsswUAVFlyQuaSysEngDoc*)pMain->GetActiveDocument();
}
CsswUAVFlyQuaSysEngView* GetViewHand()
{
	CMainFrame*pMain = GetMainFramHand();
	if (pMain == NULL) return NULL;
	else return (CsswUAVFlyQuaSysEngView*)pMain->GetActiveView();
}

bool FunReadCmrFile(CString strCmrFilePath,SSWstuCmrFileExtend& sswCmrFileExtend, vector<SSWstuCmrInfo>& vecCmrInfo)
{
	if(!PathFileExists(strCmrFilePath))
	{
		FunOutPutLogInfo("Camera paras parsing failed, file path error.");
		return false;
	}
	FILE* pfR = fopen(strCmrFilePath,"r");
	if(pfR == NULL)
	{
		FunOutPutLogInfo("Camera paras parsing failed, file open error.");
		return false;
	}
	char line[MAX_SIZE_FILE_LINE];
	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Camera paras parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line) - 1]= '\0';
	vector<CString> vParts = FunStrTok(line,"\t ");
	if(vParts.size()!=1)
	{
		FunOutPutLogInfo("Camera paras parsing failed, 1st line(Camera number) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	int nMaxColOfcfeMap = InvalidValue;
	for (int i = 0; i<SIZE_CMR_INFO_EXTEND; i++)
	{
		nMaxColOfcfeMap=max(nMaxColOfcfeMap,*(sswCmrFileExtend.pCfeColMap+i));
	}
	SSWstuCmrInfo sswCmrInfo;
	sswCmrInfo.eUnitx0y0 = sswCmrFileExtend.Unitx0y0;
	sswCmrInfo.eUnitf = sswCmrFileExtend.Unitf;
	sswCmrInfo.eUnitpixsize=sswCmrFileExtend.Unitpixsize;
	vector<CString>().swap(vParts);
	while(fgets(line,MAX_SIZE_FILE_LINE, pfR))
	{
		line[strlen(line) - 1]= '\0';
		vParts = FunStrTok(line, "\t ");
		if(vParts.size()<=nMaxColOfcfeMap) continue;

		sswCmrInfo.strLabel=vParts[*sswCmrFileExtend.pCfeColMap];
		sswCmrInfo.x0=atof(vParts[*(sswCmrFileExtend.pCfeColMap+1)]);
		sswCmrInfo.y0=atof(vParts[*(sswCmrFileExtend.pCfeColMap+2)]);
		sswCmrInfo.f=atof(vParts[*(sswCmrFileExtend.pCfeColMap+3)]);
		sswCmrInfo.fx=atof(vParts[*(sswCmrFileExtend.pCfeColMap+4)]);
		sswCmrInfo.fy=atof(vParts[*(sswCmrFileExtend.pCfeColMap+5)]);
		sswCmrInfo.pixsize=atof(vParts[*(sswCmrFileExtend.pCfeColMap+6)]);
		sswCmrInfo.k1=atof(vParts[*(sswCmrFileExtend.pCfeColMap+7)]);
		sswCmrInfo.k2=atof(vParts[*(sswCmrFileExtend.pCfeColMap+8)]);
		sswCmrInfo.p1=atof(vParts[*(sswCmrFileExtend.pCfeColMap+9)]);
		sswCmrInfo.p2=atof(vParts[*(sswCmrFileExtend.pCfeColMap+10)]);
		sswCmrInfo.a=atof(vParts[*(sswCmrFileExtend.pCfeColMap+11)]);
		sswCmrInfo.b=atof(vParts[*(sswCmrFileExtend.pCfeColMap+12)]);
		vecCmrInfo.push_back(sswCmrInfo);
		vector<CString>().swap(vParts);
	}
	if(vecCmrInfo.size() == 0)
	{
		FunOutPutLogInfo("Camera paras parsing failed, camera paras detection error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	fclose(pfR); pfR=NULL;
	FunOutPutLogInfo("Camera paras parsing successful.");
	return true;
}
bool FunReadPosFile(CString strPosFilePath,SSWstuPosFileExtend& sswPosFileExtend, vector<SSWstuPosInfo>& vecPosIngo)
{
	FILE* pfR = fopen(strPosFilePath, "r");
	if(pfR == NULL)
	{
		FunOutPutLogInfo(CString("POS paras parsing failed, file open error."));
		return false;
	}

	int nMaxColOfpfeMap = InvalidValue;
	for (int i = 0; i<SIZE_POS_INFO_EXTEND; i++)
	{
		nMaxColOfpfeMap=max(nMaxColOfpfeMap,*(sswPosFileExtend.pPfeColMap+i));
	}
	char line[MAX_SIZE_FILE_LINE];
	vector<CString> vParts;
	SSWstuPosInfo sswPosInfo;
	sswPosInfo.eFormatCoordinate = sswPosFileExtend.FormatCoor;
	sswPosInfo.eFormatAngleLBH = sswPosFileExtend.FormatAngleLBH;
	sswPosInfo.eFormatAnglePOK = sswPosFileExtend.FormatAnglePOK;

	while(fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		line[strlen(line) - 1]= '\0';		
		vParts = FunStrTok(line, "\t ");
		if(vParts.size()<=nMaxColOfpfeMap) continue;

		sswPosInfo.strLabel=FunGetFileName(vParts[*sswPosFileExtend.pPfeColMap],false).MakeLower();
		sswPosInfo.Xs = atof(vParts[*(sswPosFileExtend.pPfeColMap+1)]);
		sswPosInfo.Ys = atof(vParts[*(sswPosFileExtend.pPfeColMap+2)]);
		sswPosInfo.Zs = atof(vParts[*(sswPosFileExtend.pPfeColMap+3)]);
		sswPosInfo.omg = atof(vParts[*(sswPosFileExtend.pPfeColMap+4)]);
		sswPosInfo.phi = atof(vParts[*(sswPosFileExtend.pPfeColMap+5)]);
		sswPosInfo.kap = atof(vParts[*(sswPosFileExtend.pPfeColMap+6)]);
		vecPosIngo.push_back(sswPosInfo);
		vector<CString>().swap(vParts);
	}
	if(vecPosIngo.size() == 0)
	{
		FunOutPutLogInfo("POS paras parsing failed, POS paras detection error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	fclose(pfR); pfR=NULL;
	FunOutPutLogInfo(CString("POS paras parsing successful."));
	return true;
}
bool FunSaveSysConfigFile(SSWstuSysConfigData& sysCfgData, SSWstuSysConfigPara& sysCfgPara)
{
	CString strCfgFilePath = FunGetSysCfgPath(); 
	FILE* pfW = fopen(strCfgFilePath,"w");

	if(pfW == NULL) 
	{
		FunOutPutLogInfo("Configuration saving failed, file open error.");
		return false;
	}
	fprintf(pfW,"*SysCfg_Data*\n");
	fprintf(pfW,"[Camera_file_col_map] cmr_label, x0, y0, f, fx, fy, pixsize, k1, k2, pa, p2, a, b\n");
	for (int i = 0; i<SIZE_CMR_INFO_EXTEND; i++)
	{
		fprintf(pfW,"%d ",*(sysCfgData.CmrFileExtend.pCfeColMap+i));
	}
	fprintf(pfW, "\n");
	fprintf(pfW,"[Pos_file_col_map] pos_label, lon, lat, alt, omega, phi, kappa\n");
	for (int i = 0; i<SIZE_POS_INFO_EXTEND; i++)
	{
		fprintf(pfW,"%d ",*(sysCfgData.PosFileExtend.pPfeColMap+i));
	}
	fprintf(pfW, "\n");
	fprintf(pfW,"[Unit_x0y0_f_pixsize=pix/um/mm/cm/m]       %s\n", sswUnitType2String(sysCfgData.CmrFileExtend.Unitx0y0));
// 	fprintf(pfW,"[Unit_f=pix/um/mm/cm/m]                    %s\n", sswUnitType2String(sysCfgData.CmrFileExtend.Unitf));
// 	fprintf(pfW,"[Unit_pixsize=pix/um/mm/cm/m]              %s\n", sswUnitType2String(sysCfgData.CmrFileExtend.Unitpixsize));
	fprintf(pfW,"[Format_coordinate=LBH/XYZ]                %s\n", sswCoordinateFormat2String(sysCfgData.PosFileExtend.FormatCoor));
	fprintf(pfW,"[Format_angle_lbh=deg/rad]                 %s\n", sswAngleFormat2String(sysCfgData.PosFileExtend.FormatAngleLBH));
	fprintf(pfW,"[Format_angle_pok=deg/rad]                 %s\n", sswAngleFormat2String(sysCfgData.PosFileExtend.FormatAnglePOK));
	fprintf(pfW,"[Method_pos_img_match=name/name.ext/order] %s\n", sswPosImgMatchMethod2String(sysCfgData.PosFileExtend.MethodPosImgMch));
	fprintf(pfW,"*SysCfg_Para*\n");
	fprintf(pfW,"[ValidJudgeTimesOfRms=float]               %.3f\n",sysCfgPara.fValidJudgeTimesOfRms);
	fprintf(pfW,"[Min_MatchPointNum2Pass=uint]              %d\n",sysCfgPara.nMatchPtNumThresold);
	fprintf(pfW,"[Max_ErrOfStationPane=float]               %.3f\n",sysCfgPara.fMaxErrStationPane);
	fprintf(pfW,"[Rms_ErrOfStationPane=float]               %.3f\n",sysCfgPara.fRmsErrStationPane);
	fprintf(pfW,"[Min_overlap_y=float]                      %.3f\n",sysCfgPara.fMinOverlapY);
	fprintf(pfW,"[Min_overlap_x=float]                      %.3f\n",sysCfgPara.fMinOverlapX);
	fprintf(pfW,"[Max_hei_dif_in_area=float]                %.3f\n",sysCfgPara.fMaxHeiDifInArea);
	fprintf(pfW,"[Max_hei_dif_link_in_strip=float]          %.3f\n",sysCfgPara.fMaxHeiDifLinkInStrip);
	fprintf(pfW,"[MAX_strip_blend=float]                    %.3f\n",sysCfgPara.fMaxStripBlend);
	fprintf(pfW,"[Max_pitch=float]                          %.3f\n",sysCfgPara.fMaxPitch);
	fprintf(pfW,"[Max_pitch10_percent=float]                %.3f\n",sysCfgPara.fMaxPitch10Per);
	fprintf(pfW,"[Max_yaw=float]                            %.3f\n",sysCfgPara.fMaxYaw);
	fprintf(pfW,"[Max_yaw15_percent=float]                  %.3f\n",sysCfgPara.fMaxYaw15Per);
	fprintf(pfW,"[Max_yaw20_number=uint]                    %d\n",sysCfgPara.nMaxYaw20Num);
	fprintf(pfW,"[Max_MissArea_ratio=float]                 %.3f\n",sysCfgPara.fMissAreaRatio);
	fprintf(pfW,"[Max_AngleErr_ratio=float]                 %.3f\n",sysCfgPara.fAngleErrRatio);
	FunOutPutLogInfo(CString("Configuration saving successful."));
	fclose(pfW); pfW = NULL;
	return true;
}
bool FunReadSysConfigFile(SSWstuSysConfigData& sysCfgData, SSWstuSysConfigPara& sysCfgPara)
{
	CString strCfgFilePath= FunGetSysCfgPath();
	FILE* pfR = fopen(strCfgFilePath,"r");
	if(pfR == NULL)
	{
		FunOutPutLogInfo("Configuration parsing failed, file open error.");
		return false;
	}
	char line[MAX_SIZE_FILE_LINE];
	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	if(strcmp(line,"*SysCfg_Data*")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 1st line(*SysCfg_Data*) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	if(strcmp(line,"[Camera_file_col_map] cmr_label, x0, y0, f, fx, fy, pixsize, k1, k2, pa, p2, a, b")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 2nd line([Camera_file_col_map] cmr_label, x0, y0, f, fx, fy, pixsize, k1, k2, pa, p2, a, b) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	int nParts = sscanf(line,"%d %d %d %d %d %d %d %d %d %d %d %d %d",sysCfgData.CmrFileExtend.pCfeColMap,sysCfgData.CmrFileExtend.pCfeColMap+1,
		sysCfgData.CmrFileExtend.pCfeColMap+2,sysCfgData.CmrFileExtend.pCfeColMap+3,sysCfgData.CmrFileExtend.pCfeColMap+4,sysCfgData.CmrFileExtend.pCfeColMap+5,
		sysCfgData.CmrFileExtend.pCfeColMap+6,sysCfgData.CmrFileExtend.pCfeColMap+7,sysCfgData.CmrFileExtend.pCfeColMap+8,sysCfgData.CmrFileExtend.pCfeColMap+9,
		sysCfgData.CmrFileExtend.pCfeColMap+10,sysCfgData.CmrFileExtend.pCfeColMap+11,sysCfgData.CmrFileExtend.pCfeColMap+12);
	if(nParts!=SIZE_CMR_INFO_EXTEND)
	{
		FunOutPutLogInfo("Configuration parsing failed, 3rd line(n1 n2 n3 n4 n5 n6 n7 n8 n9 n10 n11 n12 n13) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		return false;
	}
	line[strlen(line)-1] = '\0';
	if(strcmp(line,"[Pos_file_col_map] pos_label, lon, lat, alt, omega, phi, kappa")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 4th line([Pos_file_col_map] pos_label, lon, lat, alt, omega, phi, kappa) error.");
		return false;
	}
	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts = sscanf(line,"%d %d %d %d %d %d %d",sysCfgData.PosFileExtend.pPfeColMap,sysCfgData.PosFileExtend.pPfeColMap+1, sysCfgData.PosFileExtend.pPfeColMap+2,
		sysCfgData.PosFileExtend.pPfeColMap+3,sysCfgData.PosFileExtend.pPfeColMap+4,sysCfgData.PosFileExtend.pPfeColMap+5, sysCfgData.PosFileExtend.pPfeColMap+6);
	if(nParts!=SIZE_POS_INFO_EXTEND)
	{
		FunOutPutLogInfo("Configuration parsing failed, 5th line(n1 n2 n3 n4 n5 n6 n7) error.");
		return false;
	}
	
	char strHead[100], strInfo[50]; double dat;
	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %s",strHead, strInfo);
	if(nParts!=2||strcmp(strHead,"[Unit_x0y0_f_pixsize=pix/um/mm/cm/m]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 6th line(Unit_x0y0_f_pixsize=pix/um/mm/cm/m] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgData.CmrFileExtend.Unitx0y0=sswString2UnitType(strInfo);

// 	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
// 	{
// 		FunOutPutLogInfo("Configuration parsing failed, file read error.");
// 		fclose(pfR); pfR=NULL;
// 		return false;
// 	}
// 	line[strlen(line)-1] = '\0';
// 	nParts=sscanf(line,"%s %s",strHead, strInfo);
// 	if(nParts!=2||strcmp(strHead,"[Unit_f=pix/um/mm/cm/m]")!=0)
// 	{
// 		FunOutPutLogInfo("Configuration parsing failed，文件第7行错误（[Unit_f=pix/um/mm/cm/m] value）。");
// 		fclose(pfR); pfR=NULL;
// 		return false;
// 	}
	sysCfgData.CmrFileExtend.Unitf=sswString2UnitType(strInfo);

// 	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
// 	{
// 		FunOutPutLogInfo("Configuration parsing failed, file read error.");
// 		fclose(pfR); pfR=NULL;
// 		return false;
// 	}
// 	line[strlen(line)-1] = '\0';
// 	nParts=sscanf(line,"%s %s",strHead, strInfo);
// 	if(nParts!=2||strcmp(strHead,"[Unit_pixsize=pix/um/mm/cm/m]")!=0)
// 	{
// 		FunOutPutLogInfo("Configuration parsing failed，文件第8行错误（[Unit_pixsize=pix/um/mm/cm/m] value）。");
// 		fclose(pfR); pfR=NULL;
// 		return false;
// 	}
	sysCfgData.CmrFileExtend.Unitpixsize=sswString2UnitType(strInfo);

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %s",strHead, strInfo);
	if(nParts!=2||strcmp(strHead,"[Format_coordinate=LBH/XYZ]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 7th line([Format_coordinate=LBH/XYZ] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgData.PosFileExtend.FormatCoor=sswString2CoordinateFormat(strInfo);

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %s",strHead, strInfo);
	if(nParts!=2||strcmp(strHead,"[Format_angle_lbh=deg/rad]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 8th line([Format_angle_lbh=deg/rad] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgData.PosFileExtend.FormatAngleLBH=sswString2AngleFormat(strInfo);

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %s",strHead, strInfo);
	if(nParts!=2||strcmp(strHead,"[Format_angle_pok=deg/rad]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 9th line([Format_angle_pok=deg/rad] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgData.PosFileExtend.FormatAnglePOK=sswString2AngleFormat(strInfo);

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %s",strHead, strInfo);
	if(nParts!=2||strcmp(strHead,"[Method_pos_img_match=name/name.ext/order]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 10th line([Method_pos_img_match=name/name.ext/order] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgData.PosFileExtend.MethodPosImgMch=sswString2PosImgMatchMethod(strInfo);

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	if(strcmp(line,"*SysCfg_Para*")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 11th line(*SysCfg_Para*) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
// 	fprintf(pfW,"[ValidJudgeTimesOfRms]                     %.3f\n",sysCfgPara.fValidJudgeTimesOfRms);
// 	fprintf(pfW,"[Min_MatchPointNum2Pass]                   %d\n",sysCfgPara.nMatchPtNumThresold);
	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %lf",strHead, &dat);
	if(nParts!=2||strcmp(strHead,"[ValidJudgeTimesOfRms=float]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 12th line([ValidJudgeTimesOfRms=float] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgPara.fValidJudgeTimesOfRms=dat;
	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %lf",strHead, &dat);
	if(nParts!=2||strcmp(strHead,"[Min_MatchPointNum2Pass=uint]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 13th line([Min_MatchPointNum2Pass=uint] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgPara.nMatchPtNumThresold=(int)dat;
	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %lf",strHead, &dat);
	if(nParts!=2||strcmp(strHead,"[Max_ErrOfStationPane=float]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 14th line([MAX_ErrOfStationPane=float] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgPara.fMaxErrStationPane=dat;
	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %lf",strHead, &dat);
	if(nParts!=2||strcmp(strHead,"[Rms_ErrOfStationPane=float]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 15th line([Rms_ErrOfStationPane=float] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgPara.fRmsErrStationPane=dat;
	

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %lf",strHead, &dat);
	if(nParts!=2||strcmp(strHead,"[Min_overlap_y=float]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 16th line([Min_overlap_y=float] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgPara.fMinOverlapY=dat;

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %lf",strHead, &dat);
	if(nParts!=2||strcmp(strHead,"[Min_overlap_x=float]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 17th line([Min_overlap_x=float] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgPara.fMinOverlapX=dat;

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %lf",strHead, &dat);
	if(nParts!=2||strcmp(strHead,"[Max_hei_dif_in_area=float]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 18th line([Max_hei_dif_in_area=float] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgPara.fMaxHeiDifInArea=dat;

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %lf",strHead, &dat);
	if(nParts!=2||strcmp(strHead,"[Max_hei_dif_link_in_strip=float]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 19th line([Max_hei_dif_link_in_strip=float] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgPara.fMaxHeiDifLinkInStrip=dat;

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		return false;
		fclose(pfR); pfR=NULL;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %lf",strHead, &dat);
	if(nParts!=2||strcmp(strHead,"[MAX_strip_blend=float]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 20th line([MAX_strip_blend=float] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgPara.fMaxStripBlend=dat;

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %lf",strHead, &dat);
	if(nParts!=2||strcmp(strHead,"[Max_pitch=float]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 21st line([Max_pitch=float] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgPara.fMaxPitch=dat;

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %lf",strHead, &dat);
	if(nParts!=2||strcmp(strHead,"[Max_pitch10_percent=float]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 22nd line([Max_pitch10_percent=float] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgPara.fMaxPitch10Per=dat;

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %lf",strHead, &dat);
	if(nParts!=2||strcmp(strHead,"[Max_yaw=float]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 23rd line([Max_yaw=float] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgPara.fMaxYaw=dat;

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %lf",strHead, &dat);
	if(nParts!=2||strcmp(strHead,"[Max_yaw15_percent=float]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 24th line([Max_yaw15_percent=float] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgPara.fMaxYaw15Per=dat;

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		fclose(pfR); pfR=NULL;  
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %lf",strHead, &dat);
	if(nParts!=2||strcmp(strHead,"[Max_yaw20_number=uint]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 25th line([Max_yaw20_number=uint] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgPara.nMaxYaw20Num=(int)dat;

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		fclose(pfR); pfR=NULL;  
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %lf",strHead, &dat);
	if(nParts!=2||strcmp(strHead,"[Max_MissArea_ratio=float]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 26th line([Max_MissArea_ratio=float] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgPara.fMissAreaRatio=dat;

	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		fclose(pfR); pfR=NULL;  
		FunOutPutLogInfo("Configuration parsing failed, file read error.");
		return false;
	}
	line[strlen(line)-1] = '\0';
	nParts=sscanf(line,"%s %lf",strHead, &dat);
	if(nParts!=2||strcmp(strHead,"[Max_AngleErr_ratio=float]")!=0)
	{
		FunOutPutLogInfo("Configuration parsing failed, 27th line([Max_AngleErr_ratio=float] value) error.");
		fclose(pfR); pfR=NULL;
		return false;
	}
	sysCfgPara.fAngleErrRatio= dat;

	fclose(pfR); pfR=NULL;
	FunOutPutLogInfo("Configuration parsing successful.");
	return true;
}

void FunOutPutWndLog(CMainFrame* pMain, CString strLog, bool bAdd)
{
	if(pMain==NULL) return;
	char *str = new char[strLog.GetLength()+1]; memset(str,0,strLog.GetLength()+1);
	strcpy(str,strLog);
	//	pMain->FillLogWnd(str,bAdd);
	pMain->SendMessage(ID_MSG_FILL_WND_LOG,(WPARAM)&bAdd,(LPARAM)str);
	delete[]str;
}
bool FunRemoveDirAndFile(CString strFolderPath)
{
//	CString strfBat = FunGetFileFolder(FunGetThisExePath())+"\\RemoveBat.bat";
//	FILE *fBat = fopen(strfBat,"w");
//	CString strDeletecmd = "rd \/s \/q "+ strFolderPath+"\\";
//	fprintf(fBat,"%s",strDeletecmd);
//	fclose(fBat);
//	CString strVbs = FunGetFileFolder(strfBat)+"\\RemoveBat.vbs";
 //	system(strDeletecmd);
//	char strCmd[1024]; strcpy(strCmd,strDeletecmd);
	CString  strDeletecmd= "/c rd "+strFolderPath+" /s /q";
	ShellExecute(NULL, NULL, "cmd.exe", strDeletecmd, NULL, SW_HIDE);
// 	STARTUPINFO   si; PROCESS_INFORMATION   pi;
// 	ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si); ZeroMemory(&pi, sizeof(pi));
//  	if (!CreateProcess(NULL, /*NULL*/strCmd, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
// 	{
// 		int n = GetLastError();
// 		return FALSE;
// 	}
	while(PathFileExists(strFolderPath)) 
		Sleep(10);
	return TRUE;
}


void FunVectorPt2VertexXY(void *pVector, int nDim, double *&pV)
{
	if(nDim==2)
	{
		vector<Point2D> vecPt2d = *(vector<Point2D>*)pVector;
		for (int i = 0; i<vecPt2d.size(); i++)
		{
			*(pV+2*i+0) = vecPt2d[i].x;
			*(pV+2*i+1) = vecPt2d[i].y;
		}
		// 		*(pV+2*vecPt2d.size()+0) = vecPt2d[0].x;
		// 		*(pV+2*vecPt2d.size()+1) = vecPt2d[0].y;
		vector<Point2D>().swap(vecPt2d);
	}
	else if(nDim==3)
	{
		vector<Point3D> vecPt3d = *(vector<Point3D>*)pVector;
		for (int i = 0; i<vecPt3d.size(); i++)
		{
			*(pV+2*i+0) = vecPt3d[i].X;
			*(pV+2*i+1) = vecPt3d[i].Y;
		}
		// 		*(pV+2*vecPt3d.size()+0) = vecPt3d[0].X;
		// 		*(pV+2*vecPt3d.size()+1) = vecPt3d[0].Y;
		vector<Point3D>().swap(vecPt3d);
	}
}
void FunCreateVertexColor(int nPtNum, double r,double g,double b, double *&pC)
{
	for (int i = 0; i<nPtNum; i++)
	{
		*(pC+i*4+0)=r;
		*(pC+i*4+1)=g;
		*(pC+i*4+2)=b;
		*(pC+i*4+3)=0.5*255;
	}
}

void FunCallback4FlyQuaProcess(const char *pInfoMain, float dPercentMain, const char *pInfoSub, float dPercentSub, bool bAdd2LogWnd/* = false*/)
{
	//	g_wndProgressCtrl.SetPos(dPercentMain);
	g_pMain->SendMessage(ID_MSG_WNDPROGRESSCTRL_POS,0,(LPARAM)&dPercentMain);
	CString strInfo = pInfoMain;
	if(!(dPercentMain==100&&bAdd2LogWnd==true)&&dPercentMain>=0)
		strInfo.Format("%s %-3.1lf%%",pInfoMain,dPercentMain);
	FunOutPutWndLog(g_pMain,strInfo,bAdd2LogWnd);
	if(bAdd2LogWnd) FunOutPutLogInfo(pInfoMain);   //日志文件显示条数与窗口一致
	if(bAdd2LogWnd) /*g_pMain->UpdateStatusLabel(pInfoMain)*/
		g_pMain->SendMessage(ID_MSG_UPDATE_STATUSLABEL,0,(LPARAM)pInfoMain);
}
void FunCallback4FlyQuaProcessBundle(const char *pInfoMain, float dPercentMain, const char *pInfoSub, float dPercentSub)
{
	//	g_wndProgressCtrl.SetPos(dPercentMain);
	g_pMain->SendMessage(ID_MSG_WNDPROGRESSCTRL_POS,0,(LPARAM)&dPercentMain);
}

void FunCallBackTest(FlyQuaCallBackFun pFun)
{
	int nCount = 0;
	while(1)
	{
		CString strInfo; strInfo.Format("%d",nCount);
		pFun(strInfo,100,strInfo,nCount,true);
		nCount++;
		if(nCount==100) nCount=0;
	}
}

