#ifndef strip_sorting_h
#define strip_sorting_h
#pragma once
#include <vector>
using namespace std;
#ifdef  SSWUAVFLYQUA_EXPORTS
#define SSWUAVFLY __declspec(dllexport)
#else
#define SSWUAVFLY __declspec(dllimport)
#endif
struct SSWUAVFLY stuPosInfo
{
	double Xs,Ys,Zs;
	double phi,omg,kap;
	int nStripIdx;   //航带编号
	int nIdxInStrip; //航带内
	bool bValid;     //是否参与处理  2017-03-22
};
/************************************************************************/
/*根据手绘直线重排航带
直线方程：
ax+by+c=0;
函数参数：
line_a,line_b,line_c对应直线方程三个参数
vecPosInfo POS数据
/************************************************************************/

void SSWUAVFLY strip_sorting(double line_a, double line_b, double line_c,vector<stuPosInfo>& vecPosInfo);


#endif

