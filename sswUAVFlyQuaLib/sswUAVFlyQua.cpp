// sswUAVFlyQuaLib.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "sswUAVFlyQua.h"
#include "gdal201/gdal.h"
#include "gdal201/gdal_priv.h"
#include "gdal201/ogrsf_frmts.h"
#include "gdal201/ogr_spatialref.h"
#include "pugixml.hpp"
#include "CoorConv.hpp"
#include <omp.h>
#include "sswUAVCmrDsitortionCorrect.h"
#include "sswUAVBundleAdjustment.h"
#include "Matrix.h"
#include "sswPdfLib.h"
#include "conio.h"
//读取缩略图相机参数文件
bool FunReadThumbCmrFile(CString strCmrFilePath, SSWstuCmrInfo& sswCmrInfo)
{
	FILE* pfR = fopen(strCmrFilePath,"r");
	if(pfR == NULL)
	{
		return false;
	}
	char line[MAX_SIZE_FILE_LINE];
	if(!fgets(line,MAX_SIZE_FILE_LINE,pfR))
	{
		fclose(pfR); pfR=NULL;
		return false;
	}
	line[strlen(line) - 1]= '\0';
	vector<CString> vParts = FunStrTok(line,"\t ");
	if(vParts.size()!=1)
	{
		fclose(pfR); pfR=NULL;
		return false;
	}

	sswCmrInfo.eUnitx0y0 = PIX;
	sswCmrInfo.eUnitf = PIX;
	sswCmrInfo.eUnitpixsize=MM;
	vector<CString>().swap(vParts);
	while(fgets(line,MAX_SIZE_FILE_LINE, pfR))
	{
		line[strlen(line) - 1]= '\0';
		vParts = FunStrTok(line, "\t ");
		sswCmrInfo.strLabel=vParts[vParts.size()-1];
		sswCmrInfo.x0=atof(vParts[0]);
		sswCmrInfo.y0=atof(vParts[1]);
		sswCmrInfo.f=atof(vParts[2]);
		sswCmrInfo.fx=atof(vParts[3]);
		sswCmrInfo.fy=atof(vParts[4]);
		sswCmrInfo.pixsize=atof(vParts[5]);
		sswCmrInfo.k1=atof(vParts[6]);
		sswCmrInfo.k2=atof(vParts[7]);
		sswCmrInfo.p1=atof(vParts[8]);
		sswCmrInfo.p2=atof(vParts[9]);
		sswCmrInfo.a=atof(vParts[10]);
		sswCmrInfo.b=atof(vParts[11]);
		vector<CString>().swap(vParts);
	}

	fclose(pfR); pfR=NULL;
	return true;
}
//计算直线-平面方程参数
void FunCalLineParas(Point2D pt1, Point2D pt2, double &a, double &b, double &c)
{
	if (pt1.x == pt2.x)
	{
		b = 0;
		a = 1;
		c = -pt1.x;
	}
	else
	{
		b = 1;
		a = -(pt1.y -pt2.y) / (pt1.x - pt2.x);
		c = -a*pt1.x - pt1.y;
	}
}
void FunCalLineParasByLsm(vector<Point2D> vecPt, double &a, double &b, double &c)
{
	int num = vecPt.size();
	int nCount = 0;
	for (int i = 0; i<num; i++)
	{
		if(vecPt[i].x==vecPt[0].x&&vecPt[i].y!=vecPt[0].y) 
			nCount++;
	}
	if(nCount==num)  //直线方程为x=a;
	{
		a = 1;
		b = 0;
		c = vecPt[0].x;
		return;
	}
	//直线方程为y=ax+b
	double *A = new double [num * 2]; memset(A, 0, num * 2 * sizeof(double));
	double *X = new double[2 * 1]; memset(X, 0, 2 * 1 * sizeof(double));
	double *L = new double[num * 1]; memset(L, 0, num * 1 * sizeof(double));
	double *AT = new double[2 * num]; memset(AT, 0, num * 2 * sizeof(double));
	double *ATA = new double[2 * 2]; memset(ATA, 0, 2 * 2 * sizeof(double));
	double *ATL = new double[2 * 1]; memset(ATL, 0, 2 * 1 * sizeof(double));
	for (int i = 0; i < num; i++) {
		A[i * 2 + 0] = vecPt[i].x;
		A[i * 2 + 1] = 1;
		L[i * 1 + 0] = vecPt[i].y;
	}
	CMatrix MatrixApp;
	MatrixApp.MatrixTranspose(A, AT, num, 2);
	MatrixApp.MatrixMulti(AT, A, ATA, 2, num, 2);
	MatrixApp.MatrixMulti(AT, L, ATL, 2, num, 1);

	MatrixApp.MatrixInversion_General(ATA, 2);
	MatrixApp.MatrixMulti(ATA, ATL, X, 2, 2, 1);
	a = X[0];
	b = -1;
	c = X[1];
	delete[]A;
	delete[]X;
	delete[]L;
	delete[]AT ;
	delete[]ATA;
	delete[]ATL;
}
void FunCalPaneParasBylsm(vector<Point3D> vecPt, double &a, double &b, double &c, double &d)
{
	int num = vecPt.size();
	//平面方程Z=aX+bY+c
	double *A = new double [num * 3]; memset(A, 0, num * 3 * sizeof(double));
	double *X = new double[3 * 1]; memset(X, 0, 3 * 1 * sizeof(double));
	double *L = new double[num * 1]; memset(L, 0, num * 1 * sizeof(double));
	double *AT = new double[3 * num]; memset(AT, 0, num * 3 * sizeof(double));
	double *ATA = new double[3 * 3]; memset(ATA, 0, 3 * 3 * sizeof(double));
	double *ATL = new double[3 * 1]; memset(ATL, 0, 3 * 1 * sizeof(double));
	for (int i = 0; i < num; i++) {
		A[i * 3 + 0] = vecPt[i].X;
		A[i * 3 + 1] = vecPt[i].Y;
		A[i * 3 + 2] = 1;
		L[i * 1 + 0] = vecPt[i].Z;
	}
	CMatrix MatrixApp;
	MatrixApp.MatrixTranspose(A, AT, num, 3);
	MatrixApp.MatrixMulti(AT, A, ATA, 3, num, 3);
	MatrixApp.MatrixMulti(AT, L, ATL, 3, num, 1);

	MatrixApp.MatrixInversion_General(ATA, 3);
	MatrixApp.MatrixMulti(ATA, ATL, X, 3, 3, 1);
	a = X[0];
	b = X[1];
	c = -1;
	d = X[2];
	delete[]A;
	delete[]X;
	delete[]L;
	delete[]AT ;
	delete[]ATA;
	delete[]ATL;
}
CString FunCreateStripLabel(int nStripIdx, int nIdxInStrip)
{
	CString strLabel;strLabel.Format("%s_%s","000","000");
	if(nStripIdx==InvalidValue||nIdxInStrip==InvalidValue) return" ";
	CString strStripIdx; strStripIdx.Format("%d",nStripIdx+1);
	CString strIdxInStrip; strIdxInStrip.Format("%d",nIdxInStrip+1);
//#pragma omg parallel for
	for (int i = 0; i<strStripIdx.GetLength(); i++)
	{
		strLabel.SetAt((strLabel.GetLength()-1)/2-strStripIdx.GetLength()+i,strStripIdx.GetAt(i));
	}
//#pragma omg parallel for
	for (int i = 0; i<strIdxInStrip.GetLength(); i++)
	{
		strLabel.SetAt(strLabel.GetLength()-strIdxInStrip.GetLength()+i,strIdxInStrip.GetAt(i));
	}
	return strLabel;
}
double  FunCalDataRms(vector<double> vData, double dMean)
{
	double dSum2 = 0;
	for (int i = 0; i<vData.size(); i++)
	{
		dSum2+=pow(vData[i]-dMean,2);
	}
	if(vData.size()>0) return sqrt(dSum2/vData.size());
	else return 0;
}
//借助同名点回复像对拓扑关系
void FunImgPtTopoTrans(Point2D pt1, double* CoePara, Point2D& pt2)
{
	pt2.x = CoePara[0]*pt1.x+CoePara[1]*pt1.y+CoePara[2];
	pt2.y = CoePara[3]*pt1.x+CoePara[4]*pt1.y+CoePara[5];
}
void FunCalTopoParasByLsm(vector<Point2D> vecPt1, vector<Point2D> vecPt2, double **CoePara)
{
	int num = vecPt1.size();
	double *A = new double [num * 2 * 6]; memset(A, 0, num * 2 * 6 * sizeof(double));
	double *X = new double[6 * 1]; memset(X, 0, 6 * 1 * sizeof(double));
	double *L = new double[num * 2]; memset(L, 0, num * 2 * sizeof(double));
	double *AT = new double[6 * 2 * num]; memset(AT, 0, num * 6 * 2 * sizeof(double));
	double *ATA = new double[6 * 6]; memset(ATA, 0, 6 * 6 * sizeof(double));
	double *ATL = new double[6 * 1]; memset(ATL, 0, 6 * 1 * sizeof(double));
	for (int i = 0; i < num; i++) {
		A[i * 12 + 0] = vecPt1[i].x;
		A[i * 12 + 1] = vecPt1[i].y;
		A[i * 12 + 2] = 1;
		A[i * 12 + 9]  = vecPt1[i].x;
		A[i * 12 + 10] = vecPt1[i].y;
		A[i * 12 + 11] = 1;
		L[i * 2 + 0] = vecPt2[i].x;
		L[i * 2 + 1] = vecPt2[i].y;
	}
	CMatrix MatrixApp;
	MatrixApp.MatrixTranspose(A, AT, num*2, 6);
	MatrixApp.MatrixMulti(AT, A, ATA, 6, num*2, 6);
	MatrixApp.MatrixMulti(AT, L, ATL, 6, num*2, 1);
	MatrixApp.MatrixInversion_General(ATA, 6);
	MatrixApp.MatrixMulti(ATA, ATL, X, 6, 6, 1);
	*CoePara = X;
	delete[]A;
	delete[]L;
	delete[]AT;
	delete[]ATA;
	delete[]ATL;
	Point2D pt, dpt;
}

void FunArrangeAdjPts(vector<CsswUAVImage>& vecImgInfo,vector<SSWstuAdjPtInfo> &vecAdjPts, vector<int>&vecPtNumInImgs)
{
	for (int i = 0; i<vecAdjPts.size(); i++)
	{
		for (int j = 0; j<vecAdjPts[i].nOverlayCount; j++)
		{
			int nImgIdx = vecAdjPts[i].vecImgIdx[j];
			vecImgInfo[nImgIdx].m_ImgAdjPts.vecAdjPtIdx.push_back(i);
			vecImgInfo[nImgIdx].m_ImgAdjPts.vecImgPtIdxInAdjPt.push_back(j);		
			for (int jj = 0; jj<vecAdjPts[i].nOverlayCount; jj++)
			{
				bool bExist = false;
				if(jj==j) continue;
				for(int k = 0; k<vecImgInfo[nImgIdx].m_ImgAdjPts.vecLinkImgIdx.size(); k++)
				{
					if(vecImgInfo[nImgIdx].m_ImgAdjPts.vecLinkImgIdx[k]==vecAdjPts[i].vecImgIdx[jj])
					{
						bExist=true;
						vecImgInfo[nImgIdx].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg[k].push_back(i);
						vecImgInfo[nImgIdx].m_ImgAdjPts.vvImgPtIdxInAdjPt[k].push_back(j);
						vecImgInfo[nImgIdx].m_ImgAdjPts.vvLinkImgPtIdxInAdjPt[k].push_back(jj);
						break;
					}
				}
				if(!bExist)
				{
					vecImgInfo[nImgIdx].m_ImgAdjPts.vecLinkImgIdx.push_back(vecAdjPts[i].vecImgIdx[jj]);
					vector<int> vAdjIdx4LinkImg; vAdjIdx4LinkImg.push_back(i);
					vector<int> vImgPtIdxInAdjPt; vImgPtIdxInAdjPt.push_back(j);
					vector<int> vLinkImgPtIdxInAdjPt; vLinkImgPtIdxInAdjPt.push_back(jj);
					vecImgInfo[nImgIdx].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg.push_back(vAdjIdx4LinkImg);
					vecImgInfo[nImgIdx].m_ImgAdjPts.vvImgPtIdxInAdjPt.push_back(vImgPtIdxInAdjPt);
					vecImgInfo[nImgIdx].m_ImgAdjPts.vvLinkImgPtIdxInAdjPt.push_back(vLinkImgPtIdxInAdjPt);
					vector<int>().swap(vAdjIdx4LinkImg); 
					vector<int>().swap(vImgPtIdxInAdjPt); 
					vector<int>().swap(vLinkImgPtIdxInAdjPt); 
				}
			}

		}
	}
	
	for (int i = 0; i<vecImgInfo.size(); i++)
	{
		if(!vecImgInfo[i].m_bValid) continue;
		for (int j = 0; j<vecImgInfo[i].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg.size()-1; j++)
		{
			if(vecImgInfo[i].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg.size()==0) break;
			int nSum1 = vecImgInfo[i].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg[j].size();
			for (int k = j+1; k<vecImgInfo[i].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg.size(); k++)
			{
				int nSum2 = vecImgInfo[i].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg[k].size();
				if(nSum1<nSum2)
				{
					nSum1 = nSum2;
					swap(vecImgInfo[i].m_ImgAdjPts.vecLinkImgIdx[j],vecImgInfo[i].m_ImgAdjPts.vecLinkImgIdx[k]);
					swap(vecImgInfo[i].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg[j],vecImgInfo[i].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg[k]);
					swap(vecImgInfo[i].m_ImgAdjPts.vvImgPtIdxInAdjPt[j],vecImgInfo[i].m_ImgAdjPts.vvImgPtIdxInAdjPt[k]);
					swap(vecImgInfo[i].m_ImgAdjPts.vvLinkImgPtIdxInAdjPt[j],vecImgInfo[i].m_ImgAdjPts.vvLinkImgPtIdxInAdjPt[k]);
				}
			}
		}
	}
	vector<int>().swap(vecPtNumInImgs);
	vecPtNumInImgs.resize(vecImgInfo.size(),0);
#pragma omp parallel for
	for (int i = 0; i<vecImgInfo.size(); i++)
	{
		if(!vecImgInfo[i].m_bAdjed) continue;
		if(vecImgInfo[i].m_ImgAdjPts.vecAdjPtIdx.size()==0) continue;
		vecPtNumInImgs[i]=vecImgInfo[i].m_ImgAdjPts.vecAdjPtIdx.size();
		int nAdjPtNum = vecImgInfo[i].m_ImgAdjPts.vecAdjPtIdx.size();
		vecImgInfo[i].m_ImgAdjPts.vecImgPtResX.resize(nAdjPtNum);
		vecImgInfo[i].m_ImgAdjPts.vecImgPtResY.resize(nAdjPtNum);
		vecImgInfo[i].OpenImg(vecImgInfo[i].m_strThumbPath);
		int nCols = vecImgInfo[i].GetCols();
		int nRows = vecImgInfo[i].GetRows();
		for (int j = 0; j<nAdjPtNum; j++)
		{
			int nAdjPtIdx = vecImgInfo[i].m_ImgAdjPts.vecAdjPtIdx[j];
			int nImgPtIdx = vecImgInfo[i].m_ImgAdjPts.vecImgPtIdxInAdjPt[j];
			Point2D ptImg0 = vecAdjPts[nAdjPtIdx].vecImgPt[nImgPtIdx];
			Point3D ptAdjXYZ = Point3D(vecAdjPts[nAdjPtIdx].X,vecAdjPts[nAdjPtIdx].Y,vecAdjPts[nAdjPtIdx].Z);
			Point2D ptImg1 = vecImgInfo[i].XYZ2Img(nCols,nRows,vecImgInfo[i].m_ThumbCmrInfo.f,ptAdjXYZ,true);
			Point3D ptXYZ = vecImgInfo[i].Img2XYZ(nCols,nRows,vecImgInfo[i].m_ThumbCmrInfo.f,ptImg0,ptAdjXYZ.Z,true);
			vecImgInfo[i].m_ImgAdjPts.vecImgPtResX[j]=ptImg1.x-ptImg0.x;
			vecImgInfo[i].m_ImgAdjPts.vecImgPtResY[j]=ptImg1.y-ptImg0.y;
		}
		vecImgInfo[i].CloseImg();
	}
}
void FunFitHei4ImgAdjPts4ImgPt(vector<SSWstuAdjPtInfo>& vecAdjPtsAll, double dMeanZOfCq,SSWstuAdjPt4Img& ImgAdjPtInfo, Point2D ptImgAimPt, double& Z)
{
	if(ImgAdjPtInfo.vecAdjPtIdx.size()==0)
	{
		Z=dMeanZOfCq;
		return;
	}
	double dMinDis = 1e10;
	for (int i = 0; i<ImgAdjPtInfo.vecAdjPtIdx.size(); i++)
	{
		Point2D ptImg;
		int nAdjPtIdx = ImgAdjPtInfo.vecAdjPtIdx[i];
		int nImgPtIdx = ImgAdjPtInfo.vecImgPtIdxInAdjPt[i];
		ptImg=vecAdjPtsAll[nAdjPtIdx].vecImgPt[nImgPtIdx];
		double dis = sqrt(pow(ptImg.x-ptImgAimPt.x,2)+pow(ptImg.y-ptImgAimPt.y,2));
		if(dMinDis>dis)
		{
			dMinDis=dis;
			Z = vecAdjPtsAll[nAdjPtIdx].Z;
		}
	}
}
void FunFitHei4ImgAdjPts4ObjPt(vector<SSWstuAdjPtInfo>& vecAdjPtsAll, double dMeanZOfCq,SSWstuAdjPt4Img& ImgAdjPtInfo, Point2D ptObjAimPt, double& Z)
{
	if(ImgAdjPtInfo.vecAdjPtIdx.size()==0)
	{
		Z=dMeanZOfCq;
		return;
	}
	double dMinDis = 1e10;
	for (int i = 0; i<ImgAdjPtInfo.vecAdjPtIdx.size(); i++)
	{
		Point2D ptObj;
		int nAdjPtIdx = ImgAdjPtInfo.vecAdjPtIdx[i];
		ptObj.x=vecAdjPtsAll[nAdjPtIdx].X;
		ptObj.y=vecAdjPtsAll[nAdjPtIdx].Y;
		double dis = sqrt(pow(ptObj.x-ptObjAimPt.x,2)+pow(ptObj.y-ptObjAimPt.y,2));
		if(dMinDis>dis)
		{
			dMinDis=dis;
			Z = vecAdjPtsAll[nAdjPtIdx].Z;
		}
	}
}
void FunTransPolygonVertexOrder(Point3D* &pVertex3d, int nCount /*=4*/)
{
	if(nCount!=4) return;	
	vector<int> vIdxNear;
	Point2D*pVertex2d = new Point2D[nCount];
	Point3D*pVertec3d_2 = new Point3D[nCount];
	for(int i = 0; i<nCount; i++)
	{
		pVertex2d[i].x = pVertex3d[i].X;
		pVertex2d[i].y = pVertex3d[i].Y;
		pVertec3d_2[i] = pVertex3d[i];
	}
	Point2D pt1 = pVertex2d[0];
	for (int j = 1; j<nCount; j++)
	{
		int nSide = 0;
		Point2D pt2 = pVertex2d[j];
		double a,b,c;
		FunCalLineParas(pt1,pt2,a,b,c);
		for (int k = 1; k<nCount; k++)
		{
			if(k==j) continue;
			if(a*pVertex2d[k].x+b*pVertex2d[k].y+c>0)
				nSide++;
			else 
				nSide--;
		}
		if(fabs(nSide*1.0)==nCount-2)
		{
			vIdxNear.push_back(j);
		}
	}
	if(vIdxNear.size()==0) return;
	int nIdx;
	for (int i = 1; i<nCount; i++)
	{
		bool bNear = false;
		for (int j = 0; j<vIdxNear.size(); j++)
		{
			if(i==vIdxNear[j])
			{
				bNear = true;
				continue;
			}
		}
		if(!bNear) nIdx = i;
	}
	pVertex3d[0]=pVertec3d_2[0];
	pVertex3d[1]=pVertec3d_2[vIdxNear[0]];
	pVertex3d[2]=pVertec3d_2[nIdx];
	pVertex3d[3]=pVertec3d_2[vIdxNear[1]];
	delete[]pVertec3d_2;
	delete[]pVertex2d;
	vector<int>().swap(vIdxNear);
}
OGRPolygon* FunCreatePolygon(void* pVertex, int nDim, int nCount)
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
Point2D* FunPolygonIntersect(OGRPolygon* pPolygon1, OGRPolygon* pPolygon2, int& nPtNum)
{
	Point2D* pPtInterVertex = NULL;
	if(pPolygon1==NULL||pPolygon2==NULL) return false;
	OGRGeometry *pInterGeometry = pPolygon1->Intersection(pPolygon2);
	if(pInterGeometry==NULL) 
	{
		OGRGeometryFactory::destroyGeometry(pPolygon1);
		OGRGeometryFactory::destroyGeometry(pPolygon2);
		return NULL;
	}
	if(strcmp("POLYGON",pInterGeometry->getGeometryName()) != 0)
	{
		OGRGeometryFactory::destroyGeometry(pInterGeometry);
		OGRGeometryFactory::destroyGeometry(pPolygon1);
		OGRGeometryFactory::destroyGeometry(pPolygon2);
		return NULL;
	}
	OGRPolygon *pInterPolygon12 = (OGRPolygon*)pInterGeometry;
	OGRLinearRing *pInterRing12 = pInterPolygon12->getExteriorRing();
	nPtNum = pInterRing12->getNumPoints() - 1;
	if(nPtNum<3) return NULL;
	if(pPtInterVertex!=NULL)
	{
		delete[] pPtInterVertex;
		pPtInterVertex = NULL;
	}
	pPtInterVertex = new Point2D[nPtNum];
	for (int i = 0;i<nPtNum;  i++)
	{
		OGRPoint ogrpt;
		pInterRing12->getPoint(i,&ogrpt);
		pPtInterVertex[i].x = ogrpt.getX();
		pPtInterVertex[i].y = ogrpt.getY();
	}
	return pPtInterVertex;
}
//数据排序
void FunSortByX(int nCount, float *&pX, float *&pY)
{
	for (int i = 0; i<nCount-1; i++)
	{
		for (int j = i+1;j <nCount; j++)
		{
			if(*(pX+i)>*(pX+j))
			{
				float tmp = *(pX+i);
				*(pX+i)=*(pX+j);
				*(pX+j)=tmp;
				tmp = *(pY+i);
				*(pY+i)=*(pY+j);
				*(pY+j)=tmp;
			}
		}
	}

}
void FunSortByX(int nCount, float *&pX, float *&pY,float *&pZ)
{
	for (int i = 0; i<nCount-1; i++)
	{
		for (int j = i+1;j <nCount; j++)
		{
			if(*(pX+i)>*(pX+j))
			{
				float tmp = *(pX+i);
				*(pX+i)=*(pX+j);
				*(pX+j)=tmp;
				tmp = *(pY+i);
				*(pY+i)=*(pY+j);
				*(pY+j)=tmp;
				tmp = *(pZ+i);
				*(pZ+i)=*(pZ+j);
				*(pZ+j)=tmp;
			}
		}
	}

}

//质检报告单条目记录
enum PdfListContent
{
	PdfList_CQPreView = 1,
	PdfList_POSValue = 2,
	PdfList_Overlap = 3,
	PdfList_OverlapBad = 4,
	PdfList_ElseBad =5,
	PdfList_TimeUsed = 6
};
void NewListInfoMem2Pdf(int nCols,int nRows,char **&pStrInfo/*, float *&pr, float *&pg, float *&pb*/)
{
	pStrInfo = new char *[nRows*nCols];
	for (int i = 0; i<nCols*nRows; i++)
	{
		pStrInfo[i] = new char[MAX_SIZE_FILE_LINE];
	}
}
void DeleteListInfoMem2Pdf(int nCols,int nRows,char **&pStrInfo/*, float *pr,float *pg, float *pb*/)
{
	for (int i = 0; i<nCols*nRows; i++)
	{
		delete[]pStrInfo[i];
	}
	delete[]pStrInfo;
	// 	delete[]pr;
	// 	delete[]pg;
	// 	delete[]pb;
}
/*********************************************/
/* CsswImage */
/*********************************************/
static bool g_bRegisterGdal = false;
CsswImage::CsswImage()
{
	m_pDataset = NULL;
	if(!g_bRegisterGdal)
	{
		GDALAllRegister();         //利用GDAL读取图片，先要进行注册  
		CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");   //设置支持中文路径
		g_bRegisterGdal = true;
	}
}
CsswImage::~CsswImage()
{
	if(m_pDataset) CloseImg();
}
CString CsswImage::findImageTypeGDAL(CString strDetImgName)
{
	CString strExt=FunGetFileExt(strDetImgName);
	strExt.MakeLower();
	if     (strExt=="bmp") return "BMP";
	else if(strExt=="jpg"|| strExt=="jpeg") return "JPEG";
	else if(strExt=="png") return "PNG";
	else if(strExt=="tif"||strExt=="tiff")  return "GTiff";
	else if(strExt=="gif") return "GIF";
	else return""; 
}
bool CsswImage::OpenImg(CString strImgPath, bool bReadOnly/* =true */)
{
	if(!PathFileExists(strImgPath)) return false;
	GDALAccess eAcess = bReadOnly?GA_ReadOnly:GA_Update;
	m_pDataset=(GDALDataset*) GDALOpen(strImgPath,eAcess);
	if(m_pDataset==NULL) return false;
	return true;
}
void CsswImage::CloseImg()
{
	if(m_pDataset) 
	{
		GDALClose((GDALDataset*)m_pDataset);
		m_pDataset=NULL;
	}
}
int  CsswImage::GetRows()
{
	if(m_pDataset==NULL) return InvalidValue;
	return ((GDALDataset*)m_pDataset)->GetRasterYSize();
}
int  CsswImage::GetCols()
{
	if(m_pDataset==NULL) return InvalidValue;
	return ((GDALDataset*)m_pDataset)->GetRasterXSize();
}
int  CsswImage::GetBandCount()
{

	if(m_pDataset==NULL) return InvalidValue;
	return ((GDALDataset*)m_pDataset)->GetRasterCount();
}
int  CsswImage::GetBitSize()
{

	if(m_pDataset==NULL) return InvalidValue;
	return GDALGetDataTypeSize(((GDALDataset*)m_pDataset)->GetRasterBand(1)->GetRasterDataType())/8;
}
float CsswImage::GetFocalLength(SSWUNITTYPE eUnit/* =PIX */)
{
	if(!m_pDataset) return 0;
	GDALDataset* pDataSet = (GDALDataset*)m_pDataset;
	char** pMetadata = pDataSet->GetMetadata(NULL);
	float fFocalLength = 0, fPixSize=1;
	bool bFocal = false,bPixSize = false;
	int nCount = CSLCount(pMetadata);
	if(nCount==0) return 0;
	float fCCDWidth = 35;
	if(eUnit==PIX)
	{
		for (int i = 0; i<nCount; i++)
		{
			cprintf("%s\n",pMetadata[i]);
			vector<CString> vTok = FunStrTok(pMetadata[i],"=");
			if(vTok.size() < 2) continue;
			if(vTok[0]=="EXIF_FocalLengthIn35mmFilm")
			{
				vector<CString> vTok2 = FunStrTok(vTok[1],"()");
				if(vTok2.size()==1) fFocalLength=atof(vTok2[0]);
				else if(vTok2.size()==3) fFocalLength=atof(vTok2[1]);
				fPixSize = fCCDWidth/GetCols();
				return fFocalLength/fPixSize;
			}
		}
	}else
	{
		for (int i = 0; i<nCount; i++)
		{
			cprintf("%s\n",pMetadata[i]);
			vector<CString> vTok = FunStrTok(pMetadata[i],"=");
			if(vTok.size() < 2) continue;
			if(vTok[0]=="EXIF_FocalLength")
			{
				vector<CString> vTok2 = FunStrTok(vTok[1],"()");
				if(vTok2.size()==1) fFocalLength=atof(vTok2[0]);
				else if(vTok2.size()==3) fFocalLength=atof(vTok2[1]);
				switch(eUnit)
				{
				case UM: return fFocalLength*10;
				case MM: return fFocalLength;
				case CM: return fFocalLength/10;
				case M:  return fFocalLength/1000;
				}
			}
		}
	}
	return 0;
}
bool CsswImage::ReadImg(int nStartCol, int nStartRow, int nCols, int nRows, int nMemWidth, int nMemHeight, void*pMem)
{
	if(m_pDataset==NULL) return false;
	GDALDataset* pDataset = (GDALDataset*)m_pDataset;
	nStartCol=max(nStartCol,0); nCols = min(GetCols(),nCols);
	nStartRow=max(nStartRow,0); nRows = min(GetRows(),nRows);
	int nBands=GetBandCount();
	int nBitSize=GetBitSize();	
	int *pBandMap = new int[nBands];
	for (int i=0; i<nBands; i++)
	{
		*(pBandMap+i)=i+1;;
	}

	BYTE* pRead = (BYTE*)pMem;
	if(GDT_ERROR==pDataset->RasterIO(GF_Read, nStartCol, nStartRow, nCols, nRows, pRead, nMemWidth, nMemHeight,GDT_Byte,
		nBands, pBandMap, nBands*nBitSize,nBands*nBitSize*nMemWidth,nBitSize)) return false;
	return true;
}
bool CsswImage::SaveImg(CString strSavePath, int nCols, int nRows, int nBands, BYTE*pMem,const char *pszFormat /* = JPEG */)
{
	if(strcmp(pszFormat,"GTiff")==0)
	{
		GDALDriver *poDriver=GetGDALDriverManager()->GetDriverByName(pszFormat);  
		if( poDriver == NULL ) return false;
		GDALDataset *WriteDataSet = poDriver->Create(strSavePath, nCols,nRows,nBands,GDT_Byte,NULL);  
		int pBandMap[3]={1,2,3};
		if(WriteDataSet->RasterIO(GF_Write,0,0,nCols,nRows,pMem,nCols,nRows,GDT_Byte,nBands,NULL,nBands*1,nCols*nBands*1,1)==CE_Failure )  
		{  
			return false;  
		}  
		GDALClose(poDriver);   
		GDALClose(WriteDataSet); WriteDataSet=NULL;  
		return true; 
	}
	else
	{
		if (strSavePath.IsEmpty() || pMem == NULL || nCols <1 || nRows < 1 || nBands < 1) 
		{
			return false;
		}

		CString GType = findImageTypeGDAL(strSavePath);  
		if (GType.IsEmpty())  { return false; }  
		GDALDriver *pMemDriver = NULL;  
		pMemDriver = GetGDALDriverManager()->GetDriverByName("MEM");  
		if( pMemDriver == NULL ) { return false; }  

		GDALDataset * pMemDataSet = pMemDriver->Create("",nCols,nRows,3,GDT_Byte,NULL);  
		GDALRasterBand *pBand = NULL;  
		int nLineCount = nCols * 3;  
		int pBandMap[3]={1,2,3};
		CPLErr err = pMemDataSet->RasterIO(GF_Write,0,0,nCols,nRows,pMem,nCols,nRows,GDT_Byte,3,pBandMap,nBands*1,nLineCount,1);
		if(err==CE_Failure) return false;
		GDALDriver *pDstDriver = NULL;  
		pDstDriver = (GDALDriver *)GDALGetDriverByName(GType);  
		if (pDstDriver == NULL) { return false; }  

		pDstDriver->CreateCopy(strSavePath,pMemDataSet,FALSE, NULL, NULL, NULL);  
		GDALClose(pMemDataSet);   
		return true;
	}
}

/*********************************************/
/* CsswUAVImage */
/*********************************************/
CsswUAVImage::CsswUAVImage()
{
	m_nImgIdx =InvalidValue;           
	m_nStripIdx = InvalidValue;			
	m_nIdxInStrip = InvalidValue;					

	m_strImgPath.Empty();                  
	m_strThumbPath.Empty();  
	m_strStripLabel = " ";
	m_strThumbName.Empty();
	m_ImgCmrInfo = SSWstuCmrInfo();           
	m_ThumbCmrInfo=SSWstuCmrInfo();          
	m_ImgPosInfo=SSWstuPosInfo();            
	m_AdjPosInfo=SSWstuPosInfo();            
	vector<int>().swap(m_ImgAdjPts.vecLinkImgIdx);
	vector<vector<int>>().swap(m_ImgAdjPts.vvAdjPtIdx4EachLinkImg);
	vector<vector<int>>().swap(m_ImgAdjPts.vvImgPtIdxInAdjPt);
	vector<vector<int>>().swap(m_ImgAdjPts.vvLinkImgPtIdxInAdjPt);
	m_bValid = true;                         
	m_bAdjed = false;                         
	// 	m_pImgVertexPt2d4 = NULL;
	// 	m_pThumbImgVertexPt2d4 = NULL;
	//m_pImgVertexPt3d4 = NULL;
}
CsswUAVImage::~CsswUAVImage()
{
	// 	if(m_pImgVertexPt2d4) 
	// 	{
	// 		delete[]m_pImgVertexPt2d4;
	// 		m_pImgVertexPt2d4=NULL;
	// 	}
	// 	if(m_pThumbImgVertexPt2d4) 
	// 	{
	// 		delete[]m_pThumbImgVertexPt2d4;
	// 		m_pThumbImgVertexPt2d4=NULL;
	// 	}
	for(int i = 0; i<4; i++)
	{
		m_pImgVertexPt2d4[i]=Point2D();
		m_pThumbImgVertexPt2d4[i]=Point2D();
	}
	m_nImgIdx =InvalidValue;           
	m_nStripIdx = InvalidValue;			
	m_nIdxInStrip = InvalidValue;					

	m_strImgPath.Empty();                  
	m_strThumbPath.Empty();                
	m_ImgCmrInfo = SSWstuCmrInfo();           
	m_ThumbCmrInfo=SSWstuCmrInfo();          
	m_ImgPosInfo=SSWstuPosInfo();            
	m_AdjPosInfo=SSWstuPosInfo();            

	vector<int>().swap(m_ImgAdjPts.vecLinkImgIdx);
	vector<vector<int>>().swap(m_ImgAdjPts.vvAdjPtIdx4EachLinkImg);
	vector<vector<int>>().swap(m_ImgAdjPts.vvImgPtIdxInAdjPt);
	vector<vector<int>>().swap(m_ImgAdjPts.vvLinkImgPtIdxInAdjPt);

	m_bValid = true;                         
	m_bAdjed = false;  
}
void CsswUAVImage::CalRotMatrixByPOK(SSWstuPosInfo& sswPosInfo)
{
	double phi = DegToRad(sswPosInfo.phi), omega = DegToRad(sswPosInfo.omg), kappa = DegToRad(sswPosInfo.kap);
	double cp = cos(phi), sp = sin(phi);
	double co = cos(omega), so = sin(omega);
	double ck = cos(kappa), sk = sin(kappa);
	sswPosInfo.R[0] = cp*ck - so*sp*sk; sswPosInfo.R[1] = -cp*sk - so*sp*ck; sswPosInfo.R[2] = -co*sp;
	sswPosInfo.R[3] = co*sk; sswPosInfo.R[4] = co*ck; sswPosInfo.R[5] = -so;
	sswPosInfo.R[6] = sp*ck + so*cp*sk; sswPosInfo.R[7] = -sp*sk + so*cp*ck; sswPosInfo.R[8] = co*cp;
}
Point2D CsswUAVImage::XYZ2Img(int nCols, int nRows, double f, Point3D &ptXYZ, bool bAdjed)
{
	SSWstuPosInfo sswPosInfo; 
	if(!bAdjed)	sswPosInfo = m_ImgPosInfo;
	else sswPosInfo = m_AdjPosInfo;
	if(sswPosInfo.R[0]==InvalidValue) CalRotMatrixByPOK(sswPosInfo);
	double Xs, Ys, Zs;
	Xs = sswPosInfo.Xs;
	Ys = sswPosInfo.Ys;
	Zs = sswPosInfo.Zs;
	double _X = ptXYZ.X;
	double _Y = ptXYZ.Y;
	double _Z = ptXYZ.Z;
	double X_new = sswPosInfo.R[0] * (_X - Xs) + sswPosInfo.R[3] * (_Y - Ys) + sswPosInfo.R[6] * (_Z - Zs);
	double Y_new = sswPosInfo.R[1] * (_X - Xs) + sswPosInfo.R[4] * (_Y - Ys) + sswPosInfo.R[7] * (_Z - Zs);
	double Z_new = sswPosInfo.R[2] * (_X - Xs) + sswPosInfo.R[5] * (_Y - Ys) + sswPosInfo.R[8] * (_Z - Zs);
	double x, y;
	x = -f*X_new / Z_new+nCols/2;
	y = -f*Y_new / Z_new+nRows/2;
	Point2D	ptImg = Point2D(x, y);
	return ptImg;
}
Point3D CsswUAVImage::Img2XYZ(int nCols, int nRows,double f, Point2D &ptImg, double Z, bool bAdjed)
{
	SSWstuPosInfo sswPosInfo =m_ImgPosInfo;
	if(!bAdjed) sswPosInfo = m_ImgPosInfo;
	else sswPosInfo = m_AdjPosInfo;
	if(sswPosInfo.R[0]==InvalidValue) CalRotMatrixByPOK(sswPosInfo);
	double Xs, Ys, Zs;
	Xs = sswPosInfo.Xs;
	Ys = sswPosInfo.Ys;
	Zs = sswPosInfo.Zs;
	double _px = ptImg.x-nCols/2;
	double _py = ptImg.y-nRows/2;
	double _Z = Z;
	double A1 = sswPosInfo.R[2] * _px + sswPosInfo.R[0] * f;
	double B1 = sswPosInfo.R[5] * _px + sswPosInfo.R[3] * f;
	double M1 = (sswPosInfo.R[2] * _px + sswPosInfo.R[0] * f)*Xs +
		(sswPosInfo.R[5] * _px + sswPosInfo.R[3] * f)*Ys -
		(sswPosInfo.R[8] * _px + sswPosInfo.R[6] * f)*(_Z - Zs);

	double A2 = sswPosInfo.R[2] * _py + sswPosInfo.R[1] * f;
	double B2 = sswPosInfo.R[5] * _py + sswPosInfo.R[4] * f;
	double M2 = (sswPosInfo.R[2] * _py + sswPosInfo.R[1] * f)*Xs +
		(sswPosInfo.R[5] * _py + sswPosInfo.R[4] * f)*Ys -
		(sswPosInfo.R[8] * _py +  sswPosInfo.R[7] * f)*(_Z - Zs);
	double _X = (M1*B2 - M2*B1) / (A1*B2 - A2*B1);
	double _Y = (M1*A2 - M2*A1) / (A2*B1 - A1*B2);
	Point3D ptXYZ = Point3D(_X, _Y, Z);
	return ptXYZ;
}
void CsswUAVImage::CalImg4VertexPt3d(double* pZ, bool bAdjed, bool bThumOrNot, Point3D *&pVertex3d)
{
	Point2D *pVerPtImg4 = GetImgVertexPt2d4(bThumOrNot);
	double f;
	if(bThumOrNot) f = m_ThumbCmrInfo.f;
	else f = m_ImgCmrInfo.f;
	for (int i = 0; i<4; i++)
	{
		pVertex3d[i] = Img2XYZ(pVerPtImg4[2].x,pVerPtImg4[1].y,f,pVerPtImg4[i],*(pZ+i),bAdjed);
	}
}
Point2D* CsswUAVImage::GetImgVertexPt2d4(bool bThumOrNot)
{
	if(bThumOrNot)
	{
		if(m_pThumbImgVertexPt2d4[0]==Point2D())
		{
			//		m_pThumbImgVertexPt2d4 = new Point2D[4];		
			OpenImg(m_strThumbPath);
			int nCols = GetCols();
			int nRows = GetRows();
			CloseImg();
			m_pThumbImgVertexPt2d4[0]=Point2D(0,0);
			m_pThumbImgVertexPt2d4[1]=Point2D(0,nRows);
			m_pThumbImgVertexPt2d4[2]=Point2D(nCols,nRows);
			m_pThumbImgVertexPt2d4[3]=Point2D(nCols,0);

		}
		return m_pThumbImgVertexPt2d4;
	}
	else
	{
		if(m_pImgVertexPt2d4[0]==Point2D())
		{
			//		m_pImgVertexPt2d4 = new Point2D[4];

			OpenImg(m_strImgPath);
			int nCols = GetCols();
			int nRows = GetRows();
			CloseImg();
			m_pImgVertexPt2d4[0]=Point2D(0,0);
			m_pImgVertexPt2d4[1]=Point2D(0,nRows);
			m_pImgVertexPt2d4[2]=Point2D(nCols,nRows);
			m_pImgVertexPt2d4[3]=Point2D(nCols,0);

		}
		return m_pImgVertexPt2d4;
	}

}
Point3D* CsswUAVImage::GetImgVertexPt3d4(double* pZ, bool bAdjed, bool bThumOrNot)
{
	Point3D *pVertex3d = new Point3D[4];
	CalImg4VertexPt3d( pZ, bAdjed, bThumOrNot,pVertex3d);
	return pVertex3d;
}
// Point3D* CsswUAVImage::GetImgVertexPt3d42(double Z0,bool bAdjed, bool bThumOrNot)
// {
// 	double dis = 1e10;
// 	double X,Y,Z = Z0;
// 	double f, nCols, nRows;
// 	Point3D *pVertex3d = new Point3D[4];
// 	for (int i = 0; i<4; i++)
// 	{
// 		pVertex3d[i] = Point3D(0,0,0);
// 	}
// 	if(bThumOrNot)
// 	{
// 		f = m_ThumbCmrInfo.f;
// 		OpenImg(m_strThumbPath);
// 		nCols = GetCols();
// 		nRows = GetRows();
// 	}else
// 	{
// 		f = m_ImgCmrInfo.f;
// 		OpenImg(m_strImgPath);
// 		nCols = GetCols();
// 		nRows = GetRows();
// 	}
// 	while(dis>10)
// 	{
// 		Point3D pt3d = Img2XYZ(nCols,nRows,f,Point2D(0,0),Z,bAdjed);
// 		dis = sqrt((pt3d.X-pVertex3d[0].X)*(pt3d.X-pVertex3d[0].X)+
// 			(pt3d.Y-pVertex3d[0].Y)*(pt3d.Y-pVertex3d[0].Y)+
// 			(pt3d.Z-pVertex3d[0].Z)*(pt3d.Z-pVertex3d[0].Z));
// 		/*	FunFitHei4ImgAdjPts4ObjPt()*/
// 	}
// 	return pVertex3d;
// }
/*********************************************/
/* CsswUAVFlyQuaPrj */
/*********************************************/
static bool XmlNodeJudge(pugi::xml_node& Node)
{
	if(Node==NULL)
	{
		FunOutPutLogInfo("工程文件解析错误！");
		return false;
	}
	return true;
}

#define ImageNumLimite -1
CsswUAVFlyQuaPrj::CsswUAVFlyQuaPrj()
{
	m_pMatchMatrix=NULL;
	ClearPrjData();
}
CsswUAVFlyQuaPrj::~CsswUAVFlyQuaPrj()
{
	ClearPrjData();
}
void CsswUAVFlyQuaPrj::ClearPrjData()
{
	m_strPrjXmlPath.Empty();
	m_strImpXmlPath.Empty();
	m_strImgListPath.Empty();
	m_strPosListPath.Empty();
	m_strMatchMatrixPath.Empty();
	m_strBundleAdjRepPath.Empty();
	m_strMatchPointPath.Empty();
	m_strThumbFolder.Empty();
	vector<CsswUAVImage>().swap(m_vecImgInfo);
	vector<CsswUAVImage>().swap(m_vecInvalidImgInfo);
	m_bCmrFile = false;
	m_bAutoManageImages = false;
	m_nImgNum = 0;
	m_nInvalidNum = 0;
	m_nInAdjedNum = 0;
	m_dRmsPosErr = 0;
	m_nStripNum = 0;
	m_nMaxStripIdx = 0;
	m_ImgCmrInfo = SSWstuCmrInfo();       //影像相机参数
	m_ThumbCmrInfo = SSWstuCmrInfo();     //缩略图相机参数
	m_sswPosFormat = SSWstuPosFileExtend();
	vector<int>().swap(m_vecStripIdx);
	vector<int>().swap(m_vecImgNumInEachStrip);
	vector<vector<int>>().swap(m_vvImgIdxInEachStrip);
	vector<int>().swap(m_vecBundleImgList2ImgInfo);
	vector<CString>().swap(m_vecBundleImgName);
	if(m_pMatchMatrix!=NULL) 
	{
		delete[] m_pMatchMatrix;
		m_pMatchMatrix=NULL;
	}
	ClearReportInfo(BY_POS);
	ClearStripInfo();
	ClearAdjInfo();
	m_dTimeSecsWholeProcess=0;
	m_dTimeSecsCmrDistortion =0;
	m_dTimeSecsBundleAdjustment=0;
	m_dTimeSecsTimeReadAdjRep=0;
	m_dTimeSecsRunFlyQuaChk=0;
	m_dTimeSecsWriteChkRep=0;
	m_nMatchPointNumThresold = 8;
}

bool CsswUAVFlyQuaPrj::LoadData2Prj(vector<CsswUAVImage> &vecImg,CString strPrjPath, SSWstuSysConfigPara para, bool bWithCmr)
{
	if(vecImg.size()==0)
	{
		FunOutPutLogInfo("影像数为0，工程文件保存失败！");
		return false;
	}
	if(ImageNumLimite>0)
	{
		if(vecImg.size()>ImageNumLimite)
		{
			CString strInfo;
			strInfo.Format("影像数最大为%d，超出%d的限制，工程建立失败！",vecImg.size(), ImageNumLimite);
			FunOutPutLogInfo(strInfo);
			return false;
		}
	}
	if(strPrjPath.IsEmpty()) 
	{
		FunOutPutLogInfo("工程文件存储路径有误,工程文件保存失败！");
		return false;
	}
	ClearPrjData();
	m_bCmrFile = bWithCmr;
	//	if(m_bWithCmr)
	m_ImgCmrInfo = vecImg[0].m_ImgCmrInfo;
	m_vecImgInfo=vecImg;
	m_nImgNum = vecImg.size();
	m_strPrjXmlPath=strPrjPath;
	m_strImpXmlPath=FunGetFileFolder(m_strImpXmlPath)+"\\"+FunGetFileName(m_strPrjXmlPath,false)+"imp.xml";
	m_strThumbFolder = FunGetFileFolder(m_strPrjXmlPath)+"\\CameraDistortionCorrect";
	m_strMatchAndAdjFolder = FunGetFileFolder(m_strPrjXmlPath)+"\\MatchAndAdjustment";
	m_strImgListPath = m_strMatchAndAdjFolder+"\\MatchImgList.txt";
	m_strImgListPath = m_strMatchAndAdjFolder+"\\MatchPosList.txt";
	m_strImgListPath = m_strMatchAndAdjFolder+"\\MatchMatrix.txt";
	m_strBundleAdjRepPath= m_strMatchAndAdjFolder+"\\BundleAdjustment\\Freenet_SBA.zgout";
	m_strMatchPointPath = m_strMatchAndAdjFolder+"\\BundleAdjustment\\Freenet_Match.zgout";
	SavePrj2File(strPrjPath);
	m_FlyQuaParaSet = para;
	return true;
}
bool CsswUAVFlyQuaPrj::LoadPrj4File(CString strFilePath)
{
	if(strFilePath.IsEmpty()||!PathFileExists(strFilePath)) return false;
	ClearPrjData();
	m_strPrjXmlPath=strFilePath;
	m_strImpXmlPath=FunGetFileFolder(m_strPrjXmlPath)+"\\"+FunGetFileName(m_strPrjXmlPath,false)+"_imp.xml";	
	m_strThumbFolder = FunGetFileFolder(m_strPrjXmlPath)+"\\CameraDistortionCorrect";
	m_strMatchAndAdjFolder = FunGetFileFolder(m_strPrjXmlPath)+"\\MatchAndAdjustment";
	m_strImgListPath = m_strMatchAndAdjFolder+"\\MatchImgList.txt";
	m_strPosListPath = m_strMatchAndAdjFolder+"\\MatchPosList.txt";
	m_strMatchMatrixPath = m_strMatchAndAdjFolder+"\\MatchMatrix.txt";
	m_strBundleAdjRepPath= m_strMatchAndAdjFolder+"\\BundleAdjustment\\Freenet_SBA.zgout";
	m_strMatchPointPath = m_strMatchAndAdjFolder+"\\BundleAdjustment\\Freenet_Match.zgout";
	if(!ReadPrjXml()){ClearPrjData(); return false;}
	if(!ReadImpXml()){ClearPrjData(); return false;}
	if(ImageNumLimite>0)
	{
		if(m_vecImgInfo.size()>ImageNumLimite)
		{
			CString strInfo;
			strInfo.Format("工程影像数为%d，超出%d的限制，工程打开失败！",m_vecImgInfo.size(),ImageNumLimite);
			FunOutPutLogInfo(strInfo);
			return false;
		}
	}
	CString strThumbCmrPath = m_strThumbFolder+"\\cmr_thumb.cmr";
	if(PathFileExists(strThumbCmrPath))
	{
		FunReadThumbCmrFile(strThumbCmrPath,m_ThumbCmrInfo);
	}
	for (int i = 0; i<m_nImgNum; i++)
	{
		m_vecImgInfo[i].m_ThumbCmrInfo = m_ThumbCmrInfo;
	}
	//	ReadImgList();
	for (int i = 0; i<m_vecImgInfo.size(); i++)
	{
		m_vecImgInfo[i].m_strStripLabel=FunCreateStripLabel(m_vecImgInfo[i].m_nStripIdx,m_vecImgInfo[i].m_nIdxInStrip);
	}
	return true;
}
bool CsswUAVFlyQuaPrj::ReadPrjXml()
{
	pugi::xml_document PrjXmlDoc;
	pugi::xml_parse_result ret = PrjXmlDoc.load_file(m_strPrjXmlPath,pugi::parse_full,pugi::encoding_utf8);
	if(ret.status!=pugi::status_ok)
	{
		FunOutPutLogInfo("工程xml打开失败！");
		return false;
	}
	pugi::xml_node NodeRoot = PrjXmlDoc.child("UAV_Flight_Quality_Check");
	XmlNodeJudge(NodeRoot);
	pugi::xml_node NodePrj = NodeRoot.child("Project");
	XmlNodeJudge(NodePrj);
	for (pugi::xml_node Node1 = NodePrj.first_child(); Node1!=NULL; Node1=Node1.next_sibling())
	{
		CString NodeLabel = Node1.name();
		if(NodeLabel=="AutoManageImages") m_bAutoManageImages = Node1.text().as_bool();else
		if(NodeLabel=="ImageData")
		{
			for (pugi::xml_node NodeImgChid = Node1.first_child(); NodeImgChid!=NULL; NodeImgChid=NodeImgChid.next_sibling())
			{
				CString ChidLabel = NodeImgChid.name();
					if(ChidLabel=="ImageNumber") m_nImgNum = NodeImgChid.text().as_int();else
						if(ChidLabel=="InvalidNumber") m_nInvalidNum = NodeImgChid.text().as_int();else
							if(ChidLabel=="StripNumber")
							{
								m_nStripNum = NodeImgChid.text().as_int();
								if(m_nStripNum>0)
								{
									m_vecImgNumInEachStrip.resize(m_nStripNum);
								}
							}
							for (int i = 0; i<m_nStripNum; i++)
							{
								char strStripLabel[100];sprintf(strStripLabel,"Strip_%d",i);
								if(ChidLabel==strStripLabel)
								{
									m_vecImgNumInEachStrip[i] = NodeImgChid.text().as_int();
								}
							}
			}
		}else
			if(NodeLabel=="CameraParas")
			{
				for (pugi::xml_node NodeCmrChid = Node1.first_child(); NodeCmrChid!=NULL; NodeCmrChid=NodeCmrChid.next_sibling())
				{
					CString ChidLabel = NodeCmrChid.name();
					if(ChidLabel=="WithFile") m_bCmrFile=NodeCmrChid.text().as_bool(); else
						if(ChidLabel=="Label") m_ImgCmrInfo.strLabel=NodeCmrChid.text().as_string();else
							if(ChidLabel=="Format_x0y0") m_ImgCmrInfo.eUnitx0y0=sswString2UnitType(NodeCmrChid.text().as_string());else
								if(ChidLabel=="Format_f")m_ImgCmrInfo.eUnitf=sswString2UnitType(NodeCmrChid.text().as_string());else
									if(ChidLabel=="Format_pixelsize")m_ImgCmrInfo.eUnitpixsize=sswString2UnitType(NodeCmrChid.text().as_string());else
										if(ChidLabel=="x0")m_ImgCmrInfo.x0=NodeCmrChid.text().as_double();else
											if(ChidLabel=="y0")m_ImgCmrInfo.y0=NodeCmrChid.text().as_double();else
												if(ChidLabel=="f")
												{
													vector<CString> vF= FunStrTok(NodeCmrChid.text().as_string()," ");
													if(vF.size()!=3){FunOutPutLogInfo("相机焦距信息有误，工程xml读取失败！"); return false;}
													m_ImgCmrInfo.f=atof(vF[0]);
													m_ImgCmrInfo.fx=atof(vF[1]);
													m_ImgCmrInfo.fy=atof(vF[2]);
													vector<CString>().swap(vF);
												}else
													if(ChidLabel=="pixelsize") m_ImgCmrInfo.pixsize=NodeCmrChid.text().as_double();else
														if(ChidLabel=="distortion_paras")
														{
															vector<CString> vD= FunStrTok(NodeCmrChid.text().as_string()," ");
															if(vD.size()!=6){FunOutPutLogInfo("相机畸变信息有误，工程xml读取失败！"); return false;}
															m_ImgCmrInfo.k1=atof(vD[0]);m_ImgCmrInfo.k2=atof(vD[1]);
															m_ImgCmrInfo.p1=atof(vD[2]);m_ImgCmrInfo.p2=atof(vD[3]);
															m_ImgCmrInfo.a=atof(vD[4]);	m_ImgCmrInfo.b=atof(vD[5]);
															vector<CString>().swap(vD);
														}
				}
			}else 
				if(NodeLabel=="PosParas")
				{
					for (pugi::xml_node NodePosChid = Node1.first_child(); NodePosChid!=NULL; NodePosChid=NodePosChid.next_sibling())
					{
						CString ChidLabel = NodePosChid.name();
						if(ChidLabel=="Format_POS") m_sswPosFormat.FormatCoor=sswString2CoordinateFormat(NodePosChid.text().as_string());else
							if(ChidLabel=="Format_POK")m_sswPosFormat.FormatAnglePOK=sswString2AngleFormat(NodePosChid.text().as_string());else
								if(ChidLabel=="Format_LBH")m_sswPosFormat.FormatAngleLBH=sswString2AngleFormat(NodePosChid.text().as_string());
					}
				}
	}
	pugi::xml_node NodeData = NodeRoot.child("ImportData");
	XmlNodeJudge(NodeData);
	m_strImpXmlPath = FunGetFileFolder(m_strPrjXmlPath)+"\\"+NodeData.text().as_string();
	pugi::xml_node NodePara = NodeRoot.child("FlyQuaPara");
	XmlNodeJudge(NodePara);
	for (pugi::xml_node Node1 = NodePara.first_child(); Node1!=NULL; Node1=Node1.next_sibling())
	{
		CString NodeLabel = Node1.name();
		if(NodeLabel=="MinOverlapY") m_FlyQuaParaSet.fMinOverlapY=Node1.text().as_double();else
			if(NodeLabel=="MinOverlapX") m_FlyQuaParaSet.fMinOverlapX=Node1.text().as_double();else
				if(NodeLabel=="MaxHeiDifInArea") m_FlyQuaParaSet.fMaxHeiDifInArea=Node1.text().as_double();else
					if(NodeLabel=="MaxHeiDifLinkInStrip") m_FlyQuaParaSet.fMaxHeiDifLinkInStrip=Node1.text().as_double();else
						if(NodeLabel=="MaxStripBlend") m_FlyQuaParaSet.fMaxStripBlend=Node1.text().as_double();else
							if(NodeLabel=="MaxPitch") m_FlyQuaParaSet.fMaxPitch=Node1.text().as_double();else
								if(NodeLabel=="MaxPitch10Per") m_FlyQuaParaSet.fMaxPitch10Per=Node1.text().as_double();else
									if(NodeLabel=="MaxYaw") m_FlyQuaParaSet.fMaxYaw=Node1.text().as_double();else
										if(NodeLabel=="MaxYaw15Per") m_FlyQuaParaSet.fMaxYaw15Per=Node1.text().as_double();else
											if(NodeLabel=="MaxYaw20Num") m_FlyQuaParaSet.nMaxYaw20Num=Node1.text().as_int();else
												if(NodeLabel=="nMatchPtNumThresold") m_FlyQuaParaSet.nMatchPtNumThresold=Node1.text().as_int();
	}
	return true;
}
bool CsswUAVFlyQuaPrj::ReadImpXml()
{
	pugi::xml_document ImpXmlDoc;
	pugi::xml_parse_result ret = ImpXmlDoc.load_file(m_strImpXmlPath,pugi::parse_full,pugi::encoding_utf8);
	if(ret.status!=pugi::status_ok)
	{
		FunOutPutLogInfo("输入数据xml打开失败！");
		return false;
	}
	pugi::xml_node NodeRoot = ImpXmlDoc.child("Import_data");
	XmlNodeJudge(NodeRoot);
	CsswUAVImage sswImg;
	sswImg.m_ImgPosInfo.eFormatAngleLBH = m_sswPosFormat.FormatAngleLBH;
	sswImg.m_ImgPosInfo.eFormatAnglePOK = m_sswPosFormat.FormatAnglePOK;
	sswImg.m_ImgPosInfo.eFormatCoordinate = m_sswPosFormat.FormatCoor;
	sswImg.m_ImgCmrInfo = m_ImgCmrInfo;
	for (pugi::xml_node NodeImage=NodeRoot.first_child(); NodeImage!=NULL; NodeImage=NodeImage.next_sibling())
	{
		for (pugi::xml_node Child=NodeImage.first_child(); Child!=NULL; Child=Child.next_sibling())
		{
			CString ChildLabel = Child.name();
			if(ChildLabel=="ID") sswImg.m_nImgIdx=Child.text().as_int();else
				if(ChildLabel=="Path")
				{
					sswImg.m_strImgPath=Child.text().as_string();
					sswImg.m_strThumbPath = FunGetFileFolder(m_strPrjXmlPath)+"\\CameraDistortionCorrect\\"+FunGetFileName(sswImg.m_strImgPath,false)+".jpg";
					sswImg.m_ImgPosInfo.strLabel=FunGetFileName(sswImg.m_strThumbPath,false).MakeLower();
					sswImg.m_strThumbName = FunGetFileName(sswImg.m_strThumbPath,false).MakeLower();
					sswImg.m_strImgName = FunGetFileName(sswImg.m_strImgPath,true).MakeLower();
				}else
					if(ChildLabel=="Pos")
					{
						vector<CString>vP=FunStrTok(Child.text().as_string()," ");
						if(vP.size()!=6)
						{
							FunOutPutLogInfo("影像POS信息解析错误，工程文件打开失败！");
							return false;
						}
						sswImg.m_ImgPosInfo.Xs=atof(vP[0]);sswImg.m_ImgPosInfo.phi=atof(vP[3]);
						sswImg.m_ImgPosInfo.Ys=atof(vP[1]);sswImg.m_ImgPosInfo.omg=atof(vP[4]);
						sswImg.m_ImgPosInfo.Zs=atof(vP[2]);sswImg.m_ImgPosInfo.kap=atof(vP[5]);
						vector<CString>().swap(vP);
					}else
						if(ChildLabel=="StripInfo")
						{
							vector<CString>vS=FunStrTok(Child.text().as_string(),"_");
							if(vS.size()!=2)
							{
								FunOutPutLogInfo("影像航带信息解析错误，工程文件打开失败！");
								return false;
							}
							sswImg.m_nStripIdx=atoi(vS[0]);
							sswImg.m_nIdxInStrip=atoi(vS[1]);
							vector<CString>().swap(vS);
						}else
							if(ChildLabel=="Valid")
							{
								CString strValue = Child.text().as_string();
								if(strValue == "false") sswImg.m_bValid=false;
								else sswImg.m_bValid=true;
							}else
								if(ChildLabel=="Adjustment")
								{
									CString strValue = Child.text().as_string();
									if(strValue == "false") sswImg.m_bAdjed=false;
									else sswImg.m_bAdjed=true;
								}

		}
		m_vecImgInfo.push_back(sswImg);
		if(!sswImg.m_bValid) m_vecImgIdxInvalid.push_back(m_vecImgInfo.size()-1);
	}
	if(m_nImgNum!=m_vecImgInfo.size())
	{
		FunOutPutLogInfo("影像数目错误，工程打开失败！");
		ClearPrjData();
		return false;
	}
	if(m_nStripNum>0)
	{
		m_vvImgIdxInEachStrip.resize(m_nStripNum);	
		m_vecStripIdx.resize(m_nStripNum);
		m_nMaxStripIdx=m_nStripNum;
		for (int i = 0; i<m_nStripNum; i++)
		{
			if(m_vecImgNumInEachStrip[i]<=0)
			{
				FunOutPutLogInfo("影像航带信息错误，工程打开失败！");
				ClearPrjData();
				return false;
			}
			m_vvImgIdxInEachStrip[i].resize(m_vecImgNumInEachStrip[i]);
			m_vecStripIdx[i]=i;
		}

//#pragma omp parallel for
		for (int i = 0; i<m_vecImgInfo.size(); i++)
		{
			if(!m_vecImgInfo[i].m_bValid||m_vecImgInfo[i].m_nStripIdx==InvalidValue) continue;
			int nStriIdx = m_vecImgInfo[i].m_nStripIdx;
			if(nStriIdx>m_vvImgIdxInEachStrip.size()) 
			{
				FunOutPutLogInfo("影像航带信息错误，工程打开失败！");
				ClearPrjData();
				return false;
			}
			int nIdxInStrip= m_vecImgInfo[i].m_nIdxInStrip;
			m_vvImgIdxInEachStrip[nStriIdx][nIdxInStrip]=m_vecImgInfo[i].m_nImgIdx;
		}
	}
	return true;
}
void CsswUAVFlyQuaPrj::SetPara2Prj(SSWstuSysConfigPara para)
{
	m_FlyQuaParaSet = para;
}

void CsswUAVFlyQuaPrj::CalBundleImgList2ImgInfo()
{
	m_vecBundleImgList2ImgInfo.resize(m_vecBundleImgName.size());
#pragma omp parallel for
	for (int i = 0; i<m_vecBundleImgName.size(); i++)
	{
		for(int j = 0; j<m_vecImgInfo.size(); j++)
		{
			if(m_vecBundleImgName[i]==m_vecImgInfo[j].m_strThumbName)
			{
				m_vecBundleImgList2ImgInfo[i] = j;
			}
		}
	}
}


bool CsswUAVFlyQuaPrj::SavePrj2File(CString strSavePath)
{
	if(strSavePath.IsEmpty()) return false;
	m_strPrjXmlPath=strSavePath;
	m_strImpXmlPath=FunGetFileFolder(m_strPrjXmlPath)+"\\"+FunGetFileName(m_strPrjXmlPath,false)+"_imp.xml";	
	m_strThumbFolder = FunGetFileFolder(m_strPrjXmlPath)+"\\CameraDistortionCorrect";
	m_strMatchAndAdjFolder = FunGetFileFolder(m_strPrjXmlPath)+"\\MatchAndAdjustment";
	m_strImgListPath = m_strMatchAndAdjFolder+"\\MatchImgList.txt";
	m_strPosListPath = m_strMatchAndAdjFolder+"\\MatchPosList.txt";
	m_strMatchMatrixPath = m_strMatchAndAdjFolder+"\\MatchMatrix.txt";
	m_strBundleAdjRepPath= m_strMatchAndAdjFolder+"\\BundleAdjustment\\Freenet_SBA.zgout";
	if(!SavePrjXml()) return false;
	if(!SaveImpXml()) return false;

	return true;
}
bool CsswUAVFlyQuaPrj::SavePrj2File4DYDll(CString strSavePath)
{
	pugi::xml_document ImpXmlDoc; char strValue[MAX_SIZE_FILE_LINE];
	pugi::xml_node decl = ImpXmlDoc.prepend_child(pugi::node_declaration);
	decl.append_attribute("version") = "1.0";
	decl.append_attribute("encoding") = "gb2312";
	pugi::xml_node RootNode = ImpXmlDoc.append_child("Import_data");
	pugi::xml_node Node, Child;
	//ProjectData
	Node = RootNode.append_child("ProjectData");
	Child = Node.append_child("BundleImgListFile");
	Child.append_child(pugi::node_pcdata).set_value(m_strImgListPath);
	Child = Node.append_child("BundleReportFile");
	Child.append_child(pugi::node_pcdata).set_value(m_strBundleAdjRepPath);

	Child = Node.append_child("ImageNumber"); sprintf(strValue,"%d",m_nImgNum);
	Child.append_child(pugi::node_pcdata).set_value(strValue);
	Child = Node.append_child("InvalidNumber"); sprintf(strValue,"%d",m_nInvalidNum);
	Child.append_child(pugi::node_pcdata).set_value(strValue);
	Child = Node.append_child("StripNumber"); sprintf(strValue,"%d",m_nStripNum);
	Child.append_child(pugi::node_pcdata).set_value(strValue);
	for (int i = 0; i<m_nStripNum; i++)
	{
		sprintf(strValue,"Strip_%d", i);
		Child = Node.append_child(strValue);
		sprintf(strValue,"%d", m_vecImgNumInEachStrip[i]);
		Child.append_child(pugi::node_pcdata).set_value(strValue);
	}
	//CameraParas
	Node = RootNode.append_child("CameraParas");
	Child = Node.append_child("WithFile");
	if(m_bCmrFile)
		Child.append_child(pugi::node_pcdata).set_value("1");
	else 
		Child.append_child(pugi::node_pcdata).set_value("0");
	Child = Node.append_child("Label");
	Child.append_child(pugi::node_pcdata).set_value(m_vecImgInfo[0].m_ThumbCmrInfo.strLabel);
	Child = Node.append_child("Format_x0y0");
	Child.append_child(pugi::node_pcdata).set_value(sswUnitType2String(m_vecImgInfo[0].m_ThumbCmrInfo.eUnitx0y0));
	Child = Node.append_child("Format_f");
	Child.append_child(pugi::node_pcdata).set_value(sswUnitType2String(m_vecImgInfo[0].m_ThumbCmrInfo.eUnitf));
	Child = Node.append_child("Format_pixelsize");
	Child.append_child(pugi::node_pcdata).set_value(sswUnitType2String(m_vecImgInfo[0].m_ThumbCmrInfo.eUnitpixsize));
	Child = Node.append_child("x0");
	sprintf(strValue,"%.6lf",m_vecImgInfo[0].m_ThumbCmrInfo.x0);
	Child.append_child(pugi::node_pcdata).set_value(strValue);
	Child = Node.append_child("y0");
	sprintf(strValue,"%.6lf",m_vecImgInfo[0].m_ThumbCmrInfo.y0);
	Child.append_child(pugi::node_pcdata).set_value(strValue);
	Child = Node.append_child("f");
	sprintf(strValue,"%.6lf %.6lf %.6lf",m_vecImgInfo[0].m_ThumbCmrInfo.f,m_vecImgInfo[0].m_ThumbCmrInfo.fx,m_vecImgInfo[0].m_ThumbCmrInfo.fy);
	Child.append_child(pugi::node_pcdata).set_value(strValue);
	Child = Node.append_child("pixelsize");
	sprintf(strValue,"%.6lf",m_vecImgInfo[0].m_ThumbCmrInfo.pixsize);
	Child.append_child(pugi::node_pcdata).set_value(strValue);
	Child = Node.append_child("distortion_paras");
	sprintf(strValue,"%.8lf %.8lf %.8lf %.8lf %.8lf %.8lf",m_vecImgInfo[0].m_ThumbCmrInfo.k1,m_vecImgInfo[0].m_ThumbCmrInfo.k2,
		m_vecImgInfo[0].m_ThumbCmrInfo.p1,m_vecImgInfo[0].m_ThumbCmrInfo.p2,m_vecImgInfo[0].m_ThumbCmrInfo.a,m_vecImgInfo[0].m_ThumbCmrInfo.b);
	Child.append_child(pugi::node_pcdata).set_value(strValue);
	//PosParas
	Node = RootNode.append_child("PosParas");
	Child = Node.append_child("Format_POS");
	Child.append_child(pugi::node_pcdata).set_value(sswCoordinateFormat2String(m_vecImgInfo[0].m_ImgPosInfo.eFormatCoordinate));
	Child = Node.append_child("Format_POK");
	Child.append_child(pugi::node_pcdata).set_value(sswAngleFormat2String(m_vecImgInfo[0].m_ImgPosInfo.eFormatAnglePOK));
	Child = Node.append_child("Format_LBH");
	Child.append_child(pugi::node_pcdata).set_value(sswAngleFormat2String(m_vecImgInfo[0].m_ImgPosInfo.eFormatAngleLBH));
	//ImageData
	Node = RootNode.append_child("ImageData");
	for (int i = 0; i<m_nImgNum; i++)
	{
		pugi::xml_node ImgChild = Node.append_child("Image");
		Child = ImgChild.append_child("ID");
		sprintf(strValue,"%d",i);
		Child.append_child(pugi::node_pcdata).set_value(strValue);
		Child = ImgChild.append_child("Path");
		Child.append_child(pugi::node_pcdata).set_value(m_vecImgInfo[i].m_strThumbPath);
		Child = ImgChild.append_child("Pos");
		sprintf(strValue,"%.6lf %.6lf %.6lf %.6lf %.6lf %.6lf", m_vecImgInfo[i].m_ImgPosInfo.Xs,m_vecImgInfo[i].m_ImgPosInfo.Ys,m_vecImgInfo[i].m_ImgPosInfo.Zs,
			m_vecImgInfo[i].m_ImgPosInfo.phi,m_vecImgInfo[i].m_ImgPosInfo.omg,m_vecImgInfo[i].m_ImgPosInfo.kap);
		Child.append_child(pugi::node_pcdata).set_value(strValue);		
		Child = ImgChild.append_child("StripInfo");
		sprintf(strValue,"%d_%d", m_vecImgInfo[i].m_nStripIdx, m_vecImgInfo[i].m_nIdxInStrip);
		Child.append_child(pugi::node_pcdata).set_value(strValue);
		Child = ImgChild.append_child("Valid");
		if(m_vecImgInfo[i].m_bValid) Child.append_child(pugi::node_pcdata).set_value("true");
		else Child.append_child(pugi::node_pcdata).set_value("false");
		Child = ImgChild.append_child("Adjustment");
		if(m_vecImgInfo[i].m_bAdjed) Child.append_child(pugi::node_pcdata).set_value("true");
		else Child.append_child(pugi::node_pcdata).set_value("false");
	}
	//FlyQuaParas
	ImpXmlDoc.save_file(strSavePath);
	return true;
}
bool CsswUAVFlyQuaPrj::SavePrjXml()
{
	if(m_nImgNum==0)
	{
		FunOutPutLogInfo("影像数为0，工程保存失败！");
		return false;
	}
	pugi::xml_document PrjXmlDoc; char strValue[MAX_SIZE_FILE_LINE];

	//	PrjXmlDoc.load("<foo bar='baz'><call>hey</call></foo>");

	// add a custom declaration node
	pugi::xml_node decl = PrjXmlDoc.prepend_child(pugi::node_declaration);
	decl.append_attribute("version") = "1.0";
	decl.append_attribute("encoding") = "gb2312";
	/*	decl.append_attribute("standalone") = "no";*/
	pugi::xml_node RootNode = PrjXmlDoc.append_child("UAV_Flight_Quality_Check");
	pugi::xml_node Node = RootNode.append_child("Project");
	pugi::xml_node Child = Node.append_child("AutoManageImages");
	if(m_bAutoManageImages) 
		sprintf(strValue,"%d",1);
	else 
		sprintf(strValue,"%d",0);
	Child.append_child(pugi::node_pcdata).set_value(strValue);
	
	pugi::xml_node ImgNode = Node.append_child("ImageData");
	Child = ImgNode.append_child("ImageNumber");
	sprintf(strValue,"%d",m_nImgNum);
	Child.append_child(pugi::node_pcdata).set_value(strValue);
	Child = ImgNode.append_child("InvalidNumber");
	sprintf(strValue,"%d",m_nInvalidNum);
	Child.append_child(pugi::node_pcdata).set_value(strValue);
	Child = ImgNode.append_child("StripNumber");
	sprintf(strValue,"%d",m_nStripNum);
	Child.append_child(pugi::node_pcdata).set_value(strValue);
	for (int i = 0; i<m_nStripNum; i++)
	{
		sprintf(strValue,"Strip_%d",i);
		Child = ImgNode.append_child(strValue);
		sprintf(strValue,"%d",m_vecImgNumInEachStrip[i]);
		Child.append_child(pugi::node_pcdata).set_value(strValue);
	}
	pugi::xml_node CmrNode = Node.append_child("CameraParas");
	if(m_nImgNum>0)
	{
		Child = CmrNode.append_child("WithFile");
		if(m_bCmrFile)
			Child.append_child(pugi::node_pcdata).set_value("1");
		else 
			Child.append_child(pugi::node_pcdata).set_value("0");
		Child = CmrNode.append_child("Label");
		Child.append_child(pugi::node_pcdata).set_value(m_vecImgInfo[0].m_ImgCmrInfo.strLabel);
		Child = CmrNode.append_child("Format_x0y0");
		Child.append_child(pugi::node_pcdata).set_value(sswUnitType2String(m_vecImgInfo[0].m_ImgCmrInfo.eUnitx0y0));
		Child = CmrNode.append_child("Format_f");
		Child.append_child(pugi::node_pcdata).set_value(sswUnitType2String(m_vecImgInfo[0].m_ImgCmrInfo.eUnitf));
		Child = CmrNode.append_child("Format_pixelsize");
		Child.append_child(pugi::node_pcdata).set_value(sswUnitType2String(m_vecImgInfo[0].m_ImgCmrInfo.eUnitpixsize));
		Child = CmrNode.append_child("x0");
		sprintf(strValue,"%.6lf",m_vecImgInfo[0].m_ImgCmrInfo.x0);
		Child.append_child(pugi::node_pcdata).set_value(strValue);
		Child = CmrNode.append_child("y0");
		sprintf(strValue,"%.6lf",m_vecImgInfo[0].m_ImgCmrInfo.y0);
		Child.append_child(pugi::node_pcdata).set_value(strValue);
		Child = CmrNode.append_child("f");
		sprintf(strValue,"%.6lf %.6lf %.6lf",m_vecImgInfo[0].m_ImgCmrInfo.f,m_vecImgInfo[0].m_ImgCmrInfo.fx,m_vecImgInfo[0].m_ImgCmrInfo.fy);
		Child.append_child(pugi::node_pcdata).set_value(strValue);
		Child = CmrNode.append_child("pixelsize");
		sprintf(strValue,"%.6lf",m_vecImgInfo[0].m_ImgCmrInfo.pixsize);
		Child.append_child(pugi::node_pcdata).set_value(strValue);
		Child = CmrNode.append_child("distortion_paras");
		sprintf(strValue,"%.8lf %.8lf %.8lf %.8lf %.8lf %.8lf",m_vecImgInfo[0].m_ImgCmrInfo.k1,m_vecImgInfo[0].m_ImgCmrInfo.k2,
			m_vecImgInfo[0].m_ImgCmrInfo.p1,m_vecImgInfo[0].m_ImgCmrInfo.p2,m_vecImgInfo[0].m_ImgCmrInfo.a,m_vecImgInfo[0].m_ImgCmrInfo.b);
		Child.append_child(pugi::node_pcdata).set_value(strValue);
	}
	pugi::xml_node PosNode = Node.append_child("PosParas");
	if(m_nImgNum>0)
	{
		Child = PosNode.append_child("Format_POS");
		Child.append_child(pugi::node_pcdata).set_value(sswCoordinateFormat2String(m_vecImgInfo[0].m_ImgPosInfo.eFormatCoordinate));
		Child = PosNode.append_child("Format_POK");
		Child.append_child(pugi::node_pcdata).set_value(sswAngleFormat2String(m_vecImgInfo[0].m_ImgPosInfo.eFormatAnglePOK));
		Child = PosNode.append_child("Format_LBH");
		Child.append_child(pugi::node_pcdata).set_value(sswAngleFormat2String(m_vecImgInfo[0].m_ImgPosInfo.eFormatAngleLBH));
	}
	Node = RootNode.append_child("ImportData");
	Node.prepend_child(pugi::node_pcdata).set_value(FunGetFileName(m_strPrjXmlPath,false)+"_imp.xml");
	Node = RootNode.append_child("FlyQuaPara");
	sprintf(strValue,"%.2lf%%",m_FlyQuaParaSet.fMinOverlapY);
	Child = Node.append_child("MinOverlapY");
	Child.prepend_child(pugi::node_pcdata).set_value(strValue);
	sprintf(strValue,"%.2lf%%",m_FlyQuaParaSet.fMinOverlapX);
	Child = Node.append_child("MinOverlapX");
	Child.prepend_child(pugi::node_pcdata).set_value(strValue);
	sprintf(strValue,"%.2lf",m_FlyQuaParaSet.fMaxHeiDifInArea);
	Child = Node.append_child("MaxHeiDifInArea");
	Child.prepend_child(pugi::node_pcdata).set_value(strValue);
	sprintf(strValue,"%.2lf",m_FlyQuaParaSet.fMaxHeiDifLinkInStrip);
	Child = Node.append_child("MaxHeiDifLinkInStrip");
	Child.prepend_child(pugi::node_pcdata).set_value(strValue);
	sprintf(strValue,"%.2lf%%",m_FlyQuaParaSet.fMaxStripBlend);
	Child = Node.append_child("MaxStripBlend");
	Child.prepend_child(pugi::node_pcdata).set_value(strValue);
	sprintf(strValue,"%.2lf",m_FlyQuaParaSet.fMaxPitch);
	Child = Node.append_child("MaxPitch");
	Child.prepend_child(pugi::node_pcdata).set_value(strValue);
	sprintf(strValue,"%.2lf%%",m_FlyQuaParaSet.fMaxPitch10Per);
	Child = Node.append_child("MaxPitch10Per");
	Child.prepend_child(pugi::node_pcdata).set_value(strValue);
	sprintf(strValue,"%.2lf",m_FlyQuaParaSet.fMaxYaw);
	Child = Node.append_child("MaxYaw");
	Child.prepend_child(pugi::node_pcdata).set_value(strValue);
	sprintf(strValue,"%.2lf%%",m_FlyQuaParaSet.fMaxYaw15Per);
	Child = Node.append_child("MaxYaw15Per");
	Child.prepend_child(pugi::node_pcdata).set_value(strValue);
	sprintf(strValue,"%d",m_FlyQuaParaSet.nMaxYaw20Num);
	Child = Node.append_child("MaxYaw20Num");
	Child.prepend_child(pugi::node_pcdata).set_value(strValue);
	sprintf(strValue,"%d",m_FlyQuaParaSet.nMatchPtNumThresold);
	Child = Node.append_child("nMatchPtNumThresold");
	Child.prepend_child(pugi::node_pcdata).set_value(strValue);
	PrjXmlDoc.save_file(m_strPrjXmlPath);
	return true;
}
bool CsswUAVFlyQuaPrj::SaveImpXml()
{
	pugi::xml_document ImpXmlDoc; char strValue[MAX_SIZE_FILE_LINE];
	pugi::xml_node decl = ImpXmlDoc.prepend_child(pugi::node_declaration);
	decl.append_attribute("version") = "1.0";
	decl.append_attribute("encoding") = "gb2312";
	pugi::xml_node RootNode = ImpXmlDoc.append_child("Import_data");
	pugi::xml_node Node, Child;
	for (int i = 0; i<m_nImgNum; i++)
	{
		Node = RootNode.append_child("Image");
		Child = Node.append_child("ID");
		sprintf(strValue,"%d",i);
		Child.append_child(pugi::node_pcdata).set_value(strValue);
		Child = Node.append_child("Path");
		Child.append_child(pugi::node_pcdata).set_value(m_vecImgInfo[i].m_strImgPath);
		Child = Node.append_child("Pos");
		sprintf(strValue,"%.6lf %.6lf %.6lf %.6lf %.6lf %.6lf", m_vecImgInfo[i].m_ImgPosInfo.Xs,m_vecImgInfo[i].m_ImgPosInfo.Ys,m_vecImgInfo[i].m_ImgPosInfo.Zs,
			m_vecImgInfo[i].m_ImgPosInfo.phi,m_vecImgInfo[i].m_ImgPosInfo.omg,m_vecImgInfo[i].m_ImgPosInfo.kap);
		Child.append_child(pugi::node_pcdata).set_value(strValue);		
		Child = Node.append_child("StripInfo");
		sprintf(strValue,"%d_%d", m_vecImgInfo[i].m_nStripIdx, m_vecImgInfo[i].m_nIdxInStrip);
		Child.append_child(pugi::node_pcdata).set_value(strValue);
		Child = Node.append_child("Valid");
		if(m_vecImgInfo[i].m_bValid) Child.append_child(pugi::node_pcdata).set_value("true");
		else Child.append_child(pugi::node_pcdata).set_value("false");
		Child = Node.append_child("Adjustment");
		if(m_vecImgInfo[i].m_bAdjed) Child.append_child(pugi::node_pcdata).set_value("true");
		else Child.append_child(pugi::node_pcdata).set_value("false");

	}
	ImpXmlDoc.save_file((FunGetFileFolder(m_strPrjXmlPath)+"\\"+FunGetFileName(m_strPrjXmlPath,false)+"_imp.xml"));
	return true;
}
bool CsswUAVFlyQuaPrj::SaveImgList()
{
	FILE* pfW = fopen(m_strImgListPath,"w");
	if(pfW==NULL)
	{
		FunOutPutLogInfo("影像列表文件保存错误，空三失败！");
		return false;
	}
	fprintf(pfW,"        \n");
	for (int i = 0; i<m_nImgNum; i++)
	{
		if(!m_vecImgInfo[i].m_bValid) continue;
		m_vecImgInfo[i].OpenImg(m_vecImgInfo[i].m_strThumbPath);
		int nCols=m_vecImgInfo[i].GetCols();
		int nRows=m_vecImgInfo[i].GetRows();
		double f = m_vecImgInfo[i].m_ThumbCmrInfo.f;
		fprintf(pfW,"%s %d %d %.6lf\n",FunGetFileName(m_vecImgInfo[i].m_strThumbPath,true).MakeLower(),nCols,nRows,f);
		m_vecImgInfo[i].CloseImg();
	}
	fseek(pfW,0,SEEK_SET);
	fprintf(pfW,"%d",m_nImgNum-m_nInvalidNum);
	fclose(pfW); pfW=NULL;

	return true;
}
bool CsswUAVFlyQuaPrj::SavePosList()
{
	FILE* pfW = fopen(m_strPosListPath,"w");
	if(pfW==NULL)
	{
		FunOutPutLogInfo("POS列表文件保存错误，空三失败！");
		return false;
	}
	for (int i = 0; i<m_nImgNum; i++)
	{
		if(!m_vecImgInfo[i].m_bValid) continue;
		fprintf(pfW,"%s %.6lf %.6lf %.6lf %.6lf %.6lf %.6lf\n",FunGetFileName(m_vecImgInfo[i].m_strThumbPath,false).MakeLower(),
			m_vecImgInfo[i].m_ImgPosInfo.Xs,m_vecImgInfo[i].m_ImgPosInfo.Ys,m_vecImgInfo[i].m_ImgPosInfo.Zs,
			m_vecImgInfo[i].m_ImgPosInfo.phi,m_vecImgInfo[i].m_ImgPosInfo.omg,m_vecImgInfo[i].m_ImgPosInfo.kap);
	}
	fclose(pfW); pfW=NULL;
	return true;
}
bool CsswUAVFlyQuaPrj::SaveMatchMatrix()
{
	CalMatchMatrix(m_nImgNum-m_nInvalidNum,3,15,SSW_POS_DIS);
	CalMatchMatrix(m_nImgNum-m_nInvalidNum,3,15,SSW_AREA_LINK);
	FILE* pfW = fopen(m_strMatchMatrixPath,"w");
	if(pfW==NULL||m_pMatchMatrix==NULL)
	{
		FunOutPutLogInfo("匹配列表列表文件保存错误，空三失败！");
		return false;
	}
	for (int i = 0,iValid=0; i<m_nImgNum; i++)
	{
		if(!m_vecImgInfo[i].m_bValid) continue;
		for (int j = 0, jValid=0; j<m_nImgNum; j++)
		{
			if(!m_vecImgInfo[j].m_bValid) continue;
			int nIdx = iValid*(m_nImgNum-m_nInvalidNum)+jValid;
			fprintf(pfW, "%d ", *(m_pMatchMatrix+nIdx));
			jValid++;

		}
		iValid++;
		fprintf(pfW, "\n");
	}
	fclose(pfW); pfW=NULL;
	return true;
}

void CsswUAVFlyQuaPrj::MatchMatrixByPosDis(int nMatCount, int nDim /* = 3 */, int nNearst /* = 10 */)
{
	if(m_pMatchMatrix) delete[]m_pMatchMatrix;
	m_pMatchMatrix = new int[nMatCount*nMatCount];
	memset(m_pMatchMatrix,0,nMatCount*nMatCount*sizeof(int));
	double *pPosXYZ=new double[nMatCount*3];
	int posnn=0;
	for (int i = 0; i<m_nImgNum; i++)
	{
		if(!m_vecImgInfo[i].m_bValid) continue;
		*(pPosXYZ+3*posnn) = m_vecImgInfo[i].m_ImgPosInfo.Xs;
		*(pPosXYZ+3*posnn+1) = m_vecImgInfo[i].m_ImgPosInfo.Ys;
		*(pPosXYZ+3*posnn+2) = m_vecImgInfo[i].m_ImgPosInfo.Zs;
		posnn++;
	}
	double* posdata=pPosXYZ;
	if (posdata==NULL)	return;
	nDim = min(nDim,3);
#pragma omp parallel for
	for (int i=0;i<nMatCount;i++)
	{
		std::vector<int> index(nMatCount,0);
		std::vector<double> dists(nMatCount,0);
		int iHasSearchCount=0;

		for (int j=0;j<nMatCount;j++)
		{
			if (i==j)
				continue;
			double dist = 0;
			for(int k=0;k<nDim;k++)
				dist += (*(posdata+i*nDim+k)-*(posdata+j*nDim+k))*(*(posdata+i*nDim+k)-*(posdata+j*nDim+k));
			dist = sqrt(dist);
			dists[j] = dist;
		}

		double dMinDist=99999999.0;
		int iMinID = -1;
		do 
		{
			dMinDist = 99999999.0;
			for (int j=0;j<nMatCount;j++)
			{
				if (i==j)
					continue;
				if (index[j]==1)
					continue;
				if (dMinDist>dists[j])
				{
					dMinDist = dists[j];
					iMinID = j;
				}
			}
			index[iMinID] = 1;
			iHasSearchCount++;
		} while (iHasSearchCount<nNearst&&iHasSearchCount<nMatCount-1);

		for (int j=0;j<nMatCount;j++)
		{
			if (index[j]==1)
			{
				*(m_pMatchMatrix+i*nMatCount+j)=1;
			}
		}
		for (int j=-2;j<=2;j++)
		{
			if(j==0)
				continue;
			if (i+j<0||i+j>=nMatCount)
				continue;
			*(m_pMatchMatrix+i*nMatCount+i+j)=1;
		}
	}
	if(pPosXYZ){	delete[]pPosXYZ; pPosXYZ=NULL;}
}
void CsswUAVFlyQuaPrj::MatchMatrixByAreaLink(int nMatCount, int nDim /* = 3 */)
{
	// 	if(m_pMatchMatrix) delete[]m_pMatchMatrix;
	// 	m_pMatchMatrix = new int[nMatCount*nMatCount];
	// 	memset(m_pMatchMatrix,0,nMatCount*nMatCount*sizeof(int));
	vector<OGRPolygon*> vecImagePolygon(nMatCount);
	double pZ[4]={0,0,0,0};
	int nCount = 0;
	for (int i = 0; i<m_nImgNum; i++)
	{
		if(!m_vecImgInfo[i].m_bValid) continue;
		Point3D* pVerPt4 = m_vecImgInfo[i].GetImgVertexPt3d4(pZ,false,false);
		OGRPolygon *pPolygon1 = NULL;
		pPolygon1 = FunCreatePolygon(pVerPt4,3,4);
		vecImagePolygon[nCount]=pPolygon1;
		nCount++;
	}
	for (int i = 0; i<nMatCount - 1; i++)
	{
		if(vecImagePolygon[i]==NULL) continue;
		for (int j = i+1; j<nMatCount; j++)
		{
			if(vecImagePolygon[j]==NULL) continue;
			OGRGeometry *pInterGeo = vecImagePolygon[i]->Intersection(vecImagePolygon[j]);
			if(pInterGeo==NULL) continue;
			if(pInterGeo->getGeometryType()==wkbPolygon)
			{
				m_pMatchMatrix[i*nMatCount+j]=1;
				m_pMatchMatrix[j*nMatCount+i]=1;
			}
		}
	}
	for (int i = 0; i<nMatCount; i++)
	{
		OGRGeometryFactory::destroyGeometry(vecImagePolygon[i]);
	}
	vector<OGRPolygon*>().swap(vecImagePolygon);
}
void CsswUAVFlyQuaPrj::CalImagePitch()
{
	if(m_nImgNum<=0) return;
	m_vecImagePitch.assign(m_nImgNum,InvalidValue);
#pragma omp parallel for
	for(int i = 0; i<m_nImgNum; i++)
	{
		if(m_vecImgInfo.size()<=i) continue;
		m_vecImagePitch[i]=max(m_vecImgInfo[i].m_ImgPosInfo.phi,m_vecImgInfo[i].m_ImgPosInfo.omg);
	}
}
void CsswUAVFlyQuaPrj::CalImageYaw(double Z, SSWFLYQUAMETHOD eMethod)
{
	m_vvStripImageYaw.resize(m_nStripNum);
	//#pragma omp parallel for
	for (int i = 0; i<m_nStripNum; i++)
	{
		m_vvStripImageYaw[i].assign(m_vecImgNumInEachStrip[i],0);
		for (int j = 1; j<m_vecImgNumInEachStrip[i]; j++)
		{
			int nImgIdx1 = m_vvImgIdxInEachStrip[i][j-1];
			int nImgIdx2 = m_vvImgIdxInEachStrip[i][j];
			if(m_vecImgInfo.size()<nImgIdx1||m_vecImgInfo.size()<nImgIdx2) break;
			int nCount = 0; double dSum = 0;
			vector<double> vecYaw;
			switch(eMethod)
			{
			case BY_POS:{
				//Point3D pVertexPt3d[4]; 
				double pZ[4]; memset(pZ,Z,4*sizeof(double));
				Point3D *pVertexPt3d=	m_vecImgInfo[nImgIdx1].GetImgVertexPt3d4(pZ,false,false);
				Point2D *pVertexImg4 = m_vecImgInfo[nImgIdx1].GetImgVertexPt2d4(false);
				Point2D pVertexPt2d1[4],pVertexPt2d2[4];
				for (int ii = 0; ii<4; ii++)
				{
					pVertexPt2d1[ii]=m_vecImgInfo[nImgIdx1].XYZ2Img(pVertexImg4[2].x,pVertexImg4[1].y,m_vecImgInfo[nImgIdx1].m_ImgCmrInfo.f,pVertexPt3d[ii],false);
					pVertexPt2d2[ii]=m_vecImgInfo[nImgIdx2].XYZ2Img(pVertexImg4[2].x,pVertexImg4[1].y,m_vecImgInfo[nImgIdx2].m_ImgCmrInfo.f,pVertexPt3d[ii],false);
				}
				for (int ii = 0; ii<3; ii++)
				{
					for (int jj = ii+1; jj<4; jj++)
					{
						//					if(m_vecImgInfo.size()<=i) return;
						Point2D pt11 = pVertexPt2d1[ii];
						Point2D pt12 = pVertexPt2d1[jj];
						Point2D pt21 = pVertexPt2d2[ii];
						Point2D pt22 = pVertexPt2d2[jj];
						double a1 = atan((pt12.y-pt11.y)/(pt12.x-pt11.x));
						double a2 = atan((pt22.y-pt21.y)/(pt22.x-pt21.x));
						double da = RadToDeg(a1)-RadToDeg(a2);
						dSum+=da;
						nCount++;
					}
				}
						}
						break;
			case BY_MATCH:
				for (int k = 0; k<m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vecLinkImgIdx.size(); k++)
				{
					if(nImgIdx1==m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vecLinkImgIdx[k])
					{
						for (int p = 0; p<m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg[k].size()-1; p+=2)
						{
							//				if(m_vecImgInfo.size()<=k) return;
							int nAdjId1 = m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg[k][p];
							int nAdjId2 = m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg[k][p+1];
							int nImgPtIdx11 = m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vvImgPtIdxInAdjPt[k][p];
							int nImgPtIdx12 = m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vvImgPtIdxInAdjPt[k][p+1];
							int nImgPtIdx21 = m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vvLinkImgPtIdxInAdjPt[k][p];
							int nImgPtIdx22 = m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vvLinkImgPtIdxInAdjPt[k][p+1];
							Point2D pt11 = m_vecAdjPts[nAdjId1].vecImgPt[nImgPtIdx11];
							Point2D pt12 = m_vecAdjPts[nAdjId2].vecImgPt[nImgPtIdx12];
							Point2D pt21 = m_vecAdjPts[nAdjId1].vecImgPt[nImgPtIdx21];
							Point2D pt22 = m_vecAdjPts[nAdjId2].vecImgPt[nImgPtIdx22];
							// 							double a1 = atan((pt12.y-pt11.y)/(pt12.x-pt11.x));
							// 							double a2 = atan((pt22.y-pt21.y)/(pt22.x-pt21.x));
							double a1,a2;
							if(pt12.x!=pt11.x) a1 = atan((pt12.y-pt11.y)/(pt12.x-pt11.x));
							else a1 = 0;
							if(pt22.x!=pt21.x) a2 = atan((pt22.y-pt21.y)/(pt22.x-pt21.x));
							else a2 = 0;
							double da = RadToDeg(a1)-RadToDeg(a2);
							dSum+=da;
							vecYaw.push_back(da);
							nCount++;
						}
						break;
					}
				}
				break;
			case BY_ADJUSTMENT:
				for (int k = 0; k<m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vecLinkImgIdx.size(); k++)
				{
					if(nImgIdx1==m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vecLinkImgIdx[k])
					{
						for (int p = 0; p<m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg[k].size()-1; p+=2)
						{
							int nAdjId1 = m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg[k][p];
							int nAdjId2 = m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg[k][p+1];
							int nImgPtIdx11 = m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vvImgPtIdxInAdjPt[k][p];
							int nImgPtIdx12 = m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vvImgPtIdxInAdjPt[k][p+1];
							int nImgPtIdx21 = m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vvLinkImgPtIdxInAdjPt[k][p];
							int nImgPtIdx22 = m_vecImgInfo[nImgIdx2].m_ImgAdjPts.vvLinkImgPtIdxInAdjPt[k][p+1];
							Point2D pt11 = m_vecAdjPts[nAdjId1].vecImgPt[nImgPtIdx11];
							Point2D pt12 = m_vecAdjPts[nAdjId2].vecImgPt[nImgPtIdx12];
							Point2D pt21 = m_vecAdjPts[nAdjId1].vecImgPt[nImgPtIdx21];
							Point2D pt22 = m_vecAdjPts[nAdjId2].vecImgPt[nImgPtIdx22];
							double a1,a2;
							if(pt12.x!=pt11.x) a1 = atan((pt12.y-pt11.y)/(pt12.x-pt11.x));
							else a1 = 0;
							if(pt22.x!=pt21.x) a2 = atan((pt22.y-pt21.y)/(pt22.x-pt21.x));
							else a2 = 0;
							double da = RadToDeg(a1)-RadToDeg(a2);
							dSum+=da;
							nCount++;
						}
						break;
					}
				}
				break;
			default:
				break;
			}
			if(nCount>0) m_vvStripImageYaw[i][j]=dSum/nCount;
			vector<double>().swap(vecYaw);
		}
	}

}
void CsswUAVFlyQuaPrj::CalImageFlyHeiErr(SSWFLYQUAMETHOD eMethod)
{
	if(m_nImgNum<=0||m_nStripNum<=0) return;
	m_vvStripImageFlyHeiErr.resize(m_nStripNum);
	//	m_vecStripImageFlyHeiMaxErr.resize(m_nStripNum);

#pragma omp parallel for
	for (int i= 0; i<m_nStripNum; i++)
	{
		if(m_vvStripImageFlyHeiErr.size()<i)continue;
		m_vvStripImageFlyHeiErr[i].assign(m_vecImgNumInEachStrip[i],0);
		for (int j = 1; j<m_vecImgNumInEachStrip[i]; j++)
		{
			if(m_vvStripImageFlyHeiErr.size()<i)continue;
			if(m_vecImgNumInEachStrip.size()<i) continue;
			int nImgIdx1 = m_vvImgIdxInEachStrip[i][j-1];
			int nImgIdx2 = m_vvImgIdxInEachStrip[i][j];
			SSWstuPosInfo sswPosInfo1, sswPosInfo2;
			if (m_vecImgInfo.size()<nImgIdx1||m_vecImgInfo.size()<nImgIdx2) 
			{
				continue;
			}
			if(eMethod==BY_POS||eMethod==BY_MATCH)
			{
				sswPosInfo1=m_vecImgInfo[nImgIdx1].m_ImgPosInfo;
				sswPosInfo2=m_vecImgInfo[nImgIdx2].m_ImgPosInfo;
			}
			else
			{
				sswPosInfo1=m_vecImgInfo[nImgIdx1].m_AdjPosInfo;
				sswPosInfo2=m_vecImgInfo[nImgIdx2].m_AdjPosInfo;
			}
			m_vvStripImageFlyHeiErr[i][j]=sswPosInfo2.Zs-sswPosInfo1.Zs;

		}
		//	m_vecStripImageFlyHeiMaxErr[i]=dMaxHei-dMinHei;
	}
	double dMaxHei = -1e10, dMinHei = 1e10;
	for (int i = 0; i<m_nImgNum; i++)
	{
		if(m_vecImgInfo.size()<=i) break;
		if(!m_vecImgInfo[i].m_bValid) continue;
		SSWstuPosInfo sswPosInfo1;
		if(eMethod==BY_POS||eMethod==BY_MATCH)
		{
			sswPosInfo1=m_vecImgInfo[i].m_ImgPosInfo;
		}
		else
		{
			sswPosInfo1=m_vecImgInfo[i].m_AdjPosInfo;
		}
		dMaxHei=max(dMaxHei,sswPosInfo1.Zs);
		dMinHei=min(dMinHei,sswPosInfo1.Zs);
	}
	m_dMaxHeiErr = dMaxHei-dMinHei;
}
void CsswUAVFlyQuaPrj::CalImageFlyDisErr(SSWFLYQUAMETHOD eMethod)
{
	if(m_nImgNum<=0||m_nStripNum<=0) return;
	m_vvStripImageFlyDisErr.resize(m_nStripNum);
	m_vecStripBlendPara.assign(m_nStripNum,InvalidValue);
#pragma omp parallel for
	for (int i = 0; i<m_nStripNum; i++)
	{	
		if(m_vecImgInfo.size()<i) continue;
		double dMaxDis = InvalidValue;
		m_vvStripImageFlyDisErr[i].assign(m_vecImgNumInEachStrip[i],InvalidValue);
		double strip_a,strip_b,strip_c;
		vector<Point2D> vPt2d; vPt2d.resize(m_vecImgNumInEachStrip[i]);
		for (int j = 0; j<m_vecImgNumInEachStrip[i]; j++)
		{
			if(m_vecImgInfo.size()<=i) break;
			Point2D pt2d;
			int nImgIdx = m_vvImgIdxInEachStrip[i][j];
			SSWstuPosInfo sswPosInfo;
			if(eMethod==BY_POS||eMethod==BY_MATCH)
			{
				sswPosInfo=m_vecImgInfo[nImgIdx].m_ImgPosInfo;
			}
			else
			{
				sswPosInfo=m_vecImgInfo[nImgIdx].m_AdjPosInfo;
			}
			pt2d.x = sswPosInfo.Xs;
			pt2d.y = sswPosInfo.Ys;
			vPt2d[j]=pt2d;
		}
		FunCalLineParasByLsm(vPt2d,strip_a,strip_b,strip_c);
		double M = sqrt(pow(strip_a,2)+pow(strip_b,2)+pow(strip_c,2));
		for (int j = 0; j<m_vecImgNumInEachStrip[i]; j++)
		{
			double dis = fabs(strip_a*vPt2d[j].x+strip_b*vPt2d[j].y+strip_c)/M;
			m_vvStripImageFlyDisErr[i][j]=dis;
			dMaxDis = max(dMaxDis,dis);
		}
		double dStripLen = sqrt(pow(vPt2d[0].x-vPt2d[vPt2d.size()-1].x,2)+pow(vPt2d[0].y-vPt2d[vPt2d.size()-1].y,2));
		m_vecStripBlendPara[i]=dMaxDis/dStripLen;
	}
	cprintf("========Blend of strips========\n");
	for (int i = 0; i<m_vecStripBlendPara.size(); i++)
	{
		CString strInfo;
		strInfo.Format("Strip_%d：%.2lf%%\n",i,m_vecStripBlendPara[i]*100);
		cprintf(strInfo);
	}
}
bool CsswUAVFlyQuaPrj::CalImagePosErr(SSWFLYQUAMETHOD eMethod /* = BY_ADJUSTMENT */)
{
	if(eMethod!=BY_ADJUSTMENT)
	{
		m_vecAdjImgPosDisErr.assign(m_nImgNum-m_nInvalidNum,0);
		return false;
	}
	m_vecAdjImgPosErr.resize(m_nInAdjedNum);
	m_vecAdjImgPosDisErr.resize(m_nInAdjedNum);
#pragma omp parallel for
	for (int i = 0; i<m_nInAdjedNum; i++)
	{
		int nImgIdx = m_vecBundleSuccess2ImgInfo[i];
		if(!m_vecImgInfo[nImgIdx].m_bValid) continue;
		m_vecAdjImgPosErr[i] = m_vecImgInfo[nImgIdx].m_ImgPosInfo-m_vecImgInfo[nImgIdx].m_AdjPosInfo;
		m_vecAdjImgPosErr[i].strLabel=m_vecImgInfo[nImgIdx].m_ImgPosInfo.strLabel;
		m_vecAdjImgPosDisErr[i]=sqrt(pow(m_vecAdjImgPosErr[i].Xs,2)+pow(m_vecAdjImgPosErr[i].Ys,2)+pow(m_vecAdjImgPosErr[i].Zs,2));
	}
	return true;
}
void CsswUAVFlyQuaPrj::CalStripCqPaneErr(SSWFLYQUAMETHOD eMethod /* = BY_ADJUSTMENT */)
{
	//////////////////////////////////////////////////
	///待设计
	//m_vecStripPanePara;
	vector<Point3D> vecPt3d;
	double dSumX=0,dSumY=0,dSumZ=0;
	for (int i = 0; i<m_nImgNum; i++)
	{
		if(m_vecImgInfo.size()<i) return;
		if(!m_vecImgInfo[i].m_bValid) continue;
		Point3D pt3d;
		if(eMethod==BY_ADJUSTMENT&&m_vecImgInfo[i].m_bAdjed)
		{
			pt3d.X=m_vecImgInfo[i].m_AdjPosInfo.Xs;
			pt3d.Y=m_vecImgInfo[i].m_AdjPosInfo.Ys;
			pt3d.Z=m_vecImgInfo[i].m_AdjPosInfo.Zs;
		}
		else
		{
			pt3d.X=m_vecImgInfo[i].m_ImgPosInfo.Xs;
			pt3d.Y=m_vecImgInfo[i].m_ImgPosInfo.Ys;
			pt3d.Z=m_vecImgInfo[i].m_ImgPosInfo.Zs;
		}
		dSumX+=pt3d.X;
		dSumY+=pt3d.Y;
		dSumZ+=pt3d.Z;
		vecPt3d.push_back(pt3d);
	}
	if(vecPt3d.size()<3) return;
	for (int i = 0; i<vecPt3d.size(); i++)
	{
		vecPt3d[i].X-=dSumX/vecPt3d.size();
		vecPt3d[i].Y-=dSumY/vecPt3d.size();
		vecPt3d[i].Z-=dSumZ/vecPt3d.size();
	}
	double a,b,c,d;
	FunCalPaneParasBylsm(vecPt3d,a,b,c,d);
	/*vector<double> vecDis2Pane(vecPt3d.size(), 0);*/
	m_vecDisImgAdjPos2PosPane.assign(vecPt3d.size(),0);
	double dSum = 0, dMean = 0, dRms = 0;
	double dMax = -1e10; 
	double M = sqrt(pow(a,2)+pow(b,2)+pow(c,2)+pow(d,2));
	for (int i = 0; i<vecPt3d.size(); i++)
	{
		double dis = /*fabs*/(a*vecPt3d[i].X+b*vecPt3d[i].Y+c*vecPt3d[i].Z+c)/M;
		dSum+=fabs(dis);
		dMax=max(dMax,fabs(dis));
		m_vecDisImgAdjPos2PosPane[i]=dis;
	}
	if(vecPt3d.size()>0) dMean = dSum/vecPt3d.size();
	dRms = FunCalDataRms(m_vecDisImgAdjPos2PosPane,dMean);
	m_dMaxPaneDifErr = dMax;
	m_dRmsPaneDifErr = dRms;
	cprintf("========Pane info of area========\n");
	cprintf("MaxDis  = %.2lfm\n",dMax);
	cprintf("MeanDis = %.2lfm\n",dMean);
	cprintf("RmsDis  = %.2lfm\n",dRms);

	vector<Point3D>().swap(vecPt3d);
}


double CsswUAVFlyQuaPrj::CalOverlapsByPos(int nIdx1, int nIdx2, double Z, SSWOVERLAPDIRECTION eDirection)
{
	double dOverlap = 0;
	if(nIdx1>=m_nImgNum||nIdx2>=m_nImgNum||nIdx1<0||nIdx2<0) return dOverlap;
	if(!m_vecImgInfo[nIdx1].m_bValid||!m_vecImgInfo[nIdx2].m_bValid) return dOverlap;
	double pZ[4]; memset(pZ,Z,4*sizeof(double));
	Point3D* pVerPt4_1 = m_vecImgInfo[nIdx1].GetImgVertexPt3d4(pZ,false,true);
	Point3D* pVerPt4_2 = m_vecImgInfo[nIdx2].GetImgVertexPt3d4(pZ,false,true);
	OGRPolygon *pPolygon1 = NULL, *pPolygon2 = NULL;
	pPolygon1 = FunCreatePolygon(pVerPt4_1,3,4);
	if(pPolygon1==NULL) return dOverlap;
	pPolygon2=FunCreatePolygon(pVerPt4_2,3,4);
	if(pPolygon2==NULL)
	{
		OGRGeometryFactory::destroyGeometry(pPolygon1);
		return dOverlap;
	}
	Point2D *pInterVetexPt=NULL; int nPtNum = 0;
	pInterVetexPt=FunPolygonIntersect(pPolygon1,pPolygon2,nPtNum);
	if(pInterVetexPt==NULL) return dOverlap;
	//交集范围，img2 投到img1	
	m_vecImgInfo[nIdx1].OpenImg(m_vecImgInfo[nIdx1].m_strThumbPath);
	double dFullLenX = 0, dFullLenY=0;
	dFullLenX = m_vecImgInfo[nIdx1].GetCols();
	dFullLenY = m_vecImgInfo[nIdx1].GetRows();
	for (int i = 0; i<nPtNum; i++)
	{
		if(m_vecImgInfo.size()<=i) return 0;
		Point3D pt3d = Point3D(pInterVetexPt[i].x,pInterVetexPt[i].y,Z);
		pInterVetexPt[i] = m_vecImgInfo[nIdx1].XYZ2Img(dFullLenX,dFullLenY,m_vecImgInfo[nIdx1].m_ThumbCmrInfo.f,pt3d,false);
	}
	m_vecImgInfo[nIdx1].CloseImg();
	if(eDirection==DIRECTION_Y)
	{
		if(CalOverlapByInterVertex(pInterVetexPt,nPtNum,dFullLenX,DIRECTION_X)>0.6)
			dOverlap = CalOverlapByInterVertex(pInterVetexPt, nPtNum,dFullLenY,DIRECTION_Y);
	}
	else if(eDirection==DIRECTION_X)
	{
		if(CalOverlapByInterVertex(pInterVetexPt,nPtNum,dFullLenY,DIRECTION_Y)>0.3)
			dOverlap = CalOverlapByInterVertex(pInterVetexPt, nPtNum,dFullLenX,DIRECTION_X);
	}
	dOverlap = min(1,dOverlap);
	dOverlap = max(0,dOverlap);
	return dOverlap;
}
double CsswUAVFlyQuaPrj::CalOverlapsByMatch(int nIdx1, int nIdx2, SSWOVERLAPDIRECTION eDirection)
{
	double dOverlap = 0;
	if(nIdx1>=m_nImgNum||nIdx2>=m_nImgNum||nIdx1<0||nIdx2<0) return dOverlap;
	if(!m_vecImgInfo[nIdx1].m_bValid||!m_vecImgInfo[nIdx2].m_bValid) return dOverlap;
	vector<Point2D> vecPt2d1,vecPt2d2;
	for (int i = 0; i<m_vecImgInfo[nIdx1].m_ImgAdjPts.vecLinkImgIdx.size(); i++)
	{
		int nLinImgIdx = m_vecImgInfo[m_vecImgInfo[nIdx1].m_ImgAdjPts.vecLinkImgIdx[i]].m_nImgIdx;
		if(nLinImgIdx==nIdx2)
		{
			for (int j = 0; j<m_vecImgInfo[nIdx1].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg[i].size(); j++)
			{	
				if(m_vecImgInfo.size()<=i) return 0;
				int nAdjIdx, nImgPtIdx1, nImgPtIdx2;
				nAdjIdx=m_vecImgInfo[nIdx1].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg[i][j];
				nImgPtIdx1 = m_vecImgInfo[nIdx1].m_ImgAdjPts.vvImgPtIdxInAdjPt[i][j];
				nImgPtIdx2 = m_vecImgInfo[nIdx1].m_ImgAdjPts.vvLinkImgPtIdxInAdjPt[i][j];
				vecPt2d1.push_back(m_vecAdjPts[nAdjIdx].vecImgPt[nImgPtIdx1]);
				vecPt2d2.push_back(m_vecAdjPts[nAdjIdx].vecImgPt[nImgPtIdx2]);
			}
			break;
		}
	}
	if(vecPt2d1.size()<4) 
	{
		//		return CalOverlapsByPos(nIdx1,nIdx2,m_dMeanHeiCq,eDirection);
		return dOverlap;
	}
	double *CoePara6 = new double[6]; memset(CoePara6,InvalidValue,6*sizeof(double));
	FunCalTopoParasByLsm(vecPt2d2,vecPt2d1,&CoePara6);
	Point2D pImgVertexPt2To1[4], *pTmp = new Point2D[vecPt2d1.size()];
	Point2D* pImgVertexPt2 = m_vecImgInfo[nIdx2].GetImgVertexPt2d4(true);
	for (int i = 0; i<4; i++)
	{
		FunImgPtTopoTrans(pImgVertexPt2[i],CoePara6,pImgVertexPt2To1[i]);
		//		FunImgPtTopoTrans(vecPt2d2[i],CoePara6,pTmp[i]);
	}
	double dFullLenX = 0, dFullLenY=0;
	m_vecImgInfo[nIdx1].OpenImg(m_vecImgInfo[nIdx1].m_strThumbPath);
	dFullLenX = m_vecImgInfo[nIdx1].GetCols();
	dFullLenY = m_vecImgInfo[nIdx1].GetRows();
	m_vecImgInfo[nIdx1].CloseImg();
	if(eDirection==DIRECTION_Y)
	{
		if(CalOverlapByInterVertex(pImgVertexPt2To1,4,dFullLenX,DIRECTION_X)>0.6)
			dOverlap = CalOverlapByInterVertex(pImgVertexPt2To1,4,dFullLenY,DIRECTION_Y);
	}
	else if(eDirection==DIRECTION_X)
	{
		if(CalOverlapByInterVertex(pImgVertexPt2To1,4,dFullLenY,DIRECTION_Y)>0.3)
			dOverlap = CalOverlapByInterVertex(pImgVertexPt2To1,4,dFullLenX,DIRECTION_X);
	}
	dOverlap = min(1,dOverlap);
	dOverlap = max(0,dOverlap);
	delete[]CoePara6;
	vector<Point2D>().swap(vecPt2d1);
	vector<Point2D>().swap(vecPt2d2);
	return dOverlap;
}
double CsswUAVFlyQuaPrj::CalOverlapsByAdjustment(int nIdx1, int nIdx2, SSWOVERLAPDIRECTION eDirection)
{
	double dOverlap = 0;
	if(nIdx1>=m_nImgNum||nIdx2>=m_nImgNum||nIdx1<0||nIdx2<0) return dOverlap;
	if(!m_vecImgInfo[nIdx1].m_bValid||!m_vecImgInfo[nIdx2].m_bValid) return dOverlap;
	//	Point3D pVerPt4_1[4],pVerPt4_2[4];

	double pZ[4]; memset(pZ,0,4*sizeof(double));
	Point2D *pImgVertexPt = m_vecImgInfo[nIdx1].GetImgVertexPt2d4(true);
	for (int i = 0; i<4; i++)
	{
		FunFitHei4ImgAdjPts4ImgPt(m_vecAdjPts,m_dMeanHeiCq,m_vecImgInfo[nIdx1].m_ImgAdjPts,pImgVertexPt[i],pZ[i]);
	}
	//m_vecImgInfo[nIdx1].CalImg4VertexPt3d(pZ,true,pVerPt4_1,true);
	Point3D* pVerPt4_1 = m_vecImgInfo[nIdx1].GetImgVertexPt3d4(pZ,true,true);
	pImgVertexPt = m_vecImgInfo[nIdx2].GetImgVertexPt2d4(true);
	for (int i = 0; i<4; i++)
	{
		FunFitHei4ImgAdjPts4ImgPt(m_vecAdjPts,m_dMeanHeiCq,m_vecImgInfo[nIdx2].m_ImgAdjPts,pImgVertexPt[i],pZ[i]);
	}
	//	m_vecImgInfo[nIdx2].CalImg4VertexPt3d(pZ,true,pVerPt4_2,true);
	Point3D* pVerPt4_2 = m_vecImgInfo[nIdx1].GetImgVertexPt3d4(pZ,true,true);
	OGRPolygon *pPolygon1 = NULL, *pPolygon2 = NULL;
	pPolygon1 = FunCreatePolygon(pVerPt4_1,3,4);
	if(pPolygon1==NULL) return dOverlap;
	pPolygon2=FunCreatePolygon(pVerPt4_2,3,4);
	if(pPolygon2==NULL)
	{
		OGRGeometryFactory::destroyGeometry(pPolygon1);
		return dOverlap;
	}
	Point2D *pInterVetexPt=NULL; int nPtNum = 0;
	pInterVetexPt=FunPolygonIntersect(pPolygon1,pPolygon2,nPtNum);
	if(pInterVetexPt==NULL) return dOverlap;
	//交集范围，img2 投到img1、
	double dFullLenX = 0, dFullLenY=0;
	m_vecImgInfo[nIdx1].OpenImg(m_vecImgInfo[nIdx1].m_strThumbPath);
	dFullLenX = m_vecImgInfo[nIdx1].GetCols();
	dFullLenY = m_vecImgInfo[nIdx1].GetRows();
	for (int i = 0; i<nPtNum; i++)
	{
		double Z = 0;
		FunFitHei4ImgAdjPts4ObjPt(m_vecAdjPts,m_dMeanHeiCq,m_vecImgInfo[nIdx1].m_ImgAdjPts,pInterVetexPt[i],Z);
		Point3D pt3d = Point3D(pInterVetexPt[i].x,pInterVetexPt[i].y,Z);
		pInterVetexPt[i] = m_vecImgInfo[nIdx1].XYZ2Img(dFullLenX,dFullLenY,m_vecImgInfo[nIdx1].m_ThumbCmrInfo.f,pt3d,true);
	}
	m_vecImgInfo[nIdx1].CloseImg();

	if(eDirection==DIRECTION_Y)
	{
		if(CalOverlapByInterVertex(pInterVetexPt,nPtNum,dFullLenX,DIRECTION_X)>0.6)
			dOverlap = CalOverlapByInterVertex(pInterVetexPt, nPtNum,dFullLenY,DIRECTION_Y);
	}
	else if(eDirection==DIRECTION_X)
	{
		if(CalOverlapByInterVertex(pInterVetexPt,nPtNum,dFullLenY,DIRECTION_Y)>0.3)
			dOverlap = CalOverlapByInterVertex(pInterVetexPt, nPtNum,dFullLenX,DIRECTION_X);
	}
	dOverlap = min(1,dOverlap);
	dOverlap = max(0,dOverlap);
	return dOverlap;
}
double CsswUAVFlyQuaPrj::CalOverlapByInterVertex(Point2D *pVertexPts, int nPtNum, double dFullLen, SSWOVERLAPDIRECTION eDirection)
{
	double dOverlap = 0;
	if(nPtNum<3) return dOverlap;
	if(dFullLen<=0) return dOverlap;
	switch(eDirection)
	{
	case DIRECTION_X:{
		for (int i = 0; i<nPtNum-1; i++)
		{
			for (int j = i; j<nPtNum; j++)
			{
				if(pVertexPts[i].x>pVertexPts[j].x)	
				{
					Point2D ptTmp = pVertexPts[i];
					pVertexPts[i]=pVertexPts[j];
					pVertexPts[j]=ptTmp;
				}
			}
		}
		double dMaxX1, dMaxX2, dMinX1, dMinX2;
		if(nPtNum >= 4)
		{
			dMaxX1 = min(dFullLen,pVertexPts[nPtNum-1].x);
			dMaxX2 = min(dFullLen,pVertexPts[nPtNum-2].x);
			dMinX1 = max(0,pVertexPts[0].x);
			dMinX2 = max(0,pVertexPts[1].x);
		}
		else if(nPtNum == 3)
		{
			dMaxX1 = min(dFullLen,pVertexPts[2].x);
			dMaxX2 = min(dFullLen,pVertexPts[2].x);
			dMinX1 = max(0,pVertexPts[0].x);
			dMinX2 = max(0,pVertexPts[1].x);
		}
		dOverlap = (dMaxX1+dMaxX2-dMinX1-dMinX2)/dFullLen/2;
					 }
					 break;
	case DIRECTION_Y:{
		for (int i = 0; i<nPtNum-1; i++)
		{
			for (int j = i; j<nPtNum; j++)
			{
				if(pVertexPts[i].y>pVertexPts[j].y)	
				{
					Point2D ptTmp = pVertexPts[i];
					pVertexPts[i]=pVertexPts[j];
					pVertexPts[j]=ptTmp;
				}
			}
		}
		double dMaxY1, dMaxY2, dMinY1, dMinY2;
		if(nPtNum >= 4)
		{	
			dMaxY1 = min(dFullLen,pVertexPts[nPtNum-1].y);
			dMaxY2 = min(dFullLen,pVertexPts[nPtNum-2].y);
			dMinY1 = max(0,pVertexPts[0].y);
			dMinY2 = max(0,pVertexPts[1].y);

		}
		else if(nPtNum == 3)
		{
			dMaxY1 = min(dFullLen,pVertexPts[2].y);
			dMaxY2 = min(dFullLen,pVertexPts[2].y);
			dMinY1 = max(0,pVertexPts[0].y);
			dMinY2 = max(0,pVertexPts[1].y);
		}
		dOverlap = (dMaxY1+dMaxY2-dMinY1-dMinY2)/dFullLen/2;
					 }
					 break;
	default:
		break;
	}
	return dOverlap;
}
void CsswUAVFlyQuaPrj::AnalysisFlyQuaChkParas(int nMatchPtNumThresold)
{
	m_vecImgNumInEachStripYaw20.assign(m_nStripNum,0);
	double dMinHei=1e10, dMaxHei=-1e10;
	vector<bool> vecPassedbool;
	vecPassedbool.assign(m_nImgNum,true);
	for (int i = 0; i<m_nImgNum; i++)
	{
		if(m_vecImgInfo.size()<=i) return;
		if(!m_vecImgInfo[i].m_bValid)
		{
			vecPassedbool[i]=false;
			continue;
		}
		int nStripIdx = m_vecImgInfo[i].m_nStripIdx;
		int nIdxInStrip = m_vecImgInfo[i].m_nIdxInStrip;
		int nImgIdx = m_vecImgInfo[i].m_nImgIdx;
		//航向重叠度不合格影像
		if(m_vvOverlapsInsStrips[nStripIdx][nIdxInStrip]*100<m_FlyQuaParaSet.fMinOverlapY)
		{
			m_vecImgIdxLowOverlapInsStrips.push_back(nImgIdx);
		}
		//旁向重叠度不合格影像
		if(m_vvOverlapsBetStrips[nStripIdx][nIdxInStrip]*100<m_FlyQuaParaSet.fMinOverlapX)
		{
			m_vecImgIdxLowOverlapBetStrips.push_back(nImgIdx);
		}
		//倾角不合格影像
		if(m_vecImagePitch[nImgIdx]>m_FlyQuaParaSet.fMaxPitch)
		{
			m_vecImgIdxHeiPitch.push_back(nImgIdx);
			vecPassedbool[nImgIdx]=false;
		}
		if(m_vecImagePitch[nImgIdx]>10)
		{
			m_nImgNumPitch10++;
		}
		//旋角不合格影像
		if(fabs(m_vvStripImageYaw[nStripIdx][nIdxInStrip])>m_FlyQuaParaSet.fMaxYaw)
		{
			m_vecImgIdxHeiYaw.push_back(nImgIdx);
		}
		if(fabs(m_vvStripImageYaw[nStripIdx][nIdxInStrip])>15)
		{
			m_nImgNumYaw15++;
		}
		if(fabs(m_vvStripImageYaw[nStripIdx][nIdxInStrip])>20)
		{
			m_vecImgNumInEachStripYaw20[nStripIdx]++;
		}
		//飞行高度不合格影像
		if(m_vvStripImageFlyHeiErr[nStripIdx][nIdxInStrip]>m_FlyQuaParaSet.fMaxHeiDifLinkInStrip)
		{
			m_vecImgIdxHeiHeiErr.push_back(nImgIdx);
		}
		//空三空三困难影像
		if(m_vecImgInfo[nImgIdx].m_ImgAdjPts.vecAdjPtIdx.size()<nMatchPtNumThresold)
		{
			m_vecImgIdxLowMatchPointNum.push_back(nImgIdx);
		}
	}
	//POS不合格影像
	if(m_vecAdjImgPosErr.size()>0)
	{
		int num = 0; double dSum = 0, dMean = 0;
		for (int i = 0; i<m_vecAdjImgPosErr.size(); i++)
		{
			//			if(!m_vecImgInfo[i].m_bValid||!m_vecImgInfo[i].m_bAdjed) continue;
			dSum+=m_vecAdjImgPosDisErr[i];
			num++;
		}
		dMean=dSum/num; dSum=0; /*m_nInAdjedNum = num;*/
		for (int i = 0; i<m_vecAdjImgPosErr.size(); i++)
		{
			//		if(!m_vecImgInfo[i].m_bValid||!m_vecImgInfo[i].m_bAdjed) continue;
			dSum+=pow(m_vecAdjImgPosDisErr[i]-dMean,2);
		}
		m_dRmsPosErr = sqrt(dSum/num);
		m_dMeanPosErr = dMean;
		for (int i = 0; i<m_vecAdjImgPosErr.size(); i++)
		{
			if(fabs(m_vecAdjImgPosDisErr[i]-dMean)>3*m_dRmsPosErr)
			{
				m_vecImgIdxHeiPosErr.push_back(m_vecBundleSuccess2ImgInfo[i]);
				m_vecBundleIdxHeiPosErr.push_back(i);
				vecPassedbool[m_vecBundleSuccess2ImgInfo[i]]=false;
			}
		}
	}
	//质检通过影像
	for (int i = 0; i<vecPassedbool.size(); i++)
	{
		if(vecPassedbool[i] == true)
		{
			m_vecImgIdxPassCheck.push_back(i);
		}
	}
	vector<bool>().swap(vecPassedbool);
}
void CsswUAVFlyQuaPrj::CheckMissArea(double Z, SSWFLYQUAMETHOD eMethod)
{
	vector<vector<Point3D>>().swap(m_vvMissAreaVertex);
	vector<vector<int>>().swap(m_vvMissAreaImgIdx);
	int nInerRingNum = 0;
	OGRGeometry *pGeoCombine = NULL;

	OGRPolygon *pPolygon1 = NULL;
	OGRLinearRing *pRing = NULL;
	double pZ[4]; for(int i = 0; i<4; i++) pZ[i]=Z;

	m_vvImgVertex3d.resize(m_nImgNum);
	int nCount = 0;
	switch(eMethod)
	{
	case BY_ADJUSTMENT:
		for (int i = 0; i<m_nImgNum; i++)
		{
			if(m_vecImgInfo.size()<=i) return;
			if(!m_vecImgInfo[i].m_bValid) continue;
			memset(pZ,0,4*sizeof(double));
			Point2D *pImgVertexPt = m_vecImgInfo[i].GetImgVertexPt2d4(true);
			double dSum = 0;
			for (int j = 0; j<4; j++)
			{
				FunFitHei4ImgAdjPts4ImgPt(m_vecAdjPts,m_dMeanHeiCq,m_vecImgInfo[i].m_ImgAdjPts,pImgVertexPt[j],pZ[j]);
				dSum+=pZ[j];
			}
			Point3D* pVerPt4=m_vecImgInfo[i].GetImgVertexPt3d4(pZ,true,true);
			FunTransPolygonVertexOrder(pVerPt4,4);
			for (int ii = 0; ii<4; ii++)
			{
				m_vvImgVertex3d[i].push_back(pVerPt4[ii]);
			}
			pPolygon1 = FunCreatePolygon(pVerPt4,3,4);
			if(pVerPt4) delete[]pVerPt4;
			OGRwkbGeometryType geotype = pPolygon1->getGeometryType();
			if(geotype!=wkbPolygon) continue;
			if(pPolygon1==NULL) continue;
			if(pGeoCombine==NULL) pGeoCombine = (OGRGeometry*)pPolygon1;
			else pGeoCombine= pGeoCombine->Union(pPolygon1);
			nCount++;
		}
		break;
	default:
		for (int i = 0; i<m_nImgNum; i++)
		{
			if(m_vecImgInfo.size()<=i) return;
			if(!m_vecImgInfo[i].m_bValid) continue;
			Point3D* pVerPt4 = m_vecImgInfo[i].GetImgVertexPt3d4(pZ,false,false);
			for (int ii = 0; ii<4; ii++)
			{
				m_vvImgVertex3d[i].push_back(pVerPt4[ii]);
			}
			pPolygon1 = FunCreatePolygon(pVerPt4,3,4);
			if(pVerPt4) delete[]pVerPt4;
			if(pPolygon1==NULL) continue;
			if(pGeoCombine==NULL) pGeoCombine=(OGRGeometry*)pPolygon1;
			else pGeoCombine=pGeoCombine->Union(pPolygon1);
		}
		break;
	}
	if(pGeoCombine==NULL) return;
	OGRwkbGeometryType geotype=pGeoCombine->getGeometryType();//
	vector<OGRPolygon*> vecpMissPolygon;
	OGRGeometry *pGeoMiss = NULL;
	float fAreaAll =0,fAreaMiss = 0;

	if(geotype==wkbPolygon)
	{
		m_vvAreaVertex.resize(1); 
		pGeoCombine->closeRings();
		pRing = (OGRLinearRing*)(((OGRPolygon*)pGeoCombine)->getExteriorRing());
		for (int i = 0; i<pRing->getNumPoints(); i++)
		{
			OGRPoint ogrpt; pRing->getPoint(i,&ogrpt);
			double x = ogrpt.getX();
			double y = ogrpt.getY();
			m_vvAreaVertex[0].push_back(Point3D(x,y,Z));
		}
		fAreaAll=((OGRPolygon*)pGeoCombine)->get_Area();
		nInerRingNum = ((OGRPolygon*)pGeoCombine)->getNumInteriorRings();
		for (int i = 0; i<nInerRingNum; i++)
		{
			pRing = (OGRLinearRing*)(((OGRPolygon*)pGeoCombine)->getInteriorRing(i));
			vector<Point3D> vecPoint3d;
			fAreaMiss+=pRing->get_Area();
			for (int i = 0; i<pRing->getNumPoints(); i++)
			{
				Point3D pt; OGRPoint ogrpt;
				pRing->getPoint(i,&ogrpt);
				pt.X = ogrpt.getX();
				pt.Y = ogrpt.getY();
				pt.Z = Z;
				vecPoint3d.push_back(pt);
			}
			m_vvMissAreaVertex.push_back(vecPoint3d);
			vector<Point3D>().swap(vecPoint3d);

			OGRPolygon *pPolygonMiss =(OGRPolygon*)OGRGeometryFactory::createGeometry(wkbPolygon);
			pPolygonMiss->addRing(pRing);
			vecpMissPolygon.push_back(pPolygonMiss);
		}
	}
	else if(geotype==wkbMultiPolygon)
	{
		OGRMultiPolygon *pMultiPolygon = (OGRMultiPolygon *)pGeoCombine;
		pMultiPolygon->closeRings();
		int nRef = pMultiPolygon->getNumGeometries();
		m_vvAreaVertex.resize(nRef);
		for (int i = 0; i<nRef; i++)
		{
			OGRGeometry *pGeometry = pMultiPolygon->getGeometryRef(i);
			pGeometry->closeRings();
			geotype=pGeometry->getGeometryType();
			if(geotype==wkbPolygon)
			{
				pPolygon1=(OGRPolygon*)pGeometry;
				pRing = (OGRLinearRing *)pPolygon1->getExteriorRing();
				fAreaAll+=pPolygon1->get_Area();
				for (int j = 0; j<pRing->getNumPoints(); j++)
				{
					OGRPoint ogrpt; pRing->getPoint(j,&ogrpt);
					double x = ogrpt.getX();
					double y = ogrpt.getY();
					m_vvAreaVertex[i].push_back(Point3D(x,y,Z));
				}
				nInerRingNum = pPolygon1->getNumInteriorRings();
				for (int j = 0; j<nInerRingNum; j++)
				{
					pRing = pPolygon1->getInteriorRing(j);
					fAreaMiss+=pRing->get_Area();
					vector<Point3D> vecPoint3d;
					for (int i = 0; i<pRing->getNumPoints(); i++)
					{
						Point3D pt; OGRPoint ogrpt;
						pRing->getPoint(i,&ogrpt);
						pt.X = ogrpt.getX();
						pt.Y = ogrpt.getY();
						pt.Z=Z;
						vecPoint3d.push_back(pt);
					}
					m_vvMissAreaVertex.push_back(vecPoint3d);
					vector<Point3D>().swap(vecPoint3d);
					OGRPolygon *pPolygonMiss = (OGRPolygon *)OGRGeometryFactory::createGeometry(wkbPolygon);
					pPolygonMiss->addRing(pRing);
					vecpMissPolygon.push_back(pPolygonMiss);
				}
			}
		}
	}
	m_dMissAreaRatio = fAreaMiss/fAreaAll;
	for (int i = 0; i<vecpMissPolygon.size(); i++)
	{
		vector<int> vecImgIdx;
		for (int j = 0; j<m_nImgNum; j++)
		{
			if(!m_vecImgInfo[j].m_bValid) continue;
			Point3D *pVertex3d = m_vecImgInfo[j].GetImgVertexPt3d4(pZ,false,(eMethod==BY_POS?false:true));
			OGRPolygon* pPolygon = FunCreatePolygon(pVertex3d,3,4);
			if(pPolygon->getGeometryType()==wkbPolygon)
			{
				OGRGeometry *pInter = vecpMissPolygon[i]->Intersection(pPolygon);
				geotype = pInter->getGeometryType();
				if(geotype==wkbPolygon||geotype==wkbPoint)
					vecImgIdx.push_back(j); 
			}
			if(pVertex3d) delete[]pVertex3d;
			if(pPolygon) OGRGeometryFactory::destroyGeometry(pPolygon);
		}
		m_vvMissAreaImgIdx.push_back(vecImgIdx);
		vector<int>().swap(vecImgIdx);
	}
	for (int i = 0; i<vecpMissPolygon.size(); i++)
	{
		if(vecpMissPolygon[i]) 
			OGRGeometryFactory::destroyGeometry(vecpMissPolygon[i]);
	}
	vector<OGRPolygon*>().swap(vecpMissPolygon);
	if(pGeoCombine) OGRGeometryFactory::destroyGeometry(pGeoCombine);
	//	if(pPolygon1) OGRGeometryFactory::destroyGeometry(pPolygon1);
	//	if(pRing) OGRGeometryFactory::destroyGeometry(pRing);
}

void CsswUAVFlyQuaPrj::CalMatchMatrix(int nMatCount, int nDim /* = 3 */, int nNearst /* = 10 */, SSWMatchMatrixMethod eMethod /* = SSW_POS_DIS */)
{
	switch(eMethod)
	{
	case SSW_POS_DIS:
		MatchMatrixByPosDis(nMatCount,nDim,nNearst);
		break;
	case SSW_AREA_LINK:
		MatchMatrixByAreaLink(nMatCount,nDim);
	default:
		break;
	}

}
void CsswUAVFlyQuaPrj::CheckImgValidState()
{
	if(m_nImgNum==0) return;
	double sum = 0, mean = 0, rms=0;
	vector<double> vDSinkap(m_nImgNum,0);
	for (int i = 0; i<m_nImgNum; i++)
	{
		if(m_vecImgInfo.size()<=i) return;

		vDSinkap[i]=fabs(sin(DegToRad(m_vecImgInfo[i].m_ImgPosInfo.kap)));
		sum+=vDSinkap[i];
		m_vecImgInfo[i].m_bValid=true;
	}
	mean = sum/m_nImgNum; sum=0;
	rms = FunCalDataRms(vDSinkap,mean);
	for (int i = 0; i<m_nImgNum; i++)
	{
		if(m_vecImgInfo.size()<=i) return;

		if(fabs(mean-vDSinkap[i])>m_FlyQuaParaSet.fValidJudgeTimesOfRms*rms)
		{
			m_vecImgInfo[i].m_bValid=false;
			m_nInvalidNum++;
			m_vecImgIdxInvalid.push_back(i);
			m_vecInvalidImgInfo.push_back(m_vecImgInfo[i]);
		}
	}
}
void CsswUAVFlyQuaPrj::DeleteInvalidImg()
{
	for (int i = 0; i<m_vecImgIdxInvalid.size(); i++)
	{
		for (int j = 0; j<m_vecImgInfo.size(); j++)
		{
			if(m_vecImgInfo[j].m_nImgIdx==m_vecImgIdxInvalid[i])
			{
				m_vecImgInfo.erase(m_vecImgInfo.begin()+j);
				m_vecImgIdxInvalid.erase(m_vecImgIdxInvalid.begin()+i);
				i--;
				j--;
				m_nImgNum--;
				break;
			}
		}
	}
	m_nInvalidNum = 0;
	SavePrj2File(m_strPrjXmlPath);
}
void CsswUAVFlyQuaPrj::SetImgValidState(vector<vector<Point2D>>vvAreaVertex2Select, bool bIsValid /* = true */, FlyQuaCallBackFun pFun /* = NULL */)
{
	if(vvAreaVertex2Select.size()==0) return;
	ClearStripInfo();
	vector<OGRPolygon*> vecPolygon(vvAreaVertex2Select.size());
	for (int i = 0; i<vvAreaVertex2Select.size(); i++)
	{
		OGRPolygon *pSelectPolygon = NULL;
		Point2D *pVexterPt = new Point2D[vvAreaVertex2Select[i].size()-1];
		for(int j = 0; j<vvAreaVertex2Select[i].size()-1; j++)
		{
			pVexterPt[j]=vvAreaVertex2Select[i][j];
		}
		pSelectPolygon = FunCreatePolygon(pVexterPt,2,vvAreaVertex2Select[i].size()-1);
		vecPolygon[i]=pSelectPolygon;
		delete[]pVexterPt;
		//		OGRGeometryFactory::destroyGeometry(pSelectPolygon);
	}
#pragma omg parallel for
	for(int i = 0; i<m_vecImgInfo.size(); i++)
	{
		if(m_vecImgInfo.size()<=i) continue;
		if(m_vecImgInfo[i].m_bValid==bIsValid) continue;
		OGRPoint ogrPt;
		ogrPt.setX(m_vecImgInfo[i].m_ImgPosInfo.Xs);
		ogrPt.setY(m_vecImgInfo[i].m_ImgPosInfo.Ys);
		OGRGeometry *pInet=NULL; 
		for (int j = 0; j<vecPolygon.size(); j++)
		{
			pInet=vecPolygon[j]->Intersection(&ogrPt);
			if(pInet==NULL) continue;
			CString strName = pInet->getGeometryName();
			if(strName=="POINT")
			{
				m_vecImgInfo[i].m_bValid = bIsValid;
				break;
			}
		}
	}
	m_nInvalidNum = 0;
	for (int i = 0; i<m_nImgNum; i++)
	{
		if(!m_vecImgInfo[i].m_bValid)
			m_nInvalidNum++;
	}
	for (int i = 0; i<vecPolygon.size(); i++)
	{
		OGRGeometryFactory::destroyGeometry(vecPolygon[i]);
	}
	vector<OGRPolygon*>().swap(vecPolygon);
}

bool CsswUAVFlyQuaPrj::ReOrderStrips(double dLine_a, double dLine_b, double dLine_c,FlyQuaCallBackFun pFun /* = NULL */)
{
	ClearStripInfo();
	ClearReportInfo(BY_POS);
	double sinStripDirKap = dLine_b/sqrt(pow(dLine_a,2)+pow(dLine_b,2));
	double cosStripDirKap = dLine_a/sqrt(pow(dLine_a,2)+pow(dLine_b,2));
	vector<double> vecLocXs(m_nImgNum-m_nInvalidNum,0);
	vector<double> vecLocYs(m_nImgNum-m_nInvalidNum,0);                   
	vector<int> vecLocXYIdx2ImgIdx(m_nImgNum-m_nInvalidNum,0);
	vector<int> vecImgStripIdx(m_nImgNum,InvalidValue);

	//1. 计算局部坐标（坐标轴与航带平行-垂直）
	int nCount = 0;
	if(pFun!=NULL) pFun("开始航带排列计算...",0,NULL,0,true);
	if(pFun!=NULL) pFun("航带排列计算-坐标系旋转，计算局部坐标",0,NULL,0,false);
	FunOutPutLogInfo("航带排列计算-坐标系旋转，计算局部坐标");
	/************************************************************************/
	/* 2017-6-25添加dMeanKap，辅助航带排列                                    */
	/************************************************************************/
	double dMeanKap=0;
	double dSinMeankap=0,dCosMeankap=0;
	for (int i = 0; i<m_nImgNum; i++)
	{
		if(m_vecImgInfo.size()<=i)  goto fun_end;  //检测主线程出现析构
		if(!m_vecImgInfo[i].m_bValid)  continue;
		vecLocXs[nCount] = cosStripDirKap*m_vecImgInfo[i].m_ImgPosInfo.Xs+sinStripDirKap*m_vecImgInfo[i].m_ImgPosInfo.Ys;
		vecLocYs[nCount] = cosStripDirKap*m_vecImgInfo[i].m_ImgPosInfo.Ys-sinStripDirKap*m_vecImgInfo[i].m_ImgPosInfo.Xs;
		vecLocXYIdx2ImgIdx[nCount]=i;
		nCount++;
		dMeanKap+=m_vecImgInfo[i].m_ImgPosInfo.kap;
// 		dSinMeankap+=sin(m_vecImgInfo[i].m_ImgPosInfo.kap);
// 		dCosMeankap+=cos(m_vecImgInfo[i].m_ImgPosInfo.kap);
	}
	if(nCount!=0)
	{
		dMeanKap/=nCount;
		dSinMeankap = sin(DegToRad(dMeanKap));
		dCosMeankap = cos(DegToRad(dMeanKap));
	}
	FunOutPutLogInfo("航带排列计算-局部坐标排序");
	if(pFun!=NULL) pFun("航带排列计算-局部坐标排序",10,NULL,0,false);
	//2. 按照局部坐标Xs排序-降序
	for (int i= 0; i<m_nImgNum-m_nInvalidNum-1; i++)
	{
		for (int j = i+1; j<m_nImgNum-m_nInvalidNum; j++)
		{
			if(m_vecImgInfo.size()<=i)  goto fun_end;  //检测主线程出现析构
			if(vecLocXs[i]>vecLocXs[j])
			{
				swap(vecLocXYIdx2ImgIdx[i],vecLocXYIdx2ImgIdx[j]);
				swap(vecLocXs[i],vecLocXs[j]);
				swap(vecLocYs[i],vecLocYs[j]);
			}
		}	
	}

	//3.计算排列航带距离阈值-dMinDis
	double dMinDis = 1e10;
	int n1=-1,n2=-1;
	for (int i = 0; i<m_nImgNum-m_nInvalidNum; i++)
	{
		for (int j = 0; j<m_nImgNum-m_nInvalidNum; j++)
		{
			if(m_vecImgInfo.size()<=i)  goto fun_end;  //检测主线程出现析构
			if(j==i) continue;
			if(dMinDis>sqrt(pow(vecLocXs[i]-vecLocXs[j],2)+pow(vecLocYs[i]-vecLocYs[j],2))){
				n1=i;n2=j;
				dMinDis=min(dMinDis,sqrt(pow(vecLocXs[i]-vecLocXs[j],2)+pow(vecLocYs[i]-vecLocYs[j],2)));
			}
		}
	}
	//4.排列航带
	FunOutPutLogInfo("航带排列计算-航带计算");
	if(pFun!=NULL) pFun("航带排列计算-航带计算",30,NULL,0,false);
	int nStripIdx = m_nMaxStripIdx, nImgNumInStrip = 0; double dStripStartX = InvalidValue; 
	for (int i = 0; i<m_nImgNum-m_nInvalidNum; i++)
	{
		if(m_vecImgInfo.size()<=i)  goto fun_end;  //检测主线程出现析构
		bool bChangeDirection;
		if(dStripStartX==InvalidValue)
		{
			bChangeDirection = false;
			dStripStartX=vecLocXs[i];
			vecImgStripIdx[vecLocXYIdx2ImgIdx[i]]=nStripIdx;
			nImgNumInStrip++;
			continue;
		}
		double kap1 = m_vecImgInfo[vecLocXYIdx2ImgIdx[i-1]].m_ImgPosInfo.kap;
		double kap2 = m_vecImgInfo[vecLocXYIdx2ImgIdx[i-0]].m_ImgPosInfo.kap;
		double sink1 = sin(DegToRad(kap1)), sink2 = sin(DegToRad(kap2));
		double cosk1 = cos(DegToRad(kap1)), cosk2 = cos(DegToRad(kap2));
		if((dMeanKap-kap1)*(dMeanKap-kap2)<0)
		{
			if(/*(dSinMeankap-sink1)*(dSinMeankap-sink2)<0&&*/(dCosMeankap-cosk1)*(dCosMeankap-cosk2)<0) //0度与360度周期判断
			bChangeDirection=true;
		}
// 		else if((dSinMeankap-sink1)*(dSinMeankap-sink2)<0&&(dCosMeankap-cosk1)*(dCosMeankap-cosk2)<0)
// 		{
// 			bChangeDirection=true;
// 		}
		double dis = fabs(dStripStartX-vecLocXs[i]);
		if(dis<=dMinDis&&!bChangeDirection)
		{
			dStripStartX=vecLocXs[i];
			vecImgStripIdx[vecLocXYIdx2ImgIdx[i]]=nStripIdx;
			nImgNumInStrip++;
		}
		else
		{
			m_vecImgNumInEachStrip.push_back(nImgNumInStrip);
			m_vecStripIdx.push_back(nStripIdx);
			nStripIdx++;
			nImgNumInStrip=0;
			dStripStartX=vecLocXs[i];
			vecImgStripIdx[vecLocXYIdx2ImgIdx[i]]=nStripIdx;
			nImgNumInStrip++;
			bChangeDirection = false;
		}
	}
	if(nImgNumInStrip>0) 
	{
		m_vecImgNumInEachStrip.push_back(nImgNumInStrip);
		m_vecStripIdx.push_back(nStripIdx);
	}
	FunOutPutLogInfo("航带排列计算-航带排序");
	if(pFun!=NULL) pFun("航带排列计算-航带排序",60,NULL,0,false);
	//5.航带内排序
	for (int i = 0; i<m_nImgNum-m_nInvalidNum-1; i++)
	{
		for (int j = i+1; j<m_nImgNum-m_nInvalidNum; j++)
		{
			if(m_vecImgInfo.size()<=i)  goto fun_end;  //检测主线程出现析构
			if(vecImgStripIdx[vecLocXYIdx2ImgIdx[i]]>vecImgStripIdx[vecLocXYIdx2ImgIdx[j]])
			{
				swap(vecLocXYIdx2ImgIdx[i],vecLocXYIdx2ImgIdx[j]);
				swap(vecLocXs[i],vecLocXs[j]);
				swap(vecLocYs[i],vecLocYs[j]);
			}
		}	
	}
	nCount = 0;
	for (int i = 0; i<m_vecImgNumInEachStrip.size(); i++)
	{
		for (int j = 0; j<m_vecImgNumInEachStrip[i]-1; j++)
		{
			for (int k = j+1; k<m_vecImgNumInEachStrip[i]; k++)
			{
				if(m_vecImgInfo.size()<=i)  goto fun_end;  //检测主线程出现析构
				int nIdx1 = j+nCount;
				int nIdx2 = k+nCount;
				if(vecLocYs[nIdx1]>vecLocYs[nIdx2])
				{
					swap(vecLocXYIdx2ImgIdx[nIdx1],vecLocXYIdx2ImgIdx[nIdx2]);
					swap(vecLocXs[nIdx1],vecLocXs[nIdx2]);
					swap(vecLocYs[nIdx1],vecLocYs[nIdx2]);
				}
			}
		}
		nCount+=m_vecImgNumInEachStrip[i];
	}	
	FunOutPutLogInfo("航带排列计算-航带重新编号");
	if(pFun!=NULL) pFun("航带排列计算-影像重新编号",90,NULL,0,false);
	nCount = 0;
	m_nStripNum = m_vecImgNumInEachStrip.size();
	m_nMaxStripIdx = m_nStripNum;
	m_vvImgIdxInEachStrip.resize(m_nStripNum);
	nCount = 0;
	for (int i = 0; i<m_nStripNum; i++)
	{
		vector<int> vecImgIdx(m_vecImgNumInEachStrip[i]);
		for (int j = 0; j<m_vecImgNumInEachStrip[i]; j++)
		{
			if(m_vecImgInfo.size()<=i)  goto fun_end;  //检测主线程出现析构
			int nIdx = vecLocXYIdx2ImgIdx[nCount+j];
			m_vecImgInfo[nIdx].m_nIdxInStrip=j;
			m_vecImgInfo[nIdx].m_nStripIdx=i;
			m_vecImgInfo[nIdx].m_strStripLabel=FunCreateStripLabel(i,j);
			vecImgIdx[j]=nIdx;
		}
		nCount+=m_vecImgNumInEachStrip[i];
		m_vvImgIdxInEachStrip[i]=vecImgIdx;	
		vector<int>().swap(vecImgIdx);
		CString str; str.Format("%d-%d",i,m_nStripNum);
		if(pFun!=NULL) pFun(str,100,NULL,0,false);
	}
fun_end:
	vector<double>().swap(vecLocXs);
	vector<double>().swap(vecLocYs);

	vector<int>().swap(vecImgStripIdx);
	vector<int>().swap(vecLocXYIdx2ImgIdx);
	if(pFun!=NULL) pFun("航带排列计算-进度 ",100,NULL,0,false);
	if(pFun!=NULL) pFun("航带排列计算-完成！",-1,NULL,0,false);
	FunOutPutLogInfo("航带排列计算-完成！");
	return TRUE;
}
bool CsswUAVFlyQuaPrj::ReOrderStripByHand(vector<vector<Point2D>> vvAreaVertex2Select,FlyQuaCallBackFun pFun/* = NULL*/)
{
	//1. 按照绘制的多边形对POS进行航带聚类
	vector<int> vecImgStripIdx(m_nImgNum,InvalidValue);
	if(pFun!=NULL) pFun("开始航带排列计算...",0,NULL,0,true);
	if(pFun!=NULL) pFun("航带排列计算-计算航带编号",0,NULL,0,false);
	FunOutPutLogInfo("航带排列计算-计算航带编号");

#pragma omp parallel for
	for (int i = 0; i<m_nImgNum; i++)
	{
		if(!m_vecImgInfo[i].m_bValid) continue;
		vecImgStripIdx[i]=m_vecImgInfo[i].m_nStripIdx;
	}
	for (int i = 0; i<vvAreaVertex2Select.size(); i++)
	{
		if(pFun!=NULL) pFun("航带排列计算-计算航带编号",i*40.0/vvAreaVertex2Select.size(),NULL,0,false);
		if(vvAreaVertex2Select[i].size()<3) continue;
		OGRPolygon *pSelectPolygon = NULL;
		Point2D *pVexterPt = new Point2D[vvAreaVertex2Select[i].size()-1];
		for(int j = 0; j<vvAreaVertex2Select[i].size()-1; j++)
		{
			pVexterPt[j]=vvAreaVertex2Select[i][j];
		}
		
		pSelectPolygon = FunCreatePolygon(pVexterPt,2,vvAreaVertex2Select[i].size()-1);
		if(!pSelectPolygon) continue;;
#pragma omg parallel for
		for(int j = 0; j<m_vecImgInfo.size(); j++)
		{
			if(m_vecImgInfo.size()<=i) continue;
			if(!m_vecImgInfo[j].m_bValid) continue;
			OGRPoint ogrPt;
			ogrPt.setX(m_vecImgInfo[j].m_ImgPosInfo.Xs);
			ogrPt.setY(m_vecImgInfo[j].m_ImgPosInfo.Ys);
			OGRGeometry *pInet=NULL; 
			pInet=pSelectPolygon->Intersection(&ogrPt);
			if(pInet==NULL) continue;
			CString strName = pInet->getGeometryName();
			if(strName=="POINT")
			{
				vecImgStripIdx[j]=m_nMaxStripIdx;
				//				m_vecImgInfo[j]=m_nMaxStripIdx;
			}
		}
		m_vecStripIdx.push_back(m_nMaxStripIdx);
		m_nMaxStripIdx++;
		delete[]pVexterPt;
		OGRGeometryFactory::destroyGeometry(pSelectPolygon);
	}
	FunOutPutLogInfo("航带排列计算-清除无效航带");
	if(pFun!=NULL) pFun("航带排列计算-清除无效航带",45,NULL,0,false);
	//2. 整理顺序
	vector<int>().swap(m_vecImgNumInEachStrip);
	vector<vector<int>>().swap(m_vvImgIdxInEachStrip);
	vector<int> vecStripIdxNoImg;
	//#pragma omp parallel for

	for (int i = 0; i<m_vecStripIdx.size(); i++)
	{
		vector<int> vecImgIdxInStrip;
		for (int j = 0; j<m_nImgNum; j++)
		{
			if(!m_vecImgInfo[j].m_bValid) continue;
			if(vecImgStripIdx[j]==m_vecStripIdx[i])
			{
				//	nImgNumInStrip++;
				vecImgIdxInStrip.push_back(j);
			}
		}
		if(vecImgIdxInStrip.size()>0)
		{
			m_vecImgNumInEachStrip.push_back(vecImgIdxInStrip.size());
			m_vvImgIdxInEachStrip.push_back(vecImgIdxInStrip);
		}
		else vecStripIdxNoImg.push_back(i);
		vector<int>().swap(vecImgIdxInStrip);
	}
	for (int i = 0; i<vecStripIdxNoImg.size(); i++)
	{
		for (int j = 0; j<m_vecStripIdx.size(); j++)
		{
			if(m_vecStripIdx[j]==vecStripIdxNoImg[i])
			{
				m_vecStripIdx.erase(m_vecStripIdx.begin()+j);
				j--;
				break;
			}
		}
	}
	vector<int>().swap(vecStripIdxNoImg);
	m_nStripNum = m_vecStripIdx.size();
	m_nMaxStripIdx=m_nStripNum;
	FunOutPutLogInfo("航带排列计算-航带间排序-50");
	if(pFun!=NULL) pFun("航带排列计算-航带间排序",50,NULL,0,false);
	/************************************************************************/
	/* 2017-6-25注释部分改为dMeantanKap辅助航带间排序                              */
	/************************************************************************/

	/*
	double dMeantanKap = 0; int nCount = 0;
	for (int i = 0; i<m_nImgNum; i++)
	{
		if(m_vecImgInfo.size()<=i) return;  //检测主线程出现析构
		if(!m_vecImgInfo[i].m_bValid)  continue;
		dMeantanKap+=tan(DegToRad(m_vecImgInfo[i].m_ImgPosInfo.kap));
		nCount++;
	}
	if(nCount!=0) dMeantanKap/=nCount; nCount=0;
	vector<double> vecLocXs(m_nImgNum,0);
	vector<double> vecLocYs(m_nImgNum,0);                   
	for (int i = 0; i<m_nImgNum; i++)
	{
		if(m_vecImgInfo.size()<=i)  return; //检测主线程出现析构
		if(!m_vecImgInfo[i].m_bValid)  continue;
		vecLocXs[i] = cos(atan(dMeantanKap))*m_vecImgInfo[i].m_ImgPosInfo.Xs+sin(atan(dMeantanKap))*m_vecImgInfo[i].m_ImgPosInfo.Ys;
		vecLocYs[i] = cos(atan(dMeantanKap))*m_vecImgInfo[i].m_ImgPosInfo.Ys-sin(atan(dMeantanKap))*m_vecImgInfo[i].m_ImgPosInfo.Xs;
	}
	if(pFun!=NULL) pFun("开始航带排列计算-航带间排序",65,NULL,0,false);
	FunOutPutLogInfo("航带排列计算-航带间排序-65");
	vector<double>vecdStripMeanX,vecdStripMeanY;
	vecdStripMeanX.assign(m_nStripNum,0);
	vecdStripMeanY.assign(m_nStripNum,0);
	for (int i = 0; i<m_nStripNum; i++)
	{
		int nStripIdxNow =i;
		if(m_vvImgIdxInEachStrip[i].size()==0) continue;
		for (int j = 0; j<m_vecImgNumInEachStrip[i]; j++)
		{
			int nImgIdx = m_vvImgIdxInEachStrip[i][j];
			vecdStripMeanX[nStripIdxNow]+=vecLocXs[nImgIdx];
			vecdStripMeanY[nStripIdxNow]+=vecLocYs[nImgIdx];
		}
		vecdStripMeanX[nStripIdxNow]/=(m_vecImgNumInEachStrip[i]);
		vecdStripMeanY[nStripIdxNow]/=(m_vecImgNumInEachStrip[i]);
	}
	for (int i = 0; i<m_nStripNum-1; i++)
	{
		for (int j = i+1; j<m_nStripNum; j++)
		{
			if(vecdStripMeanX[i]<vecdStripMeanX[j])
			{
				swap(vecdStripMeanX[i],vecdStripMeanX[j]);
				swap(m_vecStripIdx[i],m_vecStripIdx[j]);
				swap(m_vecImgNumInEachStrip[i],m_vecImgNumInEachStrip[j]);
				swap(m_vvImgIdxInEachStrip[i],m_vvImgIdxInEachStrip[j]);
			}
		}
	}
	if(pFun!=NULL) pFun("开始航带排列计算-航带间排序",70,NULL,0,false);
	FunOutPutLogInfo("航带排列计算-航带间排序-70");
#pragma omp parallel for
	for (int i = 0; i<m_nStripNum; i++)
	{
		for (int j = 0; j<vecImgStripIdx.size(); j++)
		{
			if(vecImgStripIdx[j]==InvalidValue)continue;
			if(vecImgStripIdx[j]==m_vecStripIdx[i])
			{
				m_vecImgInfo[j].m_nStripIdx=i;
			}
		}
		m_vecStripIdx[i]=i;
	}
	//航带内排序	
	FunOutPutLogInfo("开始航带排列计算-航带内排序");
	if(pFun!=NULL) pFun("开始航带排列计算-航带内排序",75,NULL,0,false);
	nCount = 0;

	for (int i = 0; i<m_vecImgNumInEachStrip.size(); i++)
	{
		if(pFun!=NULL) pFun("开始航带排列计算-航带内排序",75+20.0*i/m_vecImgNumInEachStrip.size(),NULL,0,false);
		vector<double> vecStripImgYs(m_vecImgNumInEachStrip[i]); 
		vector<int> vecYs2ImgIdxList(m_vecImgNumInEachStrip[i]);
#pragma omp parallel for
		for (int j = 0; j<m_vecImgNumInEachStrip[i]; j++)
		{
			vecStripImgYs[j] = vecLocYs[m_vvImgIdxInEachStrip[i][j]];
			vecYs2ImgIdxList[j]=m_vvImgIdxInEachStrip[i][j];
		}
		for (int j = 0; j<m_vecImgNumInEachStrip[i]-1; j++)
		{
			for (int k = j+1; k<m_vecImgNumInEachStrip[i]; k++)
			{
				if(vecStripImgYs[k]<vecStripImgYs[j])
				{
					swap(vecStripImgYs[j],vecStripImgYs[k]);
					swap(vecYs2ImgIdxList[j],vecYs2ImgIdxList[k]);
				}
			}
		}
		for (int i = 0; i<vecYs2ImgIdxList.size(); i++)
		{
			m_vecImgInfo[vecYs2ImgIdxList[i]].m_nIdxInStrip=i;
		}
		vector<double>().swap(vecStripImgYs); 
		vector<int>().swap(vecYs2ImgIdxList);
	}	
	vector<double>().swap(vecLocXs);
	vector<double>().swap( vecLocYs);     */    
	/************************************************************************/
/////////////////////////////////////////////////////////////////////////////////////////////

	//航带间排序
	m_nStripNum = m_vecStripIdx.size();
	m_nMaxStripIdx=m_nStripNum;
	vector<double> vecStripMinX(m_nStripNum,1e10),vecStripMaxX(m_nStripNum,-1e10);
	vector<double> vecStripMinY(m_nStripNum,1e10),vecStripMaxY(m_nStripNum,-1e10);
	for (int i = 0; i<m_nStripNum; i++)
	{
		for (int j = 0; j<m_vecImgNumInEachStrip[i]; j++)
		{
			vecStripMinX[i]=min(vecStripMinX[i],m_vecImgInfo[m_vvImgIdxInEachStrip[i][j]].m_ImgPosInfo.Xs);
			vecStripMaxX[i]=max(vecStripMaxX[i],m_vecImgInfo[m_vvImgIdxInEachStrip[i][j]].m_ImgPosInfo.Xs);
			vecStripMinY[i]=min(vecStripMinY[i],m_vecImgInfo[m_vvImgIdxInEachStrip[i][j]].m_ImgPosInfo.Ys);
			vecStripMaxY[i]=max(vecStripMaxY[i],m_vecImgInfo[m_vvImgIdxInEachStrip[i][j]].m_ImgPosInfo.Ys);
		}
		if(vecStripMinX[i]==vecStripMaxX[i]||vecStripMinY[i]==vecStripMaxY[i])
		{
			vector<double>().swap(vecStripMaxX);
			vector<double>().swap(vecStripMaxY);
			vector<double>().swap(vecStripMinX);
			vector<double>().swap(vecStripMinY);
			ClearStripInfo();
			if(pFun!=NULL) pFun("航带排列异常，请重新操作！",100,NULL,0,true);
			return false;
		}
	}
	SSWOVERLAPDIRECTION eDirStrip = (vecStripMaxY[0]-vecStripMinY[0])>(vecStripMaxX[0]-vecStripMinX[0])?DIRECTION_Y:DIRECTION_X;
	if(eDirStrip==DIRECTION_Y)
	{
		double comMinY=vecStripMinY[0],comMaxY=vecStripMaxY[0];
		for (int i = 0; i<m_nStripNum; i++)
		{
			comMinY = max(comMinY,vecStripMinY[i]);
			comMaxY = min(comMaxY,vecStripMaxY[i]);
		}
		double 	scanLineY = (comMinY+comMaxY)/2;
		vector<double> vecStripIntersecX(m_nStripNum,0);
		for (int i = 0; i<m_nStripNum; i++)
		{
			vecStripIntersecX[i]=vecStripMinX[i]+(scanLineY-vecStripMinY[i])*((vecStripMaxX[i]-vecStripMinX[i]))/(vecStripMaxY[i]-vecStripMinY[i]);
		}
		for (int i = 0; i<m_nStripNum-1; i++)
		{
			for (int j = i+1; j<m_nStripNum; j++)
			{
				if(vecStripIntersecX[i]<vecStripIntersecX[j])
				{
					swap(vecStripIntersecX[i],vecStripIntersecX[j]);
					swap(m_vecStripIdx[i],m_vecStripIdx[j]);
					swap(m_vecImgNumInEachStrip[i],m_vecImgNumInEachStrip[j]);
					swap(m_vvImgIdxInEachStrip[i],m_vvImgIdxInEachStrip[j]);
				}
			}
		}
		vector<double>().swap(vecStripIntersecX);

	}
	else if(eDirStrip==DIRECTION_X)
	{
		double comMinX=vecStripMinX[0],comMaxX=vecStripMaxX[0];
		for (int i = 0; i<m_nStripNum; i++)
		{
			comMinX = max(comMinX,vecStripMinX[i]);
			comMaxX = min(comMaxX,vecStripMaxX[i]);
		}
		double 	scanLineX = (comMinX+comMaxX)/2;
		vector<double> vecStripIntersecY(m_nStripNum,0);
		for (int i = 0; i<m_nStripNum; i++)
		{
			vecStripIntersecY[i]=vecStripMinY[i]+(scanLineX-vecStripMinX[i])*((vecStripMaxY[i]-vecStripMinY[i]))/(vecStripMaxX[i]-vecStripMinX[i]);
		}
		for (int i = 0; i<m_nStripNum-1; i++)
		{
			for (int j = i+1; j<m_nStripNum; j++)
			{
				if(vecStripIntersecY[i]<vecStripIntersecY[j])
				{
					swap(vecStripIntersecY[i],vecStripIntersecY[j]);
					swap(m_vecStripIdx[i],m_vecStripIdx[j]);
					swap(m_vecImgNumInEachStrip[i],m_vecImgNumInEachStrip[j]);
					swap(m_vvImgIdxInEachStrip[i],m_vvImgIdxInEachStrip[j]);
				}
			}
		}
		vector<double>().swap(vecStripIntersecY);
	}
	if(pFun!=NULL) pFun("开始航带排列计算-航带间排序",70,NULL,0,false);
	FunOutPutLogInfo("航带排列计算-航带间排序-70");
#pragma omp parallel for
	for (int i = 0; i<m_nStripNum; i++)
	{
		for (int j = 0; j<vecImgStripIdx.size(); j++)
		{
			if(vecImgStripIdx[j]==InvalidValue)continue;
			if(vecImgStripIdx[j]==m_vecStripIdx[i])
			{
				m_vecImgInfo[j].m_nStripIdx=i;
				//		vecImgStripIdx[j]=i;
			}
		}
		m_vecStripIdx[i]=i;
	}
	FunOutPutLogInfo("开始航带排列计算-航带内排序");
	if(pFun!=NULL) pFun("开始航带排列计算-航带内排序",75,NULL,0,false);
	//航带内排序
int	nCount = 0;
	if(eDirStrip==DIRECTION_Y)
	{
		for (int i = 0; i<m_vecImgNumInEachStrip.size(); i++)
		{
			if(pFun!=NULL) pFun("开始航带排列计算-航带内排序",75+20.0*i/m_vecImgNumInEachStrip.size(),NULL,0,false);
			vector<double> vecStripImgYs(m_vecImgNumInEachStrip[i]); 
			vector<int> vecYs2ImgIdxList(m_vecImgNumInEachStrip[i]);
#pragma omp parallel for
			for (int j = 0; j<m_vecImgNumInEachStrip[i]; j++)
			{
				vecStripImgYs[j] = m_vecImgInfo[m_vvImgIdxInEachStrip[i][j]].m_ImgPosInfo.Ys;
				vecYs2ImgIdxList[j]=m_vvImgIdxInEachStrip[i][j];
			}
			for (int j = 0; j<m_vecImgNumInEachStrip[i]-1; j++)
			{
				for (int k = j+1; k<m_vecImgNumInEachStrip[i]; k++)
				{
					if(vecStripImgYs[k]<vecStripImgYs[j])
					{
						swap(vecStripImgYs[j],vecStripImgYs[k]);
						swap(vecYs2ImgIdxList[j],vecYs2ImgIdxList[k]);
					}
				}
			}
			for (int i = 0; i<vecYs2ImgIdxList.size(); i++)
			{
				m_vecImgInfo[vecYs2ImgIdxList[i]].m_nIdxInStrip=i;
			}
			vector<double>().swap(vecStripImgYs); 
			vector<int>().swap(vecYs2ImgIdxList);
		}
	}
	else if(eDirStrip==DIRECTION_X)
	{
		for (int i = 0; i<m_vecImgNumInEachStrip.size(); i++)
		{
			if(pFun!=NULL) pFun("开始航带排列计算-航带内排序",75+20.0*i/m_vecImgNumInEachStrip.size(),NULL,0,false);
			vector<double> vecStripImgXs(m_vecImgNumInEachStrip[i]); 
			vector<int> vecXs2ImgIdxList(m_vecImgNumInEachStrip[i]);
#pragma omp parallel for
			for (int j = 0; j<m_vecImgNumInEachStrip[i]; j++)
			{
				vecStripImgXs[j] = m_vecImgInfo[m_vvImgIdxInEachStrip[i][j]].m_ImgPosInfo.Xs;
				vecXs2ImgIdxList[j]=m_vvImgIdxInEachStrip[i][j];
			}
			for (int j = 0; j<m_vecImgNumInEachStrip[i]-1; j++)
			{
				for (int k = j+1; k<m_vecImgNumInEachStrip[i]; k++)
				{
					if(vecStripImgXs[k]<vecStripImgXs[j])
					{
						swap(vecStripImgXs[j],vecStripImgXs[k]);
						swap(vecXs2ImgIdxList[j],vecXs2ImgIdxList[k]);
					}
				}
			}
			for (int i = 0; i<vecXs2ImgIdxList.size(); i++)
			{
				m_vecImgInfo[vecXs2ImgIdxList[i]].m_nIdxInStrip=i;
			}
			vector<double>().swap(vecStripImgXs); 
			vector<int>().swap(vecXs2ImgIdxList);
		}
	}


#pragma omp parallel for
	for (int i = 0; i<m_nImgNum; i++)
	{
		if(m_vecImgInfo.size()<=i) continue;
		m_vecImgInfo[i].m_strStripLabel=FunCreateStripLabel(m_vecImgInfo[i].m_nStripIdx,m_vecImgInfo[i].m_nIdxInStrip);
	}

	vector<double>().swap(vecStripMaxX);
	vector<double>().swap(vecStripMaxY);
	vector<double>().swap(vecStripMinX);
	vector<double>().swap(vecStripMinY);
	if(pFun!=NULL) pFun("航带排列计算-进度 ！",100,NULL,0,false);
	if(pFun!=NULL) pFun("航带排列计算-完成！",-1,NULL,0,false);
	FunOutPutLogInfo("航带排列计算-完成！");
	return true;
	//	ReadImgList();
}
void CsswUAVFlyQuaPrj::CmrDistortionCorrect(float fScale /* = 4.0 */, bool bChkExist /* = true */,FlyQuaCallBackFun pFun/* =NULL */)
{
	FunOutPutLogInfo("开始进行相机畸变纠正并生成影像缩略图。");
	clock_t t1 = clock();
	int nValidNum = m_nImgNum - m_nInvalidNum;
	int nCount = 0;
	pFun("正在进行影像畸变校正...",0,NULL,0,true);
	CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);
#pragma omp parallel for num_threads(omp_get_num_procs()-1)
	for (int i = 0; i<m_nImgNum; i++)
	{
		if(m_vecImgInfo.size()<=i) continue;
		if(!m_vecImgInfo[i].m_bValid) continue;
		CString strSavePath = m_strThumbFolder+"\\"+FunGetFileName(m_vecImgInfo[i].m_strImgPath,false)+".jpg";
		m_vecImgInfo[i].m_strThumbPath = strSavePath;
		m_vecImgInfo[i].m_strThumbName = FunGetFileName(strSavePath,false).MakeLower();
		CreateDirectory(m_strThumbFolder,NULL);
		CsswUAVCmrDsitortionCorrect distor; eDistortionReturn eRet;
		eRet = distor.LoadImageInfo(m_vecImgInfo[i].m_strImgPath,m_vecImgInfo[i].m_ImgCmrInfo,strSavePath,fScale);
		if(eRet!=SSWSuccess) continue;
		EnterCriticalSection(&cs);
		nCount++;	
		CString strInfo;
		if(pFun!=NULL)
		{

			if(((nValidNum>100)&&nCount%(nValidNum/100)==0)||(nValidNum<100))
			{
				strInfo.Format("正在进行影像畸变校正...");
				pFun(strInfo,nCount*100.0/nValidNum,m_vecImgInfo[i].m_strImgName,nCount*100.0/nValidNum,false);
			}
		}	
		LeaveCriticalSection(&cs);
		eRet = distor.RunDistortionCorrect(m_vecImgInfo[i].m_ThumbCmrInfo,bChkExist);
	}
	DeleteCriticalSection(&cs);
	// 	if(m_bWithCmr)
	// 	{
	m_ThumbCmrInfo = m_vecImgInfo[m_nInvalidNum].m_ThumbCmrInfo;
	CString strThumCmr = m_strThumbFolder+"\\cmr_thumb.cmr";
	FILE* pfW = fopen(strThumCmr,"w");
	if(pfW!=NULL)
	{
		fprintf(pfW,"1\n%.8lf %.8lf %.8lf %.8lf %.8lf %.8lf %.8lf %.8lf %.8lf %.8lf %.8lf %.8lf %s\n",m_ThumbCmrInfo.x0,m_ThumbCmrInfo.y0,
			m_ThumbCmrInfo.f,m_ThumbCmrInfo.fx,m_ThumbCmrInfo.fy,m_ThumbCmrInfo.pixsize,m_ThumbCmrInfo.k1,m_ThumbCmrInfo.k2,m_ThumbCmrInfo.p1,m_ThumbCmrInfo.p2,
			m_ThumbCmrInfo.a,m_ThumbCmrInfo.b,m_ThumbCmrInfo.strLabel/*+"_resample"*/);
	}

	fclose(pfW);pfW=NULL;
	//	}
	clock_t t2 = clock();

	m_dTimeSecsCmrDistortion = (double)(t2-t1)/CLOCKS_PER_SEC;
	char strlog[MAX_SIZE_FILE_LINE];
	sprintf(strlog,"相机畸变纠正及影像缩略图生产完毕：影像数%d，耗时%.3lfs",m_nImgNum-m_nInvalidNum,m_dTimeSecsCmrDistortion);
	if(pFun!=NULL)
	{
		CString strInfo="正在进行影像畸变校正...";
		pFun(strInfo,100.0,NULL,100.0,false);
		pFun(strlog,100,NULL,100.0,true);
	}
	else
	{
		FunOutPutLogInfo(strlog);
	}
}
bool CsswUAVFlyQuaPrj::BundleAdjustment(sswUAVMatchAndAdjPara para, bool bChkExist /* = true */)
{
	if(m_nImgNum==0)return false;
	if(para.Freenet_bDo)
	{
		if(!PathFileExists(m_strBundleAdjRepPath))
		{
			clock_t t1, t2;char strlog[MAX_SIZE_FILE_LINE];
			t1 = clock();
			if(para.pOutputFunc)
			{
				para.pOutputFunc("开始对缩略图进行匹配平差...",-1,NULL,0,true);
				//			para.pOutputFunc("开始缩略图匹配平差...",-1,NULL,0,true);
			}
			else  FunOutPutLogInfo("开始进行缩略图影像匹配及平差...");
			CreateDirectory(m_strMatchAndAdjFolder,NULL);
			CsswBundleAdjustment UavImgMatch;

			if(!SaveImgList()) return false;
			if(!SavePosList()) return false;
			if(!SaveMatchMatrix()) return false;

			UavImgMatch.LoadData(m_strThumbFolder,m_strImgListPath,m_strPosListPath,m_strMatchMatrixPath);
			if(!UavImgMatch.RunOneKeyMatchAndAdj(m_strMatchAndAdjFolder,para)) return false;
			t2 = clock();
			m_dTimeSecsBundleAdjustment=(double)(t2-t1)/CLOCKS_PER_SEC;
			sprintf(strlog,"匹配平差完毕，耗时%.3lfs",m_dTimeSecsBundleAdjustment);
			if(para.pOutputFunc)  para.pOutputFunc(strlog,0,NULL,0,true);
			else FunOutPutLogInfo(strlog);
		}
		return true;
	}
	else if(para.Match_bDo)
	{
		if(!PathFileExists(m_strMatchPointPath))
		{
			clock_t t1, t2;char strlog[MAX_SIZE_FILE_LINE];
			t1 = clock();
			if(para.pOutputFunc)
				para.pOutputFunc("开始对缩略图进行匹配...",-1,NULL,0,true);
			else  FunOutPutLogInfo("开始进行缩略图影像匹配...");
			CreateDirectory(m_strMatchAndAdjFolder,NULL);
			CsswBundleAdjustment UavImgMatch;

			if(!SaveImgList()) return false;
			if(!SavePosList()) return false;
			if(!SaveMatchMatrix()) return false;

			UavImgMatch.LoadData(m_strThumbFolder,m_strImgListPath,m_strPosListPath,m_strMatchMatrixPath);
			if(UavImgMatch.RunOneKeyMatchAndAdj(m_strMatchAndAdjFolder,para)) return false;
			t2 = clock();
			m_dTimeSecsBundleAdjustment=(double)(t2-t1)/CLOCKS_PER_SEC;
			sprintf(strlog,"匹配完毕，耗时%.3lfs",m_dTimeSecsBundleAdjustment);
			if(para.pOutputFunc)  para.pOutputFunc(strlog,0,NULL,0,true);
			else FunOutPutLogInfo(strlog);
		}
		return true;
	}
	return true;
}
void CsswUAVFlyQuaPrj::CalStripOverlaps(double Z, SSWFLYQUAMETHOD eMethod,FlyQuaCallBackFun pFun /* = NULL */)
{
	if(m_nImgNum<=0||m_nStripNum<=0) return;
	vector<vector<double>>().swap(m_vvOverlapsInsStrips);
	m_vvOverlapsInsStrips.resize(m_nStripNum);
	vector<vector<double>>().swap(m_vvOverlapsBetStrips);
	m_vvOverlapsBetStrips.resize(m_nStripNum);
	for (int i = 0; i<m_nStripNum; i++)
	{
		m_vvOverlapsBetStrips[i].assign(m_vecImgNumInEachStrip[i],0);
		m_vvOverlapsInsStrips[i].assign(m_vecImgNumInEachStrip[i],0);
	}
	//计算航向重叠度
	int nCount = 0;
	CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);
	//#pragma omp parallel for
	for (int i = 0; i<m_nStripNum; i++)
	{
		//		EnterCriticalSection(&cs);
		nCount++;	
		/*		LeaveCriticalSection(&cs);*/
		CString strInfo;
		if(pFun!=NULL)
		{
			strInfo.Format("正在计算航向重叠度...%-3.1lf%%",40+nCount*20.0/m_nStripNum);
			pFun(strInfo,40+nCount*20.0/m_nStripNum,NULL,40+nCount*20.0/m_nStripNum,false);
		}	
		if(m_vecImgInfo.size()<=i) continue;
		for (int j = 0; j<m_vecImgNumInEachStrip[i]; j++)
		{
			if(m_vecImgInfo.size()<=i) break;
			int nImgIdx1 = m_vvImgIdxInEachStrip[i][j];
			int nImgIdx2; double dMaxOverlapY = 0;
			for (int jj = max(j-2,0); jj<min(j+2,m_vecImgNumInEachStrip[i]-1); jj++)
			{
				if(jj==j) continue;
				nImgIdx2 = m_vvImgIdxInEachStrip[i][jj];
				double dOverlapY = 0;
				switch(eMethod)
				{
				case BY_POS:
					dOverlapY = CalOverlapsByPos(nImgIdx1,nImgIdx2,Z,DIRECTION_Y);
					break;
				case BY_MATCH:
					dOverlapY = CalOverlapsByMatch(nImgIdx1,nImgIdx2,DIRECTION_Y);
					if(dOverlapY==0)
						dOverlapY = CalOverlapsByPos(nImgIdx1,nImgIdx2,Z,DIRECTION_Y);
					break;
				case BY_ADJUSTMENT:
					dOverlapY = CalOverlapsByAdjustment(nImgIdx1,nImgIdx2,DIRECTION_Y);
					break;
				default:
					break;
				}
				dMaxOverlapY = max(dMaxOverlapY,dOverlapY);
			}

			m_vvOverlapsInsStrips[i][j]=dMaxOverlapY;
		}

	}
	//////////////////////////////////////////////////////
	//计算旁向重叠度
	vector<vector<int>> vvStripLinkStripIdx;
	for (int i = 0; i<m_nStripNum; i++)
	{
		vector<int> vecLinkStripIdx;
		for (int ii = max(i-2,0); ii<min(m_nStripNum,i+2); ii++)
		{
			if(m_vecImgInfo.size()<=i) break;
			if(ii==i) continue;
			vecLinkStripIdx.push_back(ii);
		}
		vvStripLinkStripIdx.push_back(vecLinkStripIdx);
		vector<int>().swap(vecLinkStripIdx);
	}
	nCount=0;
	//#pragma omp parallel for
	for (int i = 0; i<m_nStripNum; i++)
	{
		if(m_vecImgInfo.size()<=i) continue;
		//	EnterCriticalSection(&cs);
		nCount++;		
		//	LeaveCriticalSection(&cs);
		if(pFun!=NULL)
		{
			CString strInfo;
			strInfo.Format("正在计算旁向重叠度...%-3.1lf%%",60+nCount*20.0/m_nStripNum);
			pFun(strInfo,60+nCount*20.0/m_nStripNum,NULL,60+nCount*20.0/m_nStripNum,false);
		}	
		vector<int> vecLinkStripIdx = vvStripLinkStripIdx[i];
		for (int j = 0; j<m_vecImgNumInEachStrip[i]; j++)
		{
			int nBetStripCount=0;
			int nImgIdx1 = m_vvImgIdxInEachStrip[i][j];
			double dMaxOverlapBetStrip=0;
			if(m_vecImgInfo.size()<=nImgIdx1) break;
			for (int ii = 0; ii<vecLinkStripIdx.size(); ii++)
			{
				double dSum=0;int nCount = 0;
				for (int jj = 0; jj<m_vecImgNumInEachStrip[vecLinkStripIdx[ii]]; jj++)
				{
					if(m_vecImgInfo.size()<=nImgIdx1) break;
					int nImgIdx2 = m_vvImgIdxInEachStrip[vecLinkStripIdx[ii]][jj];
					double dOverlapX = 0;
					switch(eMethod)
					{
					case BY_POS:
						dOverlapX=CalOverlapsByPos(nImgIdx1,nImgIdx2,Z,DIRECTION_X);
						break;
					case BY_MATCH:
						dOverlapX=CalOverlapsByMatch(nImgIdx1,nImgIdx2,DIRECTION_X);
						if(dOverlapX==0)
							dOverlapX = CalOverlapsByPos(nImgIdx1,nImgIdx2,Z,DIRECTION_X);
						break;
					case BY_ADJUSTMENT:
						dOverlapX=CalOverlapsByAdjustment(nImgIdx1,nImgIdx2,DIRECTION_X);
						break;
					default:
						break;
					}
					// 					if (dOverlapX==1||dOverlapX==100)
					// 					{
					// 						CString strInfo; strInfo.Format("%d-%d",nImgIdx1,nImgIdx2);
					// 						MessageBox(NULL,strInfo,"info",MB_OK);
					// 					}
					if(dOverlapX>0)
					{
						dMaxOverlapBetStrip=max(dMaxOverlapBetStrip,dOverlapX);
					}
				}
			}
			m_vvOverlapsBetStrips[i][j]=dMaxOverlapBetStrip;
		}
		vector<int>().swap(vecLinkStripIdx);

	}
	vector<vector<int>>().swap(vvStripLinkStripIdx);
	DeleteCriticalSection(&cs);
}

void CsswUAVFlyQuaPrj::ClearAdjInfo()
{
	vector<SSWstuAdjPtInfo>().swap(m_vecAdjPts);
	for (int i = 0; i<m_nImgNum; i++)
	{
		m_vecImgInfo[i].m_bAdjed = false;
		vector<int>().swap(m_vecImgInfo[i].m_ImgAdjPts.vecAdjPtIdx);
		vector<int>().swap(m_vecImgInfo[i].m_ImgAdjPts.vecLinkImgIdx);
		vector<vector<int>>().swap(m_vecImgInfo[i].m_ImgAdjPts.vvAdjPtIdx4EachLinkImg);
		vector<vector<int>>().swap(m_vecImgInfo[i].m_ImgAdjPts.vvImgPtIdxInAdjPt);
		vector<vector<int>>().swap(m_vecImgInfo[i].m_ImgAdjPts.vvLinkImgPtIdxInAdjPt);
	}
	vector<int>().swap(m_vecBundleSuccess2ImgInfo);
	vector<int>().swap(m_vecPtNumInImgs);
	m_nInAdjedNum = 0;
	m_dRmsPosErr = 0;
	m_dMeanHeiCq = 0;
}
void CsswUAVFlyQuaPrj::ClearStripInfo()
{
	m_nStripNum=0;
	vector<int>().swap(m_vecImgNumInEachStrip);
	vector<vector<int>>().swap(m_vvImgIdxInEachStrip);
	vector<int>().swap(m_vecStripIdx);
#pragma omp parallel for
	for (int i = 0; i<m_nImgNum; i++)
	{
		m_vecImgInfo[i].m_nIdxInStrip=InvalidValue;
		m_vecImgInfo[i].m_nStripIdx=InvalidValue;
		m_vecImgInfo[i].m_strStripLabel = FunCreateStripLabel(InvalidValue,InvalidValue);
	}
	m_nMaxStripIdx=0;
}
void CsswUAVFlyQuaPrj::ClearReportInfo(SSWFLYQUAMETHOD eMethod)
{
	vector<double>().swap(m_vecImagePitch);                 
	vector<vector<double>>().swap(m_vvStripImageYaw);       
	vector<vector<double>>().swap(m_vvStripImageFlyHeiErr); 
	vector<vector<double>>().swap(m_vvStripImageFlyDisErr); 
	vector<double>().swap(m_vecStripBlendPara);             
	vector<vector<double>>().swap(m_vvOverlapsInsStrips);   
	vector<vector<double>>().swap(m_vvOverlapsBetStrips); 

	vector<int>().swap(m_vecImgIdxLowOverlapInsStrips);
	vector<int>().swap(m_vecImgIdxLowOverlapBetStrips);
	vector<int>().swap(m_vecImgIdxHeiPitch);
	vector<int>().swap(m_vecImgIdxHeiYaw);
	vector<int>().swap(m_vecImgIdxHeiHeiErr);
	vector<int>().swap(m_vecStripIdxHeiBlend);
	vector<double>().swap(m_vecStripPanePara);
	vector<SSWstuPosInfo>().swap(m_vecAdjImgPosErr);
	vector<double>().swap(m_vecAdjImgPosDisErr);
	vector<int>().swap(m_vecImgIdxHeiPosErr);
	vector<int>().swap(m_vecBundleIdxHeiPosErr);
	m_dMeanPosErr  =0;
	m_dRmsPosErr = 0;
	m_nImgNumPitch10 = 0;
	m_nImgNumYaw15 = 0;
	m_dMaxPaneDifErr = 0;
	m_dRmsPaneDifErr = 0;
	m_dMaxHeiErr = 0;
	m_dMissAreaRatio = 0;
	vector<int>().swap(m_vecImgNumInEachStripYaw20);
	vector<int>().swap(m_vecImgIdxLowMatchPointNum);
	vector<vector<Point3D>>().swap(m_vvMissAreaVertex);
	vector<vector<int>>().swap(m_vvMissAreaImgIdx);
	vector<vector<Point3D>>().swap(m_vvAreaVertex);
	vector<vector<Point3D>>().swap(m_vvImgVertex3d);
	vector<double>().swap(m_vecDisImgAdjPos2PosPane);

	if(eMethod==BY_POS){
		m_dTimeSecsCmrDistortion=0;	
		m_dTimeSecsBundleAdjustment=0;
		m_dTimeSecsTimeReadAdjRep=0;
	}
	m_dTimeSecsRunFlyQuaChk=0;
	m_dTimeSecsWholeProcess=0;
	m_dTimeSecsWriteChkRep=0;
	m_nMatchPointNumThresold = 8;
	vector<int>().swap(m_vecImgIdxPassCheck);
}
bool CsswUAVFlyQuaPrj::ReadImgList()
{
	vector<CString>().swap(m_vecBundleImgName);
	if(!PathFileExists(m_strImgListPath)) return false;
	CMemTxtFile txt;
	if(!txt.OpenFile(m_strImgListPath)) return false;
	char line[MAX_SIZE_FILE_LINE];
	const char* tmp = txt.GetFileHead();
	if(!ReadLine(tmp,line,MAX_SIZE_FILE_LINE))
		return false;
	tmp+=strlen(line)+2;
	int nImgNum = atoi(line);
	for (int i = 0; i<nImgNum; i++)
	{
		if(!ReadLine(tmp,line,MAX_SIZE_FILE_LINE))
			return false;
		tmp+=strlen(line)+2;
		char strName[1024]; int a,b; double c;
		sscanf(line,"%s %d %d %lf",strName,&a,&b,&c);
		m_vecBundleImgName.push_back(FunGetFileName(strName,false));
	}
	CalBundleImgList2ImgInfo();
	return true;
}
bool CsswUAVFlyQuaPrj::ReadAdjRep(FlyQuaCallBackFun pFun/* =NULL */)
{
	ReadImgList();
	clock_t t1 = clock();
	ClearAdjInfo(); 
	CMemTxtFile txt;
	if(!txt.OpenFile(m_strBundleAdjRepPath))
	{
		FunOutPutLogInfo("空三报告打开错误，数据读取失败！");
		return false;
	}
	char line[MAX_SIZE_FILE_LINE];
	const char* tmp = txt.GetFileHead();
	if(!ReadLine(tmp,line,MAX_SIZE_FILE_LINE))
	{
		FunOutPutLogInfo("空三报告首行读取错误，数据读取失败！");
		return false;
	}
	tmp+=strlen(line)+2;
	int nImgNum, nPtNum;
	if(2!=sscanf(line,"%d %d",&nImgNum,&nPtNum))
	{
		FunOutPutLogInfo("空三报告首行解析错误，数据读取失败！");
		return false;
	}
	if(nImgNum<=0||nPtNum<=0)
	{
		FunOutPutLogInfo("空三结果不完整，数据读取失败！");
		return false;
	}
	char strlog[MAX_SIZE_FILE_LINE];
	SSWstuPosInfo* pAdjPosInfo = new SSWstuPosInfo[nImgNum];
	if(pFun!=NULL) pFun("正在读取空三报告...",10,NULL,0,true);
	for (int i =0; i<nImgNum; i++)
	{
		SSWstuPosInfo sswAdjPos;
		if(!ReadLine(tmp,line,MAX_SIZE_FILE_LINE))
		{	
			sprintf(strlog,"空三结果，第%d张影像信息读取错误！",i);
			FunOutPutLogInfo(strlog);
			return false;
		}
		tmp+=strlen(line)+2;
		int nWidth, nHeight, df; char strLabel[MAX_SIZE_FILE_LINE];
		if(4!=sscanf(line,"%s %d %d %lf", strLabel,&nWidth,&nHeight,&df))
		{
			sprintf(strlog,"空三结果，第%d张影像信息解析错误！",i);
			FunOutPutLogInfo(strlog);
			return false;
		}
		sswAdjPos.strLabel = strLabel; sswAdjPos.strLabel.MakeLower();
		for (int j = 0; j<2; j++)   //跳过两行像主点、畸变参数信息
		{
			if(!ReadLine(tmp,line,MAX_SIZE_FILE_LINE))
			{
				sprintf(strlog,"空三结果，第%d张影像信息读取错误！",i);
				FunOutPutLogInfo(strlog);
				return false;
			}
			tmp+=strlen(line)+2;
		}
		if(!ReadLine(tmp,line,MAX_SIZE_FILE_LINE))
		{
			sprintf(strlog,"空三结果，第%d张影像外方位元素读取错误！",i);
			FunOutPutLogInfo(strlog);
			return false;
		}
		tmp+=strlen(line)+2;
		if(6!=sscanf(line,"%lf %lf %lf %lf %lf %lf",&sswAdjPos.Xs,&sswAdjPos.Ys,
			&sswAdjPos.Zs,&sswAdjPos.R[0],&sswAdjPos.R[1],&sswAdjPos.R[2]))
		{
			sprintf(strlog,"空三结果，第%d张影像外方位元素解析错误！",i);
			FunOutPutLogInfo(strlog);
			return false;
		}
		if(!ReadLine(tmp,line,MAX_SIZE_FILE_LINE))
		{
			sprintf(strlog,"空三结果，第%d张影像外方位元素读取错误！",i);
			FunOutPutLogInfo(strlog);
			return false;
		}
		tmp+=strlen(line)+2;
		if(6!=sscanf(line,"%lf %lf %lf %lf %lf %lf",&sswAdjPos.R[3],&sswAdjPos.R[4],
			&sswAdjPos.R[5],&sswAdjPos.R[6],&sswAdjPos.R[7],&sswAdjPos.R[8]))
		{
			sprintf(strlog,"空三结果，第%d张影像外方位元素解析错误！",i);
			FunOutPutLogInfo(strlog);
			return false;
		}
		*(pAdjPosInfo+i) = sswAdjPos;
		if(pFun!=NULL)
		{
			CString strInfo;
			if((nImgNum>=10&&i%(m_nImgNum/10)==0)||nImgNum<10)
			{
				strInfo = "正在读取空三报告...";
				pFun(strInfo,10+10.0*(i+1)/m_nImgNum,NULL,0,false);
			}
		}

	}
	CString *pAdjPtsLine = new CString[nPtNum*2];
	for (int i = 0; i<nPtNum*2; i++)
	{
		ReadLine(tmp,line,MAX_SIZE_FILE_LINE);
		tmp+=strlen(line)+2;
		*(pAdjPtsLine+i) = line;
	}

	m_vecAdjPts.resize(nPtNum);
	double dSumOfHei = 0;
	int nCount = 0;
	for (int i = 0; i<nPtNum; i++)
	{
		nCount++;
		if(pFun!=NULL)
		{
			CString strInfo;
			if((nPtNum>=75&&nCount%(nPtNum*5/75)==0)||nPtNum<75)
			{
				strInfo = "正在读取空三报告...";
				pFun(strInfo,20+75.0*(nCount+1)/nPtNum,NULL,0,false);
			}
		}
		double X,Y,Z; int r,g,b; int nOverlap;
		sscanf(*(pAdjPtsLine+i*2+0),"%lf %lf %lf %d %d %d",&X,&Y,&Z,&r,&g,&b);
		vector<CString> vParts = FunStrTok(*(pAdjPtsLine+i*2+1),"\t ");
		nOverlap = atoi(vParts[0]);
		SSWstuAdjPtInfo AdjPt;
		AdjPt.X=X;AdjPt.Y=Y;AdjPt.Z=Z;
		AdjPt.r=r;AdjPt.g=g;AdjPt.b=b;
		AdjPt.nOverlayCount = nOverlap;
		AdjPt.vecImgIdx.resize(nOverlap);
		AdjPt.vecImgPt.resize(nOverlap);
		dSumOfHei+=Z;
		for (int j = 0; j<nOverlap; j++)
		{
			if(m_vecImgInfo.size()==0) return true;
			int nImgId; double px,py;
			nImgId = m_vecBundleImgList2ImgInfo[atoi(vParts[j*3+1])];  //真实索引
			px = atof(vParts[j*3+2]);
			py  = atof(vParts[j*3+3]);
			Point2D pt = Point2D(px,py);
			AdjPt.vecImgIdx[j] = nImgId;
			AdjPt.vecImgPt[j] = pt;
		}
		m_vecAdjPts[i] = AdjPt;
		vector<CString>().swap(vParts);
	}
	txt.CloseFile();
	m_vecBundleSuccess2ImgInfo.resize(nImgNum);
	m_dMeanHeiCq = dSumOfHei/nPtNum;
#pragma omp parallel for
	for (int i = 0; i<nImgNum; i++)
	{
		for (int j = 0; j<m_nImgNum; j++)
		{
			if(m_vecImgInfo[j].m_bAdjed) continue;
			if((*(pAdjPosInfo+i)).strLabel==m_vecImgInfo[j].m_strImgName)
			{
				m_vecImgInfo[j].m_AdjPosInfo = *(pAdjPosInfo+i);
				m_vecImgInfo[j].m_bAdjed=true;
				m_vecBundleSuccess2ImgInfo[i]=j;
				break;
			}
		}
	}
	m_nInAdjedNum = nImgNum;
	FunArrangeAdjPts(m_vecImgInfo,m_vecAdjPts, m_vecPtNumInImgs);

	CString strInfo="正在读取空三报告...";
	if(pFun!=NULL)	pFun(strInfo,97,NULL,0,false);

	if(pAdjPosInfo)delete[]pAdjPosInfo;
	if(pAdjPtsLine)delete[]pAdjPtsLine;

	strInfo="正在读取空三报告...";
	if(pFun!=NULL) pFun(strInfo,100,NULL,0,false);
	clock_t t2 = clock();
	double dt = (double)(t2-t1)/CLOCKS_PER_SEC;
	m_dTimeSecsTimeReadAdjRep = dt;
	sprintf(strlog,"空三结果文件读取完成：影像数%d，物方点数%d，耗时%.3lfs",nImgNum,nPtNum,m_dTimeSecsTimeReadAdjRep);
	if(pFun!=NULL)
	{
		pFun(strlog,100,NULL,0,true);	
	}
	else
	{
		FunOutPutLogInfo(strlog);
	}
	return true;
}
bool CsswUAVFlyQuaPrj::ReadMatchPoint(FlyQuaCallBackFun pFun /* = NULL */)
{
	clock_t t1 = clock(); 
	ClearAdjInfo();
	ReadImgList();
	CMemTxtFile txt;
	if(!txt.OpenFile(m_strMatchPointPath))
	{
		FunOutPutLogInfo("匹配结果文件打开错误，数据读取失败！");
		return false;
	}
	if(pFun)
	{
		pFun("正在读取匹配结果...",0,NULL,0,true);
		pFun("正在读取匹配结果...",5,NULL,0,false);
	}
	const char* tmp = txt.GetFileHead();

	char line[MAX_SIZE_FILE_LINE];
	char strlog[MAX_SIZE_FILE_LINE];

	int nImgNum, nPtNum;
	if(!ReadLine(tmp,line,MAX_SIZE_FILE_LINE))
	{
		FunOutPutLogInfo("匹配结果首行读取错误，数据读取失败！");
		return false;
	}
	tmp+=strlen(line)+2;
	if(2!=sscanf(line,"%d %d",&nImgNum,&nPtNum))
	{
		FunOutPutLogInfo("匹配结果首行解析错误，数据读取失败！");
		return false;
	}
	nImgNum = m_nImgNum-m_nInvalidNum;

	m_vecAdjPts.resize(nPtNum);
	CString *pAdjPtsLine = new CString[nPtNum*2];
	for (int i = 0; i<nPtNum*2; i++)
	{
		ReadLine(tmp,line,MAX_SIZE_FILE_LINE);
		tmp+=strlen(line)+2;
		*(pAdjPtsLine+i) = line;
		if(pFun)
		{
			if((nPtNum>=10&&i%(nPtNum/10)==0)||nPtNum<10)
			{
				CString strInfo;
				strInfo.Format("正在读取匹配结果...",5+5*(i+1)/nPtNum);
				pFun(strInfo,5+5*(i+1)/nPtNum,NULL,0,false);
			}
		}
	}
	int nCount = 0;
	CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);
	//#pragma omp parallel for
	for (int i = 0; i<nPtNum; i++)
	{
		// 		EnterCriticalSection(&cs);
		nCount++;
		if(pFun!=NULL)
		{
			CString strInfo;
			if((nPtNum>=85&&nCount%(nPtNum*5/85)==0)||nPtNum<85)
			{
				strInfo.Format("正在读取匹配结果...",10+85.0*(nCount+1)/nPtNum);
				pFun(strInfo,20+75.0*(nCount+1)/nPtNum,NULL,0,false);
			}
		}
		// 		LeaveCriticalSection(&cs);
		double X,Y,Z; int r,g,b; int nOverlap;
		sscanf(*(pAdjPtsLine+i*2+0),"%lf %lf %lf %d %d %d",&X,&Y,&Z,&r,&g,&b);
		vector<CString> vParts = FunStrTok(*(pAdjPtsLine+i*2+1),"\t ");
		nOverlap = atoi(vParts[0]);
		SSWstuAdjPtInfo AdjPt;
		AdjPt.X=X;AdjPt.Y=Y;AdjPt.Z=Z;
		AdjPt.r=r;AdjPt.g=g;AdjPt.b=b;
		AdjPt.nOverlayCount = nOverlap;
		AdjPt.vecImgIdx.resize(nOverlap);
		AdjPt.vecImgPt.resize(nOverlap);
		for (int j = 0; j<nOverlap; j++)
		{
			int nImgId; double px,py;
			nImgId = m_vecBundleImgList2ImgInfo[atoi(vParts[j*3+1])];  //真实索引，空三里面索引排除了无效影像
			px = atof(vParts[j*3+2]);
			py  = atof(vParts[j*3+3]);
			Point2D pt = Point2D(px,py);
			AdjPt.vecImgIdx[j] = nImgId;
			AdjPt.vecImgPt[j] = pt;
		}
		m_vecAdjPts[i] = AdjPt;
		vector<CString>().swap(vParts);
	}
	DeleteCriticalSection(&cs);
	txt.CloseFile();
	FunArrangeAdjPts(m_vecImgInfo,m_vecAdjPts, m_vecPtNumInImgs);
	CString strInfo="正在读取匹配结果文件...";
	if(pFun!=NULL)	pFun(strInfo,97,NULL,0,false);

	if(pAdjPtsLine)delete[]pAdjPtsLine;

	strInfo="正在读取匹配结果文件...";
	if(pFun!=NULL) pFun(strInfo,100,NULL,0,false);
	clock_t t2 = clock();
	double dt = (double)(t2-t1)/CLOCKS_PER_SEC;
	m_dTimeSecsTimeReadAdjRep = dt;
	sprintf(strlog,"匹配结果读取完成：影像数%d，物方点数%d，耗时%.3lfs",nImgNum,nPtNum,m_dTimeSecsTimeReadAdjRep);
	if(pFun!=NULL)
	{
		pFun(strlog,100,NULL,0,true);	
	}
	else
	{
		FunOutPutLogInfo(strlog);
	}
	return true;
}

bool CsswUAVFlyQuaPrj::RunFlyQuaChk(double Z /* = 0 */, SSWFLYQUAMETHOD eMethod /* = BY_ADJUSTMENT */,FlyQuaCallBackFun pFun/* =NULL */)
{ 
	//	if(pFun) pFun("计算质检参数",0,NULL,0,false);
	ClearAdjInfo();
	ClearReportInfo(eMethod);
	if(m_nImgNum==0) return false;
	m_nMatchPointNumThresold = m_FlyQuaParaSet.nMatchPtNumThresold;
	clock_t t1 = clock();
	if(eMethod==BY_MATCH) 
	{
		if(!ReadMatchPoint(pFun))
		{
			if(!pFun) FunOutPutLogInfo("匹配点读取错误，质检计算失败！");
			else pFun("匹配点读取错误，质检计算失败！",0,NULL,0,true);
			return false;
		}
	}
	else if(eMethod==BY_ADJUSTMENT)
	{
		if(!ReadAdjRep(pFun))
		{
			if(!pFun) FunOutPutLogInfo("空三报告读取错误，质检计算失败！");
			else pFun("空三报告读取错误，质检计算失败！",0,NULL,0,true);
			return false;
		}
	}
	if(pFun!=NULL) pFun("质检参数计算：影像倾角",0,NULL,0,true);
	CalImagePitch();
	if(pFun!=NULL) pFun("质检参数计算：影像旋角",10,NULL,0,true);
	CalImageYaw(Z,eMethod/*BY_MATCH*/);
	if(pFun!=NULL) pFun("质检参数计算：影像航高差",20,NULL,0,true);
	CalImageFlyHeiErr(eMethod);
	if(pFun!=NULL) pFun("质检参数计算：影像偏航距",30,NULL,0,true);
	CalImageFlyDisErr(eMethod);
	if(pFun!=NULL) pFun("质检参数计算：影像重叠度",40,NULL,0,true);
	// 	if(eMethod!=BY_POS) CalStripOverlaps(Z,/*eMethod*/BY_MATCH);
	// 	else CalStripOverlaps(Z,BY_POS);
	switch(eMethod)
	{
	case BY_POS:
		CalStripOverlaps(Z,BY_POS);
		break;
	case BY_MATCH:
		CalStripOverlaps(Z,BY_MATCH);
		break;
	case BY_ADJUSTMENT:
		CalStripOverlaps(Z,BY_MATCH);
		break;
	default:
		break;
	}
	if(pFun!=NULL) pFun("质检参数计算：影像POS误差",80,NULL,0,true);
	CalImagePosErr(eMethod);
	if(pFun!=NULL) pFun("质检参数计算：参数统计",90,NULL,0,true);
	AnalysisFlyQuaChkParas(m_nMatchPointNumThresold);
	if(pFun!=NULL) pFun("质检参数计算：漏洞分析",95,NULL,0,true);
	CheckMissArea(Z,eMethod);
	if(pFun!=NULL) pFun("质检参数计算：漏洞分析",100,NULL,0,false);
	CalStripCqPaneErr(eMethod);
	clock_t t2 = clock();
	double dt = (double)(t2-t1)/CLOCKS_PER_SEC;
	m_dTimeSecsRunFlyQuaChk = dt;
	char strlog[MAX_SIZE_FILE_LINE];
	sprintf(strlog,"质检参数统计完毕，耗时%.3lfs",m_dTimeSecsRunFlyQuaChk);
	if(pFun!=NULL) pFun(strlog,100,NULL,0,true);
	else FunOutPutLogInfo(strlog);

	/*************************************/

	FILE*pfWX = fopen(FunGetFileFolder(m_strPrjXmlPath)+"\\AdjPt_X.txt","w");
	FILE*pfWY = fopen(FunGetFileFolder(m_strPrjXmlPath)+"\\AdjPt_Y.txt","w");
	FILE*pfWZ = fopen(FunGetFileFolder(m_strPrjXmlPath)+"\\AdjPt_Z.txt","w");
	FILE*pfWDis = fopen(FunGetFileFolder(m_strPrjXmlPath)+"\\PosDis2Pane.txt","w");
	int nCount = 0;
	if(pfWZ!=NULL)
	{
		for (int i = 0; i<m_vecImgInfo.size(); i++)
		{
			if(!m_vecImgInfo[i].m_bAdjed) continue;
			double Z = m_vecImgInfo[i].m_AdjPosInfo.Zs;
			nCount++;
			fprintf(pfWZ,"%-13.6lf\n",Z);
		}
	}
	nCount = 0;
	for (int i = 0; i<m_vecImgInfo.size(); i++)
	{
		if(!m_vecImgInfo[i].m_bAdjed) continue;
		if(m_vecImgInfo[i].m_ImgAdjPts.vecAdjPtIdx.size()==0) continue;
		int nAdjPtNum = m_vecImgInfo[i].m_ImgAdjPts.vecAdjPtIdx.size();
		for (int j = 0; j<nAdjPtNum; j++)
		{
			if(pfWX!=NULL) fprintf(pfWX,"%-13.6lf\n",m_vecImgInfo[i].m_ImgAdjPts.vecImgPtResX[i]);
			if(pfWY!=NULL) fprintf(pfWY,"%-13.6lf\n",m_vecImgInfo[i].m_ImgAdjPts.vecImgPtResY[i]);
			nCount++;
		}
		if(nCount>0) break;
	}
	double dmsan = 0;
	for (int i =0; i<m_vecDisImgAdjPos2PosPane.size(); i++)
	{
		dmsan+=m_vecDisImgAdjPos2PosPane[i];
	}
	dmsan/=m_vecDisImgAdjPos2PosPane.size();
	double rms = FunCalDataRms(m_vecDisImgAdjPos2PosPane,dmsan);
	if(pfWDis!=NULL)
	{
		//	fprintf(pfWDis,"%-13.6lf  %-13.6lf\n",dmsan,rms);
		for (int i = 0; i<m_vecDisImgAdjPos2PosPane.size(); i++)
		{
			fprintf(pfWDis,"%-13.6lf\n",m_vecDisImgAdjPos2PosPane[i]);
		}
	}
	fclose(pfWX);
	fclose(pfWY);
	fclose(pfWZ);
	fclose(pfWDis);
	return true;
}
bool CsswUAVFlyQuaPrj::SaveReport2File1(CString strCqViewImgPath,CString strOverlapYErrViewImgPath,CString strOverlapXErrViewImgPath,CString strPosErrViewImgPath,
	CString strPitchErrViewImgPath,CString strYawViewImgPath,SSWFLYQUAMETHOD eMethod)
{
	FunOutPutLogInfo("开始写入PDF");
	clock_t t1 = clock();
	m_strFlyQuaChkRepPath = GetCheckRepPath(eMethod);
	char *strLine = new char[MAX_SIZE_FILE_LINE*3]; 
	memset(strLine,0,MAX_SIZE_FILE_LINE*3);
	CsswPdf pdf; 
	if(!pdf.InitPdf())
	{
		FunOutPutLogInfo("PDF初始化失败！");
		return false;
	}
	int nImgIdx =1; 
	char strImgLabelInfo[MAX_SIZE_FILE_LINE]; 
	memset(strImgLabelInfo,0,MAX_SIZE_FILE_LINE);
	int nListCols/* = 2*/, nListRows/* = 5*/;
	vector<CString> vecListInfo;
	if(!pdf.AddPage()) return false;
	pdf.SetLineSpace(50);
	pdf.SetPdfTitle("无人机航摄数据质量检查报告",0,0.6,0);
	CString strLogon = FunGetFileFolder(FunGetThisExePath())+"\\logo.dll";
	int nw = 31, nh = 31;
	int nx = pdf.GetPdfPageWidth()-pdf.GetLineSpace();
	int ny = pdf.GetPdfPageHeight()-pdf.GetTitleGap()-pdf.GetLineGap();
	pdf.AddImage(strLogon,1.0,SSW_NONE,nx,ny,nw,nh); 
	sprintf(strLine,"报告生成时间：%s",FunGetSysTimeStr());
	pdf.AddText(strLine,SSW_SONG,1.2,SSW_LEFT,0,false,0,0,0,0,0,0,true);pdf.AddEmptyLine(-1.2);
	pdf.AddText("@2016 UAV Flight Quality Check",SSW_SONG,1.2,SSW_RIGHT,0,false,0.3,0.3,0.3,0,0,0,true);
	pdf.AddEmptyLine(5);

	/**************测区概述List 2*4 ***************/
	pdf.AddText("测区数据概述",SSW_HEI,2.5,SSW_LEFT);
	pdf.AddEmptyLine(3);

	nListCols = 4; nListRows = 2;
	vecListInfo.resize(nListCols*nListRows);
	vecListInfo[0*nListCols+0].Format("%s","影像总数");
	vecListInfo[0*nListCols+1].Format("%s","有效影像数");
	vecListInfo[0*nListCols+2].Format("%s","无效影像数");
	vecListInfo[0*nListCols+3].Format("%s","航带数");
	vecListInfo[1*nListCols+0].Format("%d",m_nImgNum);
	vecListInfo[1*nListCols+1].Format("%d",m_nImgNum-m_nInvalidNum);
	vecListInfo[1*nListCols+2].Format("%d",m_nInvalidNum);
	vecListInfo[1*nListCols+3].Format("%d",m_nStripNum);
	pdf.AddText("测区影像数量",SSW_SONG,1.5,SSW_LEFT);
	pdf.AddEmptyLine(1);
	cprintf("========List info in report========\n");
	cprintf("ImageInfo...");
	pdf.AddList(nListCols,nListRows,vecListInfo);
	cprintf("...Over\n");
	vector<CString>().swap(vecListInfo);
	pdf.AddEmptyLine(1);
	pdf.AddImage(strCqViewImgPath);
	pdf.AddEmptyLine(0.5);
	sprintf(strImgLabelInfo,"图%d. 测区航带示意图(注：红色点为无效影像；蓝绿颜色表示间隔航带；红色多边形为漏飞区域)",nImgIdx);
	pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
	nImgIdx++;
	pdf.AddEmptyLine(5);
	FunOutPutLogInfo("测区数据概述写入完成！");
	/***********************质检参数阈值*************************/
	nListCols = 3; nListRows = 8;
	vecListInfo.resize(nListCols*nListRows);
	vecListInfo[0*nListCols+0]="质检项";
	vecListInfo[0*nListCols+1]="阈值说明";
	vecListInfo[0*nListCols+2]="阈值大小";
	vecListInfo[1*nListCols+0]="相机参数";
	vecListInfo[1*nListCols+1]="平差后POS平整度偏差最大值(m)";
	vecListInfo[1*nListCols+2].Format("%.2f",m_FlyQuaParaSet.fMaxErrStationPane);
	vecListInfo[2*nListCols+0]="相机参数";
	vecListInfo[2*nListCols+1]="平差后POS平整度偏差中误差(m)";
	vecListInfo[2*nListCols+2].Format("%.2f",m_FlyQuaParaSet.fRmsErrStationPane);
	vecListInfo[3*nListCols+0]="航向重叠度";
	vecListInfo[3*nListCols+1]="影像合格最小航向重叠度";
	vecListInfo[3*nListCols+2].Format("%.2f%%",m_FlyQuaParaSet.fMinOverlapY);
	vecListInfo[4*nListCols+0]="旁向重叠度";
	vecListInfo[4*nListCols+1]="影像合格最小旁向重叠度";
	vecListInfo[4*nListCols+2].Format("%.2f%%",m_FlyQuaParaSet.fMinOverlapX);
	vecListInfo[5*nListCols+0]="影像倾角";
	vecListInfo[5*nListCols+1]="影像合格最大倾角";
	vecListInfo[5*nListCols+2].Format("%.2f",m_FlyQuaParaSet.fMaxPitch);
	vecListInfo[6*nListCols+0]="影像旋角";
	vecListInfo[6*nListCols+1]="影像合格最大旋角";
	vecListInfo[6*nListCols+2].Format("%.2f",m_FlyQuaParaSet.fMaxYaw);
	vecListInfo[7*nListCols+0]="空三困难影像";
	vecListInfo[7*nListCols+1]="空三困难影像判断空三点数最小值";
	vecListInfo[7*nListCols+2].Format("%d",m_nMatchPointNumThresold);
	pdf.AddText("自动质检参数阈值",SSW_SONG,1.5,SSW_LEFT);
	pdf.AddEmptyLine(1);
	cprintf("========List info in report========\n");
	cprintf("Thresold...");
	pdf.AddList(nListCols,nListRows,vecListInfo);
	cprintf("...Over\n");
	vector<CString>().swap(vecListInfo);
	FunOutPutLogInfo("自动质检参数阈值写入完成！");
	pdf.AddEmptyLine(4);
	/***********************质检结果*************************/
	pdf.AddText("质检结果",SSW_HEI,2,SSW_LEFT);
	pdf.AddEmptyLine(1);
	memset(strLine,'-',MAX_SIZE_FILE_LINE*3);
	pdf.AddText(strLine,SSW_SONG,1,SSW_LEFT,0,true,0.7,0.7,0.7);
	pdf.AddEmptyLine(-2);
	if(eMethod==BY_POS) sprintf(strLine,"质检方法——基于POS的快速质检");else
		if(eMethod==BY_MATCH) sprintf(strLine,"质检方法——基于匹配的质检方法");else
			if(eMethod==BY_ADJUSTMENT) sprintf(strLine,"质检方法——基于平差的精确质检");
	pdf.AddText(strLine,SSW_HEI,1.5,SSW_CENTER);
	pdf.AddEmptyLine(4);
	/***********************分布完整性*************************/
	pdf.AddText("数据分布完整性检查",SSW_HEI,2);
	pdf.AddEmptyLine(3);
	sprintf(strLine,"如图%d所示，航飞漏洞块数：%d块",nImgIdx-1,m_vvMissAreaImgIdx.size());
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1,0,0);
	pdf.AddEmptyLine(1.5);
	if(m_vvMissAreaImgIdx.size()>0)
	{
		int nMissImgNum = 0;
		for (int i = 0; i<m_vvMissAreaImgIdx.size(); i++)
		{
			nMissImgNum+=m_vvMissAreaImgIdx[i].size();
			if(i<m_vvMissAreaImgIdx.size()-1) nMissImgNum++;
		}
		sprintf(strLine,"漏飞区相关影像列表");
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1,0,0);
		pdf.AddEmptyLine(1.5);
		nListCols=4;nListRows=nMissImgNum+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("漏洞编号");
		vecListInfo[0*nListCols+1].Format("影像索引");
		vecListInfo[0*nListCols+2].Format("影像名");
		vecListInfo[0*nListCols+3].Format("航带信息");
		int nCount = 1;
		for (int i = 0; i<m_vvMissAreaImgIdx.size(); i++)
		{
			for (int j = 0; j<m_vvMissAreaImgIdx[i].size(); j++)
			{
				vecListInfo[nListCols*nCount+0].Format("%d",i);
				vecListInfo[nListCols*nCount+1].Format("%d",m_vvMissAreaImgIdx[i][j]);
				vecListInfo[nListCols*nCount+2].Format("%s",m_vecImgInfo[m_vvMissAreaImgIdx[i][j]].m_strImgName);
				vecListInfo[nListCols*nCount+3].Format("%s",m_vecImgInfo[m_vvMissAreaImgIdx[i][j]].m_strStripLabel);
				nCount++;
			}
			if(i<m_vvMissAreaImgIdx.size()-1)
			{
				vecListInfo[nListCols*nCount+0].Format("---");
				vecListInfo[nListCols*nCount+1].Format("---");
				vecListInfo[nListCols*nCount+2].Format("---");
				vecListInfo[nListCols*nCount+3].Format("---");
				nCount++;
			}
		}
		cprintf("MissArea...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
	}
	pdf.AddEmptyLine(4);
	FunOutPutLogInfo("漏飞区域写入完成！");
	/***********************相机参数*************************/	
	if(eMethod==BY_ADJUSTMENT)
	{	
		sprintf(strLine,"相机内参数精度分析");
		pdf.AddText(strLine,SSW_HEI,2,SSW_LEFT);
		pdf.AddEmptyLine(3);
		pdf.AddText("相机原始畸变参数",SSW_SONG,1.5,SSW_LEFT);
		pdf.AddEmptyLine(1);
		nListCols = 6; nListRows = 2;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("a");
		vecListInfo[0*nListCols+1].Format("b");
		vecListInfo[0*nListCols+2].Format("k1");
		vecListInfo[0*nListCols+3].Format("k2");
		vecListInfo[0*nListCols+4].Format("p1");
		vecListInfo[0*nListCols+5].Format("p2");
		vecListInfo[1*nListCols+0].Format("%.8lf",m_ImgCmrInfo.a);
		vecListInfo[1*nListCols+1].Format("%.8lf",m_ImgCmrInfo.b);
		vecListInfo[1*nListCols+2].Format("%.8lf",m_ImgCmrInfo.k1);
		vecListInfo[1*nListCols+3].Format("%.8lf",m_ImgCmrInfo.k2);
		vecListInfo[1*nListCols+4].Format("%.8lf",m_ImgCmrInfo.p1);
		vecListInfo[1*nListCols+5].Format("%.8lf",m_ImgCmrInfo.p2);
		cprintf("CameraInfo...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
		pdf.AddEmptyLine(2);
		// 		pdf.AddText("//////////////////////////////////",SSW_SONG,1.5,SSW_CENTER);
		// 		pdf.AddEmptyLine(2);

		sprintf(strLine,"平差结果摄站点距航飞平面，最大距离偏差%.2lfm(阈值%.2f)，距离偏差中误差%.2lfm(阈值%.2f)。",m_dMaxPaneDifErr,m_FlyQuaParaSet.fMaxErrStationPane,m_dRmsPaneDifErr,m_FlyQuaParaSet.fRmsErrStationPane);
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,0);
		pdf.AddEmptyLine(1.5);
		int nPosX = pdf.GetTextWidthInPage(strLine)+pdf.GetLineSpace();
		int nPosY = pdf.GetWriteTopInPage();
		if(m_dMaxPaneDifErr>m_FlyQuaParaSet.fMaxErrStationPane&&m_dRmsPaneDifErr>m_FlyQuaParaSet.fRmsErrStationPane)
		{
			pdf.AddText("相机参数不合格！",SSW_SONG,2,SSW_LEFT,0,false,1);
		}
		else
		{
			pdf.AddText("相机参数合格！",SSW_SONG,2,SSW_LEFT,0,false,1);
		}
		pdf.AddEmptyLine(1.5);
		float *pX = new float[m_nInAdjedNum];
		float *pY = new float[m_nInAdjedNum];
		for (int i = 0; i<m_nInAdjedNum; i++)
		{
			pX[i]=m_vecBundleSuccess2ImgInfo[i];
			pY[i]=m_vecDisImgAdjPos2PosPane[i];
		}
		pdf.AddChart(pY,pX,m_vecDisImgAdjPos2PosPane.size(),0,0,1,0,0,1,0,0,"POS偏离距离","影像索引","POS距离偏离值-m");
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 测区影像平差结果POS偏离图",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(4);
		delete[]pX;
		delete[]pY;
		FunOutPutLogInfo("相机参数精度写入完成！");
		/***********************POS精度*************************/	
		sprintf(strLine,"POS坐标信息精度分析");
		pdf.AddText(strLine,SSW_HEI,2,SSW_LEFT);
		pdf.AddEmptyLine(3);
		sprintf(strLine,"参与平差影像%d张，平差前后POS空间距离(Err)近似中误差 RMS = %.2lfm，均值%.2lfm",
			m_nInAdjedNum,m_dRmsPosErr,m_dMeanPosErr);
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,true);
		pdf.AddEmptyLine(2);
		//	sprintf(strLine,"不合格影像数%d张（Err > 3*RMS+均值 为不合格）",m_vecImgIdxHeiPosErr.size());
		sprintf(strLine,"超过3倍中误差（Err>3*RMS+Mean）影像：%d张",m_vecImgIdxHeiPosErr.size());
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1);
		pdf.AddEmptyLine(1.5);
		if(m_vecImgIdxHeiPosErr.size()>0)
		{	
			nListCols = 6; nListRows = m_vecImgIdxHeiPosErr.size()+1;
			vecListInfo.resize(nListRows*nListCols);
			vecListInfo[0*nListCols+0].Format("影像索引");
			vecListInfo[0*nListCols+1].Format("影像名");
			vecListInfo[0*nListCols+2].Format("X误差-m");
			vecListInfo[0*nListCols+3].Format("Y误差-m");
			vecListInfo[0*nListCols+4].Format("Z误差-m");
			vecListInfo[0*nListCols+5].Format("XYZ误差-m");
			for (int i = 0; i<nListRows-1; i++)
			{
				vecListInfo[(i+1)*nListCols+0].Format("%d",m_vecImgIdxHeiPosErr[i]);
				vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[m_vecImgIdxHeiPosErr[i]].m_strImgName);
				vecListInfo[(i+1)*nListCols+2].Format("%.2lf",m_vecAdjImgPosErr[m_vecBundleIdxHeiPosErr[i]].Xs);
				vecListInfo[(i+1)*nListCols+3].Format("%.2lf",m_vecAdjImgPosErr[m_vecBundleIdxHeiPosErr[i]].Ys);
				vecListInfo[(i+1)*nListCols+4].Format("%.2lf",m_vecAdjImgPosErr[m_vecBundleIdxHeiPosErr[i]].Zs);
				vecListInfo[(i+1)*nListCols+5].Format("%.2lf",m_vecAdjImgPosDisErr[m_vecBundleIdxHeiPosErr[i]]);
			}
			cprintf("PosInfo...");
			pdf.AddList(nListCols,nListRows,vecListInfo);
			cprintf("...Over\n");
			vector<CString>().swap(vecListInfo);
			pdf.AddEmptyLine(1.5);

			pdf.AddEmptyLine(1);
			pdf.AddImage(strPosErrViewImgPath,2);
			pdf.AddEmptyLine(0.5);
			sprintf(strImgLabelInfo,"图%d. POS误差超限影像分布(注：蓝色点为超限影像，绿色点为非超限影像)",nImgIdx);
			pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
			nImgIdx++;
			pdf.AddEmptyLine(3);
		}
		pX = new float[m_nInAdjedNum];
		pY = new float[m_nInAdjedNum];
		for (int i = 0; i<m_nInAdjedNum; i++)
		{
			pX[i]=m_vecBundleSuccess2ImgInfo[i];
			pY[i]=m_vecAdjImgPosDisErr[i];
		}
		pdf.AddChart(pY,pX,m_nInAdjedNum,0,0,1,0,0,1,0,0,"POS误差曲线","影像索引","POS距离误差值-m");
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 测区影像原始POS误差曲线图",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(4);
		delete[]pX;
		delete[]pY;
	}
	FunOutPutLogInfo("POS精度写入完成！");
	/***********************重叠度*************************/
	sprintf(strLine,"影像重叠度信息");
	pdf.AddText(strLine,SSW_HEI,2,SSW_LEFT);
	pdf.AddEmptyLine(3);
	CString strOverlapImg = FunGetFileFolder(strCqViewImgPath);
	if(eMethod==BY_ADJUSTMENT) strOverlapImg+="\\OverlapImg_ByBundle.jpg";else
		if(eMethod==BY_MATCH) strOverlapImg+="\\OverlapImg_ByMatch.jpg";else
			if(eMethod==BY_POS) strOverlapImg+="\\OverlapImg_ByPos.jpg";
	pdf.AddImage(strOverlapImg,1,SSW_CENTER,0,0,0,0,true);
	//pdf.AddImage(FunGetFileFolder(strViewImgPath)+"\\OverlapNote.jpg",1.0,SSW_NONE,0,350,40);
	pdf.AddEmptyLine(0.5);
	sprintf(strImgLabelInfo,"图%d. 测区重叠度示意图",nImgIdx);
	pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
	nImgIdx++;
	pdf.AddEmptyLine(3);

	double dMaxOverlapX, dMinOverlapX,dMaxOverlapY, dMinOverlapY;
	double dMeanOverlapX=0,dMeanOverlapY=0;
	dMaxOverlapX=dMaxOverlapY=-1;
	dMinOverlapX=dMinOverlapY=10;
	float *pOverlapY,*pOverlapX,*pX;
	pOverlapX=new float[m_nImgNum-m_nInvalidNum];
	pOverlapY=new float[m_nImgNum-m_nInvalidNum];
	pX=new float[m_nImgNum-m_nInvalidNum];
	int nCount = 0;
	for (int i = 0; i<m_vvOverlapsBetStrips.size(); i++)
	{
		for (int j = 0; j<m_vvOverlapsBetStrips[i].size(); j++)
		{
			int nStripIdx = i;
			int nIdxInstrip = j;
			dMaxOverlapX = max(dMaxOverlapX,m_vvOverlapsBetStrips[nStripIdx][nIdxInstrip]);
			dMinOverlapX = min(dMinOverlapX,m_vvOverlapsBetStrips[nStripIdx][nIdxInstrip]);
			dMaxOverlapY = max(dMaxOverlapY,m_vvOverlapsInsStrips[nStripIdx][nIdxInstrip]);
			dMinOverlapY = min(dMinOverlapY,m_vvOverlapsInsStrips[nStripIdx][nIdxInstrip]);
			dMeanOverlapX+=m_vvOverlapsBetStrips[nStripIdx][nIdxInstrip];
			dMeanOverlapY+=m_vvOverlapsInsStrips[nStripIdx][nIdxInstrip];
			*(pOverlapX+nCount)=m_vvOverlapsBetStrips[nStripIdx][nIdxInstrip]*100;
			*(pOverlapY+nCount)=m_vvOverlapsInsStrips[nStripIdx][nIdxInstrip]*100;
			*(pX+nCount)=GetImgIdx(nStripIdx,nIdxInstrip);
			nCount++;
		}
	}
	if(nCount>0)
	{
		dMeanOverlapX/=nCount;
		dMeanOverlapY/=nCount;
		sprintf(strLine,"单张影像按最大重叠度进行统计：");
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT);
		pdf.AddEmptyLine(2);
		sprintf(strLine,"航向重叠度最大值%3.2lf%%，最小值%3.2lf%%，平均值%3.2lf%%；",dMaxOverlapY*100,dMinOverlapY*100,dMeanOverlapY*100);
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT);
		pdf.AddEmptyLine(2);
		sprintf(strLine,"旁向重叠度最大值%3.2lf%%，最小值%3.2lf%%，平均值%3.2lf%%。",dMaxOverlapX*100,dMinOverlapX*100,dMeanOverlapX*100);					
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT);
		pdf.AddEmptyLine(3);
	}
	sprintf(strLine,"航向重叠度不合格（< %.2lf%%）影像：%d张",m_FlyQuaParaSet.fMinOverlapY,m_vecImgIdxLowOverlapInsStrips.size());
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1,0,0);
	if(m_vecImgIdxLowOverlapInsStrips.size()>0)
	{
		pdf.AddEmptyLine(1);
		nListCols=4;nListRows=m_vecImgIdxLowOverlapInsStrips.size()+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		vecListInfo[0*nListCols+2].Format("航带信息");
		vecListInfo[0*nListCols+3].Format("航向重叠度");
		for (int i = 0; i<nListRows-1; i++)
		{
			int nImgIdx = m_vecImgIdxLowOverlapInsStrips[i];
			double dOverlap=
				m_vvOverlapsInsStrips[m_vecImgInfo[nImgIdx].m_nStripIdx][m_vecImgInfo[nImgIdx].m_nIdxInStrip];
			vecListInfo[(i+1)*nListCols+0].Format("%d",nImgIdx);
			vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[nImgIdx].m_strImgName);
			vecListInfo[(i+1)*nListCols+2].Format("%s",m_vecImgInfo[nImgIdx].m_strStripLabel);
			vecListInfo[(i+1)*nListCols+3].Format("%.2lf%%",dOverlap*100);
		}
		cprintf("Overlap_Y...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
		pdf.AddEmptyLine(1);
		pdf.AddImage(strOverlapYErrViewImgPath,2);
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 航向重叠度不合格影像分布示意图(注：蓝色点为超限影像，绿色点为合格影像)",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(3);
	}
	FunOutPutLogInfo("航向重叠度写入完成！");
	pdf.AddEmptyLine(3);
	sprintf(strLine,"旁向重叠度不合格（< %.2lf%%）影像：%d张",m_FlyQuaParaSet.fMinOverlapX,m_vecImgIdxLowOverlapBetStrips.size());
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1,0,0);
	char strlog[MAX_SIZE_FILE_LINE];
	// 	sprintf(strlog,"size = %d",m_vecImgIdxLowOverlapBetStrips.size());
	// 	FunOutPutLogInfo(strlog);
	if(m_vecImgIdxLowOverlapBetStrips.size()>0)
	{
		pdf.AddEmptyLine(1);
		nListCols=4;nListRows=m_vecImgIdxLowOverlapBetStrips.size()+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		vecListInfo[0*nListCols+2].Format("航带信息");
		vecListInfo[0*nListCols+3].Format("旁向重叠度");
		for (int i = 0; i<nListRows-1; i++)
		{
			int nImgIdx = m_vecImgIdxLowOverlapBetStrips[i];
			double dOverlap=
				m_vvOverlapsBetStrips[m_vecImgInfo[nImgIdx].m_nStripIdx][m_vecImgInfo[nImgIdx].m_nIdxInStrip];
			vecListInfo[(i+1)*nListCols+0].Format("%d",nImgIdx);
			vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[nImgIdx].m_strImgName);
			vecListInfo[(i+1)*nListCols+2].Format("%s",m_vecImgInfo[nImgIdx].m_strStripLabel);
			vecListInfo[(i+1)*nListCols+3].Format("%.2lf%%",dOverlap*100);
		}
		//////////////////////////////////////////////////////////
		cprintf("Overlap_X...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
		pdf.AddEmptyLine(1.5);
		pdf.AddEmptyLine(1);
		pdf.AddImage(strOverlapXErrViewImgPath,2);
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 旁向重叠度不合格影像分布示意图(注：蓝色点为超限影像，绿色点为合格影像)",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(3);
	}
	pdf.AddEmptyLine(3);
	if(nCount>0)
	{
		FunSortByX(nCount,pX,pOverlapY,pOverlapX);
		pdf.AddChart(pOverlapY,pX,nCount,0,0,1,0,0,1.0,0,0,"航向重叠度","影像索引","重叠度(%)");
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 影像航向重叠度统计图",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(1.5);
		pdf.AddChart(pOverlapX,pX,m_nImgNum-m_nInvalidNum,0,0,1,0,0,1.0,0,0,"旁向重叠度","影像索引","重叠度(%)");
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 影像旁向重叠度统计图",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
	}
	pdf.AddEmptyLine(4);
	delete[]pOverlapX;	
	delete[]pOverlapY;
	delete[]pX;
	FunOutPutLogInfo("旁向重叠度写入完成！");
	/***********************其他质检信息*************************/
	pdf.AddText("其他质检信息",SSW_HEI,2);
	pdf.AddEmptyLine(3);

	sprintf(strLine,"倾角不合格（> %.2lfdeg）影像：%d张",m_FlyQuaParaSet.fMaxPitch,m_vecImgIdxHeiPitch.size());
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1);
	pdf.AddEmptyLine(1);
	if(m_vecImgIdxHeiPitch.size()>0)
	{	
		nListCols=3;nListRows=m_vecImgIdxHeiPitch.size()+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		vecListInfo[0*nListCols+2].Format("倾角（deg）");
		for (int i = 0; i<nListRows-1; i++)
		{
			int nImgIdx = m_vecImgIdxHeiPitch[i];
			vecListInfo[(i+1)*nListCols+0].Format("%d",nImgIdx);
			vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[nImgIdx].m_strImgName);
			vecListInfo[(i+1)*nListCols+2].Format("%.2lf",m_vecImagePitch[nImgIdx]);
		}
		cprintf("PitchInfo...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
		pdf.AddEmptyLine(1);
		pdf.AddEmptyLine(1);
		pdf.AddImage(strPitchErrViewImgPath,2);
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 倾角不合格影像分布示意图(注：蓝色点为超限影像，绿色点为合格影像)",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(3);
	}	
	float *pPitch = new float[m_nImgNum-m_nInvalidNum];
	pX = new float[m_nImgNum-m_nInvalidNum];
	nCount=0;
	for (int i = 0; i<m_vecImagePitch.size(); i++)
	{
		if(!m_vecImgInfo[i].m_bValid)continue;
		*(pPitch+nCount)=m_vecImagePitch[i];
		*(pX+nCount)=i;
		nCount++;
	}
	if(nCount>0)
	{
		pdf.AddChart(pPitch,pX,nCount,0,0,1,0,0,1,0,0,"影像倾角","影像索引","倾角值（deg）");
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 影像倾角曲线图",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(3);
	}
	FunOutPutLogInfo("倾角写入完成！");
	sprintf(strLine,"旋角不合格（> %.2lfdeg）影像：%d张",m_FlyQuaParaSet.fMaxYaw,m_vecImgIdxHeiYaw.size());
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1);
	pdf.AddEmptyLine(1);
	if(m_vecImgIdxHeiYaw.size()>0)
	{
		nListCols=3;nListRows=m_vecImgIdxHeiYaw.size()+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		vecListInfo[0*nListCols+2].Format("旋角（deg）");
		for (int i = 0; i<nListRows-1; i++)
		{
			int nImgIdx = m_vecImgIdxHeiYaw[i];
			vecListInfo[(i+1)*nListCols+0].Format("%d",nImgIdx);
			vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[nImgIdx].m_strImgName);
			vecListInfo[(i+1)*nListCols+2].Format("%.2lf",m_vvStripImageYaw[m_vecImgInfo[nImgIdx].m_nStripIdx][m_vecImgInfo[nImgIdx].m_nIdxInStrip]);
		}
		cprintf("YawInfo...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
		pdf.AddEmptyLine(1);
		pdf.AddImage(strYawViewImgPath,2);
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 旋角不合格影像分布示意图(注：蓝色点为超限影像，绿色点为合格影像)",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(3);
	}
	float *pYaw = new float[m_nImgNum-m_nInvalidNum];
	memset(pYaw,0,sizeof(float)*(m_nImgNum-m_nInvalidNum));
	nCount = 0;
	for (int i = 0; i<m_vvStripImageYaw.size(); i++)
	{
		for (int j = 0; j<m_vvStripImageYaw[i].size(); j++)
		{
			int nStripIdx = i;
			int nIdxInStrip = j;
			*(pYaw+nCount)=m_vvStripImageYaw[nStripIdx][nIdxInStrip];
			*(pX+nCount)=GetImgIdx(nStripIdx,nIdxInStrip);
			nCount++;
		}
	}
	FunSortByX(nCount,pX,pYaw);
	if(nCount>0)
	{
		pdf.AddChart(pYaw,pX,m_nImgNum-m_nInvalidNum,0,0,1,0,0,1,0,0,"影像旋角","影像索引","旋角值（deg）");
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 影像旋角曲线图",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
	}
	delete[]pYaw;
	delete[]pPitch;
	delete[]pX;
	FunOutPutLogInfo("旋角写入完成！");
	pdf.AddEmptyLine(3);
	sprintf(strLine,"飞行方向调整处无效影像：%d张",m_nInvalidNum);
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1);
	pdf.AddEmptyLine(1);
	if(m_nInvalidNum>0) 
	{
		nListCols = 2; nListRows = m_nInvalidNum+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		int nCount =0;
		for (int i = 0; i<m_nImgNum; i++)
		{
			if(!m_vecImgInfo[i].m_bValid)
			{
				vecListInfo[(nCount+1)*nListCols+0].Format("%d",i);
				vecListInfo[(nCount+1)*nListCols+1].Format("%s",m_vecImgInfo[i].m_strImgName);
				nCount++;
			}
		}
		cprintf("InvalidImg...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
	}
	FunOutPutLogInfo("飞行调整影像写入完成！");
	pdf.AddEmptyLine(3);
	if(eMethod!=BY_POS)
	{
		sprintf(strLine,"空三困难（缩略图空三点数小于%d）影像：%d张",m_nMatchPointNumThresold,m_vecImgIdxLowMatchPointNum.size());
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1);
		pdf.AddEmptyLine(1);
		if(m_vecImgIdxLowMatchPointNum.size()>0)
		{
			nListCols = 3; nListRows = m_vecImgIdxLowMatchPointNum.size()+1;
			vecListInfo.resize(nListCols*nListRows);
			vecListInfo[0*nListCols+0].Format("影像索引");
			vecListInfo[0*nListCols+1].Format("影像名");
			vecListInfo[0*nListCols+2].Format("空三点数");
			for (int i = 0; i<m_vecImgIdxLowMatchPointNum.size(); i++)
			{
				vecListInfo[(i+1)*nListCols+0].Format("%d",m_vecImgIdxLowMatchPointNum[i]);
				vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[m_vecImgIdxLowMatchPointNum[i]].m_strImgName);
				vecListInfo[(i+1)*nListCols+2].Format("%d",m_vecImgInfo[m_vecImgIdxLowMatchPointNum[i]].m_ImgAdjPts.vecAdjPtIdx.size());
			}
			cprintf("MatchErr...");
			pdf.AddList(nListCols,nListRows,vecListInfo);
			cprintf("...Over\n");
			vector<CString>().swap(vecListInfo);
			pdf.AddEmptyLine(1);
		}
		pdf.AddEmptyLine(3);
		FunOutPutLogInfo("空三困难影像写入完成！");
	}
	/***********************其他质检信息*************************/
	pdf.AddText("质检处理耗时统计",SSW_HEI,2);
	pdf.AddEmptyLine(1);
	clock_t t2 = clock();
	double dt = (double)(t2-t1)/CLOCKS_PER_SEC;
	m_dTimeSecsWriteChkRep=dt;
	m_dTimeSecsWholeProcess=m_dTimeSecsCmrDistortion+m_dTimeSecsBundleAdjustment+m_dTimeSecsTimeReadAdjRep+m_dTimeSecsRunFlyQuaChk+m_dTimeSecsWriteChkRep;
	nListCols = 2; nListRows = 7;
	vecListInfo.resize(nListCols*nListRows);
	vecListInfo[0*nListCols+0].Format("处理过程");
	vecListInfo[0*nListCols+1].Format("耗时-s");
	vecListInfo[1*nListCols+0].Format("完整流程");
	vecListInfo[1*nListCols+1].Format("%.3lf",m_dTimeSecsWholeProcess);
	vecListInfo[2*nListCols+0].Format("畸变改正");
	vecListInfo[2*nListCols+1].Format("%.3lf",m_dTimeSecsCmrDistortion);
	vecListInfo[3*nListCols+0].Format("匹配平差");
	vecListInfo[3*nListCols+1].Format("%.3lf",m_dTimeSecsBundleAdjustment);
	vecListInfo[4*nListCols+0].Format("解析平差报告");
	vecListInfo[4*nListCols+1].Format("%.3lf",m_dTimeSecsTimeReadAdjRep);
	vecListInfo[5*nListCols+0].Format("计算质检参数");
	vecListInfo[5*nListCols+1].Format("%.3lf",m_dTimeSecsRunFlyQuaChk);
	vecListInfo[6*nListCols+0].Format("保存质检报告");
	vecListInfo[6*nListCols+1].Format("%.3lf",m_dTimeSecsWriteChkRep);
	cprintf("TimeInfo...");
	bool b = pdf.AddList(nListCols,nListRows,vecListInfo);
	cprintf("...Over\n");
	vector<CString>().swap(vecListInfo);
	pdf.AddEmptyLine(1);
	FunOutPutLogInfo("报告写入完毕！");
	CString strPath = m_strFlyQuaChkRepPath;

	if(!pdf.SavePdf(strPath))
		return false;
	delete[]strLine;
	return true;
}
bool CsswUAVFlyQuaPrj::SaveReport2File_20170108(CString strCqViewImgPath,CString strOverlapYErrViewImgPath,CString strOverlapXErrViewImgPath,CString strPosErrViewImgPath, 
	CString strPitchErrViewImgPath,CString strYawViewImgPath,SSWFLYQUAMETHOD eMethod, float fOverlap4Ratio)
{
	FunOutPutLogInfo("开始写入PDF");
	clock_t t1 = clock();
	m_strFlyQuaChkRepPath = GetCheckRepPath(eMethod);
	char *strLine = new char[MAX_SIZE_FILE_LINE*3]; 
	memset(strLine,0,MAX_SIZE_FILE_LINE*3);
	CsswPdf pdf; 
	if(!pdf.InitPdf())
	{
		FunOutPutLogInfo("PDF初始化失败！");
		return false;
	}
	int nImgIdx =1; 
	char strImgLabelInfo[MAX_SIZE_FILE_LINE]; 
	memset(strImgLabelInfo,0,MAX_SIZE_FILE_LINE);
	int nListCols/* = 2*/, nListRows/* = 5*/;
	vector<CString> vecListInfo;
	if(!pdf.AddPage()) return false;
	pdf.SetLineSpace(50);
	pdf.SetPdfTitle("无人机航摄数据质量检查报告",0,0.6,0);
	CString strLogon = FunGetFileFolder(FunGetThisExePath())+"\\logo.dll";
	int nw = 31, nh = 31;
	int nx = pdf.GetPdfPageWidth()-pdf.GetLineSpace();
	int ny = pdf.GetPdfPageHeight()-pdf.GetTitleGap()-pdf.GetLineGap();
	pdf.AddImage(strLogon,1.0,SSW_NONE,nx,ny,nw,nh); 
	sprintf(strLine,"报告生成时间：%s",FunGetSysTimeStr());
	pdf.AddText(strLine,SSW_SONG,1.2,SSW_LEFT,0,false,0,0,0,0,0,0,true);pdf.AddEmptyLine(-1.2);
	pdf.AddText("@2016 UAV Flight Quality Check",SSW_SONG,1.2,SSW_RIGHT,0,false,0.3,0.3,0.3,0,0,0,true);
	pdf.AddEmptyLine(4);

	// 	
	/**************测区概述List 2*4 ***************/
	pdf.AddText("测区数据概述",SSW_HEI,2.5,SSW_LEFT);
	pdf.AddEmptyLine(3);

	nListCols = 4; nListRows = 2;
	vecListInfo.resize(nListCols*nListRows);
	vecListInfo[0*nListCols+0].Format("%s","影像总数");
	vecListInfo[0*nListCols+1].Format("%s","有效影像数");
	vecListInfo[0*nListCols+2].Format("%s","无效影像数");
	vecListInfo[0*nListCols+3].Format("%s","航带数");
	vecListInfo[1*nListCols+0].Format("%d",m_nImgNum);
	vecListInfo[1*nListCols+1].Format("%d",m_nImgNum-m_nInvalidNum);
	vecListInfo[1*nListCols+2].Format("%d",m_nInvalidNum);
	vecListInfo[1*nListCols+3].Format("%d",m_nStripNum);
	pdf.AddText("测区影像数量",SSW_SONG,1.5,SSW_LEFT);
	pdf.AddEmptyLine(1);
	cprintf("========List info in report========\n");
	cprintf("ImageInfo...");
	pdf.AddList(nListCols,nListRows,vecListInfo);
	cprintf("...Over\n");
	vector<CString>().swap(vecListInfo);
	pdf.AddEmptyLine(1);
	pdf.AddImage(strCqViewImgPath);
	pdf.AddEmptyLine(0.5);
	sprintf(strImgLabelInfo,"图%d. 测区航带示意图(注：红色点为无效影像，蓝绿颜色表示间隔航带；红色多边形为漏飞区域)",nImgIdx);
	pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
	nImgIdx++;
	pdf.AddEmptyLine(5);
	FunOutPutLogInfo("测区数据概述写入完成！");
	/***********************质检参数阈值*************************/
	nListCols = 3; nListRows = 10;
	vecListInfo.resize(nListCols*nListRows);
	vecListInfo[0*nListCols+0]="质检项";
	vecListInfo[0*nListCols+1]="阈值说明";
	vecListInfo[0*nListCols+2]="阈值大小";
	vecListInfo[1*nListCols+0]="相机参数";
	vecListInfo[1*nListCols+1]="平差后POS平整度偏差最大值(m)";
	vecListInfo[1*nListCols+2].Format("%.2f",m_FlyQuaParaSet.fMaxErrStationPane);
	vecListInfo[2*nListCols+0]="相机参数";
	vecListInfo[2*nListCols+1]="平差后POS平整度偏差中误差(m)";
	vecListInfo[2*nListCols+2].Format("%.2f",m_FlyQuaParaSet.fRmsErrStationPane);
	vecListInfo[3*nListCols+0]="航向重叠度";
	vecListInfo[3*nListCols+1]="影像合格最小航向重叠度";
	vecListInfo[3*nListCols+2].Format("%.2f%%",m_FlyQuaParaSet.fMinOverlapY);
	vecListInfo[4*nListCols+0]="旁向重叠度";
	vecListInfo[4*nListCols+1]="影像合格最小旁向重叠度";
	vecListInfo[4*nListCols+2].Format("%.2f%%",m_FlyQuaParaSet.fMinOverlapX);
	vecListInfo[5*nListCols+0]="影像倾角";
	vecListInfo[5*nListCols+1]="影像合格最大倾角";
	vecListInfo[5*nListCols+2].Format("%.2f",m_FlyQuaParaSet.fMaxPitch);
	vecListInfo[6*nListCols+0]="影像旋角";
	vecListInfo[6*nListCols+1]="影像合格最大旋角";
	vecListInfo[6*nListCols+2].Format("%.2f",m_FlyQuaParaSet.fMaxYaw);
	vecListInfo[7*nListCols+0]="空三困难影像";
	vecListInfo[7*nListCols+1]="空三困难影像判断空三点数最小值";
	vecListInfo[7*nListCols+2].Format("%d",m_nMatchPointNumThresold);
	vecListInfo[8*nListCols+0]="航飞完整度";
	vecListInfo[8*nListCols+1]="航飞漏洞面积占比可接受最大值";
	vecListInfo[8*nListCols+2].Format("%.2f%%",m_FlyQuaParaSet.fMissAreaRatio);
	vecListInfo[9*nListCols+0]="影像姿态";
	vecListInfo[9*nListCols+1]="异常影像占比可接受最大值";
	vecListInfo[9*nListCols+2].Format("%.2f%%",m_FlyQuaParaSet.fAngleErrRatio);
	pdf.AddText("自动质检参数阈值",SSW_SONG,1.5,SSW_LEFT);
	pdf.AddEmptyLine(1);
	cprintf("========List info in report========\n");
	cprintf("Thresold...");
	pdf.AddList(nListCols,nListRows,vecListInfo);
	cprintf("...Over\n");
	vector<CString>().swap(vecListInfo);
	FunOutPutLogInfo("自动质检参数阈值写入完成！");
	pdf.AddEmptyLine(4);
	/************************************************************************/
	/*1- 质检结果                                                           */
	/************************************************************************/
	pdf.AddText("质检结果",SSW_HEI,2,SSW_LEFT);
	pdf.AddEmptyLine(1);
	memset(strLine,'-',MAX_SIZE_FILE_LINE*3);
	pdf.AddText(strLine,SSW_SONG,1,SSW_LEFT,0,true,0.7,0.7,0.7);
	pdf.AddEmptyLine(-2);
	if(eMethod==BY_POS) sprintf(strLine,"质检方法——基于POS的快速质检");else
		if(eMethod==BY_MATCH) sprintf(strLine,"质检方法——基于匹配的质检方法");else
			if(eMethod==BY_ADJUSTMENT) sprintf(strLine,"质检方法——基于平差的精确质检");
	pdf.AddText(strLine,SSW_HEI,1.5,SSW_CENTER);
	pdf.AddEmptyLine(4);

	/***********************关键结果列表*************************/	
	nListCols=3;nListRows=6;
	vecListInfo.resize(nListCols*nListRows);
	vecListInfo[0*nListCols+0].Format("质检项");
	vecListInfo[0*nListCols+1].Format("检查结果");
	vecListInfo[0*nListCols+2].Format("备注信息");

	vecListInfo[1*nListCols+0].Format("相机参数");
	if(eMethod!=BY_ADJUSTMENT||!m_bCmrFile)
	{
		vecListInfo[1*nListCols+1].Format("无");
		vecListInfo[1*nListCols+2].Format("相机参数缺失");
	}
	else
	{
		if(m_dMaxPaneDifErr>m_FlyQuaParaSet.fMaxErrStationPane&&m_dRmsPaneDifErr>m_FlyQuaParaSet.fRmsErrStationPane)
		{
			vecListInfo[1*nListCols+1].Format("不合格");
			vecListInfo[1*nListCols+2].Format("建议调整");
		}
		else
		{
			vecListInfo[1*nListCols+1].Format("合格");
			vecListInfo[1*nListCols+2].Format("无");
		}
	}
	vecListInfo[2*nListCols+0].Format("航飞漏洞");
	if (m_vvMissAreaImgIdx.size()==0)
	{
		vecListInfo[2*nListCols+1].Format("合格");
		vecListInfo[2*nListCols+2].Format("无");
	}
	else
	{
		if(m_dMissAreaRatio>m_FlyQuaParaSet.fMissAreaRatio/100)
		{
			vecListInfo[2*nListCols+1].Format("不合格");
			vecListInfo[2*nListCols+2].Format("漏洞面积占比%3.2lf%%，建议补飞",m_dMissAreaRatio*100);
		}else
		{
			vecListInfo[2*nListCols+1].Format("合格");
			vecListInfo[2*nListCols+2].Format("漏洞面积占比%3.2lf%%，可接受",m_dMissAreaRatio*100);
		}
	}
	vecListInfo[3*nListCols+0].Format("影像重叠度");
	if (fOverlap4Ratio>0.5)
	{
		vecListInfo[3*nListCols+1].Format("合格");
		vecListInfo[3*nListCols+2].Format("无");
	}
	else
	{
		vecListInfo[3*nListCols+1].Format("不合格");
		vecListInfo[3*nListCols+2].Format("中心区重叠度不足，建议补飞");
	}
	vecListInfo[4*nListCols+0].Format("影像姿态角");
	float fAngleRatio = (m_vecImgIdxHeiYaw.size()+m_vecImgIdxHeiPitch.size())*1.0/(m_nImgNum-m_nInvalidNum);
	if (fAngleRatio<m_FlyQuaParaSet.fAngleErrRatio/100)
	{
		vecListInfo[4*nListCols+1].Format("合格");
		vecListInfo[4*nListCols+2].Format("无");
	}
	else
	{
		vecListInfo[4*nListCols+1].Format("不合格");
		vecListInfo[4*nListCols+2].Format("姿态角超限影像比%3.2lf%%，建议重飞",fAngleRatio*100);
	}
	vecListInfo[5*nListCols+0].Format("空三困难影像");
	if (m_vecImgIdxLowMatchPointNum.size()==0)
	{
		vecListInfo[5*nListCols+1].Format("0");
		vecListInfo[5*nListCols+2].Format("无");
	}
	else
	{
		vecListInfo[5*nListCols+1].Format("%d张",m_vecImgIdxLowMatchPointNum.size());
		vecListInfo[5*nListCols+2].Format("详见空三困难影像列表");
	}
	pdf.AddText("关键项列表",SSW_SONG,1.5,SSW_LEFT);
	pdf.AddEmptyLine(1);
	cprintf("Result...");
	pdf.AddList(nListCols,nListRows,vecListInfo);
	cprintf("...Over\n");
	vector<CString>().swap(vecListInfo);
	FunOutPutLogInfo("自动质检结果写入完成！");
	pdf.AddEmptyLine(4);

	// 	pdf.AddText("质检详情",SSW_SONG,1.5,SSW_LEFT);
	// 	pdf.AddEmptyLine(1);
	/***********************相机参数*************************/	
	if(eMethod==BY_ADJUSTMENT)
	{	
		sprintf(strLine,"相机内参数精度分析");
		pdf.AddText(strLine,SSW_HEI,1.5,SSW_LEFT);
		pdf.AddEmptyLine(2);
		if(m_bCmrFile)
		{
			pdf.AddText("相机原始畸变参数",SSW_SONG,1.5,SSW_LEFT);
			pdf.AddEmptyLine(1);
			nListCols = 6; nListRows = 2;
			vecListInfo.resize(nListCols*nListRows);
			vecListInfo[0*nListCols+0].Format("a");
			vecListInfo[0*nListCols+1].Format("b");
			vecListInfo[0*nListCols+2].Format("k1");
			vecListInfo[0*nListCols+3].Format("k2");
			vecListInfo[0*nListCols+4].Format("p1");
			vecListInfo[0*nListCols+5].Format("p2");
			vecListInfo[1*nListCols+0].Format("%.8lf",m_ImgCmrInfo.a);
			vecListInfo[1*nListCols+1].Format("%.8lf",m_ImgCmrInfo.b);
			vecListInfo[1*nListCols+2].Format("%.8lf",m_ImgCmrInfo.k1);
			vecListInfo[1*nListCols+3].Format("%.8lf",m_ImgCmrInfo.k2);
			vecListInfo[1*nListCols+4].Format("%.8lf",m_ImgCmrInfo.p1);
			vecListInfo[1*nListCols+5].Format("%.8lf",m_ImgCmrInfo.p2);
			cprintf("CameraInfo...");
			pdf.AddList(nListCols,nListRows,vecListInfo);
			cprintf("...Over\n");
			vector<CString>().swap(vecListInfo);
			pdf.AddEmptyLine(2);
			sprintf(strLine,"平差结果摄站点距航飞平面，最大偏差%.2lfm(阈值%.2f)，偏差中误差%.2lfm(阈值%.2f)。",m_dMaxPaneDifErr,m_FlyQuaParaSet.fMaxErrStationPane,m_dRmsPaneDifErr,m_FlyQuaParaSet.fRmsErrStationPane);
			pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,0);
			pdf.AddEmptyLine(1.5);
			int nPosX = pdf.GetTextWidthInPage(strLine)+pdf.GetLineSpace();
			int nPosY = pdf.GetWriteTopInPage();
			if(m_dMaxPaneDifErr>m_FlyQuaParaSet.fMaxErrStationPane&&m_dRmsPaneDifErr>m_FlyQuaParaSet.fRmsErrStationPane)
			{
				pdf.AddText("相机参数不合格！",SSW_SONG,1.5,SSW_LEFT,0,false,1);
			}
			else
			{
				pdf.AddText("相机参数合格！",SSW_SONG,1.5,SSW_LEFT,0,false,1);
			}
			pdf.AddEmptyLine(1.5);
			float *pX = new float[m_nInAdjedNum];
			float *pY = new float[m_nInAdjedNum];
			for (int i = 0; i<m_nInAdjedNum; i++)
			{
				pX[i]=m_vecBundleSuccess2ImgInfo[i];
				pY[i]=m_vecDisImgAdjPos2PosPane[i];
			}
			pdf.AddChart(pY,pX,m_vecDisImgAdjPos2PosPane.size(),0,0,1,0,0,1,0,0,"POS偏离距离","影像索引","POS距离偏离值-m");
			pdf.AddEmptyLine(0.5);
			sprintf(strImgLabelInfo,"图%d. 测区影像平差结果POS偏离图",nImgIdx);
			pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
			nImgIdx++;
			delete[]pX;
			delete[]pY;
		}
		else
		{
			pdf.AddText("无相机参数信息！",SSW_SONG,1.5,SSW_LEFT,0,false,1);
		}
		FunOutPutLogInfo("相机参数精度写入完成！");
		pdf.AddEmptyLine(3);
	}
	/***********************POS精度*************************/	
	if(eMethod==BY_ADJUSTMENT)
	{
		sprintf(strLine,"POS坐标信息精度分析");
		pdf.AddText(strLine,SSW_HEI,1.5,SSW_LEFT);
		pdf.AddEmptyLine(2);
		sprintf(strLine,"参与平差影像%d张，平差前后POS空间距离(Err)近似中误差 RMS = %.2lfm，均值%.2lfm",
			m_nInAdjedNum,m_dRmsPosErr,m_dMeanPosErr);
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,true);
		pdf.AddEmptyLine(1);
		//	sprintf(strLine,"不合格影像数%d张（Err > 3*RMS+均值 为不合格）",m_vecImgIdxHeiPosErr.size());
		sprintf(strLine,"超过3倍中误差（Err>3*RMS+Mean）影像：%d张",m_vecImgIdxHeiPosErr.size());
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1);
		pdf.AddEmptyLine(1.5);
		if(m_vecImgIdxHeiPosErr.size()>0)
		{	
			nListCols = 6; nListRows = m_vecImgIdxHeiPosErr.size()+1;
			vecListInfo.resize(nListRows*nListCols);
			vecListInfo[0*nListCols+0].Format("影像索引");
			vecListInfo[0*nListCols+1].Format("影像名");
			vecListInfo[0*nListCols+2].Format("X误差-m");
			vecListInfo[0*nListCols+3].Format("Y误差-m");
			vecListInfo[0*nListCols+4].Format("Z误差-m");
			vecListInfo[0*nListCols+5].Format("XYZ误差-m");
			for (int i = 0; i<nListRows-1; i++)
			{
				vecListInfo[(i+1)*nListCols+0].Format("%d",m_vecImgIdxHeiPosErr[i]);
				vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[m_vecImgIdxHeiPosErr[i]].m_strImgName);
				vecListInfo[(i+1)*nListCols+2].Format("%.2lf",m_vecAdjImgPosErr[m_vecBundleIdxHeiPosErr[i]].Xs);
				vecListInfo[(i+1)*nListCols+3].Format("%.2lf",m_vecAdjImgPosErr[m_vecBundleIdxHeiPosErr[i]].Ys);
				vecListInfo[(i+1)*nListCols+4].Format("%.2lf",m_vecAdjImgPosErr[m_vecBundleIdxHeiPosErr[i]].Zs);
				vecListInfo[(i+1)*nListCols+5].Format("%.2lf",m_vecAdjImgPosDisErr[m_vecBundleIdxHeiPosErr[i]]);
			}
			cprintf("PosInfo...");
			pdf.AddList(nListCols,nListRows,vecListInfo);
			cprintf("...Over\n");
			vector<CString>().swap(vecListInfo);
			pdf.AddEmptyLine(1.5);

			pdf.AddEmptyLine(1);
			pdf.AddImage(strPosErrViewImgPath,2);
			pdf.AddEmptyLine(0.5);
			sprintf(strImgLabelInfo,"图%d. POS误差超限影像分布(注：蓝色点为超限影像，绿色点为非超限影像)",nImgIdx);
			pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
			nImgIdx++;
			pdf.AddEmptyLine(3);
		}
		float* pX = new float[m_nInAdjedNum];
		float* pY = new float[m_nInAdjedNum];
		for (int i = 0; i<m_nInAdjedNum; i++)
		{
			pX[i]=m_vecBundleSuccess2ImgInfo[i];
			pY[i]=m_vecAdjImgPosDisErr[i];
		}
		pdf.AddChart(pY,pX,m_nInAdjedNum,0,0,1,0,0,1,0,0,"POS误差曲线","影像索引","POS距离误差值-m");
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 测区影像原始POS误差曲线图",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;

		delete[]pX;
		delete[]pY;
		FunOutPutLogInfo("POS精度写入完成！");
		pdf.AddEmptyLine(3);
	}
	/***********************航飞漏洞*************************/
	pdf.AddText("数据分布完整性检查",SSW_HEI,1.5);
	pdf.AddEmptyLine(2);
	sprintf(strLine,"如图%d所示，航飞漏洞块数：%d块，面积占比%3.2lf%%",1,m_vvMissAreaImgIdx.size(),m_dMissAreaRatio*100);
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1,0,0);
	pdf.AddEmptyLine(1.5);
	if(m_vvMissAreaImgIdx.size()>0)
	{
		int nMissImgNum = 0;
		for (int i = 0; i<m_vvMissAreaImgIdx.size(); i++)
		{
			nMissImgNum+=m_vvMissAreaImgIdx[i].size();
			if(i<m_vvMissAreaImgIdx.size()-1) nMissImgNum++;
		}
		sprintf(strLine,"漏飞区相关影像列表");
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1,0,0);
		pdf.AddEmptyLine(1.5);
		nListCols=4;nListRows=nMissImgNum+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("漏洞编号");
		vecListInfo[0*nListCols+1].Format("影像索引");
		vecListInfo[0*nListCols+2].Format("影像名");
		vecListInfo[0*nListCols+3].Format("航带信息");
		int nCount = 1;
		for (int i = 0; i<m_vvMissAreaImgIdx.size(); i++)
		{
			for (int j = 0; j<m_vvMissAreaImgIdx[i].size(); j++)
			{
				vecListInfo[nListCols*nCount+0].Format("%d",i);
				vecListInfo[nListCols*nCount+1].Format("%d",m_vvMissAreaImgIdx[i][j]);
				vecListInfo[nListCols*nCount+2].Format("%s",m_vecImgInfo[m_vvMissAreaImgIdx[i][j]].m_strImgName);
				vecListInfo[nListCols*nCount+3].Format("%s",m_vecImgInfo[m_vvMissAreaImgIdx[i][j]].m_strStripLabel);
				nCount++;
			}
			if(i<m_vvMissAreaImgIdx.size()-1)
			{
				vecListInfo[nListCols*nCount+0].Format("---");
				vecListInfo[nListCols*nCount+1].Format("---");
				vecListInfo[nListCols*nCount+2].Format("---");
				vecListInfo[nListCols*nCount+3].Format("---");
				nCount++;
			}
		}
		cprintf("MissArea...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
	}
	pdf.AddEmptyLine(3);
	FunOutPutLogInfo("漏飞区域写入完成！");
	/***********************重叠度*************************/
	sprintf(strLine,"影像重叠度信息");
	pdf.AddText(strLine,SSW_HEI,1.5,SSW_LEFT);
	pdf.AddEmptyLine(2);
	CString strOverlapImg = FunGetFileFolder(strCqViewImgPath);
	if(eMethod==BY_ADJUSTMENT) strOverlapImg+="\\OverlapImg_ByBundle.jpg";else
		if(eMethod==BY_MATCH) strOverlapImg+="\\OverlapImg_ByMatch.jpg";else
			if(eMethod==BY_POS) strOverlapImg+="\\OverlapImg_ByPos.jpg";
	pdf.AddImage(strOverlapImg,1,SSW_CENTER,0,0,0,0,true);
	//pdf.AddImage(FunGetFileFolder(strViewImgPath)+"\\OverlapNote.jpg",1.0,SSW_NONE,0,350,40);
	pdf.AddEmptyLine(0.5);
	sprintf(strImgLabelInfo,"图%d. 测区重叠度示意图",nImgIdx);
	pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
	nImgIdx++; 
	pdf.AddEmptyLine(3);

	double dMaxOverlapX, dMinOverlapX,dMaxOverlapY, dMinOverlapY;
	double dMeanOverlapX=0,dMeanOverlapY=0;
	dMaxOverlapX=dMaxOverlapY=-1;
	dMinOverlapX=dMinOverlapY=10;
	float *pOverlapY,*pOverlapX,*pX;
	pOverlapX=new float[m_nImgNum-m_nInvalidNum];
	pOverlapY=new float[m_nImgNum-m_nInvalidNum];
	pX=new float[m_nImgNum-m_nInvalidNum];
	int nCount = 0;
	for (int i = 0; i<m_vvOverlapsBetStrips.size(); i++)
	{
		for (int j = 0; j<m_vvOverlapsBetStrips[i].size(); j++)
		{
			int nStripIdx = i;
			int nIdxInstrip = j;
			dMaxOverlapX = max(dMaxOverlapX,m_vvOverlapsBetStrips[nStripIdx][nIdxInstrip]);
			dMinOverlapX = min(dMinOverlapX,m_vvOverlapsBetStrips[nStripIdx][nIdxInstrip]);
			dMaxOverlapY = max(dMaxOverlapY,m_vvOverlapsInsStrips[nStripIdx][nIdxInstrip]);
			dMinOverlapY = min(dMinOverlapY,m_vvOverlapsInsStrips[nStripIdx][nIdxInstrip]);
			dMeanOverlapX+=m_vvOverlapsBetStrips[nStripIdx][nIdxInstrip];
			dMeanOverlapY+=m_vvOverlapsInsStrips[nStripIdx][nIdxInstrip];
			*(pOverlapX+nCount)=m_vvOverlapsBetStrips[nStripIdx][nIdxInstrip]*100;
			*(pOverlapY+nCount)=m_vvOverlapsInsStrips[nStripIdx][nIdxInstrip]*100;
			*(pX+nCount)=GetImgIdx(nStripIdx,nIdxInstrip);
			nCount++;
		}
	}
	if(nCount>0)
	{
		dMeanOverlapX/=nCount;
		dMeanOverlapY/=nCount;
		sprintf(strLine,"单张影像按最大重叠度进行统计：");
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT);
		pdf.AddEmptyLine(1);
		sprintf(strLine,"航向重叠度最大值%3.2lf%%，最小值%3.2lf%%，平均值%3.2lf%%；",dMaxOverlapY*100,dMinOverlapY*100,dMeanOverlapY*100);
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT);
		pdf.AddEmptyLine(1);
		sprintf(strLine,"旁向重叠度最大值%3.2lf%%，最小值%3.2lf%%，平均值%3.2lf%%。",dMaxOverlapX*100,dMinOverlapX*100,dMeanOverlapX*100);					
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT);
		pdf.AddEmptyLine(2);
	}
	sprintf(strLine,"航向重叠度不合格（< %.2lf%%）影像：%d张",m_FlyQuaParaSet.fMinOverlapY,m_vecImgIdxLowOverlapInsStrips.size());
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1,0,0);
	if(m_vecImgIdxLowOverlapInsStrips.size()>0)
	{
		pdf.AddEmptyLine(1);
		nListCols=4;nListRows=m_vecImgIdxLowOverlapInsStrips.size()+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		vecListInfo[0*nListCols+2].Format("航带信息");
		vecListInfo[0*nListCols+3].Format("航向重叠度");
		for (int i = 0; i<nListRows-1; i++)
		{
			int nImgIdx = m_vecImgIdxLowOverlapInsStrips[i];
			double dOverlap=
				m_vvOverlapsInsStrips[m_vecImgInfo[nImgIdx].m_nStripIdx][m_vecImgInfo[nImgIdx].m_nIdxInStrip];
			vecListInfo[(i+1)*nListCols+0].Format("%d",nImgIdx);
			vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[nImgIdx].m_strImgName);
			vecListInfo[(i+1)*nListCols+2].Format("%s",m_vecImgInfo[nImgIdx].m_strStripLabel);
			vecListInfo[(i+1)*nListCols+3].Format("%.2lf%%",dOverlap*100);
		}
		cprintf("Overlap_Y...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
		pdf.AddEmptyLine(1);
		pdf.AddImage(strOverlapYErrViewImgPath,2);
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 航向重叠度不合格影像分布示意图(注：蓝色点为超限影像，绿色点为合格影像)",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(3);
	}
	FunOutPutLogInfo("航向重叠度写入完成！");
	pdf.AddEmptyLine(2);
	sprintf(strLine,"旁向重叠度不合格（< %.2lf%%）影像：%d张",m_FlyQuaParaSet.fMinOverlapX,m_vecImgIdxLowOverlapBetStrips.size());
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1,0,0);
	char strlog[MAX_SIZE_FILE_LINE];
	// 	sprintf(strlog,"size = %d",m_vecImgIdxLowOverlapBetStrips.size());
	// 	FunOutPutLogInfo(strlog);
	if(m_vecImgIdxLowOverlapBetStrips.size()>0)
	{
		pdf.AddEmptyLine(1);
		nListCols=4;nListRows=m_vecImgIdxLowOverlapBetStrips.size()+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		vecListInfo[0*nListCols+2].Format("航带信息");
		vecListInfo[0*nListCols+3].Format("旁向重叠度");
		for (int i = 0; i<nListRows-1; i++)
		{
			int nImgIdx = m_vecImgIdxLowOverlapBetStrips[i];
			double dOverlap=
				m_vvOverlapsBetStrips[m_vecImgInfo[nImgIdx].m_nStripIdx][m_vecImgInfo[nImgIdx].m_nIdxInStrip];
			vecListInfo[(i+1)*nListCols+0].Format("%d",nImgIdx);
			vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[nImgIdx].m_strImgName);
			vecListInfo[(i+1)*nListCols+2].Format("%s",m_vecImgInfo[nImgIdx].m_strStripLabel);
			vecListInfo[(i+1)*nListCols+3].Format("%.2lf%%",dOverlap*100);
		}
		//////////////////////////////////////////////////////////
		cprintf("Overlap_X...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
		pdf.AddEmptyLine(1.5);
		pdf.AddEmptyLine(1);
		pdf.AddImage(strOverlapXErrViewImgPath,2);
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 旁向重叠度不合格影像分布示意图(注：蓝色点为超限影像，绿色点为合格影像)",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(3);
	}
	pdf.AddEmptyLine(3);
	if(nCount>0)
	{
		FunSortByX(nCount,pX,pOverlapY,pOverlapX);
		pdf.AddChart(pOverlapY,pX,nCount,0,0,1,0,0,1.0,0,0,"航向重叠度","影像索引","重叠度(%)");
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 影像航向重叠度统计图",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(1.5);
		pdf.AddChart(pOverlapX,pX,m_nImgNum-m_nInvalidNum,0,0,1,0,0,1.0,0,0,"旁向重叠度","影像索引","重叠度(%)");
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 影像旁向重叠度统计图",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
	}
	pdf.AddEmptyLine(3);
	delete[]pOverlapX;	
	delete[]pOverlapY;
	delete[]pX;
	FunOutPutLogInfo("旁向重叠度写入完成！");
	/***********************其他质检信息*************************/
	pdf.AddText("影像姿态角评价",SSW_HEI,1.5);
	pdf.AddEmptyLine(2);

	sprintf(strLine,"倾角不合格（> %.2lfdeg）影像：%d张",m_FlyQuaParaSet.fMaxPitch,m_vecImgIdxHeiPitch.size());
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1);
	pdf.AddEmptyLine(1);
	if(m_vecImgIdxHeiPitch.size()>0)
	{	
		nListCols=3;nListRows=m_vecImgIdxHeiPitch.size()+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		vecListInfo[0*nListCols+2].Format("倾角（deg）");
		for (int i = 0; i<nListRows-1; i++)
		{
			int nImgIdx = m_vecImgIdxHeiPitch[i];
			vecListInfo[(i+1)*nListCols+0].Format("%d",nImgIdx);
			vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[nImgIdx].m_strImgName);
			vecListInfo[(i+1)*nListCols+2].Format("%.2lf",m_vecImagePitch[nImgIdx]);
		}
		cprintf("PitchInfo...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
		pdf.AddEmptyLine(1);
		pdf.AddEmptyLine(1);
		pdf.AddImage(strPitchErrViewImgPath,2);
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 倾角不合格影像分布示意图(注：蓝色点为超限影像，绿色点为合格影像)",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(3);
	}	
	float *pPitch = new float[m_nImgNum-m_nInvalidNum];
	pX = new float[m_nImgNum-m_nInvalidNum];
	nCount=0;
	for (int i = 0; i<m_vecImagePitch.size(); i++)
	{
		if(!m_vecImgInfo[i].m_bValid)continue;
		*(pPitch+nCount)=m_vecImagePitch[i];
		*(pX+nCount)=i;
		nCount++;
	}
	if(nCount>0)
	{
		pdf.AddChart(pPitch,pX,nCount,0,0,1,0,0,1,0,0,"影像倾角","影像索引","倾角值（deg）");
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 影像倾角曲线图",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(3);
	}
	FunOutPutLogInfo("倾角写入完成！");
	sprintf(strLine,"旋角不合格（> %.2lfdeg）影像：%d张",m_FlyQuaParaSet.fMaxYaw,m_vecImgIdxHeiYaw.size());
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1);
	pdf.AddEmptyLine(1);
	if(m_vecImgIdxHeiYaw.size()>0)
	{
		nListCols=3;nListRows=m_vecImgIdxHeiYaw.size()+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		vecListInfo[0*nListCols+2].Format("旋角（deg）");
		for (int i = 0; i<nListRows-1; i++)
		{
			int nImgIdx = m_vecImgIdxHeiYaw[i];
			vecListInfo[(i+1)*nListCols+0].Format("%d",nImgIdx);
			vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[nImgIdx].m_strImgName);
			vecListInfo[(i+1)*nListCols+2].Format("%.2lf",m_vvStripImageYaw[m_vecImgInfo[nImgIdx].m_nStripIdx][m_vecImgInfo[nImgIdx].m_nIdxInStrip]);
		}
		cprintf("YawInfo...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
		pdf.AddEmptyLine(1);
		pdf.AddImage(strYawViewImgPath,2);
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 旋角不合格影像分布示意图(注：蓝色点为超限影像，绿色点为合格影像)",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(3);
	}
	float *pYaw = new float[m_nImgNum-m_nInvalidNum];
	memset(pYaw,0,sizeof(float)*(m_nImgNum-m_nInvalidNum));
	nCount = 0;
	for (int i = 0; i<m_vvStripImageYaw.size(); i++)
	{
		for (int j = 0; j<m_vvStripImageYaw[i].size(); j++)
		{
			int nStripIdx = i;
			int nIdxInStrip = j;
			*(pYaw+nCount)=m_vvStripImageYaw[nStripIdx][nIdxInStrip];
			*(pX+nCount)=GetImgIdx(nStripIdx,nIdxInStrip);
			nCount++;
		}
	}
	FunSortByX(nCount,pX,pYaw);
	if(nCount>0)
	{
		pdf.AddChart(pYaw,pX,m_nImgNum-m_nInvalidNum,0,0,1,0,0,1,0,0,"影像旋角","影像索引","旋角值（deg）");
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 影像旋角曲线图",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
	}
	delete[]pYaw;
	delete[]pPitch;
	delete[]pX;
	FunOutPutLogInfo("旋角写入完成！");
	pdf.AddEmptyLine(3);
	sprintf(strLine,"飞行方向调整处无效影像：%d张",m_nInvalidNum);
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1);
	pdf.AddEmptyLine(1);
	if(m_nInvalidNum>0) 
	{
		nListCols = 2; nListRows = m_nInvalidNum+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		int nCount =0;
		for (int i = 0; i<m_nImgNum; i++)
		{
			if(!m_vecImgInfo[i].m_bValid)
			{
				vecListInfo[(nCount+1)*nListCols+0].Format("%d",i);
				vecListInfo[(nCount+1)*nListCols+1].Format("%s",m_vecImgInfo[i].m_strImgName);
				nCount++;
			}
		}
		cprintf("InvalidImg...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
	}
	FunOutPutLogInfo("飞行调整影像写入完成！");
	pdf.AddEmptyLine(3);
	if(eMethod!=BY_POS)
	{
		sprintf(strLine,"空三困难（缩略图空三点数小于%d）影像：%d张",m_nMatchPointNumThresold,m_vecImgIdxLowMatchPointNum.size());
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1);
		pdf.AddEmptyLine(1);
		if(m_vecImgIdxLowMatchPointNum.size()>0)
		{
			nListCols = 3; nListRows = m_vecImgIdxLowMatchPointNum.size()+1;
			vecListInfo.resize(nListCols*nListRows);
			vecListInfo[0*nListCols+0].Format("影像索引");
			vecListInfo[0*nListCols+1].Format("影像名");
			vecListInfo[0*nListCols+2].Format("空三点数");
			for (int i = 0; i<m_vecImgIdxLowMatchPointNum.size(); i++)
			{
				vecListInfo[(i+1)*nListCols+0].Format("%d",m_vecImgIdxLowMatchPointNum[i]);
				vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[m_vecImgIdxLowMatchPointNum[i]].m_strImgName);
				vecListInfo[(i+1)*nListCols+2].Format("%d",m_vecImgInfo[m_vecImgIdxLowMatchPointNum[i]].m_ImgAdjPts.vecAdjPtIdx.size());
			}
			cprintf("MatchErr...");
			pdf.AddList(nListCols,nListRows,vecListInfo);
			cprintf("...Over\n");
			vector<CString>().swap(vecListInfo);
			pdf.AddEmptyLine(1);
		}
		pdf.AddEmptyLine(3);
		FunOutPutLogInfo("空三困难影像写入完成！");
	}
	/***********************其他质检信息*************************/
	pdf.AddText("质检处理耗时统计",SSW_HEI,2);
	pdf.AddEmptyLine(1);
	clock_t t2 = clock();
	double dt = (double)(t2-t1)/CLOCKS_PER_SEC;
	m_dTimeSecsWriteChkRep=dt;
	m_dTimeSecsWholeProcess=m_dTimeSecsCmrDistortion+m_dTimeSecsBundleAdjustment+m_dTimeSecsTimeReadAdjRep+m_dTimeSecsRunFlyQuaChk+m_dTimeSecsWriteChkRep;
	nListCols = 2; nListRows = 7;
	vecListInfo.resize(nListCols*nListRows);
	vecListInfo[0*nListCols+0].Format("处理过程");
	vecListInfo[0*nListCols+1].Format("耗时-s");
	vecListInfo[1*nListCols+0].Format("完整流程");
	vecListInfo[1*nListCols+1].Format("%.3lf",m_dTimeSecsWholeProcess);
	vecListInfo[2*nListCols+0].Format("畸变改正");
	vecListInfo[2*nListCols+1].Format("%.3lf",m_dTimeSecsCmrDistortion);
	vecListInfo[3*nListCols+0].Format("匹配平差");
	vecListInfo[3*nListCols+1].Format("%.3lf",m_dTimeSecsBundleAdjustment);
	vecListInfo[4*nListCols+0].Format("解析平差报告");
	vecListInfo[4*nListCols+1].Format("%.3lf",m_dTimeSecsTimeReadAdjRep);
	vecListInfo[5*nListCols+0].Format("计算质检参数");
	vecListInfo[5*nListCols+1].Format("%.3lf",m_dTimeSecsRunFlyQuaChk);
	vecListInfo[6*nListCols+0].Format("保存质检报告");
	vecListInfo[6*nListCols+1].Format("%.3lf",m_dTimeSecsWriteChkRep);
	cprintf("TimeInfo...");
	bool b = pdf.AddList(nListCols,nListRows,vecListInfo);
	cprintf("...Over\n");
	vector<CString>().swap(vecListInfo);
	pdf.AddEmptyLine(1);
	FunOutPutLogInfo("报告写入完毕！");
	CString strPath = m_strFlyQuaChkRepPath;

	if(!pdf.SavePdf(strPath))
		return false;
	delete[]strLine;
	return true;
}

bool CsswUAVFlyQuaPrj::SaveReport2File20170108_(CString strCqViewImgPath,CString strOverlapYErrViewImgPath,CString strOverlapXErrViewImgPath,CString strPosErrViewImgPath, 
	CString strPitchErrViewImgPath,CString strYawViewImgPath,CString strMatchMatchErrImgPath)
{
	FunOutPutLogInfo("开始写入PDF");
	clock_t t1 = clock();
	m_strFlyQuaChkRepPath = GetCheckRepPath(BY_ADJUSTMENT);
	char *strLine = new char[MAX_SIZE_FILE_LINE*3]; 
	memset(strLine,0,MAX_SIZE_FILE_LINE*3);
	CsswPdf pdf; 
	if(!pdf.InitPdf())
	{
		FunOutPutLogInfo("PDF初始化失败！");
		return false;
	}
	int nImgIdx =1; 
	char strImgLabelInfo[MAX_SIZE_FILE_LINE]; 
	memset(strImgLabelInfo,0,MAX_SIZE_FILE_LINE);
	int nListCols/* = 2*/, nListRows/* = 5*/;
	vector<CString> vecListInfo;
	if(!pdf.AddPage()) return false;
	pdf.SetLineSpace(50);
	pdf.SetPdfTitle("无人机航摄数据质量检查报告",0,0.6,0);
	CString strLogon = FunGetFileFolder(FunGetThisExePath())+"\\logo.dll";
	int nw = 31, nh = 31;
	int nx = pdf.GetPdfPageWidth()-pdf.GetLineSpace();
	int ny = pdf.GetPdfPageHeight()-pdf.GetTitleGap()-pdf.GetLineGap();
	pdf.AddImage(strLogon,1.0,SSW_NONE,nx,ny,nw,nh); 
	sprintf(strLine,"报告生成时间：%s",FunGetSysTimeStr());
	pdf.AddText(strLine,SSW_SONG,1.2,SSW_LEFT,0,false,0,0,0,0,0,0,true);pdf.AddEmptyLine(-1.2);
	//pdf.AddText("@2016 UAV Flight Quality Check",SSW_SONG,1.2,SSW_RIGHT,0,false,0.3,0.3,0.3,0,0,0,true);
	pdf.AddEmptyLine(5);

	/************************************************************************/
	/* 第一部分 质检报告概述                                                 */
	/************************************************************************/
	pdf.AddText("质检报告概述",SSW_HEI,2.5,SSW_CENTER);
	pdf.AddEmptyLine(2);
	memset(strLine,'-',MAX_SIZE_FILE_LINE*3);
	pdf.AddText(strLine,SSW_SONG,1,SSW_LEFT,0,true,0.7,0.7,0.7);
	pdf.AddEmptyLine(-2);
	sprintf(strLine,"质检方法——基于平差的精确质检");
	pdf.AddText(strLine,SSW_HEI,1.5,SSW_RIGHT,0,false,0.3,0.3,0.3,pdf.GetPdfValidWidth()-pdf.GetTextWidthInPage(strLine)-pdf.GetLineSpace()/2,pdf.GetWriteTopInPage());
	pdf.AddEmptyLine(3);
	/**************测区影像数量 ***************/
	pdf.AddText("1.测区影像数量",SSW_HEI,2,SSW_LEFT);
	pdf.AddEmptyLine(2);
	nListCols = 4; nListRows = 2;
	vecListInfo.resize(nListCols*nListRows);
	vecListInfo[0*nListCols+0].Format("%s","影像总数");
	vecListInfo[0*nListCols+1].Format("%s","有效影像数");
	vecListInfo[0*nListCols+2].Format("%s","无效影像数");
	vecListInfo[0*nListCols+3].Format("%s","航带数");
	vecListInfo[1*nListCols+0].Format("%d",m_nImgNum);
	vecListInfo[1*nListCols+1].Format("%d",m_nImgNum-m_nInvalidNum);
	vecListInfo[1*nListCols+2].Format("%d",m_nInvalidNum);
	vecListInfo[1*nListCols+3].Format("%d",m_nStripNum);
	cprintf("========List info in report========\n");
	cprintf("ImageInfo...");
	pdf.AddList(nListCols,nListRows,vecListInfo);
	cprintf("...Over\n");
	vector<CString>().swap(vecListInfo);
	pdf.AddEmptyLine(3);
	FunOutPutLogInfo("测区影像数量写入完成！");

	/**************数据完整性检查 ***************/
	pdf.AddText("2.数据分布完整性检查",SSW_HEI,2,SSW_LEFT);
	pdf.AddEmptyLine(2);
	pdf.AddImage(strCqViewImgPath);
	pdf.AddEmptyLine(0.5);
	sprintf(strImgLabelInfo,"图%d. 测区航带示意图(注：红色点为无效影像，蓝绿颜色表示间隔航带；红色多边形为漏飞区域)",nImgIdx);
	pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
	nImgIdx++;
	pdf.AddEmptyLine(1);
	sprintf(strLine,"如图%d所示，航飞漏洞块数：%d块，面积占比%3.2lf%%",1,m_vvMissAreaImgIdx.size(),m_dMissAreaRatio*100);
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1,0,0);
// 	if(m_vvMissAreaImgIdx.size()>0)
// 	{
// 		pdf.AddEmptyLine(1.5);
// 		int nMissImgNum = 0;
// 		for (int i = 0; i<m_vvMissAreaImgIdx.size(); i++)
// 		{
// 			nMissImgNum+=m_vvMissAreaImgIdx[i].size();
// 			if(i<m_vvMissAreaImgIdx.size()-1) nMissImgNum++;
// 		}
// 		sprintf(strLine,"漏飞区相关影像列表");
// 		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1,0,0);
// 		pdf.AddEmptyLine(1.5);
// 		nListCols=4;nListRows=nMissImgNum+1;
// 		vecListInfo.resize(nListCols*nListRows);
// 		vecListInfo[0*nListCols+0].Format("漏洞编号");
// 		vecListInfo[0*nListCols+1].Format("影像索引");
// 		vecListInfo[0*nListCols+2].Format("影像名");
// 		vecListInfo[0*nListCols+3].Format("航带信息");
// 		int nCount = 1;
// 		for (int i = 0; i<m_vvMissAreaImgIdx.size(); i++)
// 		{
// 			for (int j = 0; j<m_vvMissAreaImgIdx[i].size(); j++)
// 			{
// 				vecListInfo[nListCols*nCount+0].Format("%d",i);
// 				vecListInfo[nListCols*nCount+1].Format("%d",m_vvMissAreaImgIdx[i][j]);
// 				vecListInfo[nListCols*nCount+2].Format("%s",m_vecImgInfo[m_vvMissAreaImgIdx[i][j]].m_strImgName);
// 				vecListInfo[nListCols*nCount+3].Format("%s",m_vecImgInfo[m_vvMissAreaImgIdx[i][j]].m_strStripLabel);
// 				nCount++;
// 			}
// 			if(i<m_vvMissAreaImgIdx.size()-1)
// 			{
// 				vecListInfo[nListCols*nCount+0].Format("---");
// 				vecListInfo[nListCols*nCount+1].Format("---");
// 				vecListInfo[nListCols*nCount+2].Format("---");
// 				vecListInfo[nListCols*nCount+3].Format("---");
// 				nCount++;
// 			}
// 		}
// 		cprintf("MissArea...");
// 		pdf.AddList(nListCols,nListRows,vecListInfo);
// 		cprintf("...Over\n");
// 		vector<CString>().swap(vecListInfo);
// 	}
	pdf.AddEmptyLine(3);
	FunOutPutLogInfo("漏飞区域写入完成！");

	/***********************影像重叠度信息*************************/
	pdf.AddText("3.影像重叠度信息",SSW_HEI,2,SSW_LEFT);
	pdf.AddEmptyLine(2);
	CString strOverlapImg = FunGetFileFolder(strCqViewImgPath)+"\\OverlapImg_ByBundle.jpg";
	pdf.AddImage(strOverlapImg,1,SSW_CENTER,0,0,0,0,true);
	//pdf.AddImage(FunGetFileFolder(strViewImgPath)+"\\OverlapNote.jpg",1.0,SSW_NONE,0,350,40);
	pdf.AddEmptyLine(0.5);
	sprintf(strImgLabelInfo,"图%d. 测区重叠度示意图",nImgIdx);
	pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
	nImgIdx++; 
	pdf.AddEmptyLine(3);
	FunOutPutLogInfo("影像重叠度信息写入完成！");

	/***********************空三困难影像*************************/
	pdf.AddText("4.空三困难影像",SSW_HEI,2,SSW_LEFT);
	pdf.AddEmptyLine(2);
	pdf.AddImage(strMatchMatchErrImgPath,2);
	pdf.AddEmptyLine(0.5);
	sprintf(strImgLabelInfo,"图%d. 测区空三困难影像分布图(注：蓝色点为空三困难影像，绿色点为正常影像)",nImgIdx);
	pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
	nImgIdx++; 
	pdf.AddEmptyLine(1);
	sprintf(strLine,"如图%d所示，空三困难影像（空三点数<%d）：%d张",3,m_nMatchPointNumThresold,m_vecImgIdxLowMatchPointNum.size());
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1,0,0);
	pdf.AddEmptyLine(3);
	FunOutPutLogInfo("空三困难影像写入完成！");

	/***********************质检结论*************************/
	pdf.AddText("5.质检结论",SSW_HEI,2,SSW_LEFT);
	pdf.AddEmptyLine(2);
	nListCols=3;nListRows=6;
	vecListInfo.resize(nListCols*nListRows);
	vecListInfo[0*nListCols+0].Format("质检项");
	vecListInfo[0*nListCols+1].Format("检查结果");
	vecListInfo[0*nListCols+2].Format("备注信息");

	vecListInfo[1*nListCols+0].Format("相机参数");
	if(!m_bCmrFile)
	{
		vecListInfo[1*nListCols+1].Format("无");
		vecListInfo[1*nListCols+2].Format("相机参数缺失");
	}
	else
	{
		if(m_dMaxPaneDifErr>m_FlyQuaParaSet.fMaxErrStationPane&&m_dRmsPaneDifErr>m_FlyQuaParaSet.fRmsErrStationPane)
		{
			vecListInfo[1*nListCols+1].Format("不合格");
			vecListInfo[1*nListCols+2].Format("建议自检校重新获取");
		}
		else
		{
			vecListInfo[1*nListCols+1].Format("合格");
			vecListInfo[1*nListCols+2].Format("无");
		}
	}
	vecListInfo[2*nListCols+0].Format("航飞漏洞");
	if (m_vvMissAreaImgIdx.size()==0)
	{
		vecListInfo[2*nListCols+1].Format("合格");
		vecListInfo[2*nListCols+2].Format("无");
	}
	else
	{
		if(m_dMissAreaRatio>m_FlyQuaParaSet.fMissAreaRatio/100)
		{
			vecListInfo[2*nListCols+1].Format("不合格");
			vecListInfo[2*nListCols+2].Format("漏洞面积占比%3.2lf%%，建议补飞",m_dMissAreaRatio*100);
		}else
		{
			vecListInfo[2*nListCols+1].Format("合格");
			vecListInfo[2*nListCols+2].Format("漏洞面积占比%3.2lf%%，可接受",m_dMissAreaRatio*100);
		}
	}
	vecListInfo[3*nListCols+0].Format("影像重叠度");
	vecListInfo[3*nListCols+1].Format("见图2");
	vecListInfo[3*nListCols+2].Format("测区主要区域重叠度达到5则合格");
	vecListInfo[4*nListCols+0].Format("空三困难影像");
	if (m_vecImgIdxLowMatchPointNum.size()==0)
	{
		vecListInfo[4*nListCols+1].Format("0");
		vecListInfo[4*nListCols+2].Format("无");
	}
	else
	{
		vecListInfo[4*nListCols+1].Format("%d张",m_vecImgIdxLowMatchPointNum.size());
		vecListInfo[4*nListCols+2].Format("详见空三困难影像列表");
	}
	vecListInfo[5*nListCols+0].Format("影像姿态角");
	float fAngleRatio = (m_vecImgIdxHeiYaw.size()+m_vecImgIdxHeiPitch.size())*1.0/(m_nImgNum-m_nInvalidNum);
	if (fAngleRatio<m_FlyQuaParaSet.fAngleErrRatio/100)
	{
		vecListInfo[5*nListCols+1].Format("合格");
		vecListInfo[5*nListCols+2].Format("无");
	}
	else
	{
		vecListInfo[5*nListCols+1].Format("不合格");
		vecListInfo[5*nListCols+2].Format("姿态角超限影像比%3.2lf%%，建议重飞",fAngleRatio*100);
	}
	cprintf("Result...");
	pdf.AddList(nListCols,nListRows,vecListInfo);
	cprintf("...Over\n");
	vector<CString>().swap(vecListInfo);
	FunOutPutLogInfo("质检结论写入完成！");
	pdf.AddEmptyLine(2);
	pdf.AddText("备注说明：",SSW_HEI,1.5,SSW_LEFT,0,false,1);
	pdf.AddEmptyLine(1);
	sprintf(strLine,"%s%s","1) 空三匹配困难影像形成原因可能是：影像存在大片弱纹理、重复纹理情况（水域、沙漠、雪山、森林等）； ",
		"影像成像质量差，拍摄模糊，有大片阴影或曝光过度等。请查看原始影像进行分析。");
	pdf.AddText(strLine, SSW_SONG,1,SSW_LEFT,0,false);
	pdf.AddEmptyLine(1);
	sprintf(strLine,"%s","2) 若影像姿态角不合格，会降低后续内业空三处理质量，导致线划图（DLG）绘制困难。");
	pdf.AddText(strLine,SSW_SONG,1,SSW_LEFT,0,false);
	pdf.AddEmptyLine(5);

	/************************************************************************/
	/* 第二部分 质检报告详述                                                 */
	/************************************************************************/
	pdf.AddText("质检报告详述",SSW_HEI,2.5,SSW_CENTER);
	pdf.AddEmptyLine(3);
	/***********************质检参数阈值*************************/
	nListCols = 3; nListRows = 10;
	vecListInfo.resize(nListCols*nListRows);
	vecListInfo[0*nListCols+0]="质检项";
	vecListInfo[0*nListCols+1]="阈值说明";
	vecListInfo[0*nListCols+2]="阈值大小";
	vecListInfo[1*nListCols+0]="相机参数";
	vecListInfo[1*nListCols+1]="平差后POS平整度偏差最大值(m)";
	vecListInfo[1*nListCols+2].Format("%.2f",m_FlyQuaParaSet.fMaxErrStationPane);
	vecListInfo[2*nListCols+0]="相机参数";
	vecListInfo[2*nListCols+1]="平差后POS平整度偏差中误差(m)";
	vecListInfo[2*nListCols+2].Format("%.2f",m_FlyQuaParaSet.fRmsErrStationPane);
	vecListInfo[3*nListCols+0]="航向重叠度";
	vecListInfo[3*nListCols+1]="影像合格最小航向重叠度";
	vecListInfo[3*nListCols+2].Format("%.2f%%",m_FlyQuaParaSet.fMinOverlapY);
	vecListInfo[4*nListCols+0]="旁向重叠度";
	vecListInfo[4*nListCols+1]="影像合格最小旁向重叠度";
	vecListInfo[4*nListCols+2].Format("%.2f%%",m_FlyQuaParaSet.fMinOverlapX);
	vecListInfo[5*nListCols+0]="影像倾角";
	vecListInfo[5*nListCols+1]="影像合格最大倾角";
	vecListInfo[5*nListCols+2].Format("%.2f",m_FlyQuaParaSet.fMaxPitch);
	vecListInfo[6*nListCols+0]="影像旋角";
	vecListInfo[6*nListCols+1]="影像合格最大旋角";
	vecListInfo[6*nListCols+2].Format("%.2f",m_FlyQuaParaSet.fMaxYaw);
	vecListInfo[7*nListCols+0]="空三困难影像";
	vecListInfo[7*nListCols+1]="空三困难影像判断空三点数最小值";
	vecListInfo[7*nListCols+2].Format("%d",m_nMatchPointNumThresold);
	vecListInfo[8*nListCols+0]="航飞完整度";
	vecListInfo[8*nListCols+1]="航飞漏洞面积占比可接受最大值";
	vecListInfo[8*nListCols+2].Format("%.2f%%",m_FlyQuaParaSet.fMissAreaRatio);
	vecListInfo[9*nListCols+0]="影像姿态";
	vecListInfo[9*nListCols+1]="异常影像占比可接受最大值";
	vecListInfo[9*nListCols+2].Format("%.2f%%",m_FlyQuaParaSet.fAngleErrRatio);
	pdf.AddText("1.自动质检参数阈值",SSW_HEI,2,SSW_LEFT);
	pdf.AddEmptyLine(2);
	cprintf("========List info in report========\n");
	cprintf("Thresold...");
	pdf.AddList(nListCols,nListRows,vecListInfo);
	cprintf("...Over\n");
	vector<CString>().swap(vecListInfo);
	pdf.AddEmptyLine(3);
	FunOutPutLogInfo("自动质检参数阈值写入完成！");

	/***********************相机内参数分析*************************/
	pdf.AddText("2.相机内参数精度分析",SSW_HEI,2,SSW_LEFT);
	pdf.AddEmptyLine(2);
	if(m_bCmrFile)
	{
		pdf.AddText("相机原始畸变参数",SSW_SONG,1.5,SSW_LEFT);
		pdf.AddEmptyLine(1);
		nListCols = 6; nListRows = 2;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("a");
		vecListInfo[0*nListCols+1].Format("b");
		vecListInfo[0*nListCols+2].Format("k1");
		vecListInfo[0*nListCols+3].Format("k2");
		vecListInfo[0*nListCols+4].Format("p1");
		vecListInfo[0*nListCols+5].Format("p2");
		vecListInfo[1*nListCols+0].Format("%.8lf",m_ImgCmrInfo.a);
		vecListInfo[1*nListCols+1].Format("%.8lf",m_ImgCmrInfo.b);
		vecListInfo[1*nListCols+2].Format("%.8lf",m_ImgCmrInfo.k1);
		vecListInfo[1*nListCols+3].Format("%.8lf",m_ImgCmrInfo.k2);
		vecListInfo[1*nListCols+4].Format("%.8lf",m_ImgCmrInfo.p1);
		vecListInfo[1*nListCols+5].Format("%.8lf",m_ImgCmrInfo.p2);
		cprintf("CameraInfo...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
		pdf.AddEmptyLine(2);
// 		sprintf(strLine,"平差结果摄站点距航飞平面，最大偏差%.2lfm(阈值%.2f)，偏差中误差%.2lfm(阈值%.2f)。",m_dMaxPaneDifErr,m_FlyQuaParaSet.fMaxErrStationPane,m_dRmsPaneDifErr,m_FlyQuaParaSet.fRmsErrStationPane);
// 		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,0);
// 		pdf.AddEmptyLine(1.5);
		int nPosX = pdf.GetTextWidthInPage(strLine)+pdf.GetLineSpace();
		int nPosY = pdf.GetWriteTopInPage();
		if(m_dMaxPaneDifErr>m_FlyQuaParaSet.fMaxErrStationPane&&m_dRmsPaneDifErr>m_FlyQuaParaSet.fRmsErrStationPane)
		{
			sprintf(strLine,"平差结果摄站点距航飞平面，最大偏差%.2lfm(大于阈值%.2f)，偏差中误差%.2lfm(大于阈值%.2f)。",m_dMaxPaneDifErr,m_FlyQuaParaSet.fMaxErrStationPane,m_dRmsPaneDifErr,m_FlyQuaParaSet.fRmsErrStationPane);
			pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,0);
			pdf.AddEmptyLine(1.5);
			pdf.AddText("相机参数不合格！",SSW_SONG,1.5,SSW_LEFT,0,false,1);
		}
		else
		{
			if(m_dMaxPaneDifErr>m_FlyQuaParaSet.fMaxErrStationPane)
			{
				sprintf(strLine,"平差结果摄站点距航飞平面，最大偏差%.2lfm(大于阈值%.2f)，偏差中误差%.2lfm(小于阈值%.2f)。",m_dMaxPaneDifErr,m_FlyQuaParaSet.fMaxErrStationPane,m_dRmsPaneDifErr,m_FlyQuaParaSet.fRmsErrStationPane);
			}else 
			if(m_dRmsPaneDifErr>m_FlyQuaParaSet.fRmsErrStationPane)
			{
				sprintf(strLine,"平差结果摄站点距航飞平面，最大偏差%.2lfm(小于阈值%.2f)，偏差中误差%.2lfm(大于阈值%.2f)。",m_dMaxPaneDifErr,m_FlyQuaParaSet.fMaxErrStationPane,m_dRmsPaneDifErr,m_FlyQuaParaSet.fRmsErrStationPane);
			}else
			{
				sprintf(strLine,"平差结果摄站点距航飞平面，最大偏差%.2lfm(小于阈值%.2f)，偏差中误差%.2lfm(小于阈值%.2f)。",m_dMaxPaneDifErr,m_FlyQuaParaSet.fMaxErrStationPane,m_dRmsPaneDifErr,m_FlyQuaParaSet.fRmsErrStationPane);
			}
			pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,0);
			pdf.AddEmptyLine(1.5);
			pdf.AddText("相机参数合格！",SSW_SONG,1.5,SSW_LEFT,0,false,1);
		}
		pdf.AddEmptyLine(1.5);
		float *pX = new float[m_nInAdjedNum];
		float *pY = new float[m_nInAdjedNum];
		for (int i = 0; i<m_nInAdjedNum; i++)
		{
			pX[i]=m_vecBundleSuccess2ImgInfo[i];
			pY[i]=m_vecDisImgAdjPos2PosPane[i];
		}
		pdf.AddChart(pY,pX,m_vecDisImgAdjPos2PosPane.size(),0,0,1,0,0,1,0,0,"POS偏离距离","影像索引","POS距离偏离值-m");
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 测区影像平差结果POS偏离图",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		delete[]pX;
		delete[]pY;
	}
	else
	{
		pdf.AddText("无相机参数信息！",SSW_SONG,1.5,SSW_LEFT,0,false,1);
	}
	pdf.AddEmptyLine(3);
	FunOutPutLogInfo("相机参数精度写入完成！");

	/***********************POS精度*************************/	
	pdf.AddText("3.初始POS坐标精度分析",SSW_HEI,2,SSW_LEFT);
	pdf.AddEmptyLine(2);
	sprintf(strLine,"参与平差影像%d张，平差前后POS空间距离(Err)近似中误差 RMS = %.2lfm，均值%.2lfm",
		m_nInAdjedNum,m_dRmsPosErr,m_dMeanPosErr);
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,true);
	pdf.AddEmptyLine(1);
	//	sprintf(strLine,"不合格影像数%d张（Err > 3*RMS+均值 为不合格）",m_vecImgIdxHeiPosErr.size());
	sprintf(strLine,"超过3倍中误差（Err>3*RMS+Mean）影像：%d张",m_vecImgIdxHeiPosErr.size());
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1);
	pdf.AddEmptyLine(1.5);
	if(m_vecImgIdxHeiPosErr.size()>0)
	{	
		nListCols = 6; nListRows = m_vecImgIdxHeiPosErr.size()+1;
		vecListInfo.resize(nListRows*nListCols);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		vecListInfo[0*nListCols+2].Format("X误差-m");
		vecListInfo[0*nListCols+3].Format("Y误差-m");
		vecListInfo[0*nListCols+4].Format("Z误差-m");
		vecListInfo[0*nListCols+5].Format("XYZ误差-m");
		for (int i = 0; i<nListRows-1; i++)
		{
			vecListInfo[(i+1)*nListCols+0].Format("%d",m_vecImgIdxHeiPosErr[i]);
			vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[m_vecImgIdxHeiPosErr[i]].m_strImgName);
			vecListInfo[(i+1)*nListCols+2].Format("%.2lf",m_vecAdjImgPosErr[m_vecBundleIdxHeiPosErr[i]].Xs);
			vecListInfo[(i+1)*nListCols+3].Format("%.2lf",m_vecAdjImgPosErr[m_vecBundleIdxHeiPosErr[i]].Ys);
			vecListInfo[(i+1)*nListCols+4].Format("%.2lf",m_vecAdjImgPosErr[m_vecBundleIdxHeiPosErr[i]].Zs);
			vecListInfo[(i+1)*nListCols+5].Format("%.2lf",m_vecAdjImgPosDisErr[m_vecBundleIdxHeiPosErr[i]]);
		}
		cprintf("PosInfo...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
		pdf.AddEmptyLine(1.5);

		pdf.AddEmptyLine(1);
		pdf.AddImage(strPosErrViewImgPath,2);
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. POS误差超限影像分布(注：蓝色点为超限影像，绿色点为非超限影像)",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(3);
	}
	float* pX = new float[m_nInAdjedNum];
	float* pY = new float[m_nInAdjedNum];
	for (int i = 0; i<m_nInAdjedNum; i++)
	{
		pX[i]=m_vecBundleSuccess2ImgInfo[i];
		pY[i]=m_vecAdjImgPosDisErr[i];
	}
	pdf.AddChart(pY,pX,m_nInAdjedNum,0,0,1,0,0,1,0,0,"POS误差曲线","影像索引","POS距离误差值-m");
	pdf.AddEmptyLine(0.5);
	sprintf(strImgLabelInfo,"图%d. 测区影像原始POS误差曲线图",nImgIdx);
	pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
	nImgIdx++;

	delete[]pX;
	delete[]pY;
	pdf.AddEmptyLine(3);
	FunOutPutLogInfo("POS精度写入完成！");


	/***********************航飞漏洞*************************/
	pdf.AddText("4.数据分布完整性检查",SSW_HEI,2);
	
	if(m_vvMissAreaImgIdx.size()>0)
	{
		pdf.AddEmptyLine(2);
		int nMissImgNum = 0;
		for (int i = 0; i<m_vvMissAreaImgIdx.size(); i++)
		{
			nMissImgNum+=m_vvMissAreaImgIdx[i].size();
			if(i<m_vvMissAreaImgIdx.size()-1) nMissImgNum++;
		}
		sprintf(strLine,"漏飞区相关影像列表");
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1,0,0);
		pdf.AddEmptyLine(1.5);
		nListCols=4;nListRows=nMissImgNum+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("漏洞编号");
		vecListInfo[0*nListCols+1].Format("影像索引");
		vecListInfo[0*nListCols+2].Format("影像名");
		vecListInfo[0*nListCols+3].Format("航带信息");
		int nCount = 1;
		for (int i = 0; i<m_vvMissAreaImgIdx.size(); i++)
		{
			for (int j = 0; j<m_vvMissAreaImgIdx[i].size(); j++)
			{
				vecListInfo[nListCols*nCount+0].Format("%d",i);
				vecListInfo[nListCols*nCount+1].Format("%d",m_vvMissAreaImgIdx[i][j]);
				vecListInfo[nListCols*nCount+2].Format("%s",m_vecImgInfo[m_vvMissAreaImgIdx[i][j]].m_strImgName);
				vecListInfo[nListCols*nCount+3].Format("%s",m_vecImgInfo[m_vvMissAreaImgIdx[i][j]].m_strStripLabel);
				nCount++;
			}
			if(i<m_vvMissAreaImgIdx.size()-1)
			{
				vecListInfo[nListCols*nCount+0].Format("---");
				vecListInfo[nListCols*nCount+1].Format("---");
				vecListInfo[nListCols*nCount+2].Format("---");
				vecListInfo[nListCols*nCount+3].Format("---");
				nCount++;
			}
		}
		cprintf("MissArea...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
	}
	else
	{
		pdf.AddEmptyLine(2);
		sprintf(strLine,"测区影像覆盖完整，无明显漏洞！");
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1,0,0);
	}
	pdf.AddEmptyLine(3);
	FunOutPutLogInfo("漏飞区域写入完成！");

	/***********************重叠度*************************/
	sprintf(strLine,"5.影像重叠度信息");
	pdf.AddText(strLine,SSW_HEI,2,SSW_LEFT);
	pdf.AddEmptyLine(2);
	double dMaxOverlapX, dMinOverlapX,dMaxOverlapY, dMinOverlapY;
	double dMeanOverlapX=0,dMeanOverlapY=0;
	dMaxOverlapX=dMaxOverlapY=-1;
	dMinOverlapX=dMinOverlapY=10;
	float *pOverlapY,*pOverlapX;
	pOverlapX=new float[m_nImgNum-m_nInvalidNum];
	pOverlapY=new float[m_nImgNum-m_nInvalidNum];
	pX=new float[m_nImgNum-m_nInvalidNum];
	int nCount = 0;
	for (int i = 0; i<m_vvOverlapsBetStrips.size(); i++)
	{
		for (int j = 0; j<m_vvOverlapsBetStrips[i].size(); j++)
		{
			int nStripIdx = i;
			int nIdxInstrip = j;
			dMaxOverlapX = max(dMaxOverlapX,m_vvOverlapsBetStrips[nStripIdx][nIdxInstrip]);
			dMinOverlapX = min(dMinOverlapX,m_vvOverlapsBetStrips[nStripIdx][nIdxInstrip]);
			dMaxOverlapY = max(dMaxOverlapY,m_vvOverlapsInsStrips[nStripIdx][nIdxInstrip]);
			dMinOverlapY = min(dMinOverlapY,m_vvOverlapsInsStrips[nStripIdx][nIdxInstrip]);
			dMeanOverlapX+=m_vvOverlapsBetStrips[nStripIdx][nIdxInstrip];
			dMeanOverlapY+=m_vvOverlapsInsStrips[nStripIdx][nIdxInstrip];
			*(pOverlapX+nCount)=m_vvOverlapsBetStrips[nStripIdx][nIdxInstrip]*100;
			*(pOverlapY+nCount)=m_vvOverlapsInsStrips[nStripIdx][nIdxInstrip]*100;
			*(pX+nCount)=GetImgIdx(nStripIdx,nIdxInstrip);
			nCount++;
		}
	}
	if(nCount>0)
	{
		dMeanOverlapX/=nCount;
		dMeanOverlapY/=nCount;
		sprintf(strLine,"单张影像按最大重叠度进行统计：");
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT);
		pdf.AddEmptyLine(1);
		sprintf(strLine,"航向重叠度最大值%3.2lf%%，最小值%3.2lf%%，平均值%3.2lf%%；",dMaxOverlapY*100,dMinOverlapY*100,dMeanOverlapY*100);
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT);
		pdf.AddEmptyLine(1);
		sprintf(strLine,"旁向重叠度最大值%3.2lf%%，最小值%3.2lf%%，平均值%3.2lf%%。",dMaxOverlapX*100,dMinOverlapX*100,dMeanOverlapX*100);					
		pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT);
		pdf.AddEmptyLine(2);
	}
	sprintf(strLine,"航向重叠度不合格（< %.2lf%%）影像：%d张",m_FlyQuaParaSet.fMinOverlapY,m_vecImgIdxLowOverlapInsStrips.size());
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1,0,0);
	if(m_vecImgIdxLowOverlapInsStrips.size()>0)
	{
		pdf.AddEmptyLine(1);
		nListCols=4;nListRows=m_vecImgIdxLowOverlapInsStrips.size()+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		vecListInfo[0*nListCols+2].Format("航带信息");
		vecListInfo[0*nListCols+3].Format("航向重叠度");
		for (int i = 0; i<nListRows-1; i++)
		{
			int nImgIdx = m_vecImgIdxLowOverlapInsStrips[i];
			double dOverlap=
				m_vvOverlapsInsStrips[m_vecImgInfo[nImgIdx].m_nStripIdx][m_vecImgInfo[nImgIdx].m_nIdxInStrip];
			vecListInfo[(i+1)*nListCols+0].Format("%d",nImgIdx);
			vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[nImgIdx].m_strImgName);
			vecListInfo[(i+1)*nListCols+2].Format("%s",m_vecImgInfo[nImgIdx].m_strStripLabel);
			vecListInfo[(i+1)*nListCols+3].Format("%.2lf%%",dOverlap*100);
		}
		cprintf("Overlap_Y...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
		pdf.AddEmptyLine(1);
		pdf.AddImage(strOverlapYErrViewImgPath,2);
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 航向重叠度不合格影像分布示意图(注：蓝色点为超限影像，绿色点为合格影像)",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(3);
	}
	FunOutPutLogInfo("航向重叠度写入完成！");
	pdf.AddEmptyLine(2);
	sprintf(strLine,"旁向重叠度不合格（< %.2lf%%）影像：%d张",m_FlyQuaParaSet.fMinOverlapX,m_vecImgIdxLowOverlapBetStrips.size());
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1,0,0);
	char strlog[MAX_SIZE_FILE_LINE];
	if(m_vecImgIdxLowOverlapBetStrips.size()>0)
	{
		pdf.AddEmptyLine(1);
		nListCols=4;nListRows=m_vecImgIdxLowOverlapBetStrips.size()+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		vecListInfo[0*nListCols+2].Format("航带信息");
		vecListInfo[0*nListCols+3].Format("旁向重叠度");
		for (int i = 0; i<nListRows-1; i++)
		{
			int nImgIdx = m_vecImgIdxLowOverlapBetStrips[i];
			double dOverlap=
				m_vvOverlapsBetStrips[m_vecImgInfo[nImgIdx].m_nStripIdx][m_vecImgInfo[nImgIdx].m_nIdxInStrip];
			vecListInfo[(i+1)*nListCols+0].Format("%d",nImgIdx);
			vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[nImgIdx].m_strImgName);
			vecListInfo[(i+1)*nListCols+2].Format("%s",m_vecImgInfo[nImgIdx].m_strStripLabel);
			vecListInfo[(i+1)*nListCols+3].Format("%.2lf%%",dOverlap*100);
		}
		//////////////////////////////////////////////////////////
		cprintf("Overlap_X...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
		pdf.AddEmptyLine(1.5);
		pdf.AddEmptyLine(1);
		pdf.AddImage(strOverlapXErrViewImgPath,2);
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 旁向重叠度不合格影像分布示意图(注：蓝色点为超限影像，绿色点为合格影像)",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(3);
	}
	pdf.AddEmptyLine(3);
	if(nCount>0)
	{
		FunSortByX(nCount,pX,pOverlapY,pOverlapX);
		pdf.AddChart(pOverlapY,pX,nCount,0,0,1,0,0,1.0,0,0,"航向重叠度","影像索引","重叠度(%)");
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 影像航向重叠度统计图",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(1.5);
		pdf.AddChart(pOverlapX,pX,m_nImgNum-m_nInvalidNum,0,0,1,0,0,1.0,0,0,"旁向重叠度","影像索引","重叠度(%)");
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 影像旁向重叠度统计图",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
	}
	delete[]pOverlapX;	
	delete[]pOverlapY;
	delete[]pX;
	pdf.AddEmptyLine(3);
	FunOutPutLogInfo("旁向重叠度写入完成！");

	/***********************空三困难影像*************************/
	pdf.AddText("6.空三困难影像",SSW_HEI,2);
	pdf.AddEmptyLine(2);
	if(m_vecImgIdxLowMatchPointNum.size()>0)
	{
		nListCols = 3; nListRows = m_vecImgIdxLowMatchPointNum.size()+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		vecListInfo[0*nListCols+2].Format("空三点数");
		for (int i = 0; i<m_vecImgIdxLowMatchPointNum.size(); i++)
		{
			vecListInfo[(i+1)*nListCols+0].Format("%d",m_vecImgIdxLowMatchPointNum[i]);
			vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[m_vecImgIdxLowMatchPointNum[i]].m_strImgName);
			vecListInfo[(i+1)*nListCols+2].Format("%d",m_vecImgInfo[m_vecImgIdxLowMatchPointNum[i]].m_ImgAdjPts.vecAdjPtIdx.size());
		}
		cprintf("MatchErr...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);

	}
	else
	{
		pdf.AddText("无空三困难影像！",SSW_SONG,1.5,SSW_LEFT,0,true,1);
	}
	pdf.AddEmptyLine(3);
	FunOutPutLogInfo("旁向重叠度写入完成！");
	/***********************影像姿态角评价*************************/
	pdf.AddText("7.影像姿态角评价",SSW_HEI,2);
	pdf.AddEmptyLine(2);
	sprintf(strLine,"倾角不合格（> %.2lfdeg）影像：%d张",m_FlyQuaParaSet.fMaxPitch,m_vecImgIdxHeiPitch.size());
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1);
	pdf.AddEmptyLine(1);
	if(m_vecImgIdxHeiPitch.size()>0)
	{	
		nListCols=3;nListRows=m_vecImgIdxHeiPitch.size()+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		vecListInfo[0*nListCols+2].Format("倾角（deg）");
		for (int i = 0; i<nListRows-1; i++)
		{
			int nImgIdx = m_vecImgIdxHeiPitch[i];
			vecListInfo[(i+1)*nListCols+0].Format("%d",nImgIdx);
			vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[nImgIdx].m_strImgName);
			vecListInfo[(i+1)*nListCols+2].Format("%.2lf",m_vecImagePitch[nImgIdx]);
		}
		cprintf("PitchInfo...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
		pdf.AddEmptyLine(1);
		pdf.AddEmptyLine(1);
		pdf.AddImage(strPitchErrViewImgPath,2);
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 倾角不合格影像分布示意图(注：蓝色点为超限影像，绿色点为合格影像)",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(3);
	}	
	float *pPitch = new float[m_nImgNum-m_nInvalidNum];
	pX = new float[m_nImgNum-m_nInvalidNum];
	nCount=0;
	for (int i = 0; i<m_vecImagePitch.size(); i++)
	{
		if(!m_vecImgInfo[i].m_bValid)continue;
		*(pPitch+nCount)=m_vecImagePitch[i];
		*(pX+nCount)=i;
		nCount++;
	}
	if(nCount>0)
	{
		pdf.AddChart(pPitch,pX,nCount,0,0,1,0,0,1,0,0,"影像倾角","影像索引","倾角值（deg）");
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 影像倾角曲线图",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(3);
	}
	FunOutPutLogInfo("倾角写入完成！");
	sprintf(strLine,"旋角不合格（> %.2lfdeg）影像：%d张",m_FlyQuaParaSet.fMaxYaw,m_vecImgIdxHeiYaw.size());
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1);
	pdf.AddEmptyLine(1);
	if(m_vecImgIdxHeiYaw.size()>0)
	{
		nListCols=3;nListRows=m_vecImgIdxHeiYaw.size()+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		vecListInfo[0*nListCols+2].Format("旋角（deg）");
		for (int i = 0; i<nListRows-1; i++)
		{
			int nImgIdx = m_vecImgIdxHeiYaw[i];
			vecListInfo[(i+1)*nListCols+0].Format("%d",nImgIdx);
			vecListInfo[(i+1)*nListCols+1].Format("%s",m_vecImgInfo[nImgIdx].m_strImgName);
			vecListInfo[(i+1)*nListCols+2].Format("%.2lf",m_vvStripImageYaw[m_vecImgInfo[nImgIdx].m_nStripIdx][m_vecImgInfo[nImgIdx].m_nIdxInStrip]);
		}
		cprintf("YawInfo...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
		pdf.AddEmptyLine(1);
		pdf.AddImage(strYawViewImgPath,2);
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 旋角不合格影像分布示意图(注：蓝色点为超限影像，绿色点为合格影像)",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
		pdf.AddEmptyLine(3);
	}
	float *pYaw = new float[m_nImgNum-m_nInvalidNum];
	memset(pYaw,0,sizeof(float)*(m_nImgNum-m_nInvalidNum));
	nCount = 0;
	for (int i = 0; i<m_vvStripImageYaw.size(); i++)
	{
		for (int j = 0; j<m_vvStripImageYaw[i].size(); j++)
		{
			int nStripIdx = i;
			int nIdxInStrip = j;
			*(pYaw+nCount)=m_vvStripImageYaw[nStripIdx][nIdxInStrip];
			*(pX+nCount)=GetImgIdx(nStripIdx,nIdxInStrip);
			nCount++;
		}
	}
	FunSortByX(nCount,pX,pYaw);
	if(nCount>0)
	{
		pdf.AddChart(pYaw,pX,m_nImgNum-m_nInvalidNum,0,0,1,0,0,1,0,0,"影像旋角","影像索引","旋角值（deg）");
		pdf.AddEmptyLine(0.5);
		sprintf(strImgLabelInfo,"图%d. 影像旋角曲线图",nImgIdx);
		pdf.AddText(strImgLabelInfo,SSW_HEI,1,SSW_CENTER);
		nImgIdx++;
	}
	delete[]pYaw;
	delete[]pPitch;
	delete[]pX;
	FunOutPutLogInfo("旋角写入完成！");
	pdf.AddEmptyLine(3);
	sprintf(strLine,"飞行方向调整处无效影像：%d张",m_nInvalidNum);
	pdf.AddText(strLine,SSW_SONG,1.5,SSW_LEFT,0,false,1);
	pdf.AddEmptyLine(1);
	if(m_nInvalidNum>0) 
	{
		nListCols = 2; nListRows = m_nInvalidNum+1;
		vecListInfo.resize(nListCols*nListRows);
		vecListInfo[0*nListCols+0].Format("影像索引");
		vecListInfo[0*nListCols+1].Format("影像名");
		int nCount =0;
		for (int i = 0; i<m_nImgNum; i++)
		{
			if(!m_vecImgInfo[i].m_bValid)
			{
				vecListInfo[(nCount+1)*nListCols+0].Format("%d",i);
				vecListInfo[(nCount+1)*nListCols+1].Format("%s",m_vecImgInfo[i].m_strImgName);
				nCount++;
			}
		}
		cprintf("InvalidImg...");
		pdf.AddList(nListCols,nListRows,vecListInfo);
		cprintf("...Over\n");
		vector<CString>().swap(vecListInfo);
	}
	FunOutPutLogInfo("飞行调整影像写入完成！");
	pdf.AddEmptyLine(3);

	/***********************其他质检信息*************************/
	pdf.AddText("8.质检处理耗时统计",SSW_HEI,2);
	pdf.AddEmptyLine(2);
	clock_t t2 = clock();
	double dt = (double)(t2-t1)/CLOCKS_PER_SEC;
	m_dTimeSecsWriteChkRep=dt;
	m_dTimeSecsWholeProcess=m_dTimeSecsCmrDistortion+m_dTimeSecsBundleAdjustment+m_dTimeSecsTimeReadAdjRep+m_dTimeSecsRunFlyQuaChk+m_dTimeSecsWriteChkRep;
	nListCols = 2; nListRows = 7;
	vecListInfo.resize(nListCols*nListRows);
	vecListInfo[0*nListCols+0].Format("处理过程");
	vecListInfo[0*nListCols+1].Format("耗时-s");
	vecListInfo[1*nListCols+0].Format("完整流程");
	vecListInfo[1*nListCols+1].Format("%.3lf",m_dTimeSecsWholeProcess);
	vecListInfo[2*nListCols+0].Format("畸变改正");
	vecListInfo[2*nListCols+1].Format("%.3lf",m_dTimeSecsCmrDistortion);
	vecListInfo[3*nListCols+0].Format("匹配平差");
	vecListInfo[3*nListCols+1].Format("%.3lf",m_dTimeSecsBundleAdjustment);
	vecListInfo[4*nListCols+0].Format("解析平差报告");
	vecListInfo[4*nListCols+1].Format("%.3lf",m_dTimeSecsTimeReadAdjRep);
	vecListInfo[5*nListCols+0].Format("计算质检参数");
	vecListInfo[5*nListCols+1].Format("%.3lf",m_dTimeSecsRunFlyQuaChk);
	vecListInfo[6*nListCols+0].Format("保存质检报告");
	vecListInfo[6*nListCols+1].Format("%.3lf",m_dTimeSecsWriteChkRep);
	cprintf("TimeInfo...");
	bool b = pdf.AddList(nListCols,nListRows,vecListInfo);
	cprintf("...Over\n");
	vector<CString>().swap(vecListInfo);
	pdf.AddEmptyLine(1);
	FunOutPutLogInfo("报告写入完毕！");
	CString strPath = m_strFlyQuaChkRepPath;

	if(!pdf.SavePdf(strPath))
		return false;
	delete[]strLine;
	return true;
}