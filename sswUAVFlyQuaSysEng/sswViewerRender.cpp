#include "stdafx.h"
#include "sswViewerRender.h"
//opengl
PFNGLGENBUFFERSARBPROC glGenBuffersARB = NULL;     // VBO Name Generation Procedure
PFNGLBINDBUFFERARBPROC glBindBufferARB = NULL;     // VBO Bind Procedure
PFNGLBUFFERDATAARBPROC glBufferDataARB = NULL;     // VBO Data Loading Procedure
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = NULL;   // VBO Deletion Procedure
bool isGLExtensionSupported(const char *extension)  //VBO
{
	if (extension == NULL) return true;
	if (strlen(extension) < 3) return true;
	const GLubyte *extensions = NULL;
	const GLubyte *start;
	GLubyte *where, *terminator;

	/* Extension names should not have spaces. */
	where = (GLubyte *)strchr(extension, ' ');
	if (where || *extension == '/0')
		return false;

	/* It takes a bit of care to be fool-proof about parsing the
	OpenGL extensions string. Don't be fooled by sub-strings,
	etc. */
	extensions = glGetString(GL_EXTENSIONS);
	start = extensions;
	for (;;)
	{
		where = (GLubyte*)strstr((const char *)start, extension);
		if (!where) break;

		terminator = where + strlen(extension);
		if ((where == start || *(where - 1) == ' ') && (*terminator == ' ' || *terminator == '/0'))
			return true;
		start = terminator;
	}

	return false;
}  //vbo支持性检查
void SetLogicalPalette(HPALETTE &hPalette) //设置逻辑调色板
{
	struct
	{
		WORD Version;
		WORD NumberOfEntries;
		PALETTEENTRY aEntries[256];
	} logicalPalette = { 0x300, 256 };

	BYTE reds[] = { 0, 36, 72, 109, 145, 182, 218, 255 };
	BYTE greens[] = { 0, 36, 72, 109, 145, 182, 218, 255 };
	BYTE blues[] = { 0, 85, 170, 255 };

	for (int colorNum = 0; colorNum < 256; ++colorNum)
	{
		logicalPalette.aEntries[colorNum].peRed =
			reds[colorNum & 0x07];
		logicalPalette.aEntries[colorNum].peGreen =
			greens[(colorNum >> 0x03) & 0x07];
		logicalPalette.aEntries[colorNum].peBlue =
			blues[(colorNum >> 0x06) & 0x03];
		logicalPalette.aEntries[colorNum].peFlags = 0;
	}
	hPalette = CreatePalette((LOGPALETTE*)&logicalPalette);
}
bool SetupPixelFormat(CClientDC *pDC, HPALETTE &hPalett) //设置像素格式
{
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),    // pfd结构的大小 
		1,                                // 版本号 
		PFD_DRAW_TO_WINDOW |              // 支持在窗口中绘图 
		PFD_SUPPORT_OPENGL |              // 支持 OpenGL 
		PFD_SUPPORT_GDI |
		PFD_DOUBLEBUFFER,                 // 双缓存模式 
		PFD_TYPE_RGBA,                    // RGBA 颜色模式 
		24,                               // 24 位颜色深度 
		0, 0, 0, 0, 0, 0,                 // 忽略颜色位 
		0,                                // 没有非透明度缓存 
		0,                                // 忽略移位位 
		0,                                // 无累加缓存 
		0, 0, 0, 0,                       // 忽略累加位 
		32,                               // 32 位深度缓存     
		0,                                // 无模板缓存 
		0,                                // 无辅助缓存 
		PFD_MAIN_PLANE,                   // 主层 
		0,                                // 保留 
		0, 0, 0                           // 忽略层,可见性和损毁掩模 
	};
	int pixelformat;
	pixelformat = ::ChoosePixelFormat(pDC->GetSafeHdc(), &pfd);//选择像素格式
	::SetPixelFormat(pDC->GetSafeHdc(), pixelformat, &pfd);	//设置像素格式
	if (pfd.dwFlags & PFD_NEED_PALETTE)
		SetLogicalPalette(hPalett);	//设置逻辑调色板
	return true;
}
bool InitOpenGL(HGLRC &hRC, CClientDC*pDC, HPALETTE &hPalett)//初始化OpenGL
{
	//	SetupPixelFormat(pDC, hPalett);
	//生成绘制描述表
	if (hRC == NULL)hRC = wglCreateContext(pDC->GetSafeHdc());
	// 	wglMakeCurrent(NULL, NULL);
	// 	 	//置当前绘制描述表
	// 	wglMakeCurrent(m_pDC->GetSafeHdc(), hRC);
	return true;
}
void PrepareDraw(float r, float g, float b, float d)
{
	//wglMakeCurrent(m_pDC->GetSafeHdc(), g_hRC);
	glClearColor(r, g, b, d);		 // 设置刷新背景色
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// 刷新背景
}
void EndDraw(CClientDC *m_pDC)
{
	glFlush();
	SwapBuffers(m_pDC->GetSafeHdc());
}
void SetBackColor(CsswViewerRender& BkRender, CRectD rect, float *pR, float *pG, float *pB, float *pA, bool bGradual )
{
	double *pV = new double[4*2];
	double *pC = new double[4*4];
	vector<Point2D> vPt2d4;
	vPt2d4.push_back(rect.LeftTop());
	vPt2d4.push_back(rect.LeftBottom());
	vPt2d4.push_back(rect.RightBottom());
	vPt2d4.push_back(rect.RightTop());
	FunVectorPt2VertexXY(&vPt2d4,2,pV);
	if(bGradual)
	{
		for (int i = 0; i<4; i++)
		{
			*(pC+i*4+0)=*(pR+i);
			*(pC+i*4+1)=*(pG+i);
			*(pC+i*4+2)=*(pB+i);
			*(pC+i*4+3) =*(pA+i);
		}
	}else
	{
		for (int i = 0; i<4; i++)
		{
			*(pC+i*4+0)=*pR;
			*(pC+i*4+1)=*pG;
			*(pC+i*4+2)=*pB;
			*(pC+i*4+3) =*pA;
		}
	}
	BkRender.FillVectorVBO(pV,pC,4);
	vector<Point2D>().swap(vPt2d4);
	delete[]pC; pC=NULL;
	delete[]pV; pV=NULL;
	BkRender.Display();

// 	glBegin(GL_POLYGON);
// 	glColor3f(*pR,    *pG,   *pB);     	glVertex2f(rect.left, rect.top);  
// 	glColor3f(*(pR+1),*(pG+1),*(pB+1));	glVertex2f(rect.left, rect.bottom);
// 	glColor3f(*(pR+2),*(pG+2),*(pB+2));	glVertex2f(rect.right, rect.bottom);
// 	glColor3f(*(pR+3),*(pG+3),*(pB+3));	glVertex2f(rect.right, rect.top);
// 	glEnd();  
	
}


