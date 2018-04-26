#pragma once
#include "sswUAVFlyQuaEng.h"
#define MAX_ADD_DISTORTION_ITER 10
static enum eDistortionReturn
{
	SSWSuccess=0,
	SSWPathErr=1,
	SSWOpenErr=2,
	SSWReadErr=3,
	SSWCmrErr=4,
	SSWPosErr=5,
	SSWDataErr=6,
	SSWSaveErr=7
};
static CString ReturnDat2Info(eDistortionReturn eReturn)
{
	switch(eReturn)
	{
	case SSWSuccess: return "successful!";
	case SSWPathErr: return "path error!";
	case SSWOpenErr: return "open error!";
	case SSWReadErr: return "read error!";
	case SSWCmrErr:  return "camera paras error!";
	case SSWPosErr:  return "POS paras error!";
	case SSWDataErr: return "data loading error!";
	case SSWSaveErr: return "save error!";
	}
}
class CsswUAVCmrDsitortionCorrect :
	public CsswUAVImage
{
public:
	CsswUAVCmrDsitortionCorrect();
	~CsswUAVCmrDsitortionCorrect();

	eDistortionReturn LoadImageInfo(CString strImgPath, SSWstuCmrInfo& sswCmrInfo, CString strSavePath, float fScale = 1.0);
	eDistortionReturn RunDistortionCorrect(SSWstuCmrInfo& cmrNew, bool bChkExist = false);
	string GetResImgPath(){ return m_strSavePath;}
protected:
	void Remove_Distortion_Level3_2(double xori,double yori, double &xdst, double &ydst);
	void Add_Distortion_Level3_2(double xori,double yori, double &xdst, double &ydst);
protected:
	bool m_bDataReady;
	CString m_strImgPath;
	CString m_strSavePath;
	float m_fScale;
	SSWstuCmrInfo m_CmrInfo;
};
