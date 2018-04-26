#include "stdafx.h"
#include "resource.h"
#include "MyMultiThread.h"
#include "MainFrm.h"
#include "sswUAVFlyQuaSysDoc.h"
#include "sswUAVFlyQuaSysView.h"
#include "MyFunctions.h"
#include "GradientProgressCtrl.h"
#include "PrjNewDlg.h"
#include <omp.h>
extern CGradientProgressCtrl g_wndProgressCtrl;
extern CMainFrame g_pMain;

static struct stuParaMultiThreadImgPosMatch
{
	CPrjNewDlg* pDlg;
	ImgPosMatchDirectiong eMatchDirection;
	stuParaMultiThreadImgPosMatch()
	{
		pDlg=NULL;
		eMatchDirection=IMG2POS;
	}
};

void WINAPI MultiThreadImgPosMatch(LPVOID p)
{
	CPrjNewDlg* pDlg = (CPrjNewDlg*)p;
	if(pDlg==NULL) return;
	pDlg->ImgAndPosMatch();
}

void WINAPI MultiThreadCmrDistortionCorrect(LPVOID p)
{
	CsswUAVFlyQuaSysDoc* pDoc = (CsswUAVFlyQuaSysDoc*)p;
	if(pDoc==NULL) return;
	pDoc->m_FlyQuaPrj.CmrDistortionCorrect();
}
void WINAPI MultiThreadReOrderStrips(LPVOID p)
{
	//g_wndProgressCtrl.ShowWindow(true);

	CsswUAVFlyQuaSysView* pView = (CsswUAVFlyQuaSysView*)p;
	if(pView==NULL) return;
	bool bShow =true;
	pView->m_pDoc->m_pMain->SendMessage(ID_MSG_WNDPROGRESSCTRL_SHOW,0,(LPARAM)&bShow);
	pView->SendMessage(ID_MSG_RESORT_STRIPS);
	//g_wndProgressCtrl.ShowWindow(false);
	bShow =false;
	pView->m_pDoc->m_pMain->SendMessage(ID_MSG_WNDPROGRESSCTRL_SHOW,0,(LPARAM)&bShow);
}
void WINAPI MultiThreadSetImgValidState(LPVOID p)
{
	//g_wndProgressCtrl.ShowWindow(true);

	CsswUAVFlyQuaSysView* pView = (CsswUAVFlyQuaSysView*)p;
	if(pView==NULL) return;
	bool bShow =true;
	pView->m_pDoc->m_pMain->SendMessage(ID_MSG_WNDPROGRESSCTRL_SHOW,0,(LPARAM)&bShow);
	pView->SendMessage(ID_MSG_SET_IMGVALID_STATE);
//	pView->OnMsgSetImgInvalid(0,0);
	bShow =false;
	pView->m_pDoc->m_pMain->SendMessage(ID_MSG_WNDPROGRESSCTRL_SHOW,0,(LPARAM)&bShow);
}

