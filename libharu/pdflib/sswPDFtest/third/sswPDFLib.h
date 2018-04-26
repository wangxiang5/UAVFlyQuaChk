#ifndef ssw_pdf_lib_h
#define ssw_pdf_lib_h
#pragma once
#include <afx.h>
#include <vector>
//#include <string>
using namespace std;

#ifdef  SSWPDF_EXPORTS
#define SSWPDF_API __declspec(dllexport)
#else
#define SSWPDF_API __declspec(dllimport)
#endif
// 
#ifndef SSWPDF_EXPORTS
#pragma comment(lib, "sswPDFLib.lib")
#pragma message("Automatically Linking With sswPDFLib.dll")
#endif 



enum SSWPDF_API SSW_PDF_PageSize
{
	SSW_SIZE_LETTER = 0,
	SSW_SIZE_A3 = 1,
	SSW_SIZE_A4 = 2,
	SSW_SIZE_A5 = 3
};
enum SSWPDF_API SSW_PDF_PageDirection
{
	SSW_PORTAIT = 0,
	SSW_LANDSCAPE = 1
};
enum SSWPDF_API SSW_PDF_ContentStruct
{
	SSW_CENTER = 0,
	SSW_LEFT = 1,
	SSW_RIGHT = 2,
	SSW_NONE = 3
};
enum SSWPDF_API SSW_PDF_ChartType
{
	SSW_POINT = 0,
	SSW_LINE = 1
};
enum SSWPDF_API SSW_PDF_FontType
{
	SSW_SONG = 0,
	SSW_HEI = 1
};
class SSWPDF_API CsswPdf
{
public:
	CsswPdf();
	~CsswPdf();

public:
	bool InitPdf();
	bool AddPage(SSW_PDF_PageSize ePageSize = SSW_SIZE_LETTER, SSW_PDF_PageDirection ePageDirection = SSW_PORTAIT);   //添加页面
	bool SetPdfTitle(const char* strTitle, float r = 0, float g = 0, float b = 0, float fFontScale = 3); //设置标题
	bool SetFont(SSW_PDF_FontType eType = SSW_SONG, int nFontHeight = 8);  //设置字体
	void SetTitleGap(int nGap=40);
	void SetTailGap(int nGap=40);
	void SetLineGap(int nGap=16);
	void SetLineSpace(int nSpace=2);
	int GetTextWidthInPage(const char *strText);
	int GetWriteTopInPage();
	bool AddText(const char* strText, SSW_PDF_FontType eFontType = SSW_SONG, float fFontScale = 1.5,SSW_PDF_ContentStruct eContentStruct = SSW_LEFT, 
		int nPageCount2NewPage = 0, bool bCutLine = false,float r = 0.0,float g =0.0,float b = 0.0,int nPosX = 0,int nPosY = 0, int nSpace = 0,  bool bAddPage = true);
	bool AddEmptyLine(float fScaleLineGap = 1/*,bool bAddPage = true*/);
	bool AddImage(const char* strImageFilePath,float fScale = 1.0, SSW_PDF_ContentStruct eContentStruct=SSW_CENTER,int nPosX=0,int nPosY=0,int nWidth=0, int nHeight=0,bool bAddLabel = false);
// 	bool AddList(int nCols, int nRows, int *pRowHei, int *pColWidth, char** pListInfo, float *pr = NULL,
// 		float *pg = NULL, float *pb = NULL, SSW_PDF_ContentStruct eContent = SSW_CENTER);
	bool AddList(int nCols, int nRows,char** pListInfo);
	bool AddList(int nCols, int nRows,vector<CString> pListInfo, bool bTitle = true);
	bool AddChart(float *pY, float *pX, int nPointCount, float r, float g, float b,int nPosX, int nPosY, float fChartExtendSize = 1.0, 
		int nWidth = 0, int nHeight = 0,const char *pTitle = NULL, const char* pAxisXLabel = NULL, const char* pAxisYLabel = NULL,
		SSW_PDF_ChartType eType = SSW_LINE,SSW_PDF_ContentStruct eContent = SSW_CENTER);
	bool SavePdf(const char* strSavePath);
	////////////////////////////////
	int GetFontHeight(){return m_nPageFontHeight;}
	int GetPdfPageWidth(){return m_nPageWidth;}
	int GetPdfPageHeight(){return m_nPageHeigh;}
	int GetPdfValidWidth(){return m_nPageWidth-2*m_nLineSpace;}
	int GetPdfValidHeight(){return m_nPageHeigh-m_nTitleGap-m_nTailGap;}
	int GetTitleGap(){return m_nTitleGap;}
	int GetLineSpace(){return m_nLineSpace;}
	int GetLineGap(){return m_nLineGap;}
protected:
	void* m_pdf;   //文档句柄
	vector<void*> m_vecPages;//页面句柄
	int m_nPages;
	void* m_font;  //字体
	int m_nLineFontHeight;
	int m_nPageFontHeight;
	int m_nLineWeight;
	int m_nTitleGap;
	int m_nTailGap;
	int m_nLineGap;
	int m_nLineSpace;
	int m_nPageWidth;
	int m_nPageHeigh;
	int m_nWriteTop;
};

#endif