#define MAX_CHAR 128

void selectFont(int size, int charset, const char* face) {
	HFONT hFont = CreateFontA(size, 0, 0, 0, FW_MEDIUM, 0, 0, 0,
		charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, face);
	HFONT hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);
	DeleteObject(hOldFont);
}
void DisplayString(const char* str)
{
	selectFont(16, ANSI_CHARSET, "Comic Sans MS");
	static int isFirstCall = 1;
	static GLuint lists;
	if (isFirstCall)
	{
		isFirstCall = 0;
		lists = glGenLists(MAX_CHAR);
		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);
	}
	for (int i = 0; i < strlen(str); i++)
	{
		glCallList(lists + *(str + i));
	}
	
// 		int len, i;
// 		wchar_t* wstring;
// 		HDC hDC = wglGetCurrentDC();
// 		GLuint list = glGenLists(1);
// 
// 		// 计算字符的个数
// 		// 如果是双字节字符的（比如中文字符），两个字节才算一个字符
// 		// 否则一个字节算一个字符
// 		len = 0;
// 		for(i=0; str[i]!='\0'; ++i)
// 		{
// 			if( IsDBCSLeadByte(str[i]) )
// 				++i;
// 			++len;
// 		}
// 
// 		// 将混合字符转化为宽字符
// 		wstring = (wchar_t*)malloc((len+1) * sizeof(wchar_t));
// 		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, wstring, len);
// 		wstring[len] = L'\0';
// 
// 		// 逐个输出字符
// 		for(i=0; i<len; ++i)
// 		{
// 			wglUseFontBitmapsW(hDC, wstring[i], 1, list);
// 			glCallList(list);
// 		}
// 
// 		// 回收所有临时资源
// 		free(wstring);
// 		glDeleteLists(list, 1);
}

