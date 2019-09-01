// ��� MFC ʾ��Դ������ʾ���ʹ�� MFC Microsoft Office Fluent �û����� 
// (��Fluent UI��)����ʾ�������ο���
// ���Բ��䡶Microsoft ������ο����� 
// MFC C++ ������渽����ص����ĵ���
// ���ơ�ʹ�û�ַ� Fluent UI ����������ǵ����ṩ�ġ�
// ��Ҫ�˽��й� Fluent UI ��ɼƻ�����ϸ��Ϣ�������  
// http://msdn.microsoft.com/officeui��
//
// ��Ȩ����(C) Microsoft Corporation
// ��������Ȩ����

// sswUAVFlyQuaSysDoc.cpp : CsswUAVFlyQuaSysDoc ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "sswUAVFlyQuaSys.h"
#endif
#include "MainFrm.h"
#include "sswUAVFlyQuaSysDoc.h"
#include "sswUAVFlyQuaSysView.h"
#include <propkey.h>
#include "CoorConv.hpp"
#include "PrjNewDlg.h"
#include "GradientProgressCtrl.h"
#include "MyMultiThread.h"
#include "MyFunctions.h"
#include <omp.h>
#include <io.h>
#include "gdal201/ogrsf_frmts.h"
#include "gdal201/ogr_spatialref.h"

#include "PdfViewDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CsswUAVFlyQuaSysDoc
extern CGradientProgressCtrl g_wndProgressCtrl;
IMPLEMENT_DYNCREATE(CsswUAVFlyQuaSysDoc, CDocument)

	BEGIN_MESSAGE_MAP(CsswUAVFlyQuaSysDoc, CDocument)
		ON_COMMAND(ID_BUTTON_PRJ_NEW, &CsswUAVFlyQuaSysDoc::OnButtonPrjNew)
		ON_COMMAND(ID_BUTTON_RUN_FLYQUA_CHECK, &CsswUAVFlyQuaSysDoc::OnButtonRunFlyquaCheck)
		ON_UPDATE_COMMAND_UI(ID_BUTTON_RUN_FLYQUA_CHECK, &CsswUAVFlyQuaSysDoc::OnUpdateButtonRunFlyquaCheck)
		ON_COMMAND(ID_BUTTON_FLYQUA_REPORT, &CsswUAVFlyQuaSysDoc::OnButtonFlyquaReport)
		ON_UPDATE_COMMAND_UI(ID_BUTTON_FLYQUA_REPORT, &CsswUAVFlyQuaSysDoc::OnUpdateButtonFlyquaReport)
		ON_COMMAND(ID_BUTTON_PRJ_OPEN, &CsswUAVFlyQuaSysDoc::OnButtonPrjOpen)
// 		ON_COMMAND(ID_COMBO_CHECK_METHOD, &CsswUAVFlyQuaSysDoc::OnComboCheckMethod)
// 		ON_UPDATE_COMMAND_UI(ID_COMBO_CHECK_METHOD, &CsswUAVFlyQuaSysDoc::OnUpdateComboCheckMethod)
		ON_UPDATE_COMMAND_UI(ID_BUTTON_PRJ_NEW, &CsswUAVFlyQuaSysDoc::OnUpdateButtonPrjNew)
		ON_UPDATE_COMMAND_UI(ID_BUTTON_PRJ_OPEN, &CsswUAVFlyQuaSysDoc::OnUpdateButtonPrjOpen)
		ON_COMMAND(ID_BUTTON_PRJ_SAVE, &CsswUAVFlyQuaSysDoc::OnButtonPrjSave)
		ON_UPDATE_COMMAND_UI(ID_BUTTON_PRJ_SAVE, &CsswUAVFlyQuaSysDoc::OnUpdateButtonPrjSave)
		ON_COMMAND(ID_EDIT_MEAN_HEIGHT, &CsswUAVFlyQuaSysDoc::OnEditMeanHeight)
		ON_UPDATE_COMMAND_UI(ID_EDIT_MEAN_HEIGHT, &CsswUAVFlyQuaSysDoc::OnUpdateEditMeanHeight)
		ON_COMMAND(ID_FILE_NEW, &CsswUAVFlyQuaSysDoc::OnFileNew)
		ON_COMMAND(ID_FILE_OPEN, &CsswUAVFlyQuaSysDoc::OnFileOpen)
		ON_COMMAND(ID_FILE_SAVE, &CsswUAVFlyQuaSysDoc::OnFileSave)
		ON_COMMAND(ID_BUTTON_DATA_ORDER, &CsswUAVFlyQuaSysDoc::OnButtonDataOrder)
		ON_UPDATE_COMMAND_UI(ID_BUTTON_DATA_ORDER, &CsswUAVFlyQuaSysDoc::OnUpdateButtonDataOrder)
// 		ON_COMMAND(ID_EDIT_MATCHPOINT_NUM_THRESOLD, &CsswUAVFlyQuaSysDoc::OnEditMatchpointNumThresold)
// 		ON_UPDATE_COMMAND_UI(ID_EDIT_MATCHPOINT_NUM_THRESOLD, &CsswUAVFlyQuaSysDoc::OnUpdateEditMatchpointNumThresold)
	END_MESSAGE_MAP()

	// CsswUAVFlyQuaSysDoc ����/����

	CsswUAVFlyQuaSysDoc::CsswUAVFlyQuaSysDoc()
	{
		// TODO: �ڴ����һ���Թ������
		m_bPrjLoaded=false;
		m_bInRunning = false;
		m_pView=NULL;
		m_pMain=NULL;
		m_nImgNum = 0;
		m_dMeanHei = 0;
		m_eFlyQuaChkMethod = BY_ADJUSTMENT;
		m_bTransXYInView = false;
		m_bAutoManageImages=false;
//		m_nMatchPointNumThresold = 8;
	
	}

	CsswUAVFlyQuaSysDoc::~CsswUAVFlyQuaSysDoc()
	{
	}

	BOOL CsswUAVFlyQuaSysDoc::OnNewDocument()
	{
		if (!CDocument::OnNewDocument())
			return FALSE;

		// TODO: �ڴ�������³�ʼ������
		// (SDI �ĵ������ø��ĵ�)
		if(m_pMain==NULL) m_pMain = GetMainFramHand();
		return TRUE;
	}




	// CsswUAVFlyQuaSysDoc ���л�

	void CsswUAVFlyQuaSysDoc::Serialize(CArchive& ar)
	{
		if (ar.IsStoring())
		{
			// TODO: �ڴ���Ӵ洢����
		}
		else
		{
			// TODO: �ڴ���Ӽ��ش���
		}
	}

