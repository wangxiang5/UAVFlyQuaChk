#pragma once

#include "opengl/glew.h"
#include "opengl/glut.h"
#include "MyFunctions.h"
enum sswVectorType
{//矢量类型
	vLine=0,  //线
	vPoint=1, //点
	vPane = 2 //面
};

//opengl相关配置
//bool IsGlExtensionSupported(const char* extension); //Opengl VBO检测

class CsswViewerRender;
bool isGLExtensionSupported(const char *extension);  //VBO
void SetLogicalPalette(HPALETTE &hPalette);
bool SetupPixelFormat(CClientDC *pDC, HPALETTE &hPalett);
bool InitOpenGL(HGLRC &hRC, CClientDC*pDC, HPALETTE &hPalett);
void PrepareDraw(float r, float g, float b, float d);
void EndDraw(CClientDC *m_pDC);
void SetBackColor(CsswViewerRender& BkRender, CRectD rect, float *pR, float *pG, float *pB, float *pA, bool bGradual  = true);
void DisplayString(const char* str);
class CsswViewerRender
{
public:
	CsswViewerRender();
	~CsswViewerRender();
public:
	//创建节点VBO _nVertexNum节点个数,_eTypeVBO节点图形类型 _bSupportVBO是否支持VBO
	void CreateVectorVBO(int _nVertexNum, sswVectorType _eType = vLine, bool _bSupportVBO = true);   //创建矢量绘制VBO
	void FillVectorVBO(GLdouble *_pV, GLdouble* _pColor, int _nVertexNum);//填充VBO数据
	void Display(float nSize = 1,int nGap = 0);
	void DeleteVBO();
	bool IsVBOFilled();
private:
	GLuint m_nVboId;
	GLuint m_nIdxVboId;
	int m_nVertexNum;
	bool m_bSupportVBO;
	bool m_bIsVBOFilled;
	sswVectorType m_eVertexType;
};

enum enmRemderType
{
	Render_Rect = 0,
	Render_Point3d = 1,
	Render_Point2d = 2
};
class CsswImgRender:public CsswUAVImage
{
public:
	CsswImgRender();
	~CsswImgRender();
	bool CreateTexture(CString strImgPath);
	bool FillTexture();
	void SetDisplayPos(CRect rect);
	void DisplayTexture(void *pDisplayArea, enmRemderType eType = Render_Rect, int nPtNum = 4); 
	void DeleteTexture();
	float GetScaleYX(){
		return 1.0*m_nRowTexture/m_nColTexture;
	}
	int GetTextureSizeX(){return m_nColTexture;};
	int GetTextureSizeY(){return m_nRowTexture;}
	bool IsFilled()
	{
		return m_bFilled;
	}

protected:
	bool LoadImgDat(BYTE*pData);
	CString m_strImgPath;
	int m_nColTexture;
	int m_nRowTexture;
	GLuint m_nIdTexture;
	bool m_bTextured;
	bool m_bFilled;
	CRect m_RectDisplay;
};