void WINAPI MultiThreadFlyQuaChkFun(LPVOID p)
{

 	CsswUAVFlyQuaSysDoc* pDoc = (CsswUAVFlyQuaSysDoc*)p;
	if(pDoc==NULL) return;
	if(pDoc->m_nImgNum==0) return;

 	bool bShow =true;
   	pDoc->m_pMain->SendMessage(ID_MSG_WNDPROGRESSCTRL_SHOW,0,(LPARAM)&bShow);
   	if(pDoc->m_eFlyQuaChkMethod!=BY_POS) 
   	{
   		if(pDoc->m_vecImgInfo.size()==0)  return;  //检测主线程出现析构
   		pDoc->m_vecImgInfo[0].OpenImg(pDoc->m_vecImgInfo[0].m_strImgPath);
   		int nCols = pDoc->m_vecImgInfo[0].GetCols();
   		int nRows = pDoc->m_vecImgInfo[0].GetRows();
   		pDoc->m_vecImgInfo[0].CloseImg();
   		float fScale = max(nCols, nRows)*1.0/1280;
   		pDoc->m_FlyQuaPrj.CmrDistortionCorrect(fScale,true,FunCallback4FlyQuaProcess);
   		sswUAVMatchAndAdjPara param;
   		param.nThreadNum = omp_get_num_procs();		//线程数
   		param.Sift_bCheckExist = true;				//置check为false，每次都重新做空三
   		param.Sift_nWindow = 10;
   		param.PickPoint_nNumPerGrid = 0;
   		param.SBA_bWithPos = true;
   		param.SBA_GPS_Precise = 20.0;
   		if(pDoc->m_eFlyQuaChkMethod==BY_MATCH)
   		{
   			param.Freenet_bDo = false; //自由网平差
   			param.SBA_bDo = false;    //整体平差
   		}
   		param.pOutputFunc = FunCallback4FlyQuaProcess;
   		param.pOutputFuncBundle=FunCallback4FlyQuaProcessBundle;
 		if(!pDoc->m_FlyQuaPrj.BundleAdjustment(param, true))
			{
				AfxMessageBox("匹配平差错误，质检失败！");
				pDoc->m_bInRunning = false;
				 return;
		};
		if(pDoc->m_FlyQuaPrj.GetStripNum()==0)
		{
			AfxMessageBox("航带信息有误，质检参数计算失败！");
			return;
		}
		//pDoc->m_eFlyQuaChkMethod=BY_POS;
   		pDoc->m_FlyQuaPrj.RunFlyQuaChk(pDoc->m_dMeanHei,pDoc->m_eFlyQuaChkMethod,FunCallback4FlyQuaProcess);
   		pDoc->m_pView->SendMessage(ID_MSG_CREATE_MISSRENDER,0,0);
   		if(pDoc->m_eFlyQuaChkMethod==BY_ADJUSTMENT)
   		{	
   			pDoc->m_pMain->m_wndDataChart.PostMessage(ID_MSG_FILL_DATACHART,0,0);
   		}
   		else
   		{
   			pDoc->m_pMain->m_wndDataChart.PostMessage(ID_MSG_FILL_DATACHART,0,0);
   			pDoc->m_pMain->m_wndImgAndPt.m_bDisplayAdjPtVBO = false;
   			pDoc->m_pMain->m_wndImgAndPt.SendMessage(WM_ERASEBKGND);
   		}
   	}
   	else
   	{
		if(pDoc->m_FlyQuaPrj.GetStripNum()==0)
		{
			AfxMessageBox("航带信息有误，质检参数计算失败！");
			pDoc->m_bInRunning=false;
			return;
		}
   	    pDoc->m_FlyQuaPrj.RunFlyQuaChk(pDoc->m_dMeanHei, BY_POS,FunCallback4FlyQuaProcess);
   		pDoc->m_pView->PostMessage(ID_MSG_CREATE_MISSRENDER,0,0);
   		pDoc->m_pMain->m_wndDataChart.PostMessage(ID_MSG_FILL_DATACHART,0,0);
   		pDoc->m_pMain->m_wndImgAndPt.m_bDisplayAdjPtVBO = false;
   		pDoc->m_pMain->m_wndImgAndPt.SendMessage(WM_ERASEBKGND);
   	}
    	
   	/**************************质检计算完毕****************************/
//	pDoc->m_pView->ReInitCqView();
	pDoc->m_pView->m_bViewCqArea = true;
	pDoc->m_pView->m_bViewImgArea = false;
	pDoc->m_pView->m_bViewMissArea = true;
	pDoc->m_pView->m_bSaveView2Img = true;
	pDoc->m_pView->SendMessage(WM_ERASEBKGND);
//	pDoc->m_pView->m_bSaveView2Img = false;
	FunOutPutLogInfo("准备报告素材...");
	float npos = 0;
	pDoc->m_pMain->SendMessage(ID_MSG_WNDPROGRESSCTRL_POS,0,(LPARAM)&npos);
	pDoc->m_pMain->PostMessage(ID_MSG_UPDATE_STATUSLABEL,0,(LPARAM)("准备报告素材..."));
	int nCur = 0;
	for (int i = 5; i>=0; i--)
	{
		pDoc->m_pMain->m_wndImgAndPt.m_bSaveView2Img=true;
		pDoc->m_pMain->m_wndDataChart.m_ComboDataExtend.SetCurSel(i);
		pDoc->m_pMain->m_wndDataChart.m_eQualityType=SSWQUALITYTYPE(i);
		pDoc->m_pMain->m_wndImgAndPt.SendMessage(ID_MSG_FILL_REPRENDER,0,(LPARAM)pDoc);
		Sleep(1000);
		npos = (5-i)*20;
		pDoc->m_pMain->SendMessage(ID_MSG_WNDPROGRESSCTRL_POS,0,(LPARAM)&npos);
	}
// 	pDoc->m_pMain->m_wndImgAndPt.m_bSaveView2Img=false;
	pDoc->m_pMain->SendMessage(ID_MSG_WNDPROGRESSCTRL_POS,0,(LPARAM)&npos);
	pDoc->m_pMain->PostMessage(ID_MSG_UPDATE_STATUSLABEL,0,(LPARAM)("就绪"));
	vector<CString> vecViewImagePath;
	vecViewImagePath.push_back(pDoc->m_pView->m_strCqViewImagePath);
	vecViewImagePath.push_back(pDoc->m_pView->m_strOverlapYErrViewImagePath);
	vecViewImagePath.push_back(pDoc->m_pView->m_strOverlapXErrViewImagePath);
	vecViewImagePath.push_back(pDoc->m_pView->m_strPosErrViewImagePath);
	vecViewImagePath.push_back(pDoc->m_pView->m_strPitchErrViewImagePath);
	vecViewImagePath.push_back(pDoc->m_pView->m_strYawErrViewImagePath);
	vecViewImagePath.push_back(pDoc->m_pView->m_strPtNumErrViewImagePath);
	char strViewImgPath[MAX_SIZE_FILE_PATH]; strcpy(strViewImgPath,pDoc->m_pView->m_strCqViewImagePath);
	Sleep(500);
	pDoc->m_pMain->SendMessage(ID_MSG_SAVE_REPORT,(WPARAM)&vecViewImagePath,(LPARAM)&pDoc->m_eFlyQuaChkMethod);
	if(pDoc->m_FlyQuaPrj.ifAutoManageImages())
		pDoc->ManageImgFile3();	
	vector<CString>().swap(vecViewImagePath);
	bShow =false;
	pDoc->m_pMain->SendMessage(ID_MSG_WNDPROGRESSCTRL_SHOW,0,(LPARAM)&bShow);
//	pDoc->m_pMain->SendMessage(ID_MSG_OPEN_REPORT,0,0);
	pDoc->OnButtonFlyquaReport();
	pDoc->m_bInRunning=false;
}

void WINAPI MultiThreadCreateImgTexture(LPVOID p)
{
	CsswUAVFlyQuaSysDoc* pDoc = (CsswUAVFlyQuaSysDoc*)p;
	pDoc->m_pMain->m_wndImgAndPt.OnMsgCreateImgTextures(0,0);
}

// void WINAPI MultiThreadCreateCqProjectVbo(LPVOID p)
// {
//	CsswUAVFlyQuaSysView *pView = (CsswUAVFlyQuaSysView*) p;
//	pView->m_pDoc->m_FlyQuaPrj.CalCqVertexByProject(pView->m_pDoc->m_dMeanHei);
//	pView->SendMessage(ID_MSG_CREATE_MISSRENDER);
//}