#ifdef SHARED_HANDLERS

	// ����ͼ��֧��
	void CsswUAVFlyQuaSysDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
	{
		// �޸Ĵ˴����Ի����ĵ�����
		dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

		CString strText = _T("TODO: implement thumbnail drawing here");
		LOGFONT lf;

		CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
		pDefaultGUIFont->GetLogFont(&lf);
		lf.lfHeight = 36;

		CFont fontDraw;
		fontDraw.CreateFontIndirect(&lf);

		CFont* pOldFont = dc.SelectObject(&fontDraw);
		dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
		dc.SelectObject(pOldFont);
	}

	// ������������֧��
	void CsswUAVFlyQuaSysDoc::InitializeSearchContent()
	{
		CString strSearchContent;
		// ���ĵ����������������ݡ�
		// ���ݲ���Ӧ�ɡ�;���ָ�

		// ����:  strSearchContent = _T("point;rectangle;circle;ole object;")��
		SetSearchContent(strSearchContent);
	}

	void CsswUAVFlyQuaSysDoc::SetSearchContent(const CString& value)
	{
		if (value.IsEmpty())
		{
			RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
		}
		else
		{
			CMFCFilterChunkValueImpl *pChunk = NULL;
			ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
			if (pChunk != NULL)
			{
				pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
				SetChunkValue(pChunk);
			}
		}
	}

#endif // SHARED_HANDLERS

	// CsswUAVFlyQuaSysDoc ���

#ifdef _DEBUG
	void CsswUAVFlyQuaSysDoc::AssertValid() const
	{
		CDocument::AssertValid();
	}

	void CsswUAVFlyQuaSysDoc::Dump(CDumpContext& dc) const
	{
		CDocument::Dump(dc);
	}