CsswViewerRender::CsswViewerRender()
{
//	m_nVboId = InvalidValue;
	m_nVertexNum = 0;
	m_eVertexType = vLine;
	m_bSupportVBO = true;
	m_bIsVBOFilled = false;

}

CsswViewerRender::~CsswViewerRender()
{
//	if (m_nVboId!= NULL) glDeleteBuffersARB(1, (GLuint*)m_nVboId);
//	m_nVboId = InvalidValue;
	m_nVertexNum = 0;
	m_eVertexType = vLine;
	m_bSupportVBO = true;
	m_bIsVBOFilled = false;
}

void CsswViewerRender::CreateVectorVBO(int _nVertexNum, sswVectorType _eType /* = vLine */, bool _bSupportVBO /* = true */)
{
	if (_bSupportVBO)  //支持VBO
	{
		glGenBuffersARB(1, &m_nVboId);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_nVboId);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, _nVertexNum * 6 * sizeof(GLdouble), 0, GL_STATIC_DRAW_ARB);
	//释放绑定
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	}
	else
	{
		
	}
	m_eVertexType = _eType;
	m_bSupportVBO = _bSupportVBO;
	m_nVertexNum = _nVertexNum;
}
void CsswViewerRender::FillVectorVBO(GLdouble *_pV, GLdouble* _pColor, int _nVertexNum)
{
	_nVertexNum = min(m_nVertexNum, _nVertexNum);
	if (m_bSupportVBO)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_nVboId);
		glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, _nVertexNum * 2 * sizeof(GLdouble), _pV);
		glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, _nVertexNum * 2 * sizeof(GLdouble), _nVertexNum * 4 * sizeof(GLdouble), _pColor);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		m_bIsVBOFilled = true;
	}
	else
	{
		
	}
}
void CsswViewerRender::Display(float nSize /* = 1 */,int nGap /* = 0 */)
{
	if (m_eVertexType == vLine)  //显示四边形
	{
		if (m_bSupportVBO&&glIsBufferARB(m_nVboId))
		{
			glEnableClientState(GL_VERTEX_ARRAY);      // Enable Vertex Arrays
			glEnableClientState(GL_COLOR_ARRAY);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_nVboId);
			glLineWidth(nSize);
			// Set The Vertex Pointer To The Vertex Buffer
			// 这里必须将后面那个指针设为 NULL
			glVertexPointer(2, GL_DOUBLE,0, (char *)NULL);
			glColorPointer(4, GL_DOUBLE, nGap, (void*)(m_nVertexNum * 2 * sizeof(GLdouble)));
			glDrawArrays(GL_LINE_LOOP, 0, m_nVertexNum); // Draw All Of The Triangles At Once
	//		glDrawElements(GL_LINE_STRIP,m_nVertexNum,GL_UNSIGNED_SHORT,0);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
			glDisableClientState(GL_COLOR_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY); //停止使用vbo
		

		}
		else
		{

		}
	}
	else if (m_eVertexType == vPoint)
	{
		if (m_bSupportVBO&&glIsBufferARB(m_nVboId))
		{
			glEnableClientState(GL_VERTEX_ARRAY);      // Enable Vertex Arrays
			glEnableClientState(GL_COLOR_ARRAY);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_nVboId);
			// 				glSetColor(m_red_point, m_green_point, m_blue_point);  //颜色额外设置
			glPointSize(nSize);
			// Set The Vertex Pointer To The Vertex Buffer

			// 这里必须将后面那个指针设为 NULL
			glVertexPointer(2, GL_DOUBLE, 0, (char *)NULL);
			glColorPointer(4, GL_DOUBLE, 0, (void*)(m_nVertexNum * 2 * sizeof(GLdouble)));
			glDrawArrays(GL_POINTS, 0, m_nVertexNum); // Draw All Of The Triangles At Once
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
			glDisableClientState(GL_COLOR_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY); //停止使用vbo
	//		glColor3f(1.0, 1.0, 1.0);
		}
		else
		{
			glBindVertexArray(m_nVboId); // Bind our Vertex Array Object  
			glVertexAttribPointer((GLuint)0, 2, GL_DOUBLE, GL_FALSE, 0, 0); // Set up our vertex attributes pointer  
			glEnableVertexAttribArray(0); // Disable our Vertex Array Object  
			//	glBindVertexArray(0); // Disable our Vertex Buffer Object  
			glPointSize(2);
			// Set The Vertex Pointer To The Vertex Buffer
			glDrawArrays(GL_POINTS, 0, m_nVertexNum); // Draw our square  
			glBindVertexArray(0); // Unbind our Vertex Array Object
		}
	}
	else if(m_eVertexType==vPane)
	{
		if (m_bSupportVBO&&glIsBufferARB(m_nVboId))
		{
			glEdgeFlag(true);
			glEnableClientState(GL_VERTEX_ARRAY);      // Enable Vertex Arrays
			glEnableClientState(GL_COLOR_ARRAY);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_nVboId);
			glLineWidth(nSize);
			// Set The Vertex Pointer To The Vertex Buffer
			// 这里必须将后面那个指针设为 NULL
			glVertexPointer(2, GL_DOUBLE,0, (char *)NULL);
			glColorPointer(4, GL_DOUBLE, nGap, (void*)(m_nVertexNum * 2 * sizeof(GLdouble)));
			glDrawArrays(GL_POLYGON, 0, m_nVertexNum); // Draw All Of The Triangles At Once
			//		glDrawElements(GL_LINE_STRIP,m_nVertexNum,GL_UNSIGNED_SHORT,0);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
			glDisableClientState(GL_COLOR_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY); //停止使用vbo
		}
	}
}
void CsswViewerRender::DeleteVBO()
{
	if (m_bSupportVBO)
	{
		if (glIsBufferARB(m_nVboId))
			glDeleteBuffersARB(1, &m_nVboId);
	}

	else
	{
		if (glIsVertexArray(m_nVboId))
			glDeleteVertexArrays(1, &m_nVboId);
	}
	m_nVertexNum = 0;
	m_eVertexType = vLine;
	m_bIsVBOFilled = false;
}
bool CsswViewerRender::IsVBOFilled()
{
	return m_bIsVBOFilled;
}


