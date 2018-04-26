#include "stdafx.h"
#include "MyCreateFlyQuaPrj4ParaFile.h"
#include "MainFrm.h"
#include "sswUAVFlyQuaSysEngDoc.h"
#include "sswUAVFlyQuaSysEngView.h"
#include <omp.h>
extern CMainFrame *g_pMain;
#define UnitPix 0
#define UnitUM  1
#define UnitMM  2
#define UnitCM  3
#define UnitM   4
#define FormatXYZ "XYZ"
#define FormatBLH "BLH"
#define FormatDEG "DEG"
#define FormatRAD "RAD"
bool ReadParaFile(CString strParaFilePath,stuPrjParas &Paras)
{
	if(!PathFileExists(strParaFilePath))
	{
		FunOutPutLogInfo("Configuration file path error!"+strParaFilePath);
		return false;
	}
	FILE *pfR = fopen(strParaFilePath,"r");
	if(pfR==NULL)
	{
		FunOutPutLogInfo("Configuration file open error!"+strParaFilePath);
		return false;
	}
	char strLine[MAX_SIZE_FILE_LINE];
	int nCount = 0;
	while(fgets(strLine,MAX_SIZE_FILE_LINE,pfR))
	{
		strLine[strlen(strLine)-1] = '\0';
		if(nCount==8) break;
		vector<CString> vParts = FunStrTok(strLine,"\t ");
		if(vParts.size()<2)
		{
			FunOutPutLogInfo("Configuration file read error!"+strParaFilePath);
			return false;
		}
		if(vParts[0]=="[Head_CmrPath]"&&vParts.size()==2)
		{
			Paras.strCmrFilePath = vParts[1];
			nCount++;
		}else
			if(vParts[0]=="[Head_CmrFormat]"&&vParts.size()==14)
			{
				for (int i = 1; i<14; i++)
				{
					Paras.stuDataCfg.CmrFileExtend.pCfeColMap[i-1] = atoi(vParts[i]);
				}
				nCount++;
			}else
				if(vParts[0]=="[Head_CmrUnit]"&&vParts.size()==4)
				{
					switch(atoi(vParts[1]))
					{
					case UnitPix:
						Paras.stuDataCfg.CmrFileExtend.Unitx0y0 = PIX;
						break;
					case UnitUM:
						Paras.stuDataCfg.CmrFileExtend.Unitx0y0 = UM;
						break;
					case UnitMM:
						Paras.stuDataCfg.CmrFileExtend.Unitx0y0 = MM;
						break;
					case UnitCM:
						Paras.stuDataCfg.CmrFileExtend.Unitx0y0 = CM;
						break;
					case UnitM:
						Paras.stuDataCfg.CmrFileExtend.Unitx0y0 = M;
						break;
					}
					switch(atoi(vParts[2]))
					{
					case UnitPix:
						Paras.stuDataCfg.CmrFileExtend.Unitf = PIX;
						break;
					case UnitUM:
						Paras.stuDataCfg.CmrFileExtend.Unitf = UM;
						break;
					case UnitMM:
						Paras.stuDataCfg.CmrFileExtend.Unitf = MM;
						break;
					case UnitCM:
						Paras.stuDataCfg.CmrFileExtend.Unitf = CM;
						break;
					case UnitM:
						Paras.stuDataCfg.CmrFileExtend.Unitf = M;
						break;
					}
					switch(atoi(vParts[3]))
					{
					case UnitPix:
						Paras.stuDataCfg.CmrFileExtend.Unitpixsize = PIX;
						break;
					case UnitUM:
						Paras.stuDataCfg.CmrFileExtend.Unitpixsize = UM;
						break;
					case UnitMM:
						Paras.stuDataCfg.CmrFileExtend.Unitpixsize = MM;
						break;
					case UnitCM:
						Paras.stuDataCfg.CmrFileExtend.Unitpixsize = CM;
						break;
					case UnitM:
						Paras.stuDataCfg.CmrFileExtend.Unitpixsize = M;
						break;
					}
					nCount++;
				}else
					if(vParts[0]=="[Head_PosPath]"&&vParts.size()==2)
					{
						Paras.strPosFilePath = vParts[1];
						nCount++;
					}else
						if(vParts[0]=="[Head_PosFormat]"&&vParts.size()==8)
						{
							for (int i=1; i<8; i++)
							{
								Paras.stuDataCfg.PosFileExtend.pPfeColMap[i-1]=atoi(vParts[i]);
							}
							nCount++;
						}else
							if(vParts[0]=="[Head_PosUnit]"&&vParts.size()==4)
							{
								if(vParts[1]==FormatXYZ) 
									Paras.stuDataCfg.PosFileExtend.FormatCoor=XYZ;
								else
									if(vParts[1]==FormatBLH)
									{
										Paras.stuDataCfg.PosFileExtend.FormatCoor=LBH;
										if(vParts[2]==FormatRAD)
											Paras.stuDataCfg.PosFileExtend.FormatAngleLBH=RAD;
										else 
											Paras.stuDataCfg.PosFileExtend.FormatAngleLBH=DEG;
									}
									if(vParts[3]==FormatRAD)
										Paras.stuDataCfg.PosFileExtend.FormatAnglePOK=RAD;
									else
										Paras.stuDataCfg.PosFileExtend.FormatAnglePOK=DEG;
									nCount++;
							}else
								if(vParts[0]=="[Head_ImgFolder]"&&vParts.size()==2)
								{
									Paras.strImgFolder = vParts[1];
									nCount++;
								}else
									if(vParts[0]=="[Head_PrjPath]"&&vParts.size()==2)
									{
										Paras.strPrjFilePath = vParts[1];
										nCount++;
									}
									vector<CString>().swap(vParts);
	}
	if(nCount!=8)
	{
		FunOutPutLogInfo("Configuration file format error!"+strParaFilePath);
		return false;
	}
	return true;
}
bool SraeChFilePath(CString strFolder, vector<CString> &vecImgPath)
{
	FunSearchFile(strFolder,"jpg", vecImgPath);
	FunSearchFile(strFolder,"JPG", vecImgPath);
	FunSearchFile(strFolder,"jpeg",vecImgPath);
	FunSearchFile(strFolder,"JPEG",vecImgPath);
	FunSearchFile(strFolder,"tif", vecImgPath);
	FunSearchFile(strFolder,"TIF" ,vecImgPath);
	FunSearchFile(strFolder,"tiff",vecImgPath);
	FunSearchFile(strFolder,"TIFF",vecImgPath);
	if(vecImgPath.size()==0) return false;
	return true;
}
bool MatchImgAndPos(vector<SSWstuPosInfo> &vecPosInfo, vector<CString> &vecImgPath, vector<int> &vecImgIdx2PosIdx)
{
	if(vecPosInfo.size()==0||vecPosInfo.size()==0) return false;
	vector<CString> vecImgName(vecImgPath.size());
	vecImgIdx2PosIdx.assign(vecImgPath.size(),InvalidValue);
#pragma omp  parallel for
	for (int i = 0; i<vecImgPath.size(); i++)
	{
		vecImgName[i] = FunGetFileName(vecImgPath[i],false).MakeLower();
	}
	int nImgPosed = 0;
	for (int i = 0; i<vecImgName.size(); i++)
	{
		for (int j = 0; j<vecPosInfo.size(); j++)
		{
			if(vecPosInfo[j].strLabel==vecImgName[i])
			{
				vecImgIdx2PosIdx[i]=j;
				nImgPosed++;
// 				g_wndProgressCtrl.SetPos((i*m_vecAllPosInfo.size()+j)*100.0/(m_vecAllImgPath.size()*m_vecAllPosInfo.size()));
// 				m_nImgPosed++;
				break;
			}
		}
	}
	if(nImgPosed==0) return false;
	return true;
}
bool CreateFlyQuaPrj(CsswUAVFlyQuaPrj &Prj, CString strParasFilePath)
{
	CsswUAVFlyQuaSysEngDoc *pDoc = GetDocHand();
	if(pDoc->m_pView==NULL)pDoc->m_pView=GetViewHand();
	if(pDoc->m_pMain==NULL)pDoc->m_pMain=GetMainFramHand();
	if(pDoc->m_pView!=NULL)pDoc->m_pView->m_pDoc=pDoc;
	if(pDoc->m_pMain!=NULL)pDoc->m_pMain->m_pDoc = pDoc;
	if(pDoc->m_pMain->m_wndImgAndPt.m_pDoc==NULL)pDoc->m_pMain->m_wndImgAndPt.m_pDoc = pDoc;
	pDoc->m_pMain->UpdateStatusLabel("Creating a project...");

	stuPrjParas Paras;
	if(!ReadParaFile(strParasFilePath,Paras))
	{
		FunOutPutWndLog(g_pMain,"Configuration file parsing error, project creating failed!"+strParasFilePath,true);
		MessageBoxEx(NULL,"Configuration file parsing error, project creating failed!\n"+strParasFilePath,"Error",MB_OK, MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US));
		pDoc->m_pMain->UpdateStatusLabel("Project creating failed...");
		return false;
	}
	vector<SSWstuCmrInfo> vecCmrInfo;
	if(!FunReadCmrFile(Paras.strCmrFilePath,Paras.stuDataCfg.CmrFileExtend,vecCmrInfo))
	{
		FunOutPutWndLog(g_pMain,"Camera file parsing error, project creating failed!"+Paras.strCmrFilePath,true);
		MessageBoxEx(NULL,"Camera file parsing error, project creating failed!\n"+Paras.strCmrFilePath,"Error",MB_OK, MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US));
			pDoc->m_pMain->UpdateStatusLabel("Project creating failed...");
		vector<SSWstuCmrInfo>().swap(vecCmrInfo);
		return false;
	}
	vector<SSWstuPosInfo> vecPosInfo;
	if(!FunReadPosFile(Paras.strPosFilePath,Paras.stuDataCfg.PosFileExtend,vecPosInfo))
	{
		FunOutPutWndLog(g_pMain,"POS file parsing error, project creating failed!"+Paras.strPosFilePath,true);
		MessageBoxEx(NULL,"POS file parsing error, project creating failed!\n"+Paras.strPosFilePath,"Error",MB_OK, MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US));
			pDoc->m_pMain->UpdateStatusLabel("Project creating failed...");
		vector<SSWstuCmrInfo>().swap(vecCmrInfo);
		vector<SSWstuPosInfo>().swap(vecPosInfo);
		return false;
	}
	vector<CString> vecImgPath;
	if(!SraeChFilePath(Paras.strImgFolder,vecImgPath))
	{
		FunOutPutWndLog(g_pMain,"Image folder parsing error, project creating failed!"+Paras.strImgFolder,true);
		MessageBoxEx(NULL,"Image folder parsing error, project creating failed!\n"+Paras.strImgFolder,"Error",MB_OK, MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US));
		pDoc->m_pMain->UpdateStatusLabel("Project creating failed...");

		vector<SSWstuCmrInfo>().swap(vecCmrInfo);
		vector<SSWstuPosInfo>().swap(vecPosInfo);
		vector<CString>().swap(vecImgPath);
		return false;
	}
	vector<int> vecImgIdx2PosIdx;
	if(!MatchImgAndPos(vecPosInfo,vecImgPath,vecImgIdx2PosIdx))
	{
		FunOutPutWndLog(g_pMain,"Image and POS matching error, project creating failed!"+Paras.strImgFolder,true);
		MessageBoxEx(NULL,"Matching Image to POS error, project creating failed!\n"+Paras.strImgFolder,"Error",MB_OK, MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US));
		pDoc->m_pMain->UpdateStatusLabel("Project creating failed...");

		vector<SSWstuCmrInfo>().swap(vecCmrInfo);
		vector<SSWstuPosInfo>().swap(vecPosInfo);
		vector<CString>().swap(vecImgPath);
		vector<int>().swap(vecImgIdx2PosIdx);
		return false;
	}
	vector<CsswUAVImage> vecImgInfo;
	int nCount = 0;
	for (int i = 0; i<vecImgIdx2PosIdx.size(); i++)
	{
		if(vecImgIdx2PosIdx[i]==InvalidValue) continue;
		CsswUAVImage img;
		img.m_nImgIdx = nCount;
		img.m_strImgPath=vecImgPath[i];
		img.m_ImgCmrInfo=vecCmrInfo[0];
		img.m_ImgPosInfo = vecPosInfo[vecImgIdx2PosIdx[i]];	
		img.m_strImgName = FunGetFileName(img.m_strImgPath,true).MakeLower();
		vecImgInfo.push_back(img);
		nCount++;
	}
	SSWstuSysConfigPara cfgParas;
	pDoc->PrjNew(Paras.strPrjFilePath,vecImgInfo,cfgParas, true);
	pDoc->m_pMain->UpdateStatusLabel("Ready");
}