#endif //_DEBUG


	// CsswUAVFlyQuaSysDoc ����
	bool CsswUAVFlyQuaSysDoc::GetConfigInfo(SSWstuSysConfigData &data, SSWstuSysConfigPara &para)
	{
		if(!FunReadSysConfigFile(data,para))
		{
			para = SSWstuSysConfigPara();
			return false;
		}
		return true;
	}
	void CsswUAVFlyQuaSysDoc::InitViewRect()
	{
		double l,r,t,b;
		r=t=-1e10;
		l=b=1e10;
		for (int i = 0; i<m_vecImgInfo.size(); i++)
		{
			l=min(l,m_vecImgInfo[i].m_ImgPosInfo.Xs);
			r=max(r,m_vecImgInfo[i].m_ImgPosInfo.Xs);
			t=max(t,m_vecImgInfo[i].m_ImgPosInfo.Ys);
			b=min(b,m_vecImgInfo[i].m_ImgPosInfo.Ys);
		}
		m_pView->m_RectPosGeo = CRectD(l-100,t+100,r+100,b-100);
		m_pView->m_RectCqViewGeo = m_pView->m_RectPosGeo;
		m_pView->m_ptClnCenter2Geo = m_pView->m_RectCqViewGeo.CenterPoint();
		m_pView->m_fZoomRateCln2Geo = min(m_pView->m_RectCln.Width()*1.0/m_pView->m_RectCqViewGeo.Width(),
			m_pView->m_RectCln.Height()*1.0/m_pView->m_RectCqViewGeo.Height())*0.7;
		//m_pView->ResetRectCln2Geo();
		m_pView->m_fZoomRateCln2GeoMin = m_pView->m_fZoomRateCln2Geo/20;
		m_pView->m_fZoomRateCln2GeoMax = m_pView->m_fZoomRateCln2Geo*20;
		m_pMain->m_wndImgAndPt.InitRepView();

	}
	void CsswUAVFlyQuaSysDoc::FillPosRender()
	{
		m_vecImgInfo=m_FlyQuaPrj.GetImgInfo();
		GLdouble* pv = new GLdouble[m_vecImgInfo.size()*2];
		GLdouble* pc = new GLdouble[m_vecImgInfo.size()*4];
		for (int i = 0; i<m_vecImgInfo.size(); i++)
		{
			*(pv+2*i+0)=m_vecImgInfo[i].m_ImgPosInfo.Xs;
			*(pv+2*i+1)=m_vecImgInfo[i].m_ImgPosInfo.Ys;
			if(!m_vecImgInfo[i].m_bValid)   //ת����ЧӰ��
			{
				*(pc+4*i+0)=100;
				*(pc+4*i+1)=0;
				*(pc+4*i+2)=0;
				*(pc+4*i+3)=255;
			}else
				if(m_vecImgInfo[i].m_nStripIdx==InvalidValue)  //��ת�򺽴�ʧ��
				{
					*(pc+4*i+0)=255;
					*(pc+4*i+1)=110;
					*(pc+4*i+2)=180;
					*(pc+4*i+3)=255;
				}else
					if(m_vecImgInfo[i].m_nStripIdx%2)  //��������
					{
						*(pc+4*i+0)=0;
						*(pc+4*i+1)=100;
						*(pc+4*i+2)=0;
						*(pc+4*i+3)=255;
					}else
					{
						*(pc+4*i+0)=0;
						*(pc+4*i+1)=0;
						*(pc+4*i+2)=180;
						*(pc+4*i+3)=255;
					}
		}
		m_pView->m_PosRender.FillVectorVBO(pv,pc,m_vecImgInfo.size());
		delete[]pv; pv=NULL;
		delete[]pc; pc=NULL;

		m_pView->Invalidate();


	}
	void CsswUAVFlyQuaSysDoc::UpdatePrjTree()
	{
		vector<CString> vecWndTreeItem(2+m_FlyQuaPrj.GetStripNum());
		CString strItem;strItem.Format("����-%s",m_strPrjFilePath);
		vecWndTreeItem[0]=strItem;
		strItem.Format("����ת�����Ӱ��-%d��",m_FlyQuaPrj.GetInvalidImgNum());
		vecWndTreeItem[1]=strItem;
		for (int i = 0; i<m_FlyQuaPrj.GetStripNum(); i++)
		{
			strItem.Format("����Strip_%d-%d��",i+1,m_FlyQuaPrj.GetImgNumInStrip(i));
			vecWndTreeItem[i+2] = strItem;
		}
		m_pMain->ShowWndTree(vecWndTreeItem);
		vector<CString>().swap(vecWndTreeItem);
	}
	void CsswUAVFlyQuaSysDoc::ReSortStrips()
	{
		g_wndProgressCtrl.ShowWindow(true);
		m_FlyQuaPrj.ReOrderStrips(m_pView->m_LineParaA,m_pView->m_LineParaB,m_pView->m_LineParaC,FunCallback4FlyQuaProcess);
		m_FlyQuaPrj.SavePrj2File(m_strPrjFilePath); 
		FillPosRender();
		m_pView->m_bSaveView2Img=true;
		CString str; str.Format("����������ɣ�������%d",m_FlyQuaPrj.GetStripNum());
		FunOutPutWndLog(m_pMain,str,true);
		UpdatePrjTree();
		g_wndProgressCtrl.ShowWindow(false);
	}
	void CsswUAVFlyQuaSysDoc::ManageImgFile5()
	{
		FunCallback4FlyQuaProcess("����Ӱ��",1,"��ЧӰ��",0,true);
		vector<CsswUAVImage> vecImgInfo = m_FlyQuaPrj.GetImgInfo();
		vector<int> vecAimIdx;
		CString strImgFolder = FunGetFileFolder(m_strPrjFilePath)+"\\ImageManagement";	
		CString strImgInvalidFolder = strImgFolder+"\\1_ImageInvalid";
		CString strImgPitchErrFolder = strImgFolder+"\\2_ImagePitchErr";
		CString strImgPosErrFolder = strImgFolder+"\\3_ImagePosErr";
		CString strImgAdjFolder = strImgFolder+"\\4_ImageAdjErr";
		CString strImgPassFolder = strImgFolder+"\\5_ImagePassCheck";	
		while(PathFileExists(strImgFolder)) FunRemoveDirAndFile(strImgFolder);
		while(!PathFileExists(strImgFolder)) FunCreateDir4Path(strImgFolder);
		while(!PathFileExists(strImgInvalidFolder)) FunCreateDir4Path(strImgInvalidFolder);
		while(!PathFileExists(strImgPitchErrFolder)) FunCreateDir4Path(strImgPitchErrFolder);
		while(!PathFileExists(strImgPosErrFolder)) FunCreateDir4Path(strImgPosErrFolder);
		while(!PathFileExists(strImgAdjFolder)) FunCreateDir4Path(strImgAdjFolder);
		while(!PathFileExists(strImgPassFolder)) FunCreateDir4Path(strImgPassFolder);

		FunCallback4FlyQuaProcess("����Ӱ��",2,"��ЧӰ��",0,false);
		/*��ЧӰ��*/
		CString strImgInvalidList = strImgInvalidFolder+"\\1_InvalidList.txt";	
		FunCreateDir4Path(FunGetFileFolder(strImgInvalidList));
		FILE *pfW = fopen(strImgInvalidList,"w");
		for (int i = 0; i<vecImgInfo.size(); i++)
		{
			if(!vecImgInfo[i].m_bValid)
			{
				CString strOldPath = vecImgInfo[i].m_strImgPath;
				if(!PathFileExists(strOldPath)) continue;
				CString strNewPath = strImgInvalidFolder+"\\"+FunGetFileName(strOldPath,true);
				CopyFile(strOldPath,strNewPath,TRUE);
				fprintf(pfW,"%s\n",strNewPath);
			}
		}
		fclose(pfW);

		FunCallback4FlyQuaProcess("����Ӱ��",25,"����쳣Ӱ��",0,false);
		/*��ǹ���Ӱ��*/

		CString strImgPitchErrList = strImgPitchErrFolder+"\\2_PitchErrList.txt";
		FunCreateDir4Path(FunGetFileFolder(strImgPitchErrList));
		pfW = fopen(strImgPitchErrList,"w");
		vecAimIdx = m_FlyQuaPrj.GetImgIdxHeiPitch();
		for (int i = 0; i<vecAimIdx.size(); i++)
		{
			FunCallback4FlyQuaProcess("����Ӱ��",25+i*25/vecAimIdx.size(),"����쳣Ӱ��",0,false);
			CString strOldPath = vecImgInfo[vecAimIdx[i]].m_strImgPath;
			if(!PathFileExists(strOldPath)) continue;
			CString strNewPath = strImgPitchErrFolder+"\\"+FunGetFileName(strOldPath,true);
			CopyFile(strOldPath,strNewPath,FALSE);
			fprintf(pfW,"%s\n",strNewPath);
		}
		fclose(pfW);
		vector<int>().swap(vecAimIdx);

		FunCallback4FlyQuaProcess("����Ӱ��",50,"POS�����쳣Ӱ��",0,false);
		/*Pos�����쳣Ӱ��*/
	
		CString strImgPosErrList = strImgPosErrFolder+"\\3_PosErrList.txt";	
				FunCreateDir4Path(FunGetFileFolder(strImgPosErrList));
		pfW = fopen(strImgPosErrList,"w");
		vecAimIdx = m_FlyQuaPrj.GetImgIdxHeiPosErr();
		for (int i = 0; i<vecAimIdx.size(); i++)
		{
			FunCallback4FlyQuaProcess("����Ӱ��",50+i*25/vecAimIdx.size(),"POS�����쳣Ӱ��",0,false);
			CString strOldPath = vecImgInfo[vecAimIdx[i]].m_strImgPath;
			if(!PathFileExists(strOldPath)) continue;
			CString strNewPath = strImgPosErrFolder+"\\"+FunGetFileName(strOldPath,true);
			CopyFile(strOldPath,strNewPath,FALSE);
			fprintf(pfW,"%s\n",strNewPath);
		}
		fclose(pfW);
		vector<int>().swap(vecAimIdx);

		FunCallback4FlyQuaProcess("����Ӱ��",75,"��������Ӱ��",0,false);
		/*��������Ӱ��*/
		CString strImgAdjErrList = strImgAdjFolder+"\\4_AdjErrList.txt";
		FunCreateDir4Path(FunGetFileFolder(strImgAdjErrList));
		pfW = fopen(strImgAdjErrList,"w");
		vecAimIdx = m_FlyQuaPrj.GetImgIdxLowAdjPoint();
		for (int i = 0; i<vecAimIdx.size(); i++)
		{
			FunCallback4FlyQuaProcess("����Ӱ��",75+i*10/vecAimIdx.size(),"��������Ӱ��",0,false);
			CString strOldPath = vecImgInfo[vecAimIdx[i]].m_strImgPath;
			if(!PathFileExists(strOldPath)) continue;
			CString strNewPath = strImgAdjFolder+"\\"+FunGetFileName(strOldPath,true);
			CopyFile(strOldPath,strNewPath,FALSE);
			fprintf(pfW,"%s\n",strNewPath);
		}
		fclose(pfW);
		vector<int>().swap(vecAimIdx);

		FunCallback4FlyQuaProcess("����Ӱ��",85,"�ʼ�ϸ�Ӱ��",0,false);
		vecAimIdx = m_FlyQuaPrj.GetImgIdxPassCheck();
		/*����Ӱ��*/
		CString strImgPassCheckList = strImgPassFolder+"\\5_PassCheckList.txt";
				FunCreateDir4Path(FunGetFileFolder(strImgPassCheckList));
		pfW = fopen(strImgPassCheckList,"w");
		for (int i = 0; i<vecAimIdx.size(); i++)
		{
			FunCallback4FlyQuaProcess("����Ӱ��",85+i*15/vecAimIdx.size(),"�ʼ�ϸ�Ӱ��",0,false);
			CString strOldPath = vecImgInfo[vecAimIdx[i]].m_strImgPath;
			if(!PathFileExists(strOldPath)) continue;
			CString strNewPath = strImgPassFolder+"\\"+FunGetFileName(strOldPath,true);
			CopyFile(strOldPath,strNewPath,FALSE);
			fprintf(pfW,"%s\n",strNewPath);
		}
		fclose(pfW);
		vector<int>().swap(vecAimIdx);
		if(vecImgInfo.size()>0) vector<CsswUAVImage>().swap(vecImgInfo);
		FunCallback4FlyQuaProcess("����Ӱ��",100,"���",0,false);
		m_pMain->UpdateStatusLabel("����");
	}
	void CsswUAVFlyQuaSysDoc::ManageImgFile3()
	{
		FunCallback4FlyQuaProcess("����Ӱ��",1,"��ЧӰ��",0,true);
		vector<CsswUAVImage> vecImgInfo = m_FlyQuaPrj.GetImgInfo();
		vector<int> vecAimIdx;
		CString strImgFolder = FunGetFileFolder(m_strPrjFilePath)+"\\ImageManagement";	
		CString strImgInvalidFolder = strImgFolder+"\\1_ImageInvalid";
		CString strImgAdjFolder = strImgFolder+"\\2_ImageAdjErr";
		CString strImgElseFolder = strImgFolder+"\\3_ImageElse";	
		while(PathFileExists(strImgFolder)) FunRemoveDirAndFile(strImgFolder);
		while(!PathFileExists(strImgFolder)) FunCreateDir4Path(strImgFolder);
		while(!PathFileExists(strImgInvalidFolder)) FunCreateDir4Path(strImgInvalidFolder);
		while(!PathFileExists(strImgAdjFolder)) FunCreateDir4Path(strImgAdjFolder);
		while(!PathFileExists(strImgElseFolder)) FunCreateDir4Path(strImgElseFolder);

		FunCallback4FlyQuaProcess("����Ӱ��",2,"��ЧӰ��",0,false);
		/*��ЧӰ��*/
		CString strImgInvalidList = strImgInvalidFolder+"\\1_InvalidList.txt";	
		FunCreateDir4Path(FunGetFileFolder(strImgInvalidList));
		FILE *pfW = fopen(strImgInvalidList,"w");
		for (int i = 0; i<vecImgInfo.size(); i++)
		{
			if(!vecImgInfo[i].m_bValid)
			{
				CString strOldPath = vecImgInfo[i].m_strImgPath;
				if(!PathFileExists(strOldPath)) continue;
				CString strNewPath = strImgInvalidFolder+"\\"+FunGetFileName(strOldPath,true);
				CopyFile(strOldPath,strNewPath,TRUE);
				fprintf(pfW,"%s\n",strNewPath);
			}
		}
		fclose(pfW);
		/*��������Ӱ��*/
		FunCallback4FlyQuaProcess("����Ӱ��",25,"��������",0,false);
		CString strImgAdjErrList = strImgAdjFolder+"\\2_AdjErrList.txt";
		FunCreateDir4Path(FunGetFileFolder(strImgAdjErrList));
		pfW = fopen(strImgAdjErrList,"w");
		vecAimIdx = m_FlyQuaPrj.GetImgIdxLowAdjPoint();
		for (int i = 0; i<vecAimIdx.size(); i++)
		{
			FunCallback4FlyQuaProcess("����Ӱ��",35+i*10/vecAimIdx.size(),"��������Ӱ��",0,false);
			CString strOldPath = vecImgInfo[vecAimIdx[i]].m_strImgPath;
			if(!PathFileExists(strOldPath)) continue;
			CString strNewPath = strImgAdjFolder+"\\"+FunGetFileName(strOldPath,true);
			CopyFile(strOldPath,strNewPath,FALSE);
			fprintf(pfW,"%s\n",strNewPath);
		}
		fclose(pfW);
		vector<int>().swap(vecAimIdx);
		/*����Ӱ��*/
		FunCallback4FlyQuaProcess("����Ӱ��",35,"����Ӱ��",0,false);
		CString strImgElseList = strImgElseFolder+"\\3_ElseList.txt";
		FunCreateDir4Path(FunGetFileFolder(strImgElseList));

		pfW = fopen(strImgElseList,"w");
		vecAimIdx = m_FlyQuaPrj.GetImgIdxHeiPitch();
		/*Pitch�쳣*/
		for (int i = 0; i<vecAimIdx.size(); i++)
		{
			FunCallback4FlyQuaProcess("����Ӱ��",35+i*25/vecAimIdx.size(),"����Ӱ��",0,false);
			CString strOldPath = vecImgInfo[vecAimIdx[i]].m_strImgPath;
			if(!PathFileExists(strOldPath)) continue;
			CString strNewPath = strImgElseFolder+"\\"+FunGetFileName(strOldPath,true);
			CopyFile(strOldPath,strNewPath,FALSE);
			fprintf(pfW,"%s\n",strNewPath);
		}
		vector<int>().swap(vecAimIdx);
		/*Pos�����쳣Ӱ��*/
		vecAimIdx = m_FlyQuaPrj.GetImgIdxHeiPosErr();
		for (int i = 0; i<vecAimIdx.size(); i++)
		{
			FunCallback4FlyQuaProcess("����Ӱ��",60+i*25/vecAimIdx.size(),"����Ӱ��",0,false);
			CString strOldPath = vecImgInfo[vecAimIdx[i]].m_strImgPath;
			if(!PathFileExists(strOldPath)) continue;
			CString strNewPath = strImgElseFolder+"\\"+FunGetFileName(strOldPath,true);
			CopyFile(strOldPath,strNewPath,FALSE);
			fprintf(pfW,"%s\n",strNewPath);
		}
		vector<int>().swap(vecAimIdx);

		FunCallback4FlyQuaProcess("����Ӱ��",85,"����",0,false);
		vecAimIdx = m_FlyQuaPrj.GetImgIdxPassCheck();
		for (int i = 0; i<vecAimIdx.size(); i++)
		{
			FunCallback4FlyQuaProcess("����Ӱ��",85+i*15/vecAimIdx.size(),"����Ӱ��",0,false);
			CString strOldPath = vecImgInfo[vecAimIdx[i]].m_strImgPath;
			if(!PathFileExists(strOldPath)) continue;
			CString strNewPath = strImgElseFolder+"\\"+FunGetFileName(strOldPath,true);
			CopyFile(strOldPath,strNewPath,FALSE);
			fprintf(pfW,"%s\n",strNewPath);
		}
		fclose(pfW);
		vector<int>().swap(vecAimIdx);
		if(vecImgInfo.size()>0) vector<CsswUAVImage>().swap(vecImgInfo);
		FunCallback4FlyQuaProcess("����Ӱ��",100,"���",0,false);
		m_pMain->UpdateStatusLabel("����");
	}

	void CsswUAVFlyQuaSysDoc::ClearData()
	{
		vector<CsswUAVImage>().swap(m_vecImgInfo);
		vector<CString>().swap(m_vecInvalidImgPath);
		//		m_bPrjLoaded = false;
		m_bTransXYInView = false;
		m_nImgNum = 0;
		m_bCmrFile = true;
		m_fOverlapRatio4 = 0;
		//	m_dMeanHei = 0;
		m_FlyQuaPrj.ClearPrjData();
		m_pView->DeleteCurLineInfo();
		m_bAutoManageImages=false;
		m_bPrjLoaded = false;
	}
	void CsswUAVFlyQuaSysDoc::UnionParaFormat()
	{
#pragma omp parallel for
		for (int i = 0; i<m_vecImgInfo.size(); i++)
		{
			if(m_vecImgInfo[i].m_ImgPosInfo.eFormatCoordinate==LBH)
			{
				UTMCoor xy;
				int zone = (m_vecImgInfo[i].m_ImgPosInfo.Xs + 180.0) / 6 + 1;
				if(m_vecImgInfo[i].m_ImgPosInfo.eFormatAngleLBH==DEG) 
					LatLonToUTMXY(DegToRad(m_vecImgInfo[i].m_ImgPosInfo.Ys), DegToRad(m_vecImgInfo[i].m_ImgPosInfo.Xs), zone, xy);
				else
					LatLonToUTMXY(m_vecImgInfo[i].m_ImgPosInfo.Ys, m_vecImgInfo[i].m_ImgPosInfo.Xs, zone, xy);
				m_vecImgInfo[i].m_ImgPosInfo.Xs = xy.x;
				m_vecImgInfo[i].m_ImgPosInfo.Ys = xy.y;
			}
			if(m_vecImgInfo[i].m_ImgPosInfo.eFormatAnglePOK == RAD)
			{
				m_vecImgInfo[i].m_ImgPosInfo.phi = RadToDeg(m_vecImgInfo[i].m_ImgPosInfo.phi);
				m_vecImgInfo[i].m_ImgPosInfo.omg = RadToDeg(m_vecImgInfo[i].m_ImgPosInfo.omg);
				m_vecImgInfo[i].m_ImgPosInfo.kap = RadToDeg(m_vecImgInfo[i].m_ImgPosInfo.kap);
			}
			// 			if (m_vecImgInfo[i].m_ImgCmrInfo.eUnitx0y0 != PIX&&m_vecImgInfo[i].m_ImgCmrInfo.eUnitpixsize != PIX)  //x0,y0ת��Ϊ���ص�λ
			// 			{
			// 				m_vecImgInfo[i].m_ImgCmrInfo.x0= m_vecImgInfo[i].m_ImgCmrInfo.x0*m_vecImgInfo[i].m_ImgCmrInfo.eUnitx0y0/m_vecImgInfo[i].m_ImgCmrInfo.eUnitpixsize/m_vecImgInfo[i].m_ImgCmrInfo.pixsize;
			// 				m_vecImgInfo[i].m_ImgCmrInfo.y0= m_vecImgInfo[i].m_ImgCmrInfo.y0*m_vecImgInfo[i].m_ImgCmrInfo.eUnitx0y0/m_vecImgInfo[i].m_ImgCmrInfo.eUnitpixsize/m_vecImgInfo[i].m_ImgCmrInfo.pixsize;
			// 			}
			if (m_vecImgInfo[i].m_ImgCmrInfo.eUnitf != PIX&&m_vecImgInfo[i].m_ImgCmrInfo.eUnitpixsize != PIX) //f,fx,fyת��Ϊ���ص�λ
			{
				m_vecImgInfo[i].m_ImgCmrInfo.f = m_vecImgInfo[i].m_ImgCmrInfo.f*m_vecImgInfo[i].m_ImgCmrInfo.eUnitf / m_vecImgInfo[i].m_ImgCmrInfo.eUnitpixsize / m_vecImgInfo[i].m_ImgCmrInfo.pixsize;
				m_vecImgInfo[i].m_ImgCmrInfo.fx = m_vecImgInfo[i].m_ImgCmrInfo.fx*m_vecImgInfo[i].m_ImgCmrInfo.eUnitf / m_vecImgInfo[i].m_ImgCmrInfo.eUnitpixsize / m_vecImgInfo[i].m_ImgCmrInfo.pixsize;
				m_vecImgInfo[i].m_ImgCmrInfo.fy = m_vecImgInfo[i].m_ImgCmrInfo.fy*m_vecImgInfo[i].m_ImgCmrInfo.eUnitf / m_vecImgInfo[i].m_ImgCmrInfo.eUnitpixsize / m_vecImgInfo[i].m_ImgCmrInfo.pixsize;
			}
			m_vecImgInfo[i].m_ImgCmrInfo.eUnitx0y0=PIX;
			m_vecImgInfo[i].m_ImgCmrInfo.eUnitf=PIX;
			m_vecImgInfo[i].m_ImgPosInfo.eFormatCoordinate=XYZ;
			m_vecImgInfo[i].m_ImgPosInfo.eFormatAnglePOK=DEG;
			m_vecImgInfo[i].m_ImgPosInfo.eFormatAngleLBH=DEG;
		}
	}

	bool CsswUAVFlyQuaSysDoc::PrjNew(CString strPath,vector<CsswUAVImage> &vecImgInfo, SSWstuSysConfigPara para, bool bWithCmr, bool bAutoManageImages)
	{
		ClearData();
		m_strPrjFilePath=strPath;
		m_vecImgInfo=vecImgInfo;
		m_bAutoManageImages=bAutoManageImages;
		UnionParaFormat();
		if(!m_FlyQuaPrj.LoadData2Prj(m_vecImgInfo,m_strPrjFilePath,para,bWithCmr))
		{
			CString strInfo; strInfo.Format("�½�����ʧ�ܣ����%s��־�ļ���",FunGetSyslogPath());
			AfxMessageBox(strInfo);
			m_pMain->UpdateStatusLabel("���̴���ʧ�ܣ�");
			return false;
		}
		m_FlyQuaPrj.setAutoManageImages(m_bAutoManageImages);
		m_FlyQuaPrj.CheckImgValidState();
		// 		if(m_FlyQuaPrj.GetInvalidImgNum()>0)
		// 		{
		// 			CString strInfo;
		// 			strInfo.Format("��⵽%d����ЧӰ���Ƿ�ɾ����",m_FlyQuaPrj.GetInvalidImgNum());
		// 			if(MessageBox(NULL,strInfo,"��ʾ",MB_YESNO)==IDYES)
		// 			{
		// 				m_FlyQuaPrj.DeleteInvalidImg();
		// 			}
		// 		}
		if(!m_FlyQuaPrj.SavePrj2File(m_strPrjFilePath))
		{
			CString strInfo; strInfo.Format("�½�����ʧ�ܣ����%s��־�ļ���",FunGetSyslogPath());
			AfxMessageBox(strInfo);
			m_pMain->UpdateStatusLabel("���̴���ʧ�ܣ�");
			return false;
		}
		m_vecImgInfo = m_FlyQuaPrj.GetImgInfo();
		m_nImgNum = m_vecImgInfo.size();
		m_pMain->ShowRibbonValue();
		m_pMain->m_wndDataChart.OnComboDatachartExtend();
		m_pMain->m_wndImgAndPt.DeleteAllRender();
		if(!m_bPrjLoaded) m_pMain->CreatePaneWnd();
		m_bPrjLoaded=true;
		m_pView->DeleteViewVBO();
		m_pView->m_PosRender.CreateVectorVBO(m_vecImgInfo.size(),vPoint,true);
		InitViewRect();
		FillPosRender();
		UpdatePrjTree();
		FunOutPutWndLog(m_pMain,("���̴����ɹ�=" +m_strPrjFilePath),true);
		return true;
	}

	void CsswUAVFlyQuaSysDoc::OnButtonPrjNew()
	{
		if(m_pView==NULL) m_pView=GetViewHand();
		if(m_pMain==NULL) m_pMain=GetMainFramHand();
		if(m_pView!=NULL) m_pView->m_pDoc=this;
		if(m_pMain!=NULL) m_pMain->m_pDoc = this;
		if(m_pMain->m_wndImgAndPt.m_pDoc==NULL) m_pMain->m_wndImgAndPt.m_pDoc = this;
		m_pMain->UpdateStatusLabel("���ڴ�������...");
		CPrjNewDlg dlg;
		if(dlg.DoModal()==IDOK)
		{
			SSWstuCmrInfo cmr;
			if(!dlg.GetCmrInfo(cmr, m_bCmrFile))
			{
				ClearData();
				AfxMessageBox("Ӱ�񽹾�������󣬹��̴���ʧ�ܣ�");
				return;
			}
			if(!PrjNew(dlg.GetPrjFilePath(),dlg.LoadImg2Prj(),dlg.GetFlyQuaPara(),m_bCmrFile,dlg.m_bAutoManageImage))
				return;
			m_bPrjLoaded = true;
			m_pView->m_bStripResorted = false;
		}
		m_pMain->UpdateStatusLabel("����");

	}
	void CsswUAVFlyQuaSysDoc::OnButtonPrjOpen()
	{
		// TODO: �ڴ���������������
		if(m_pView==NULL) m_pView=GetViewHand();
		if(m_pMain==NULL) m_pMain=GetMainFramHand();
		if(m_pView!=NULL) m_pView->m_pDoc=this;
		if(m_pMain!=NULL) m_pMain->m_pDoc = this;
		if(m_pMain->m_wndImgAndPt.m_pDoc==NULL) m_pMain->m_wndImgAndPt.m_pDoc = this;
		m_pMain->UpdateStatusLabel("����ѡ�񹤳��ļ�...");
		CString FilePathName;
		CFileDialog dlg(TRUE, //TRUEΪOPEN�Ի���FALSEΪSAVE AS�Ի���
			NULL, 
			NULL,
			OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			(LPCTSTR)_TEXT("XML Files (*.xml)|*.xml|All Files (*.*)|*.*||"),
			NULL);
		if(dlg.DoModal()==IDOK)
		{
			FilePathName = dlg.GetPathName(); //�ļ�����������FilePathName��
			if(FilePathName==m_strPrjFilePath) return;	
			ClearData();
			if(!m_FlyQuaPrj.LoadPrj4File(FilePathName))
			{
				CString strInfo; strInfo.Format("���̴�ʧ�ܣ����%s��־�ļ���",FunGetSyslogPath());
				AfxMessageBox(strInfo);
				m_pMain->UpdateStatusLabel("���̴�ʧ�ܣ�");
				return;
			}
			SSWstuSysConfigPara para_cfg;
			SSWstuSysConfigData data_cfg;
			if(GetConfigInfo(data_cfg,para_cfg))
			{
				if(!(para_cfg==m_FlyQuaPrj.GetPara()))
				{
					CString strInfo; strInfo.Format("������ò����빤���ʼ������һ�£��Ƿ����������ã�");
					if(MessageBox(NULL,strInfo,"��ʾ",MB_YESNO)==IDYES)
					{
						FunSaveSysConfigFile(data_cfg,m_FlyQuaPrj.GetPara());
					}
				}
			}
			if(!m_bPrjLoaded) m_pMain->CreatePaneWnd();
			m_bPrjLoaded=true;
			m_bAutoManageImages = m_FlyQuaPrj.ifAutoManageImages();
			m_strPrjFilePath = FilePathName;
// 			if(m_FlyQuaPrj.GetInvalidImgNum()==0)
// 			{
// 				m_FlyQuaPrj.CheckImgValidState();
// 				m_FlyQuaPrj.SavePrj2File(m_strPrjFilePath);
// 			}
			m_vecImgInfo=m_FlyQuaPrj.GetImgInfo();
			m_nImgNum = m_vecImgInfo.size();
			m_pMain->ShowRibbonValue();

			if(m_FlyQuaPrj.GetStripNum()>0)
			{
				m_pView->m_bStripResorted=true;
			}else
			{
				m_pView->m_bStripResorted = false;
			}
			m_pView->DeleteCurLineInfo();
			m_pView->DeleteViewVBO();
			m_pView->m_PosRender.CreateVectorVBO(m_vecImgInfo.size(),vPoint,true);
			m_pMain->m_wndDataChart.OnComboDatachartExtend();
			m_pMain->m_wndImgAndPt.DeleteAllRender();
			//	m_pView->m_PosRectRender.CreateVectorVBO(5,vLine,true);
			InitViewRect();
			FillPosRender();

			UpdatePrjTree();
			m_pView->SaveView2Img(m_pView->m_strCqViewImagePath);
			FunOutPutWndLog(m_pMain,("���̴򿪳ɹ�=" +m_strPrjFilePath),true);	
		
			m_pMain->UpdateStatusLabel("����");
			m_FlyQuaPrj.SavePrj2File4DYDll(m_strPrjFilePath+"DYTest.xml");
		}


	}
	void CsswUAVFlyQuaSysDoc::OnFileNew()
	{
		// TODO: �ڴ���������������
		OnButtonPrjNew();
	}
	void CsswUAVFlyQuaSysDoc::OnFileOpen()
	{
		// TODO: �ڴ���������������
		OnButtonPrjOpen();
	}
	void CsswUAVFlyQuaSysDoc::OnFileSave()
	{
		// TODO: �ڴ���������������
		OnButtonPrjSave();
	}

	void CsswUAVFlyQuaSysDoc::OnButtonRunFlyquaCheck()
	{
		// TODO: �ڴ���������������
		m_pView->m_bActStripResort = false;
		CString strMethod;
		if(m_eFlyQuaChkMethod==BY_POS) strMethod="����POS�Ŀ����ʼ�";
		if(m_eFlyQuaChkMethod==BY_MATCH) strMethod="����ƥ����ʼ췽��";
		if(m_eFlyQuaChkMethod==BY_ADJUSTMENT) strMethod="����ƽ��ľ�ȷ�ʼ�";
		CString str; str.Format("���������ʼ죬������%s",strMethod);
		FunOutPutWndLog(m_pMain,str,true);
		DeleteFile(m_pView->m_strCqViewImagePath);
		if(m_eFlyQuaChkMethod==BY_MATCH&&PathFileExists(m_FlyQuaPrj.GetMatchPointPath()))
		{
			switch(MessageBox(NULL,"��⵽�ʼ칤�����Ѵ���ƥ�����ļ����Ƿ��������ɣ�","��ʾ",MB_YESNOCANCEL))
			{
			case IDYES:
				DeleteFile(m_FlyQuaPrj.GetAdjRepPath());
				break;
			case IDNO:
				break;
			default:
				return;
			}
		}else
			if(m_eFlyQuaChkMethod==BY_ADJUSTMENT&&PathFileExists(m_FlyQuaPrj.GetAdjRepPath()))
			{
				switch(MessageBox(NULL,"��⵽�ʼ칤�����Ѵ���ƽ�����ļ����Ƿ��������ɣ�","��ʾ",MB_YESNOCANCEL))
				{
				case IDYES:
					DeleteFile(m_FlyQuaPrj.GetAdjRepPath());
					break;
				case IDNO:
					break;
				default:
					return;
				}
				// 				if(MessageBox(NULL,"��⵽�ʼ칤�����Ѵ���ƽ�����ļ����Ƿ��������ɣ�","��ʾ",MB_YESNOCANCEL)==IDYES)
				// 				{
				// 					DeleteFile(m_FlyQuaPrj.GetAdjRepPath());
				// 				}
				// 				else if(MessageBox(NULL,"��⵽�ʼ칤�����Ѵ���ƽ�����ļ����Ƿ��������ɣ�","��ʾ",MB_YESNOCANCEL)==IDCANCEL)
				// 					return;
			}
			m_bInRunning = true;
			HANDLE hThread;DWORD IdThread;
			hThread = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE)MultiThreadFlyQuaChkFun,
				(LPVOID*)this, 0, &IdThread);
