// sswPDFLib.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "sswPDFLib.h"
#include <string>
#include <algorithm>
#include <io.h>
#include "conio.h"
jmp_buf g_env;
void _stdcall error_handler(HPDF_STATUS error_no,HPDF_STATUS detail_no, void *user_data)
{
	char strLog[1024];
	sprintf(strLog,"ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,	(HPDF_UINT)detail_no);
	longjmp(g_env, 1);
	//setjmp(env);
}
void PDFStrcpyPosStr(char *pDest,int nStartPos,char *pSrc,int nSrcSize)
{
	int nPos = 0;int i = 0;
	for (i= 0;i < 120;i++)
	{
		*pDest = ' ';
		if (i == nStartPos)
		{
			strcpy(pDest,pSrc);
			i += nSrcSize;
			pDest += nSrcSize;
		}else
			pDest++;

	}
	pDest[i] = '\0';
}
BOOL PDFTextOut(HPDF_Page Page,HPDF_Font font,HPDF_REAL nx,HPDF_REAL ny,string szText,int nFontSize)
{
	if (!HPDF_Page_BeginText (Page))
	{
		if (!HPDF_Page_SetFontAndSize (Page, font, nFontSize))
		{
			if (!HPDF_Page_MoveTextPos (Page, nx,ny))
			{
				if (!HPDF_Page_ShowText (Page, szText.c_str()))
				{
					if (!HPDF_Page_EndText (Page))
					{

					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}else
		{
			return false;
		}
	}
	return TRUE;
}
void PutImage(HPDF_Page page, int x,int y,int nwidth, int nheight, HPDF_Image& hpdfImage)
{
	if (hpdfImage)
		HPDF_Page_DrawImage(page, hpdfImage,x,y,nwidth,nheight);
}
BOOL CreateDirectoryTree(string szFileName)
{
	if (szFileName.empty())
	{
		return TRUE;
	}
	string szFilePath = szFileName;
	int nblash  = -1;
	int nDirExist = -1;
	nblash = szFilePath.rfind('\\',szFilePath.size());
	if (nblash >= 0)
	{
		szFilePath.replace(nblash,szFilePath.size(),"");
		nDirExist = _access(szFilePath.c_str(),0);
		if (nDirExist == -1)//not exist
		{
			CreateDirectoryTree(szFilePath);
			CreateDirectory(szFilePath.c_str(),NULL);
		}else
		{	
			return TRUE; 
		}
	}else
	{
		return FALSE;
	}
}


CsswPdf::CsswPdf()
{
	m_pdf=NULL;
//	m_strLine = NULL;
//	memset(m_strLine,0,120);
	m_nLineFontHeight=8;
	m_nPageFontHeight=8;
	m_nLineWeight=120;
	m_nPages=0;
	m_nTailGap=40;
	m_nTitleGap=40;
	m_nLineGap=8;
	m_nLineSpace=50;
	
}
CsswPdf::~CsswPdf()
{
//	if(m_pdf) HPDF_Free((HPDF_Doc)m_pdf);
//	if(m_strLine!=NULL) delete[]((char*)m_strLine);
}
//jmp_buf g_env;
bool CsswPdf::InitPdf()
{	
	m_pdf = HPDF_New(error_handler,NULL);//创建一个pdf句柄
	if (m_pdf==NULL)
	{
		return false;
	}

	if(setjmp(g_env))//查看是否句柄产生异常
	{
		HPDF_Free((HPDF_Doc)m_pdf);
		return false;
	}

	if (!HPDF_SetCompressionMode((HPDF_Doc)m_pdf,HPDF_COMP_ALL))//设置压缩格式
	{
		if (!HPDF_SetPageMode((HPDF_Doc)m_pdf,HPDF_PAGE_MODE_USE_OUTLINE))//设置页面模式使用轮廓
		{

		}else
		{
			return false;
		}

	}else
	{
		return false;
	}
	HPDF_UseCNSFonts((HPDF_Doc)m_pdf);
	HPDF_UseCNTFonts((HPDF_Doc)m_pdf);
	HPDF_UseCNTEncodings((HPDF_Doc)m_pdf);
	HPDF_UseCNSEncodings((HPDF_Doc)m_pdf);
// 	if (!HPDF_UseCNSFonts((HPDF_Doc)m_pdf))
// 	{
// 		if (!HPDF_UseCNTFonts((HPDF_Doc)m_pdf))
// 		{
// 			if (!HPDF_UseCNTEncodings((HPDF_Doc)m_pdf))
// 			{
// 				if (!HPDF_UseCNSEncodings((HPDF_Doc)m_pdf))
// 				{
// 					return false;
// 				}
// 			}
// 		}
// 	}
	m_font = HPDF_GetFont ((HPDF_Doc)m_pdf, "SimSun", "GB-EUC-H");
	return true;
}

bool CsswPdf::AddPage(SSW_PDF_PageSize ePageSize /* = SSW_SIZE_A4 */, SSW_PDF_PageDirection ePageDirection /* = SSW_PORTAIT */)
{
	if(m_pdf==NULL) return false;
	HPDF_Page Page = HPDF_AddPage((HPDF_Doc)m_pdf);
	HPDF_PageSizes pageSize;
	HPDF_PageDirection pageDirection;
	switch(ePageSize)
	{
	case SSW_SIZE_LETTER:pageSize=HPDF_PAGE_SIZE_LETTER; break;
	case SSW_SIZE_A3:pageSize=HPDF_PAGE_SIZE_A3; break;
	case SSW_SIZE_A4:pageSize=HPDF_PAGE_SIZE_A4; m_nLineWeight = 120;break;
	case SSW_SIZE_A5:pageSize=HPDF_PAGE_SIZE_A5; break;
	default:pageSize=HPDF_PAGE_SIZE_A4; break;
	}
	switch(ePageDirection)
	{
	case SSW_PORTAIT:pageDirection=HPDF_PAGE_PORTRAIT; break;
	case SSW_LANDSCAPE:pageDirection=HPDF_PAGE_PORTRAIT; break;
	default:pageDirection=HPDF_PAGE_PORTRAIT; break;
	}
	HPDF_Page_SetSize (Page, pageSize, pageDirection);
	m_vecPages.push_back(Page);
	m_nPages++;
	m_nPageWidth=HPDF_Page_GetWidth(Page);
	m_nPageHeigh=HPDF_Page_GetHeight(Page);
	m_nWriteTop=m_nPageHeigh-m_nTitleGap;
	return true;
}
bool CsswPdf::SetPdfTitle(const char* strTitle, float r /* = 0 */, float g /* = 0 */, float b /* = 0 */, float fFontScale /* = 3 */)
{
	string strTitle2 = strTitle;
	if(m_vecPages.size()<=0) AddPage();
 	HPDF_Page page = (HPDF_Page)m_vecPages[m_vecPages.size()-1];
	HPDF_Page_SetRGBFill(page,r,g,b);
	HPDF_Page_BeginText (page);
	HPDF_Page_SetFontAndSize (page,(HPDF_Font)m_font, m_nLineFontHeight*fFontScale);
	int tw = HPDF_Page_TextWidth (page, strTitle);
	HPDF_Page_MoveTextPos (page, (HPDF_Page_GetWidth (page) - tw) / 2,HPDF_Page_GetHeight (page)-m_nTitleGap);
	HPDF_Page_ShowText (page, strTitle);
	HPDF_Page_EndText (page);
	m_nWriteTop = HPDF_Page_GetHeight (page)-m_nTitleGap - m_nLineFontHeight*fFontScale;
	int x0 = m_nLineSpace;
	int x1 = m_nPageWidth-m_nLineSpace;
	int y = m_nWriteTop+m_nLineFontHeight*fFontScale/2;
	HPDF_Page_SetLineWidth(page,1);
	HPDF_Page_SetRGBFill(page,0,0,0);
	HPDF_Page_MoveTo(page,x0,y);
	HPDF_Page_LineTo(page,x1,y);
	HPDF_Page_Stroke(page);
	return true;
}
bool CsswPdf::SetFont(SSW_PDF_FontType eType /* = SSW_SONG */, int nFontHeight /* = 8 */)
{
	if(m_pdf==NULL) return false;
	char strFontName[100], *strEncodingName= "GB-EUC-H";
	switch(eType)
	{
	case SSW_HEI:
		strcpy(strFontName,"SimHei");
		break;
	default:
		strcpy(strFontName,"SimSun");
		break;
	}
	m_font =HPDF_GetFont ((HPDF_Doc)m_pdf, strFontName, strEncodingName);
	m_nLineFontHeight = nFontHeight;
	return true;
}
void CsswPdf::SetTailGap(int nGap/* =40 */)
{
	if(nGap<=0) return;
	m_nTailGap = nGap;
}
void CsswPdf::SetTitleGap(int nGap/* =40 */)
{
	if(nGap<=0) return;
	m_nTitleGap=nGap;
}
void CsswPdf::SetLineGap(int nGap/* =16 */)
{
	if(nGap<=m_nLineFontHeight) return;
	m_nLineGap=nGap;
}
void CsswPdf::SetLineSpace(int nSpace/* =2 */)
{
	m_nLineSpace=nSpace;
}
int CsswPdf::GetTextWidthInPage(const char *strText)
{
	if(strText == NULL) return 0;
	else return HPDF_Page_TextWidth((HPDF_Page)m_vecPages[m_vecPages.size()-1],strText);
}
int CsswPdf::GetWriteTopInPage()
{
	return m_nWriteTop;
}
bool CsswPdf::AddText(const char* strText, SSW_PDF_FontType eFontType /* = SSW_SONG */, float fFontScale /* = 1.5 */,
	SSW_PDF_ContentStruct eContentStruct /* = SSW_LEFT */, int nPageCount2NewPage /* = 0 */,bool bCutLine /* = false */,
	float r /* = 0.0 */,float g /* =0.0 */,float b /* = 0.0 */,int nPosX /* = 0 */,int nPosY /* = 0 */,
	int nSpace /* = 0 */, bool bAddPage /* = true */)
{
	if(strlen(strText)==0) 
		return false;
	if(m_vecPages.size()==0) AddPage();
	int nleft = 0, nstrLenth = strlen(strText);
	do {
		int nPosX2, nlenLine;
		if(eContentStruct!=SSW_NONE){
			if(m_nWriteTop-m_nLineGap<=m_nTailGap)//当前页结束
			{
				if(bAddPage) 
				{
					AddPage();
					m_nWriteTop=m_nPageHeigh-m_nTitleGap;
					nPosY=0;
				}
			}	
			if(nPosY==0) nPosY = m_nWriteTop;
		}
		HPDF_Page page = (HPDF_Page)m_vecPages[m_vecPages.size()-1-nPageCount2NewPage];
		HPDF_Page_SetRGBFill(page,r,g,b);    //字体颜色
		HPDF_Page_SetRGBStroke(page,0.0,0,0); //边框色
		HPDF_Page_BeginText (page);
		//HPDF_Page_Stroke(page);
		SetFont(eFontType);
		HPDF_Page_SetFontAndSize (page,(HPDF_Font)m_font, m_nLineFontHeight*fFontScale);
		int tw = HPDF_Page_TextWidth (page, strText+nleft);	
		nlenLine =min(HPDF_Page_GetWidth(page)-2*m_nLineSpace,tw); 
		if(eContentStruct==SSW_LEFT) nPosX2 = m_nLineSpace;else
			if(eContentStruct==SSW_CENTER) nPosX2=(HPDF_Page_GetWidth (page) - nlenLine) / 2;else
				if(eContentStruct==SSW_RIGHT) nPosX2=(HPDF_Page_GetWidth (page) - nlenLine)-m_nLineSpace;else
					nPosX2=nPosX;
	//	char *strTmp = new char[nlenLine*nstrLenth/tw]; memset(strTmp,0,nlenLine*nstrLenth/tw);
		CString strTmp = strText;
		strTmp = strTmp.Mid(nleft,min(strlen(strText)-nleft,nlenLine*nstrLenth/tw));
	//	strncpy(strTmp,strText+nleft,nlenLine*nstrLenth/tw);
		nleft+=nlenLine*nstrLenth/tw;	
	//	strTmp[nlenLine*nstrLenth/tw]='\0';
		HPDF_Page_MoveTextPos (page,nPosX2,nPosY);
		HPDF_Page_ShowText (page, strTmp);
	//	if(strTmp) delete[]strTmp;
		/*-m_nLineGap*/;
		nPosY -= 2*m_nLineFontHeight;
	//	AddEmptyLine(1.5);
		m_nWriteTop = nPosY+8;
		HPDF_Page_EndText(page);
	} while (nleft<nstrLenth&&!bCutLine);
	/*HPDF_Page_EndText ((HPDF_Page)m_vecPages[m_vecPages.size()-1]);*/
	return true;
}
bool CsswPdf::AddEmptyLine(float fScaleLineGap /* = 1 *//*, bool bAddPage = true*/)
{
	m_nWriteTop-=fScaleLineGap*m_nLineGap;
	if(m_nWriteTop<=m_nTailGap)
	{
		AddPage();
		m_nWriteTop=m_nPageHeigh-m_nTitleGap-m_nLineGap;
	}
	return true;
}


bool CsswPdf::AddImage(const char* strImageFilePath,float fScale /* = 1.0 */, 
	SSW_PDF_ContentStruct eContentStruct/* =SSW_CENTER */,
	int pos_x/* =0 */,int pos_y/* =0 */,int nw/* =0 */, int nh/* =0 */, bool bAddLabel /*= false*/)
{
	if(access(strImageFilePath,0)!=0) return false;
	char strExt2[100]; const char *ps = strrchr(strImageFilePath,'.');
	strcpy(strExt2,ps+1); string strExt= strExt2;
	transform(strExt.begin(),strExt.end(),strExt.begin(),tolower);
	HPDF_Image hImg = NULL;
	if(strExt=="jpg"||strExt=="jpeg"||strExt=="dll") hImg=HPDF_LoadJpegImageFromFile((HPDF_Doc)m_pdf,strImageFilePath);
	else if(strExt=="png") hImg=HPDF_LoadPngImageFromFile((HPDF_Doc)m_pdf,strImageFilePath);
	else return false;
	if(hImg==NULL) return false;
	int nShowWidth, nShowHeight;
	int nPosX,nPosY;
	int img_w=HPDF_Image_GetWidth(hImg);
	int img_h=HPDF_Image_GetHeight(hImg);
	if(eContentStruct!=SSW_NONE){		
		float fSizeScale = min(1.0*img_w/m_nPageWidth,1.0*img_h/m_nPageHeigh)*fScale;
		if(img_w<img_h)
		{
			nShowHeight = fSizeScale*m_nPageHeigh;
			nShowWidth = nShowHeight*img_w/img_h;
		}
		else
		{
			nShowWidth = fSizeScale*m_nPageWidth;
			nShowHeight = nShowWidth*img_h/img_w;
		}
		nShowHeight = min(img_h*fScale,m_nPageHeigh/2);
		nShowWidth = img_w*nShowHeight/img_h;
		if(nShowWidth>m_nPageWidth*2/3)
		{
			nShowWidth=m_nPageWidth*2/3;
			nShowHeight = img_h*nShowWidth/img_w;
		}
		switch(eContentStruct)
		{
		case SSW_CENTER:
			nPosX=(m_nPageWidth-nShowWidth)/2;
			break;
		case SSW_LEFT:
			nPosX=0;
			break;
		case SSW_RIGHT:
			nPosX=m_nPageWidth-nShowWidth;
			break;
		}
		if(m_nWriteTop-m_nLineGap*2-nShowHeight<m_nTailGap)  //当页空白处不足，新增一页
		{
			AddPage();
			m_nWriteTop=m_nPageHeigh-m_nTitleGap;
		}
		m_nWriteTop=m_nWriteTop-nShowHeight;
		nPosY=m_nWriteTop;
		m_nWriteTop-=m_nLineGap;
	}
	else
	{
		if(nw>nh)
		{
			nShowWidth=nw;
			nShowHeight = img_h*nw/img_w;
		}else
		{
			nShowHeight=nh;
			nShowWidth = img_w*nh/img_h;
		}
		nPosX = m_nPageWidth-m_nLineSpace-nShowWidth;
		nPosY = m_nPageHeigh-m_nTitleGap-m_nLineGap;
	}
	PutImage((HPDF_Page)m_vecPages[m_vecPages.size()-1],nPosX,nPosY,nShowWidth,nShowHeight,hImg);

	if(bAddLabel)
	{
		nPosY-=m_nLineGap;
		nPosX  = nPosX+nShowWidth/3/*+nShowWidth*2/3/7/2*/;
		char *vStrInfo[7] = {"0","1","2","3","4","5","10"};
		AddText("影像重叠数",SSW_SONG,1,SSW_NONE,0,0,0,0,0,max(0,nPosX-60),nPosY);
		for (int i = 0; i<7; i++)
		{
			  AddText(vStrInfo[i],SSW_SONG,1,SSW_NONE,0,0,0,0,0,nPosX+i*nShowWidth*2/3/7,nPosY);
		}
	}
	return true;
}
bool CsswPdf::AddList(int nCols, int nRows,char** pListInfo)
{
//	AllocConsole();  
	cprintf("Cols = %d   Rows = %d\n",nCols,nRows);
	if(m_vecPages.size()==0) AddPage();
	HPDF_Page page = (HPDF_Page) m_vecPages[m_vecPages.size()-1];
	HPDF_Page_SetLineWidth(page,0.5);
	HPDF_Page *pListRowPage = new HPDF_Page[nRows];   //List每一行所在页
	//memset(pListRowPage,page,nRows*sizeof(HPDF_Page));
	for(int i = 0; i<nRows; i++) *(pListRowPage+i) = page;
	
	int *pColLeft   = new int[nCols]; memset(pColLeft,0,nCols*sizeof(int));
	int *pRowTop    = new int[nRows];memset(pRowTop,0,nRows*sizeof(int));
	int *pRowBot    = new int[nRows];memset(pRowBot,0,nRows*sizeof(int));
	int *pColWidth  = new int[nCols];memset(pColWidth,0,nCols*sizeof(int));
	int *pRowHei    = new int[nRows];memset(pRowHei,0,nRows*sizeof(int));
	int *pColMaxLen = new int[nCols]; memset(pColMaxLen,0,nCols*sizeof(int));

	int nListWidth = m_nPageWidth-2*m_nLineSpace;
	int nListHeight = 0;	int nSumLen = 0;

	for (int i = 0; i<nCols; i++)
	{
		pColMaxLen[i]=0;
		for (int j = 0; j<nRows; j++)
		{
			char strlog[1024];
			sprintf(strlog,"%d-%d\n",i,j);
			cprintf(strlog);
			pColMaxLen[i] = max(pColMaxLen[i],HPDF_Page_TextWidth (page, pListInfo[j*nCols+i]));
		}
		pColMaxLen[i]+=2;
		nSumLen+=pColMaxLen[i];
	}
	int nLeftLast = m_nLineSpace;
	for (int i = 0; i<nCols; i++)
	{
		pColWidth[i] = pColMaxLen[i]*nListWidth/nSumLen;
		pColLeft[i] = nLeftLast;
		nLeftLast+=pColWidth[i];
	}
	for (int i =0; i<nRows; i++)
	{
		nListHeight+=2*m_nLineFontHeight;
		pRowHei[i]=2*m_nLineFontHeight;
	}
	cprintf("a\n");
	int nLeft0 = m_nLineSpace;
	int xl,xr,yt,yb;
	xl = nLeft0;
	xr = m_nPageWidth-m_nLineSpace;
	for (int i = 0; i<nCols; i++)
	{
		pColLeft[i] = xl;
		xl=pColLeft[i]+pColWidth[i];
	}

	yt = m_nWriteTop; 
	xl = nLeft0;
	xr = xl+nListWidth;
	vector<int> vecRowPageIdx(nRows);
	for (int i = 0; i<nRows; i++)
	{
		yb = yt-pRowHei[i];
		if(yb<m_nTailGap)
		{
			AddPage();
			page = (HPDF_Page)m_vecPages[m_vecPages.size()-1];	
			HPDF_Page_SetLineWidth(page,0.5);
			m_nWriteTop = m_nPageHeigh-m_nTitleGap;
			yt = m_nWriteTop;yb = yt-pRowHei[i];
		}
		pRowTop[i] = yt;
		pRowBot[i] = yb;
		pListRowPage[i]=page;
		vecRowPageIdx[i] = m_vecPages.size()-1;
		yt = yb; m_nWriteTop = yb-m_nLineGap;
	}
	cprintf("b\n");
	//	m_nWriteTop = pRowBot[nRows-1] - m_nLineGap;
	//绘制横线
	vector<int> vecRowIdx2ChangePage;
	vecRowIdx2ChangePage.push_back(0);
	for (int i = 0; i<nRows; i++)
	{
		yt = pRowTop[i];  yb = pRowBot[i];
		HPDF_Page_SetGrayStroke (pListRowPage[i], 0.5);
		HPDF_Page_MoveTo(pListRowPage[i],xl,yt); 
		HPDF_Page_LineTo(pListRowPage[i],xr,yt);	
		HPDF_Page_Stroke(pListRowPage[i]);
		if(i<nRows-1)
		{
			if(pListRowPage[i]!=pListRowPage[i+1]) //当前行之后换页，绘制该行下面一条线
			{
				HPDF_Page_SetGrayStroke (pListRowPage[i], 0.5);
 				HPDF_Page_MoveTo(pListRowPage[i],xl,yb); 
 				HPDF_Page_LineTo(pListRowPage[i],xr,yb);
				HPDF_Page_Stroke(pListRowPage[i]);
				vecRowIdx2ChangePage.push_back(i+1);
			}
		}
	}
	cprintf("c\n");
	//绘制竖线
	HPDF_Page_MoveTo(pListRowPage[nRows-1],xl,pRowTop[nRows-1]-pRowHei[nRows-1]);
	HPDF_Page_LineTo(pListRowPage[nRows-1],xr,pRowTop[nRows-1]-pRowHei[nRows-1]);
	HPDF_Page_Stroke(pListRowPage[nRows-1]);
	if(vecRowIdx2ChangePage.size()==1) //表格在同一页
	{
		page = (HPDF_Page)m_vecPages[m_vecPages.size()-1];
		yt = pRowTop[0]; yb = yt-nListHeight;
		for (int i = 0; i<nCols; i++)
		{
			xl = pColLeft[i];
			HPDF_Page_MoveTo(page,xl,yt);
			HPDF_Page_LineTo(page,xl,yb);
			HPDF_Page_Stroke(page);
		}
		HPDF_Page_MoveTo(page,nLeft0+nListWidth,yt);
		HPDF_Page_LineTo(page,nLeft0+nListWidth,yb);
		HPDF_Page_Stroke(page);	
		HPDF_Page_SetGrayStroke (page, 1);
	}
	else
	{
		for (int i = 0; i<vecRowIdx2ChangePage.size(); i++)
		{	
	//		page = pListRowPage[vecRowIdx2ChangePage[i]];
			page =(HPDF_Page) m_vecPages[vecRowPageIdx[vecRowIdx2ChangePage[i]]];
			HPDF_Page_SetGrayStroke (page, 0.5);
			yt = pRowTop[vecRowIdx2ChangePage[i]];
			yb = yt;
			if(i<vecRowIdx2ChangePage.size()-1)
			{
				for (int j = vecRowIdx2ChangePage[i]; j<vecRowIdx2ChangePage[i+1]; j++)
				{
					yb-=pRowHei[j];
				}
			}
			else
			{
				for (int j = vecRowIdx2ChangePage[i]; j<nRows; j++)
				{
					yb-=pRowHei[j];
				}
			}
			for (int k = 0; k<nCols; k++)
			{
				xl = pColLeft[k];
				HPDF_Page_MoveTo(page,xl,yt);
				HPDF_Page_LineTo(page,xl,yb);
				HPDF_Page_Stroke(page);
			}
			HPDF_Page_MoveTo(page,nLeft0+nListWidth,yt);
			HPDF_Page_LineTo(page,nLeft0+nListWidth,yb);	
			HPDF_Page_Stroke(page);
			HPDF_Page_SetGrayStroke (page, 1);
		}
	}
		cprintf("d\n");
	//	显示list内容
	float r, g, b;
	for (int i= 0; i<nRows; i++)
	{
		yt = pRowTop[i]-(pRowHei[i]+m_nLineFontHeight)/2;
		for (int j = 0; j<nCols; j++)
		{
			xl = pColLeft[j]+2;
			r=g=b=0;
			AddText(pListInfo[i*nCols+j],SSW_SONG,1.5,SSW_NONE,
				m_vecPages.size()-vecRowPageIdx[i]-1,false,r,g,b,xl,yt,0,false);
		//	AddEmptyLine(1);
			m_nWriteTop = yt-m_nLineGap;
		}
	}
	cprintf("e\n");

	delete[]pColLeft    ;pColLeft     = NULL;
	delete[]pRowTop     ;pRowTop      = NULL;
	delete[]pRowBot     ;pRowBot      = NULL;
	delete[]pColWidth   ;pColWidth    = NULL;
	delete[]pRowHei     ;pRowHei      = NULL;
	delete[]pColMaxLen  ;pColMaxLen   = NULL;
	delete[]pListRowPage;pListRowPage = NULL;
	vector<int>().swap(vecRowPageIdx);
	vector<int>().swap(vecRowIdx2ChangePage);
	cprintf("f\n");
	return true;

}
bool CsswPdf::AddList(int nCols, int nRows,vector<CString>pListInfo, bool bTitle/* = true*/)
{
	if(m_vecPages.size()==0) AddPage();
	HPDF_Page page = (HPDF_Page) m_vecPages[m_vecPages.size()-1];
	HPDF_Page_SetLineWidth(page,0.5);
// 	HPDF_Page *pListRowPage = new HPDF_Page[nRows];   //List每一行所在页
// 	for(int i = 0; i<nRows; i++) *(pListRowPage+i) = page;

// 	int *pColLeft   = new int[nCols]; memset(pColLeft,0,nCols*sizeof(int));
// 	int *pRowTop    = new int[nRows];memset(pRowTop,0,nRows*sizeof(int));
// 	int *pRowBot    = new int[nRows];memset(pRowBot,0,nRows*sizeof(int));
// 	int *pColWidth  = new int[nCols];memset(pColWidth,0,nCols*sizeof(int));
// 	int *pRowHei    = new int[nRows];memset(pRowHei,0,nRows*sizeof(int));
// 	int *pColMaxLen = new int[nCols]; memset(pColMaxLen,0,nCols*sizeof(int));
//	vector<HPDF_Page> pListRowPage; pListRowPage.assign(nRows,page);
	vector<int>pColLeft  ;pColLeft.assign(nCols,0);  
	vector<int>pRowTop   ;pRowTop.assign(nRows,0);   
	vector<int>pRowBot   ;pRowBot.assign(nRows,0);      
	vector<int>pColWidth ;pColWidth.assign(nCols,0);    
	vector<int>pRowHei   ;pRowHei.assign(nRows,0);      
	vector<int>pColMaxLen;pColMaxLen.assign(nCols,0);   


	int nListWidth = m_nPageWidth-2*m_nLineSpace;
	int nListHeight = 0;	int nSumLen = 0;

	for (int i = 0; i<nCols; i++)
	{
		pColMaxLen[i]=0;
		for (int j = 0; j<nRows; j++)
		{
			pColMaxLen[i] = max(pColMaxLen[i],/*HPDF_Page_TextWidth(page, pListInfo[j*nCols+i])*/pListInfo[j*nCols+i].GetLength());
		}
		pColMaxLen[i]+=2;
		nSumLen+=pColMaxLen[i];
	}
	int nLeftLast = m_nLineSpace;
	for (int i = 0; i<nCols; i++)
	{
		pColWidth[i] = pColMaxLen[i]*nListWidth/nSumLen;
		pColLeft[i] = nLeftLast;
		nLeftLast+=pColWidth[i];
	}
	for (int i =0; i<nRows; i++)
	{
		nListHeight+=2*m_nLineFontHeight;
		pRowHei[i]=2*m_nLineFontHeight;
	}
	cprintf("a ");
	int nLeft0 = m_nLineSpace;
	int xl,xr,yt,yb;
	xl = nLeft0;
	xr = m_nPageWidth-m_nLineSpace;
	for (int i = 0; i<nCols; i++)
	{
		pColLeft[i] = xl;
		xl=pColLeft[i]+pColWidth[i];
	}

	yt = m_nWriteTop; 
	xl = nLeft0;
	xr = xl+nListWidth;
	vector<int> vecRowPageIdx(nRows);
	for (int i = 0; i<nRows; i++)
	{
		yb = yt-pRowHei[i];
		if(yb<m_nTailGap)
		{
			AddPage();
			page = (HPDF_Page)m_vecPages[m_vecPages.size()-1];	
			HPDF_Page_SetLineWidth(page,0.5);
			m_nWriteTop = m_nPageHeigh-m_nTitleGap;
			yt = m_nWriteTop;yb = yt-pRowHei[i];
		}
		pRowTop[i] = yt;
		pRowBot[i] = yb;
	//	pListRowPage[i]=page;
		vecRowPageIdx[i] = m_vecPages.size()-1;
		yt = yb; m_nWriteTop = yb-m_nLineGap;
	}
	cprintf("b ");
	//	m_nWriteTop = pRowBot[nRows-1] - m_nLineGap;
	//绘制横线
	vector<int> vecRowIdx2ChangePage;
	vecRowIdx2ChangePage.push_back(0);
	for (int i = 0; i<nRows; i++)
	{
		yt = pRowTop[i];  yb = pRowBot[i];
		HPDF_Page_SetGrayStroke ((HPDF_Page)m_vecPages[vecRowPageIdx[i]], 0.5);
		HPDF_Page_MoveTo((HPDF_Page)m_vecPages[vecRowPageIdx[i]],xl,yt); 
		HPDF_Page_LineTo((HPDF_Page)m_vecPages[vecRowPageIdx[i]],xr,yt);	
		HPDF_Page_Stroke((HPDF_Page)m_vecPages[vecRowPageIdx[i]]);
		if(i<nRows-1)
		{
			if((HPDF_Page)m_vecPages[vecRowPageIdx[i]]!=(HPDF_Page)m_vecPages[vecRowPageIdx[i+1]]) //当前行之后换页，绘制该行下面一条线
			{
				HPDF_Page_SetGrayStroke ((HPDF_Page)m_vecPages[vecRowPageIdx[i]], 0.5);
				HPDF_Page_MoveTo((HPDF_Page)m_vecPages[vecRowPageIdx[i]],xl,yb); 
				HPDF_Page_LineTo((HPDF_Page)m_vecPages[vecRowPageIdx[i]],xr,yb);
				HPDF_Page_Stroke((HPDF_Page)m_vecPages[vecRowPageIdx[i]]);
				vecRowIdx2ChangePage.push_back(i+1);
			}
		}
	}
	cprintf("c ");
	//绘制竖线
	HPDF_Page_MoveTo((HPDF_Page)m_vecPages[vecRowPageIdx[nRows-1]],xl,pRowTop[nRows-1]-pRowHei[nRows-1]);
	HPDF_Page_LineTo((HPDF_Page)m_vecPages[vecRowPageIdx[nRows-1]],xr,pRowTop[nRows-1]-pRowHei[nRows-1]);
	HPDF_Page_Stroke((HPDF_Page)m_vecPages[vecRowPageIdx[nRows-1]]);
	if(vecRowIdx2ChangePage.size()==1) //表格在同一页
	{
		page = (HPDF_Page)m_vecPages[m_vecPages.size()-1];
		yt = pRowTop[0]; yb = yt-nListHeight;
		for (int i = 0; i<nCols; i++)
		{
			xl = pColLeft[i];
			HPDF_Page_MoveTo(page,xl,yt);
			HPDF_Page_LineTo(page,xl,yb);
			HPDF_Page_Stroke(page);
		}
		HPDF_Page_MoveTo(page,nLeft0+nListWidth,yt);
		HPDF_Page_LineTo(page,nLeft0+nListWidth,yb);
		HPDF_Page_Stroke(page);	
		HPDF_Page_SetGrayStroke (page, 1);
	}
	else
	{
		for (int i = 0; i<vecRowIdx2ChangePage.size(); i++)
		{	
			//		page = pListRowPage[vecRowIdx2ChangePage[i]];
			page =(HPDF_Page) m_vecPages[vecRowPageIdx[vecRowIdx2ChangePage[i]]];
			HPDF_Page_SetGrayStroke (page, 0.5);
			yt = pRowTop[vecRowIdx2ChangePage[i]];
			yb = yt;
			if(i<vecRowIdx2ChangePage.size()-1)
			{
				for (int j = vecRowIdx2ChangePage[i]; j<vecRowIdx2ChangePage[i+1]; j++)
				{
					yb-=pRowHei[j];
				}
			}
			else
			{
				for (int j = vecRowIdx2ChangePage[i]; j<nRows; j++)
				{
					yb-=pRowHei[j];
				}
			}
			for (int k = 0; k<nCols; k++)
			{
				xl = pColLeft[k];
				HPDF_Page_MoveTo(page,xl,yt);
				HPDF_Page_LineTo(page,xl,yb);
				HPDF_Page_Stroke(page);
			}
			HPDF_Page_MoveTo(page,nLeft0+nListWidth,yt);
			HPDF_Page_LineTo(page,nLeft0+nListWidth,yb);	
			HPDF_Page_Stroke(page);
			HPDF_Page_SetGrayStroke (page, 1);
		}
	}
	cprintf("d ");
	//	显示list内容
	float r, g, b;
	if(bTitle)
	{
		yt = pRowTop[0]-(pRowHei[0]+m_nLineFontHeight)/2;
		for (int j = 0; j<nCols; j++)
		{
			xl = pColLeft[j]+2;
			r=g=b=0;
			AddText(pListInfo[0*nCols+j],SSW_HEI,1,SSW_NONE,	m_vecPages.size()-vecRowPageIdx[0]-1,false,r,g,b,xl,yt,0,false);
		}	
		m_nWriteTop = yt-m_nLineGap;
		for (int i= 1; i<nRows; i++)
		{
			yt = pRowTop[i]-(pRowHei[i]+m_nLineFontHeight)/2;
			for (int j = 0; j<nCols; j++)
			{
				xl = pColLeft[j]+2;
				r=g=b=0;
				AddText(pListInfo[i*nCols+j],SSW_SONG,1,SSW_NONE,	m_vecPages.size()-vecRowPageIdx[i]-1,false,r,g,b,xl,yt,0,false);
			}	
			m_nWriteTop = yt-m_nLineGap;
		}
	}else
	{
		for (int i= 0; i<nRows; i++)
		{
			yt = pRowTop[i]-(pRowHei[i]+m_nLineFontHeight)/2;
			for (int j = 0; j<nCols; j++)
			{
				xl = pColLeft[j]+2;
				r=g=b=0;
				AddText(pListInfo[i*nCols+j],SSW_SONG,1,SSW_NONE,	m_vecPages.size()-vecRowPageIdx[i]-1,false,r,g,b,xl,yt,0,false);
			}	
			m_nWriteTop = yt-m_nLineGap;
		}
	}
	cprintf("e ");

// 	delete[]pColLeft    ;pColLeft     = NULL;
// 	delete[]pRowTop     ;pRowTop      = NULL;
// 	delete[]pRowBot     ;pRowBot      = NULL;
// 	delete[]pColWidth   ;pColWidth    = NULL;
// 	delete[]pRowHei     ;pRowHei      = NULL;
// 	delete[]pColMaxLen  ;pColMaxLen   = NULL;
//	delete[]pListRowPage;pListRowPage = NULL;
	vector<int>().swap(pColLeft  );
	vector<int>().swap(pRowTop   );
	vector<int>().swap(pRowBot   );
	vector<int>().swap(pColWidth );
	vector<int>().swap(pRowHei   );
	vector<int>().swap(pColMaxLen);
	vector<int>().swap(vecRowPageIdx);
	vector<int>().swap(vecRowIdx2ChangePage);
// 	pColLeft.~vector();
// 	pRowTop.~vector();    
// 	pRowBot.~vector();    
// 	pColWidth.~vector();  
// 	pRowHei.~vector();    
// 	pColMaxLen.~vector();
// 	vecRowPageIdx.~vector();
// 	vecRowIdx2ChangePage.~vector();
	
	//Sleep(500);
	cprintf("f ");
	return true;
}


bool CsswPdf::AddChart(float *pY, float *pX, int nPointCount, float r, float g, float b,int nPosX, int nPosY,
	float fChartExtendSize /* = 1.0 */, int nWidth /* = 0 */, int nHeight /* = 0 */,const char *pTitle /* = NULL */,
	const char* pAxisXLabel /* = NULL */, const char* pAxisYLabel /* = NULL */, SSW_PDF_ChartType eType /* = SSW_LINE */,
	SSW_PDF_ContentStruct eContent /* = SSW_CENTER */)
{
	if(pX==NULL||pY==NULL||nPointCount<2) return false;
	if(m_vecPages.size()==0) AddPage();
	//转换至pdf坐标
	int nChartWidth,nChartHeight;
	if(nWidth<=m_nPageWidth/5) nChartWidth = m_nPageWidth*3/5;
	else nChartWidth = nWidth;
	if(nHeight <=50) nChartHeight = 180;
	else nChartHeight = nHeight;
	int left,right,top,bottom, nPosYTitle;
	if(eContent==SSW_CENTER) left=(m_nPageWidth-nChartWidth)/2;else
		if(eContent==SSW_LEFT) left=m_nLineSpace;else
			if(eContent==SSW_RIGHT) left = m_nPageWidth-nChartWidth-m_nLineSpace;
	right = left+nChartWidth;
	int nPicHeight=nChartHeight+7*m_nLineFontHeight;
	HPDF_Page page ;
	if(nPosY==0)
	{
		if(m_nWriteTop-nPicHeight<m_nTailGap) AddPage();
		page = (HPDF_Page) m_vecPages[m_vecPages.size()-1];
		top = m_nWriteTop - m_nLineFontHeight;
	}
	else
	{
		if(nPosY-nPicHeight<m_nTailGap) 
		{
			AddPage(); 
			nPosY = m_nWriteTop;
		}
		page = (HPDF_Page) m_vecPages[m_vecPages.size()-1];
		top = nPosY - m_nLineFontHeight;
	}
 	bottom = top-nChartHeight;

	float fMaxY=pY[0],fMaxX=pX[0],fMinY=pY[0],fMinX=pX[0];
	for (int i = 0; i<nPointCount; i++)
	{
		fMinX=min(fMinX,*(pX+i));
		fMinY=min(fMinY,*(pY+i));
		fMaxX=max(fMaxX,*(pX+i));
		fMaxY=max(fMaxY,*(pY+i));
	}
	float *pXPos = new float[nPointCount]; memcpy(pXPos,pX,sizeof(float)*nPointCount); 
	float *pYPos = new float[nPointCount]; memcpy(pYPos,pY,sizeof(float)*nPointCount); 
	if(fMaxX==fMinX) 
		for(int i = 0; i<nPointCount; i++) 
			*(pXPos+i)=left+nChartWidth*1.0/2;
	else
		for(int i = 0; i<nPointCount; i++) 
			*(pXPos+i)=(*(pX+i)-fMinX)*nChartWidth/(fMaxX-fMinX)+left;
	if(fMaxY==fMinY) 
		for(int i = 0; i<nPointCount; i++) 
			*(pYPos+i)=top-nChartHeight*1.0/2;
	else
		for(int i = 0; i<nPointCount; i++) 
			*(pYPos+i)=(*(pY+i)-fMinY)*nChartHeight/(fMaxY-fMinY)+top-nChartHeight;
	
	float fMaxYPos=pYPos[0],fMaxXPos=pXPos[0],fMinYPos=pYPos[0],fMinXPos=pXPos[0];
	for (int i = 0; i<nPointCount; i++)
	{
		fMinXPos=min(fMinXPos,*(pXPos+i));
		fMinYPos=min(fMinYPos,*(pYPos+i));
		fMaxXPos=max(fMaxXPos,*(pXPos+i));
		fMaxYPos=max(fMaxYPos,*(pYPos+i));
	}
	
	
	//绘制坐标轴-矩形
	HPDF_Page_SetRGBStroke(page,0.3,0.3,0.3);
	HPDF_Page_SetLineWidth(page,2);
	HPDF_Page_MoveTo (page, left,top);	HPDF_Page_LineTo (page, left,bottom);
	HPDF_Page_MoveTo (page, left,top);	HPDF_Page_LineTo (page, right,top);
	HPDF_Page_MoveTo (page, right,bottom);	HPDF_Page_LineTo (page, left,bottom);
	HPDF_Page_MoveTo (page, right,bottom);	HPDF_Page_LineTo (page, right,top);
	HPDF_Page_Stroke(page);
	float x0,y0,x1,y1;
	HPDF_Page_SetRGBStroke(page,r,g,b);
	HPDF_Page_SetLineWidth(page,fChartExtendSize);
	x0 = *pXPos;y0 = *pYPos;
	for (int i = 1; i<=nPointCount-1; i++)
	{
		x1 = *(pXPos+i);y1 = *(pYPos+i);
		HPDF_Page_MoveTo (page, x0,y0);
		HPDF_Page_LineTo (page, x1,y1);
		HPDF_Page_Stroke(page);
		x0 = x1; y0 = y1;
	}
	if(pTitle!=NULL) 
	{
		AddText(pTitle,SSW_SONG, 1,SSW_CENTER,0,false,0,0,0,0,0,0,true);
		AddEmptyLine(1);
	}
//	m_nWriteTop = bottom-m_nLineGap;
	int nGridXCount = min(nPointCount-1,10);
	if(pAxisXLabel!=NULL)
	{
		float fXPosGap = (fMaxXPos-fMinXPos)/nGridXCount;
		int pos_y = bottom-m_nLineGap;
		if(nGridXCount==10)
		{
			for (int i = 0; i<nGridXCount+1 ;i++)
			{
				char strXlabel[1024];
				int nId = pX[min(max(0,0+i*nPointCount/nGridXCount-1),int(pX[nPointCount-1]))];
				sprintf(strXlabel,"%d",nId);
				int pos_x = fMinXPos+i*fXPosGap-HPDF_Page_TextWidth (page, strXlabel)/2;
				AddText(strXlabel,SSW_SONG,1, SSW_NONE,0,false,0,0,0,pos_x,pos_y,0,true);
				//	AddEmptyLine(1);
				if(i>0&&i<nGridXCount)
				{	
					x0 = pos_x; y0=top;
					x1 = pos_x; y1 = bottom;
					HPDF_Page_SetGrayStroke (page, 0.3);
					HPDF_Page_SetLineWidth(page,0.05);
					HPDF_Page_MoveTo (page, x0,y0);
					HPDF_Page_LineTo (page, x1,y1);
					HPDF_Page_Stroke(page);
					HPDF_Page_SetGrayStroke (page, 1.0);
				}
			}
		}else
		{
			for (int i = 0; i<nGridXCount+1 ;i++)
			{
				char strXlabel[1024];
				int nId = pX[i];
				sprintf(strXlabel,"%d",nId);
				int pos_x = fMinXPos+i*fXPosGap-HPDF_Page_TextWidth (page, strXlabel)/2;
				AddText(strXlabel,SSW_SONG,1, SSW_NONE,0,false,0,0,0,pos_x,pos_y,0,true);
				//	AddEmptyLine(1);
				if(i>0&&i<nGridXCount)
				{	
					x0 = pos_x; y0=top;
					x1 = pos_x; y1 = bottom;
					HPDF_Page_SetGrayStroke (page, 0.3);
					HPDF_Page_SetLineWidth(page,0.05);
					HPDF_Page_MoveTo (page, x0,y0);
					HPDF_Page_LineTo (page, x1,y1);
					HPDF_Page_Stroke(page);
					HPDF_Page_SetGrayStroke (page, 1.0);
				}
			}
		}
		m_nWriteTop = pos_y-m_nLineGap;
		AddText(pAxisXLabel,SSW_SONG, 1,SSW_CENTER,0,false,0,0,0,0,0,0,true);
		AddEmptyLine(1);
	}
	int nGridYCount = 10;
	if(pAxisYLabel!=NULL)
	{
		float fYPosGap = (fMaxYPos-fMinYPos)/nGridYCount;
		float fYGap = (fMaxY-fMinY)/nGridYCount;
		if(fYGap == 0)
		{
	//		if(pAxisXLabel!=NULL) continue;
			int pos_y = top-nChartHeight/2;
			char strYLabel[1024];
			sprintf(strYLabel,"%.2lf%",fMinY);
			int pos_x = left- HPDF_Page_TextWidth (page, strYLabel);
			AddText(strYLabel,SSW_SONG,1, SSW_NONE,0,false,0,0,0,pos_x,pos_y,0,true);
			x0 = left;  y0 = pos_y;
			x1 = right; y1 = pos_y;
			HPDF_Page_SetGrayStroke (page, 0.3);
			HPDF_Page_SetLineWidth(page,0.05);
			HPDF_Page_MoveTo (page, x0,y0);
			HPDF_Page_LineTo (page, x1,y1);
			HPDF_Page_Stroke(page);
			HPDF_Page_SetGrayStroke (page, 1.0);

		}
		else
		{
			for (int i = 0; i<nGridYCount+1; i++)
			{
				int pos_y = bottom+i*fYPosGap-m_nLineFontHeight/2;
				if(pAxisXLabel!=NULL&&i==0) pos_y+=m_nLineFontHeight/4;
				char strYLabel[1024];
				sprintf(strYLabel,"%.2lf%",fMinY+i*fYGap);
				int pos_x = left- HPDF_Page_TextWidth (page, strYLabel);
				AddText(strYLabel,SSW_SONG,1, SSW_NONE,0,false,0,0,0,pos_x,pos_y,0,true);
				if(i>0&&i<nGridYCount)
				{	
					x0 = left; y0 = pos_y;
					x1 = right; y1 = pos_y;
					HPDF_Page_SetGrayStroke (page, 0.3);
					HPDF_Page_SetLineWidth(page,0.03);
					HPDF_Page_MoveTo (page, x0,y0);
					HPDF_Page_LineTo (page, x1,y1);
					HPDF_Page_Stroke(page);
					HPDF_Page_SetGrayStroke (page, 1.0);
				}

			}
		}
		AddText(pAxisYLabel,SSW_SONG,1, SSW_NONE,0,false,0,0,0,left-HPDF_Page_TextWidth (page, pAxisYLabel),top+m_nLineGap,0,true);
	}
	m_nWriteTop = bottom-3*m_nLineGap;
}

bool CsswPdf::SavePdf(const char* strSavePath)
{
	int nn = HPDF_SaveToFile((HPDF_Doc)m_pdf,strSavePath);
	if (HPDF_OK!=nn) 
	{
	//	HPDF_Free((HPDF_Doc)m_pdf);
		return false;
	}
//	HPDF_Free((HPDF_Doc)m_pdf);
	return true;
}
