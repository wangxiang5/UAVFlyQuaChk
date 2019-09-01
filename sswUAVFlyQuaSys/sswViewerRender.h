#pragma once
#include "opengl/glew.h"
#include "opengl/glut.h"

#include "MyFunctions.h"
enum sswVectorType
{//ʸ������
	vLine=0,  //��
	vPoint=1, //��
	vPane = 2 //��
};

//opengl�������
//bool IsGlExtensionSupported(const char* extension); //Opengl VBO���

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
	//�����ڵ�VBO _nVertexNum�ڵ����,_eTypeVBO�ڵ�ͼ������ _bSupportVBO�Ƿ�֧��VBO
	void CreateVectorVBO(int _nVertexNum, sswVectorType _eType = vLine, bool _bSupportVBO = true);   //����ʸ������VBO
	void FillVectorVBO(GLdouble *_pV, GLdouble* _pColor, int _nVertexNum);//���VBO����
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