// 			CString strImgFolder = FunGetFileFolder(m_strPrjFilePath)+"\\ImageManagement";
// 				FunRemoveDirAndFile(strImgFolder);
// 			while(PathFileExists(strImgFolder)) Sleep(100);
			
	}
	void CsswUAVFlyQuaSysDoc::OnUpdateButtonRunFlyquaCheck(CCmdUI *pCmdUI)
	{
		// TODO: �ڴ������������û����洦��������
		if(!m_bPrjLoaded) pCmdUI->Enable(m_bPrjLoaded);
		else if(m_pView->m_bStripResorted) pCmdUI->Enable(!m_bInRunning);
		else pCmdUI->Enable(m_pView->m_bStripResorted);
	}
	void CsswUAVFlyQuaSysDoc::OnButtonFlyquaReport()
	{
		// TODO: �ڴ���������������	
		m_strFlyQuaRepPath = m_FlyQuaPrj.GetCheckRepPath(m_eFlyQuaChkMethod);
		CString strOpenBat = FunGetFileFolder(m_strFlyQuaRepPath)+"\\RepOpen.bat";
		FILE *pfW = fopen(strOpenBat,"w"); 
		fprintf(pfW,"start %s\ndel %0",m_strFlyQuaRepPath);
		fclose(pfW); Sleep(10);//strOpenBat = "start "+m_strFlyQuaRepPath;
		ShellExecute(NULL, NULL, strOpenBat, NULL, NULL, SW_HIDE);

//		system(m_strFlyQuaRepPath);
// 		if(!PathFileExists(m_strFlyQuaRepPath))
// 		{
// 			CString strInfo;
// 			if(m_eFlyQuaChkMethod==BY_ADJUSTMENT) strInfo = "����ƽ����ʼ챨�治���ڣ�������ִ���ʼ������";else
// 				if(m_eFlyQuaChkMethod==BY_MATCH) strInfo = "����ƥ����ʼ챨�治���ڣ�������ִ���ʼ������";else
// 					if(m_eFlyQuaChkMethod==BY_POS) strInfo = "����POS���ʼ챨�治���ڣ�������ִ���ʼ������";
// 			AfxMessageBox(strInfo);
// 		}
// 		else
// 		{
// 			m_pMain->SendMessage(ID_MSG_OPEN_REPORT);
//		}

	}
	void CsswUAVFlyQuaSysDoc::OnUpdateButtonFlyquaReport(CCmdUI *pCmdUI)
	{
		// TODO: �ڴ������������û����洦��������
		if(!m_bPrjLoaded) pCmdUI->Enable(m_bPrjLoaded);
		else pCmdUI->Enable(!m_bInRunning);
	}
