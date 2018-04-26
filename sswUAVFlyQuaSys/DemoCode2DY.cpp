//与多边形相关参考代码
//需要使用gdal，ogr
#include "gdal201/gdal.h"
#include "gdal201/gdal_priv.h"
#include "gdal201/ogrsf_frmts.h"
#include "gdal201/ogr_spatialref.h"

/************************************************************************/
/* 节点构造多边形                                                                     */
//vecPoint->point[]
/************************************************************************/
//pVertex: point[]
//nCount: 点个数
//返回多边形对象
OGRPolygon* FunCreatePolygon(void* pVertex,  int nCount, int nDim = 2)
{
	OGRPolygon* pPolygon = NULL;
	if((nDim!=2&&nDim!=3)||nCount<3) 
	{
		pPolygon=NULL;
		return NULL;
	}
	pPolygon = (OGRPolygon*) OGRGeometryFactory::createGeometry(wkbPolygon);
	OGRLinearRing * pRing = (OGRLinearRing*) OGRGeometryFactory::createGeometry(wkbLinearRing);
	if(pRing==NULL||pPolygon==NULL)
	{
		pPolygon = NULL; 
		return NULL;
	}
	if(nDim==2)
	{
		Point2D *pV = (Point2D*) pVertex;
		for (int i = 0; i<nCount; i++)
		{
			pRing->addPoint(pV[i].x,pV[i].y);
		}
	}
	else if(nDim==3)
	{
		Point3D *pV = (Point3D*) pVertex;
		for (int i = 0; i<nCount; i++)
		{
			pRing->addPoint(pV[i].X,pV[i].Y);
		}
	}
	pRing->closeRings();
	pPolygon->addRing(pRing);
	if(pRing) OGRGeometryFactory::destroyGeometry(pRing);
	return pPolygon;
}

/************************************************************************/
/* 多边形与pos点求交                                                                     */
/************************************************************************/

OGRPolygon * pPolygon = FunCreatePolygon(void* pVertex,  int nCount, int nDim = 2) ; //生成多边形
for(int i = 0; i<vecPos.size(); i++) //遍历POS点
{
	OGRPoint ogrPt;  //ogrPos点
	ogrPt.setX(vecPos[i]..Xs);
	ogrPt.setY(vecPos[i]..Ys);

	OGRGeometry *pInet=NULL;   // 求交结果

		pInet=pPolygon->Intersection(&ogrPt);  //多边形与点求交
		if(pInet==NULL) 
			{
				//为框选点设置对应属性
		}
		CString strName = pInet->getGeometryName();
		if(strName=="POINT")   
		{
			//框选点设置对应属性
}

/************************************************************************/
/* VBO函数                                                                     */
/************************************************************************/
		//根据
		CreateVectorVBO(int _nVertexNum, sswVectorType _eType = vLine, bool _bSupportVBO = true);   //创建矢量绘制VBO

		FillVectorVBO(GLdouble *_pV, GLdouble* _pColor, int _nVertexNum);//填充VBO数据
		Display(float nSize = 1,int nGap = 0);
		DeleteVBO();