CsswImgRender::CsswImgRender()
{
	m_nColTexture=InvalidValue;
	m_nRowTexture=InvalidValue;
	m_nIdTexture=InvalidValue;
	m_bTextured=false;
	m_bFilled=false;
	glShadeModel(GL_FLAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);
// 	glEnable(GL_ALPHA_TEST);
// 	glAlphaFunc(GL_GREATER, 0);
	/* 说明映射方式*/
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
CsswImgRender::~CsswImgRender()
{

}

bool CsswImgRender::CreateTexture(CString strImgPath)
{
	m_strImgPath = strImgPath;
	if(!OpenImg(m_strImgPath, true)) return false;
	int nCols = GetCols();
	int nRows = GetRows();
	glGenTextures(1,&m_nIdTexture);
	glBindTexture(GL_TEXTURE_2D,m_nIdTexture);
	glEnable(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SIZE_IMG_TEXTURE, SIZE_IMG_TEXTURE, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	//设置纹理过滤
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glDisable(GL_TEXTURE_2D);
	/* 定义纹理 */
	m_bTextured=true;
	if(nCols>nRows) 
	{
		m_nRowTexture=1.0*nRows/nCols*SIZE_IMG_TEXTURE;
		m_nColTexture=SIZE_IMG_TEXTURE;
	}
	else 
	{
		m_nColTexture=1.0*nCols/nRows*SIZE_IMG_TEXTURE;
		m_nRowTexture=SIZE_IMG_TEXTURE;
	}
	CloseImg();
	return true;
}
bool CsswImgRender::LoadImgDat(BYTE*pData)
{
	if(!OpenImg(m_strImgPath,true)) return false;
	int nCols = GetCols();
	int nRows = GetRows();
	int nBands=GetBandCount();
	BYTE* pRead = new BYTE[m_nColTexture*m_nRowTexture*nBands];
	bool b = ReadImg(0,0,nCols,nRows,m_nColTexture,m_nRowTexture,pRead);
	BYTE* pRed = pRead + 0;
	BYTE* pGreen = pRead + 1;
	BYTE* pBlue = pRead +2;
	BYTE* pPixel = pData;
	for (int i = 0; i < m_nColTexture*m_nRowTexture; i++)
	{
		*pPixel++ = *(pRed + nBands* i);
		*pPixel++ = *(pGreen + nBands * i);
		*pPixel++ = *(pBlue + nBands * i);
		if (*(pRed + nBands* i) == 0 &&
			*(pGreen + nBands* i) == 0 &&
			*(pBlue + nBands * i) == 0)
		{
			*pPixel++ =0;
		}
		else
		{
			*pPixel++ = 1;
		}

	}
	delete[]pRead; pRead=NULL;
	CloseImg();
}
bool CsswImgRender::FillTexture()
{
	if(!m_bTextured) return false;
 	BYTE* pData = new BYTE[4*m_nColTexture*m_nRowTexture]; 
	memset(pData,0,4*m_nColTexture*m_nRowTexture);
	LoadImgDat(pData);
	/* 启动纹理映射 */
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_nIdTexture);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0,m_nColTexture,m_nRowTexture,GL_RGBA, GL_UNSIGNED_BYTE, pData);
	delete[]pData; pData = NULL;
	glDisable(GL_TEXTURE_2D);
	m_bFilled=true;
	return true;
}
void CsswImgRender::SetDisplayPos(CRect rect)
{
	m_RectDisplay = rect;
}
void CsswImgRender::DisplayTexture(void *pDisplayArea, enmRemderType eType /* = Render_Rect */, int nPtNum /* = 4 */)
{
	if(eType==Render_Rect) m_RectDisplay = *(CRect*)pDisplayArea;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_nIdTexture);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(m_RectDisplay.left, m_RectDisplay.top, 0.0f);//左上角
	glTexCoord2f(1.0f, 0.0f); glVertex3f(m_RectDisplay.right, m_RectDisplay.top, 0.0f);// 左下角	
	glTexCoord2f(1.0f, GetScaleYX()); glVertex3f(m_RectDisplay.right, m_RectDisplay.bottom, 0.0f);// 右下角
	glTexCoord2f(0.0f, GetScaleYX()); glVertex3f(m_RectDisplay.left, m_RectDisplay.bottom, 0.0f); // 右上角
	glEnd();
	glDisable(GL_TEXTURE_2D);
}
void CsswImgRender::DeleteTexture()
{
	if(glIsTexture(m_nIdTexture))
	{
		glDeleteTextures(1,&m_nIdTexture);
	}
}