// 	void CsswUAVFlyQuaSysDoc::OnComboCheckMethod()
// 	{
// 		// TODO: �ڴ���������������
// 		CMFCRibbonComboBox *pComb = NULL;
// 		pComb = DYNAMIC_DOWNCAST(CMFCRibbonComboBox,m_pMain->m_wndRibbonBar.FindByID(ID_COMBO_CHECK_METHOD));
// 		m_eFlyQuaChkMethod = SSWFLYQUAMETHOD(pComb->GetCurSel());
// 	}
// 	void CsswUAVFlyQuaSysDoc::OnUpdateComboCheckMethod(CCmdUI *pCmdUI)
// 	{
// 		// TODO: �ڴ������������û����洦��������
// 		if(!m_bPrjLoaded) pCmdUI->Enable(FALSE);
// 		else pCmdUI->Enable(!m_bInRunning);
// 	}
	void CsswUAVFlyQuaSysDoc::OnUpdateButtonPrjNew(CCmdUI *pCmdUI)
	{
		// TODO: �ڴ������������û����洦��������
		if(!m_bPrjLoaded) pCmdUI->Enable(!m_bPrjLoaded);
		else pCmdUI->Enable(!m_bInRunning);
	}
	void CsswUAVFlyQuaSysDoc::OnUpdateButtonPrjOpen(CCmdUI *pCmdUI)
	{
		// TODO: �ڴ������������û����洦��������
		if(!m_bPrjLoaded) pCmdUI->Enable(!m_bPrjLoaded);
		else pCmdUI->Enable(!m_bInRunning);
	}
	void CsswUAVFlyQuaSysDoc::OnButtonPrjSave()
	{
		// TODO: �ڴ���������������
		if(m_bPrjLoaded) m_FlyQuaPrj.SavePrj2File(m_strPrjFilePath);
	}
	void CsswUAVFlyQuaSysDoc::OnUpdateButtonPrjSave(CCmdUI *pCmdUI)
	{
		// TODO: �ڴ������������û����洦��������
		if(!m_bPrjLoaded) pCmdUI->Enable(m_bPrjLoaded);
		else pCmdUI->Enable(!m_bInRunning);
	}
	void CsswUAVFlyQuaSysDoc::OnEditMeanHeight()
	{
		// TODO: �ڴ���������������
		CMFCRibbonEdit* pEdit = NULL; CString str;
		pEdit = DYNAMIC_DOWNCAST(CMFCRibbonEdit,m_pMain->m_wndRibbonBar.FindByID(ID_EDIT_MEAN_HEIGHT));
		m_dMeanHei=atof(pEdit->GetEditText());
		m_pView->m_nNodeLevel = int(m_dMeanHei);
		m_pView->m_nKeyNodeNum = m_pView->m_nNodeLevel * m_pView->m_nNodeLevel - m_pView->m_nNodeLevel + 1;
		m_pView->m_KeyNodePointRender.DeleteVBO();
		for (int i = 0; i<m_pView->m_vecKeyNodeLineRender.size(); i++)
		{
			m_pView->m_vecKeyNodeLineRender[i].DeleteVBO();
		}
		vector<CsswViewerRender>().swap(m_pView->m_vecKeyNodeLineRender);
		m_pView->Invalidate();
	}
	void CsswUAVFlyQuaSysDoc::OnUpdateEditMeanHeight(CCmdUI *pCmdUI)
	{
		// TODO: �ڴ������������û����洦��������
		if(!m_bPrjLoaded) pCmdUI->Enable(FALSE);
		else pCmdUI->Enable(!m_bInRunning);
	}




	void CsswUAVFlyQuaSysDoc::OnButtonDataOrder()
	{
		// TODO: �ڴ���������������

	}


	void CsswUAVFlyQuaSysDoc::OnUpdateButtonDataOrder(CCmdUI *pCmdUI)
	{
		// TODO: �ڴ������������û����洦��������
		pCmdUI->Enable(FALSE);
		// 		if(!m_bPrjLoaded) pCmdUI->Enable(false);else
		// 		if(m_bInRunning) pCmdUI->Enable(false);else
		// 			if(m_pView->m_vecCqExtRingRender.size()==0) pCmdUI->Enable(false);
		// 			else pCmdUI->Enable(true);
	}


// 	void CsswUAVFlyQuaSysDoc::OnEditMatchpointNumThresold()
// 	{
// 		// TODO: Add your command handler code here
// 		CMFCRibbonEdit* pEdit = NULL; CString str;
// 		pEdit = DYNAMIC_DOWNCAST(CMFCRibbonEdit,m_pMain->m_wndRibbonBar.FindByID(ID_EDIT_MATCHPOINT_NUM_THRESOLD));
// 		m_nMatchPointNumThresold=atoi(pEdit->GetEditText());
// 	}
// 
// 
// 	void CsswUAVFlyQuaSysDoc::OnUpdateEditMatchpointNumThresold(CCmdUI *pCmdUI)
// 	{
// 		// TODO: Add your command update UI handler code here
// 		if(!m_bPrjLoaded) pCmdUI->Enable(FALSE);
// 		else if(m_bInRunning) pCmdUI->Enable(FALSE);
// 		else if(m_eFlyQuaChkMethod==BY_POS) pCmdUI->Enable(FALSE);
// 		else pCmdUI->Enable(TRUE);
// 	}
