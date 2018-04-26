#include "StripSorting.h"
#define InvalidValue -99999
void strip_sorting(double line_a, double line_b, double line_c,vector<stuPosInfo>& vecPosInfo)
{
	double sinStripDirKap = line_b/sqrt(pow(line_a,2)+pow(line_b,2));
	double cosStripDirKap = line_a/sqrt(pow(line_a,2)+pow(line_b,2));
	vector<int> vecValidIdx;
	for (int i = 0; i<vecPosInfo.size(); i++)
	{
		vecPosInfo[i].nIdxInStrip=InvalidValue;
		vecPosInfo[i].nStripIdx=InvalidValue;
		if(vecPosInfo[i].bValid)
			vecValidIdx.push_back(i);
	}
	if(vecValidIdx.size()==0) return;


	vector<double> vecLocXs(vecValidIdx.size(),0);
	vector<double> vecLocYs(vecValidIdx.size(),0);                   
	vector<int> vecLocXYIdx2ImgIdx(vecValidIdx.size(),0);
	vector<int> vecImgStripIdx(vecPosInfo.size(),InvalidValue);

	//1. 计算局部坐标（坐标轴与航带平行-垂直）
	int nCount = 0;
	for (int i = 0; i<vecValidIdx.size(); i++)
	{
		int nIdx = vecValidIdx[i];
		vecLocXs[nCount] = cosStripDirKap*vecPosInfo[nIdx].Xs+sinStripDirKap*vecPosInfo[nIdx].Ys;
		vecLocYs[nCount] = cosStripDirKap*vecPosInfo[nIdx].Ys-sinStripDirKap*vecPosInfo[nIdx].Xs;
		vecLocXYIdx2ImgIdx[nCount]=nIdx;
		nCount++;
	}
	//2. 按照局部坐标Xs排序-降序
	for (int i= 0; i<vecValidIdx.size()-1; i++)
	{
		for (int j = i+1; j<vecValidIdx.size(); j++)
		{
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
	for (int i = 0; i<vecValidIdx.size(); i++)
	{
		for (int j = 0; j<vecValidIdx.size(); j++)
		{
			if(j==i) continue;
			if(dMinDis>sqrt(pow(vecLocXs[i]-vecLocXs[j],2)+pow(vecLocYs[i]-vecLocYs[j],2))){
				n1=i;n2=j;
				dMinDis=min(dMinDis,sqrt(pow(vecLocXs[i]-vecLocXs[j],2)+pow(vecLocYs[i]-vecLocYs[j],2)));
			}
		}
	}
	//4.排列航带
	int nStripIdx = 0, nImgNumInStrip = 0, nStripNum; 
	double dStripStartX = InvalidValue; 
	vector<int> vecImgNumInEachStrip, vecStripIdx;
	vector<vector<int>> vvImgIdxInStrips;
	for (int i = 0; i<vecValidIdx.size(); i++)
	{
		if(dStripStartX==InvalidValue)
		{
			dStripStartX=vecLocXs[i];
			vecImgStripIdx[vecLocXYIdx2ImgIdx[i]]=nStripIdx;
			nImgNumInStrip++;
			continue;
		}
		double dis = fabs(dStripStartX-vecLocXs[i]);
		if(dis<=dMinDis)
		{
			dStripStartX=vecLocXs[i];
			vecImgStripIdx[vecLocXYIdx2ImgIdx[i]]=nStripIdx;
			nImgNumInStrip++;
		}
		else
		{
			vecImgNumInEachStrip.push_back(nImgNumInStrip);
			vecStripIdx.push_back(nStripIdx);
			nStripIdx++;
			nImgNumInStrip=0;
			dStripStartX=vecLocXs[i];
			vecImgStripIdx[vecLocXYIdx2ImgIdx[i]]=nStripIdx;
			nImgNumInStrip++;
		}
	}
	if(nImgNumInStrip>0) 
	{
		vecImgNumInEachStrip.push_back(nImgNumInStrip);
		vecStripIdx.push_back(nStripIdx);
	}
	//5.航带内排序
	for (int i = 0; i<vecValidIdx.size()-1; i++)
	{
		for (int j = i+1; j<vecValidIdx.size(); j++)
		{
			if(vecImgStripIdx[vecLocXYIdx2ImgIdx[i]]>vecImgStripIdx[vecLocXYIdx2ImgIdx[j]])
			{
				swap(vecLocXYIdx2ImgIdx[i],vecLocXYIdx2ImgIdx[j]);
				swap(vecLocXs[i],vecLocXs[j]);
				swap(vecLocYs[i],vecLocYs[j]);
			}
		}	
	}
	nCount = 0;
	for (int i = 0; i<vecImgNumInEachStrip.size(); i++)
	{
		for (int j = 0; j<vecImgNumInEachStrip[i]-1; j++)
		{
			for (int k = j+1; k<vecImgNumInEachStrip[i]; k++)
			{
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
		nCount+=vecImgNumInEachStrip[i];
	}	
	nCount = 0;
	nStripNum = vecImgNumInEachStrip.size();
	vvImgIdxInStrips.resize(nStripNum);
	nCount = 0;
	for (int i = 0; i<nStripNum; i++)
	{		
		vector<int> vecImgIdx(vecImgNumInEachStrip[i]);
		for (int j = 0; j<vecImgNumInEachStrip[i]; j++)
		{
			int nIdx = vecLocXYIdx2ImgIdx[nCount+j];
			vecPosInfo[nIdx].nIdxInStrip=j;
			vecPosInfo[nIdx].nStripIdx=i;
			vecImgIdx[j]=nIdx;
		}
		nCount+=vecImgNumInEachStrip[i];
		vvImgIdxInStrips[i]=vecImgIdx;	
		vector<int>().swap(vecImgIdx);
	}
	vector<int>().swap(vecValidIdx);
	vector<double>().swap(vecLocXs);
	vector<double>().swap(vecLocYs);
	vector<int>().swap(vecImgStripIdx);
	vector<int>().swap(vecLocXYIdx2ImgIdx);
	vector<int>().swap(vecImgNumInEachStrip);
	vector<int>().swap(vecStripIdx);
	for (int i = 0; i<vvImgIdxInStrips.size(); i++)
	{
		vector<int>().swap(vvImgIdxInStrips[i]);
	}
	vector<vector<int>>().swap(vvImgIdxInStrips);
	
}