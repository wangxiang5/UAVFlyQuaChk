// sswPDFtest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <io.h>
#include <string>
using namespace std;
int _tmain1(int argc, _TCHAR* argv[])
{
	CsswPdf pdf;
	pdf.InitPdf();
	pdf.AddPage();
	pdf.SetPdfTitle("Pdf Test",0,0.5,0,4);
	pdf.AddText("表1. XXXXXX",SSW_HEI);
	pdf.AddText("表1. XXXXXX",SSW_SONG);
	int nPageWidth = pdf.GetPdfPageWidth();
	int nPageHeight = pdf.GetPdfPageHeight();
	int nFontHeight = pdf.GetFontHeight();
	int nCols = 2;
	int nRows = 5;
	int pWidth[2]={nPageWidth*1/5,nPageWidth*2/5};
	int pHeight[5]={nFontHeight*2,nFontHeight*2,nFontHeight*2,nFontHeight*2,nFontHeight*2};
	char **pInfo = new char *[nRows*nCols];
	float *pr = new float[nCols*nRows]; memset(pr,0,nCols*nRows*sizeof(float));
	float *pg = new float[nCols*nRows]; memset(pg,0,nCols*nRows*sizeof(float));
	float *pb = new float[nCols*nRows]; memset(pb,0,nCols*nRows*sizeof(float));
	pr[3] = 1;

	for (int i = 0; i<nRows; i++)
	{
		for (int j = 0; j<nCols; j++)
		{
			pInfo[i*nCols+j] = new char[1024];
			if(j==0) sprintf(pInfo[i*nCols+j],"第%d行",i);
			else sprintf(pInfo[i*nCols+j],"有效信息%d",i);
		}
	}
// 	pdf.AddList(nCols,nRows,pHeight,pWidth,pInfo);
// 	pdf.AddList(nCols,nRows,pHeight,pWidth,pInfo);
// 	pdf.AddList(nCols,nRows,pHeight,pWidth,pInfo);
// 	pdf.AddList(nCols,nRows,pHeight,pWidth,pInfo);
// 	pdf.AddList(nCols,nRows,pHeight,pWidth,pInfo);
	pdf.AddImage("C:\\Users\\Administrator\\Desktop\\wx\\test\\ssw_prj\\prj01\\source\\0.jpg");
 	pdf.AddList(nCols,nRows,pHeight,pWidth,pInfo);
	float pY[100], pX[100];
	for (int i = 0; i<100; i++)
	{
		pX[i]=i;
		pY[i]=rand()%10000/100;
	}
	pdf.AddChart(pY,pX,100,0,1,0,0,0,0.1,0,0,"曲线图","影像编号","参数值");



	pdf.SavePdf("MiFirstPdf.pdf");
	return 0;
}

void main()
{
	CsswPdf pdf;
	pdf.InitPdf();
	pdf.AddPage();
	pdf.SetPdfTitle("Pdf Test",0,0.5,0,4);
	int nPageWidth = pdf.GetPdfValidWidth();
	int nPageHeight = pdf.GetPdfPageHeight();
	int nFontHeight = pdf.GetFontHeight();
	int nCols = 4, nRows = 300;
	char**pInfo;
	pInfo = new char *[nRows*nCols];
	for (int i = 0; i<nCols*nRows; i++)
	{
		pInfo[i] = new char[1024];
		sprintf(pInfo[i],"%d",i);
	}
	float*pr = new float[nCols*nRows]; memset(pr,0,nCols*nRows*sizeof(float));
	float*pg = new float[nCols*nRows]; memset(pg,0,nCols*nRows*sizeof(float));
	float*pb = new float[nCols*nRows]; memset(pb,0,nCols*nRows*sizeof(float));
//	pdf.AddList(nCols,nRows,pHeight,pWidth,pInfo,pr,pg,pb);
//	pdf.SetLineSpace(50);
	pdf.AddText("时间：XX-XX-XX",SSW_HEI,1.1,SSW_RIGHT,false,0,0,0,0,0,0,44,0,true);
	pdf.AddText("测区概述",SSW_HEI,2,SSW_LEFT,false,0,0,0,0,0,0,8,0,true);
	pdf.AddList(nCols,nRows,pInfo);
		pdf.AddList(nCols,nRows,pInfo);
	pdf.SavePdf("MiPdf2.pdf");


}