#include "stdafx.h"
#include "sswUAVCmrDsitortionCorrect.h"
#include <omp.h>
#include <io.h>
CsswUAVCmrDsitortionCorrect::CsswUAVCmrDsitortionCorrect(void)
{
	m_bDataReady = false;;
	m_fScale = 1.0;
}
CsswUAVCmrDsitortionCorrect::~CsswUAVCmrDsitortionCorrect(void)
{
}


void CsswUAVCmrDsitortionCorrect::Remove_Distortion_Level3_2(double xori,double yori, double &xdst, double &ydst)
{
	double f  = m_CmrInfo.f;
	double x0 = m_CmrInfo.x0;
	double y0 = m_CmrInfo.y0;
	double k1 = m_CmrInfo.k1;
	double k2 = m_CmrInfo.k2;
	double p1 = m_CmrInfo.p1;
	double p2 = m_CmrInfo.p2;
	double a = m_CmrInfo.a;
	double b = m_CmrInfo.b;
	double x1 = xori;
	double y1 = yori;
	double x2 = x1*x1;
	double y2 = y1*y1;
	double xy = x1*y1;
	double r2 = x2 + y2;
	double r4 = r2*r2;
	double delta_x = x1*(k1*r2 + k2*r4) + p1*(r2 + 2.0*x2) + 2.0*p2*xy + a*x1 + b*y1;
	double delta_y = y1*(k1*r2 + k2*r4) + p2*(r2 + 2.0*y2) + 2.0*p1*xy;
	xdst = xori - delta_x;
	ydst = yori - delta_y;
}
void CsswUAVCmrDsitortionCorrect::Add_Distortion_Level3_2(double xori,double yori, double &xdst, double &ydst)
{
	double ori_xy[2], undis_xy[2];
	ori_xy[0] = xori;
	ori_xy[1] = yori;
	for (int i=0; i<MAX_ADD_DISTORTION_ITER; i++)
	{
		ori_xy[0] -= m_CmrInfo.x0;
		ori_xy[1] -= m_CmrInfo.y0;
		Remove_Distortion_Level3_2(ori_xy[0],ori_xy[1], undis_xy[0],undis_xy[1]);
		ori_xy[0] = xori + (ori_xy[0] - undis_xy[0]);
		ori_xy[1] = yori + (ori_xy[1] - undis_xy[1]);
	}
	xdst = ori_xy[0];
	ydst = ori_xy[1];
}
eDistortionReturn CsswUAVCmrDsitortionCorrect::LoadImageInfo(CString strImgPath, SSWstuCmrInfo& sswCmrInfo, CString strSavePath, float fScale /* = 1.0 */
	)
{
	eDistortionReturn eReturn;
	if(!PathFileExists(strImgPath)) 
	{
		m_bDataReady=false;
		return SSWPathErr;
	}
	else if(!OpenImg(strImgPath,true)) 
	{
		m_bDataReady=false;
		return SSWOpenErr;
	}	
	else if(!sswCmrInfo.IsValid())
	{
		m_bDataReady=false;
		CloseImg();
		return SSWCmrErr;
	}

	m_strImgPath = strImgPath;
	m_strSavePath = strSavePath;
	m_CmrInfo=sswCmrInfo;
//	m_bWithCmr = bWithCmr;
	if(fScale>1) m_fScale = fScale;
	m_bDataReady=true; 
	CloseImg();
	return SSWSuccess;
}
eDistortionReturn CsswUAVCmrDsitortionCorrect::RunDistortionCorrect(SSWstuCmrInfo& cmrNew, bool bChkExist /* = false */)
{
// 	if(m_bWithCmr)
// 	{
		//修正相机参数
		cmrNew=m_CmrInfo;
		cmrNew.x0 = 0;
		cmrNew.y0 = 0;
		cmrNew.strLabel += "_resample";
		cmrNew.pixsize = cmrNew.pixsize*m_fScale;
		cmrNew.f = cmrNew.f/m_fScale;
		cmrNew.fx = cmrNew.fx/m_fScale;
		cmrNew.fy = cmrNew.fy/m_fScale;
		cmrNew.k1=cmrNew.k2=0;
		cmrNew.p1=cmrNew.p2=0;
		cmrNew.a=cmrNew.b=0;
//	}
	if(bChkExist&&PathFileExists(m_strSavePath)) return SSWSuccess; 
	if(!m_bDataReady) return SSWDataErr;
	if(!OpenImg(m_strImgPath,true)) return SSWOpenErr;
	int nCols = GetCols();
	int nRows = GetRows();
	int nBands= GetBandCount();
	int nBands2Cal = min(3,nBands);
	int dCols=0, dRows=0;     //畸变纠正后原始比例影像长宽
	int sCols=0, sRows=0;    //重采样后影像长宽

	//原始影像灰度，供插值使用
	BYTE *pData4OriImg=NULL, *pData4ResImg = NULL;
	if(!m_CmrInfo.IsDistortion()/*||!m_bWithCmr*/)  //相机无畸变或者无相机参数，直接进行重采样
	{
		if(m_fScale==1.0&&nBands==nBands2Cal)
		{
			CloseImg();
			if(!CopyFile(m_strImgPath,m_strSavePath,FALSE)) return SSWSaveErr;
			return SSWSuccess;
		}
		else
		{
			sCols = nCols/m_fScale;
			sRows = nRows/m_fScale;
			pData4OriImg = new BYTE[sCols*sRows*nBands];memset(pData4OriImg,0,sCols*sRows*nBands);
			if(!ReadImg(0,0,nCols,nRows,sCols,sRows,pData4OriImg)) 
			{	
				CloseImg();
				if(pData4OriImg) delete[] pData4OriImg; pData4OriImg=NULL;
				return SSWReadErr;
			}	
			CloseImg();
			if(nBands==nBands2Cal)
			{
				if(!SaveImg(m_strSavePath,sCols,sRows,nBands2Cal,pData4OriImg))
				{
					if(pData4OriImg) delete[] pData4OriImg; pData4OriImg=NULL;
					return SSWSaveErr;
				}
				if(pData4OriImg) delete[] pData4OriImg; pData4OriImg=NULL;
				return SSWSuccess;
			}
			else if(nBands>nBands2Cal)
			{
				pData4ResImg = new BYTE[sCols*sRows*nBands2Cal];memset(pData4ResImg,0,sCols*sRows*nBands2Cal);
				for (int i = 0; i<sCols*sRows; i++)
				{
					for (int nb = 0;nb<nBands2Cal; nb++)
					{
						*(pData4ResImg+i*nBands2Cal+nb)=*(pData4OriImg+i*nBands+nb);
					}
				}
				if(!SaveImg(m_strSavePath,sCols,sRows,nBands2Cal,pData4OriImg))
				{
					if(pData4OriImg)delete[] pData4OriImg; pData4OriImg=NULL;
					if(pData4ResImg)delete[] pData4ResImg; pData4ResImg=NULL;
					return SSWSaveErr;
				}
			}
			if(pData4OriImg)delete[] pData4OriImg; pData4OriImg=NULL;
			if(pData4ResImg)delete[] pData4ResImg; pData4ResImg=NULL;
			return SSWSuccess;
		}
	}
	else
	{

		//计算原始影像的四个角点的纠正后坐标，确定纠正后的影像范围
		double cornerx[4] = {-nCols/2.0,-nCols/2.0,nCols/2.0,nCols/2.0}
		,cornery[4] = {-nRows/2.0,nRows/2.0,-nRows/2.0,nRows/2.0};
		double cornerx_d[4],cornery_d[4];
		for (int n=0;n<4;n++)
		{
			cornerx[n] = cornerx[n]*m_CmrInfo.pixsize-m_CmrInfo.x0;
			cornery[n] = cornery[n]*m_CmrInfo.pixsize-m_CmrInfo.y0;
			Remove_Distortion_Level3_2(cornerx[n],cornery[n],cornerx_d[n],cornery_d[n]);
		}
		double maxxd(0.0),minxd(999999.0),maxyd(0.0),minyd(999999.0);
		for (int n=0;n<4;n++)
		{
			maxxd = max(maxxd,cornerx_d[n]);
			minxd = min(minxd,cornerx_d[n]);
			maxyd = max(maxyd,cornery_d[n]);
			minyd = min(minyd,cornery_d[n]);
		}
		double startx,starty;		//纠正起始坐标
		if (maxxd>-minxd)
		{
			dCols = maxxd*2/m_CmrInfo.pixsize;
			startx =  -maxxd;
		}
		else
		{
			dCols = -minxd*2/m_CmrInfo.pixsize;
			startx = minxd;
		}
		if (maxyd>-minyd)
		{
			dRows = maxyd*2/m_CmrInfo.pixsize;
			starty = -maxyd;
		}
		else
		{
			dRows = -minyd*2/m_CmrInfo.pixsize;
			starty = minyd;
		}

		//------开始纠正------//
		int sRows=dRows/m_fScale;  //缩放影像宽
		int sCols=dCols/m_fScale;  //缩放影像高
		double neighbour_x[4],neighbour_y[4];		//领域4个点坐标，采样方法为双线性采样
		double neighbour_dx[4],neighbour_dy[4];	//纠正后的领域4个点坐标
		pData4OriImg = new BYTE[nCols*nRows*nBands];memset(pData4OriImg,0,nCols*nRows*nBands);
		if(!ReadImg(0,0,nCols,nRows,nCols,nRows,pData4OriImg))
		{
			if(pData4OriImg)delete[] pData4OriImg; pData4OriImg=NULL;
			if(pData4ResImg)delete[] pData4ResImg; pData4ResImg=NULL;
			CloseImg();
			return SSWReadErr;
		}
		CloseImg();
		pData4ResImg = new BYTE[sCols*sRows*nBands2Cal]; memset(pData4ResImg,0,sCols*sRows*nBands2Cal);
		double pixelSize = m_CmrInfo.pixsize;
		for (int r=0;r<sRows;r++)
		{
			for (int c=0;c<sCols;c++)
			{	
				double ori_cp = c*m_fScale								//像素坐标-列
					,ori_rp = r*m_fScale;									//像素坐标-行
				double ori_x = (ori_cp-dCols/2.0)*pixelSize				//像平面坐标x
					,ori_y = (ori_rp-dCols/2.0)*pixelSize;					//像平面坐标y
				neighbour_x[0] = neighbour_x[1] = int(ori_cp)*pixelSize;		
				neighbour_x[2] = neighbour_x[3] = (int(ori_cp)+1)*pixelSize;
				neighbour_y[0] = neighbour_y[2] = int(ori_rp)*pixelSize;
				neighbour_y[1] = neighbour_y[3] = (int(ori_rp)+1)*pixelSize;
				for (int n=0;n<4;n++)
				{
					neighbour_x[n] += startx;
					neighbour_y[n] += starty;
					Add_Distortion_Level3_2(neighbour_x[n],neighbour_y[n],neighbour_dx[n],neighbour_dy[n]);
					//转换成像素坐标
					neighbour_dx[n] = (neighbour_dx[n]+m_CmrInfo.x0)/m_CmrInfo.pixsize+nCols/2.0;
					neighbour_dy[n] = (neighbour_dy[n]+m_CmrInfo.y0)/m_CmrInfo.pixsize+nRows/2.0;
				}
				//双线性采样
				BYTE *gOriAddWeight = new BYTE[nBands2Cal]; memset(gOriAddWeight,0,nBands2Cal); //采样结果
				double weight[4] = {0.0};
				weight[0] = (1-(ori_cp-int(ori_cp)))*(1-(ori_rp-int(ori_rp)));
				weight[1] = (1-(ori_cp-int(ori_cp)))*(ori_rp-int(ori_rp));
				weight[2] = (ori_cp-int(ori_cp))*(1-(ori_rp-int(ori_rp)));
				weight[3] = (ori_cp-int(ori_cp))*(ori_rp-int(ori_rp));
				for (int n=0;n<4;n++)
				{
					if (neighbour_dx[n]>=0&&neighbour_dx[n]<nCols&&
						neighbour_dy[n]>=0&&neighbour_dy[n]<nRows)
					{	
						int ny = (int)neighbour_dy[n];
						int nx = (int)neighbour_dx[n];
						for (int nb = 0; nb<nBands2Cal; nb++)
						{
							(*(gOriAddWeight+nb))+=weight[n]*(*(pData4OriImg+(ny*nCols+nx)*nBands+nb));
						}
					}
					else
					{
						for (int nb=0; nb<nBands2Cal; nb++)
						{
							*(gOriAddWeight+nb)=0;
						}
						break;
					}
				}
				for (int nb = 0; nb<nBands2Cal; nb++)
				{
					*(pData4ResImg+(r*sCols+c)*nBands2Cal+nb)=*(gOriAddWeight+nb);
				}
				delete[]gOriAddWeight;gOriAddWeight=NULL;
			}
		}
	
		if(!SaveImg(m_strSavePath,sCols,sRows,nBands2Cal,pData4ResImg)) 
		{
			if(pData4OriImg)delete[] pData4OriImg; pData4OriImg=NULL;
			if(pData4ResImg)delete[] pData4ResImg; pData4ResImg=NULL;
			return SSWSaveErr;
		}
		if(pData4OriImg)delete[] pData4OriImg; pData4OriImg=NULL;
		if(pData4ResImg)delete[] pData4ResImg; pData4ResImg=NULL;
		return SSWSuccess;
	}
}