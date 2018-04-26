#pragma once
#include "MyDataBaseType.h"
//保存系统配置文件
bool FunSaveSysConfigFile(DYstuSysConfigPara& sysCfgPara);
//解析系统配置文件
bool FunReadSysConfigFile(DYstuSysConfigPara& sysCfgPara);

//删除目录及文件
bool FunRemoveDirAndFile(CString strFolderPath);

void FunVectorPt2VertexXY(void *pVector, int nDim, double *&pV);
void FunCreateVertexColor(int nPtNum, double r,double g,double b, double *&pC);


Point2D ptCln2Geo(Point2D ptClnCenter2Geo, float fScale, CRect rectCln, CPoint& ptCln);
CPoint ptGeo2Cln(Point2D ptClnCenter2Geo, float fScale, CRect rectCln, Point2D& ptGeo);
void CalLineParas(void* p1, void* p2, int nDims, float &a, float &b, float &c);
double LineY2X(double y, float a, float b, float c);

