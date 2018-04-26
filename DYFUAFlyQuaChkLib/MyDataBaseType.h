#ifndef _DY_data_base_type_h
#define _DY_data_base_type_h
#pragma once
#include <afx.h>  
#include <Shlwapi.h>
#include <algorithm>
#include <vector>
using namespace std;
#include "DYUAVFlyQuaChkPro.h"

#define InvalidValue          -99999


#define MAX_FILE_NUM         10000
#define MAX_SIZE_FILE_PATH   1024
#define MAX_SIZE_FILE_LINE   1024
#define MAX_SIZE_LOG_FILE    (1024*1024)
#define SIZE_CMR_INFO_EXTEND 13
#define SIZE_POS_INFO_EXTEND 7



template <class DataType>
class Point2_
{
public:
	typedef Point2_<DataType> MiType;
	Point2_(){x=InvalidValue;y=InvalidValue;}
	//		: Point2_(DataType(InvalidValue), DataType(InvalidValue)){}
	Point2_(const DataType _x, const DataType _y)
		: x(_x), y(_y){}
	Point2_(const MiType &p)
		: x(p.x), y(p.y){}
	~Point2_(){}

private:

public:
	DataType x;
	DataType y;

public:
	// define MiType operators

	MiType operator+(MiType pt){
		return Point2_(x + pt.x, y + pt.y);
	}
	MiType operator-(MiType pt){
		return Point2_(x - pt.x, y - pt.y);
	}
	MiType operator-(){
		return Point2_(-x, -y);
	}
	MiType operator*(MiType pt){
		return Point2_(x *= pt.x, y *= pt.y);
	}
	MiType operator=(MiType pt){
		return Point2_(x = pt.x, y = pt.y);
	}
	bool operator!=(MiType pt){
		return (x != pt.x|| y != pt.y);
	}
	bool operator==(MiType pt){
		return (x == pt.x && y == pt.y);
	}
	MiType operator+=(MiType pt){
		return (*this = *this + pt);
	}
	MiType operator-=(MiType pt){
		return (*this = *this - pt);
	}
	MiType operator*(DataType k){
		return Point2_(x *= k, y *= k);
	}
	MiType operator/(DataType k){
		if (k != 0) return Point2_(x /= k, y /= k);
		else return Point2_(x, y);
	}
};

template <class DataType>
class Point3_
{
public:
	typedef Point3_<DataType> MiType;

	Point3_(){X=InvalidValue;Y=InvalidValue;Z=InvalidValue;}
	//		: Point3_(DataType(InvalidValue), DataType(InvalidValue), DataType(InvalidValue)){}
	Point3_(const DataType _x, const DataType _y, const DataType _z)
		: X(_x), Y(_y), Z(_z){}
	~Point3_(){}


private:

public:
	DataType X;
	DataType Y;
	DataType Z;

public:

	MiType operator+(MiType gt){
		return Point3_(X + gt.X, Y + gt.Y, Z + gt.Z);
	}
	MiType operator-(MiType gt){
		return Point3_(X - gt.X, Y - gt.Y, Z - gt.Z);
	}
	MiType operator-(){
		return Point3_(-X, -Y, -Z);
	}
	MiType operator*(MiType gt){
		return Point3_(X *= gt.X, Y *= gt.Y, Z *= gt.Z);
	}
	MiType operator/(DataType a){
		if (a != DataType(0))
			return Point3_(X /= a, Y /= a, Z /= a);
		else return Point3_(999999999999999, 999999999999999, 9999999999999999);
	}
	MiType operator=(MiType gt){
		return Point3_(X = gt.X, Y = gt.Y, Z = gt.Z);
	}
	bool operator==(MiType gt){
		return (X == gt.X && Y == gt.Y && Z = gt.Z);
	}
	MiType operator+=(MiType gt){
		return (*this = *this + gt);
	}
	MiType operator-=(MiType gt){
		return (*this = *this - gt);
	}
	MiType operator/=(DataType a){
		if (a != DataType(0))
			return (*this = *this / a);
		else return Point3_(999999999999999, 999999999999999, 9999999999999999);
	}
	bool operator!=(MiType pt){
		return (X != pt.X|| Y != pt.Y||Z!=pt.Z);
	}
};

template <class DataType>
class CRect_
{
public:  //构造初始化
	typedef CRect_<DataType> MiType;
	CRect_(){}
	//		: CRect_(DataType(InvalidValue), DataType(InvalidValue), DataType(InvalidValue), DataType(InvalidValue)){}
	CRect_(const DataType _l, const DataType _t, const DataType _r, const DataType _b)
		: left(_l), top(_t), right(_r), bottom(_b){}
	CRect_(Point2_<DataType> p1, Point2_<DataType> p2)
		: left(p1.x), top(p1.y), right(p2.x), bottom(p2.y){}
	CRect_(const MiType &p)
		: left(p.left), top(p.top), right(p.right), bottom(p.bottom){}
public:
	DataType left;
	DataType right;
	DataType top;
	DataType bottom;

	Point2_<DataType> RightTop(){ return Point2_<DataType>(right, top); };
	Point2_<DataType> LeftBottom(){ return Point2_<DataType>(left, bottom); };
	Point2_<DataType> LeftTop(){ return Point2_<DataType>(left, top); };
	Point2_<DataType> RightBottom(){ return Point2_<DataType>(right, bottom); };
	Point2_<DataType> CenterPoint(){ return Point2_<DataType>((right + left) / 2, (top + bottom) / 2); };

	DataType Width(){ return fabs(right - left); };
	DataType Height(){ return fabs(top - bottom); };
	DataType Area(){ return Width()*Height(); };

	bool operator==(MiType drect){ //矩形赋值
		return (left == drect.left&& top == drect.top&& right == drect.right&& bottom == drect.bottom);
	}
	MiType operator=(MiType drect){ //矩形赋值
		return CRect_(left = drect.left, top = drect.top, right = drect.right, bottom = drect.bottom);
	}

	MiType operator+(MiType drect){//矩形求并
		return CRect_(left = min(left, drect.left), top = max(top, drect.top),
			right = max(right, drect.right), bottom = min(bottom, drect.bottom));
	}

public:
	bool Intersect_Rect(MiType dr){
		if (left > dr.right || right < dr.left ||
			top<dr.bottom || bottom>dr.top)
			return false;
		return true;
	}
	bool Intersect_Pt(Point2_<DataType> pt){
		if ((pt.x - left)*(pt.x - right) < 0 && (pt.y - top)*(pt.y - bottom) < 0)
			return true;
		return false;
	}

private:

};

typedef Point2_<double> Point2D;
typedef Point3_<double> Point3D;
typedef CRect_<double> CRectD;
inline bool   FunIntersectCRectD(CRectD rd1, CRectD rd2, CRectD &rd1_2)
{
	double l, t, r, b;
	l = max(rd1.left, rd2.left);
	r = min(rd1.right, rd2.right);
	if (r < l) return false;
	if (rd1.top >= rd1.bottom)
	{
		t = min(rd1.top, rd2.top);
		b = max(rd1.bottom, rd2.bottom);
		if (t < b) return false;
	}
	if (rd1.top < rd1.bottom)
	{
		t = max(rd1.top, rd2.top);
		b = min(rd1.bottom, rd2.bottom);
		if (t > b) return false;
	}
	rd1_2 = CRectD(l, t, r, b);
	return true;
};
inline bool   FunCombineCRectD(CRectD rd1, CRectD rd2, CRectD &rd1_2)
{
	if (rd1.Width()*rd1.Height() == 0 && rd2.Width()*rd2.Height() != 0)
	{
		rd1_2 = rd2; return true;
	}
	if (rd1.Width()*rd1.Height() != 0 && rd2.Width()*rd2.Height() == 0)
	{
		rd1_2 = rd1; return true;
	}
	double l, t, r, b;
	l = min(rd1.left, rd2.left);
	r = max(rd1.right, rd2.right);
	if (rd1.top > rd1.bottom)
	{
		t = max(rd1.top, rd2.top);
		b = min(rd1.bottom, rd2.bottom);
	}
	else
	{
		t = min(rd1.top, rd2.top);
		b = max(rd1.bottom, rd2.bottom);
	}
	rd1_2 = CRectD(l, t, r, b); return true;
};
inline double FunDistacePoint2D(Point2D pt1, Point2D pt2)
{
	return sqrt(pow(pt1.x-pt2.x,2)+pow(pt1.y-pt2.y,2));
};


#ifndef _global_functions
#define _global_functions
//获取文件目录
inline CString FunGetFileFolder(CString strFullPath)
{
	CString strFolder = strFullPath.Left(strFullPath.ReverseFind('\\'));
	return strFolder;
}
//获取文件名
inline CString FunGetFileName(CString strFullPath, bool bExt)
{
	CString strName;int nPos1=0;
	nPos1 = strFullPath.ReverseFind('\\');
	if(bExt)
	{
		strName=strFullPath.Right(strFullPath.GetLength()-nPos1-1);
	}
	else
	{
		int nPos2 = strFullPath.ReverseFind('.'); 
		if(nPos2==-1) nPos2 = strFullPath.GetLength();
		strName = strFullPath.Mid(nPos1+1,nPos2-nPos1-1);
	}
	return strName;
}
//获取文件后缀
inline CString FunGetFileExt(CString strFullPath)
{
	CString strExt = strFullPath.Right(strFullPath.GetLength()-strFullPath.ReverseFind('.')-1);
	return strExt;
}
//获取当前exe路径
inline CString FunGetThisExePath()
{
	char strExePath[MAX_SIZE_FILE_PATH]; memset(strExePath, 0, MAX_SIZE_FILE_PATH);
	GetModuleFileName(NULL, strExePath, sizeof(strExePath));
	return strExePath;
}
inline CString FunGetSysCfgPath()
{
	CString strLogPath = FunGetThisExePath();
	int np1 = strLogPath.ReverseFind('.');
	strLogPath=strLogPath.Left(np1);
	strLogPath+=".ini";
	return strLogPath;
}
//获取日志文件路径
inline CString	FunGetSyslogPath()
{
	CString strLogPath = FunGetFileFolder(FunGetThisExePath())+"\\"+FunGetFileName(FunGetThisExePath(),false)+"_FlyQua.log";
	return strLogPath;
}
//获取系统时间short sTime[8]
inline void	FunGetSysTimeDat(short* dTime)
{
	//	short dTime[8];
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	dTime[0] = (short)systime.wYear;
	dTime[1] = (short)systime.wMonth;
	dTime[2] = (short)systime.wDay;
	dTime[3] = (short)systime.wHour;
	dTime[4] = (short)systime.wMinute;
	dTime[5] = (short)systime.wSecond;
	dTime[6] = (short)systime.wMilliseconds;
	//	return dTime;
}
//获取系统时间 string
inline CString	FunGetSysTimeStr()
{
	short dTime[8]; FunGetSysTimeDat(dTime);
	char strTime[100], strTemp[10];
	if (dTime[0] < 1000) /*strTemp.Format("0%d-", dTime[0]);*/sprintf(strTemp,"0%d-",dTime[0]);
	else /*strTemp.Format("%d-", dTime[0]);*/sprintf(strTemp,"%d-",dTime[0]);
	/*strTime = strTemp;*/sprintf(strTime, "%s",strTemp);//Y-
	if (dTime[1] < 10) /*strTemp.Format("0%d-", dTime[1]);*/sprintf(strTemp,"0%d-",dTime[1]);
	else /*strTemp.Format("%d-", dTime[1]);*/sprintf(strTemp,"%d-",dTime[1]);
	/*strTime += strTemp;*/sprintf(strTime, "%s%s",strTime,strTemp);//Y-M-
	if (dTime[2] < 10) /*strTemp.Format("0%d ", dTime[2]);*/sprintf(strTemp,"0%d ",dTime[2]);
	else /*strTemp.Format("%d ", dTime[2]);*/sprintf(strTemp,"%d ",dTime[2]);
	/*strTime += strTemp;*/sprintf(strTime, "%s%s",strTime,strTemp);//Y-M-D
	if (dTime[3] < 10) /*strTemp.Format("0%d:", dTime[3]);*/sprintf(strTemp,"0%d:",dTime[3]);
	else /*strTemp.Format("%d:", dTime[3]);*/sprintf(strTemp,"%d:",dTime[3]);
	/*strTime += strTemp;*/sprintf(strTime, "%s%s",strTime,strTemp);//Y-M-D H:
	if (dTime[4] < 10) /*strTemp.Format("0%d:", dTime[4]);*/sprintf(strTemp,"0%d:",dTime[4]);
	else /*strTemp.Format("%d:", dTime[4]);*/sprintf(strTemp,"%d:",dTime[4]);
	/*strTime += strTemp;*/sprintf(strTime, "%s%s",strTime,strTemp);//Y-M-D h:m:
	if (dTime[5] < 10) /*strTemp.Format("0%d", dTime[5]);*/sprintf(strTemp,"0%d",dTime[5]);
	else /*strTemp.Format("%d", dTime[5]);*/sprintf(strTemp,"%d",dTime[5]);
	/*strTime += strTemp;*/sprintf(strTime, "%s%s",strTime,strTemp);//Y-M-D h:m:s
	return strTime;
}
//获取文件大小
inline long  	FunGetFileSize(CString strFilePath)
{
	FILE *pfCheckSize = fopen(strFilePath, "rb");
	long nFileSize = 0;
	if (pfCheckSize != NULL)
	{
		fseek(pfCheckSize, 0, SEEK_END);
		nFileSize = ftell(pfCheckSize);
	}
	if (pfCheckSize != NULL)
	{
		fclose(pfCheckSize); pfCheckSize = NULL;
	}
	return nFileSize;
}
//输出日志信息
inline void	FunOutPutLogInfo(CString strLogInfo)
{
	//	char* strLogFilePath = FunReplaceString(FunGetThisExePath(),".exe",".log");
	CString strLogFilePath = FunGetSyslogPath();
	CString strTime =FunGetSysTimeStr();
	long nFileSize = FunGetFileSize(strLogFilePath);
	if (nFileSize >= MAX_SIZE_LOG_FILE) DeleteFile(strLogFilePath);
	FILE *pfW = fopen(strLogFilePath, "at+");
	if (pfW != NULL)
	{
		if (nFileSize > 0 && strLogInfo == "\n") fprintf(pfW, "\n");
		else if (strLogInfo != "\n") fprintf(pfW, "%s ===> %s\n",strTime, strLogInfo);
		fclose(pfW); pfW = NULL;
	}
	printf("%s ===> %s\n",strTime, strLogInfo);
}
//分割字符串
inline vector<CString> FunStrTok(CString str, CString strDot)
{
	vector<CString> vPart;
	char strInfo[MAX_SIZE_FILE_PATH]; strcpy(strInfo, str);
	char* p = strtok(strInfo, strDot);
	while (p!=NULL)
	{
		vPart.push_back(p);
		p = strtok(NULL, strDot);
	}
	return vPart;
}
//检索文件
inline bool FunSearchFile(CString strFolderPath,CString strExt, vector<CString>& vecFilePath)
{
	if (!PathFileExists(strFolderPath))
	{
		FunOutPutLogInfo("文件检索失败，目录路径错误。");
		return false;
	}

	CString path = strFolderPath;
	CString  strFileName, strFileExt;
	if (path.Right(path.GetLength()-1) != "\\");//路径要以/结尾 
	path += _T("\\");
	path += _T("*.*");//CFileFind 类查找路径/*.gz时，指查找路径下所有文件 
	//path += _T("*.tif");//CFileFind 类查找路径/*.tiff时，指查找路径下所有文件 
	CFileFind ff;
	BOOL res = ff.FindFile(path);
	while (res)
	{
		res = ff.FindNextFile();
		//如果不是. ..（.表示当前目录，..表示上一级目录，也是以文件形式存在的)
		//也不是路径/文件夹，即如果是文件，就删除，你的操作就是获取文件名然后和要查找的文件进行比较
		if (!ff.IsDots() && !ff.IsDirectory())
		{
			if (ff.IsReadOnly())
			{
				::SetFileAttributes(ff.GetFilePath(), FILE_ATTRIBUTE_NORMAL);
			}
			//strFilePath是当前搜索到的文件的完整路径，含文件名和后缀
			CString strFilePath;
			strFilePath = ff.GetFilePath();

			//取得当前文件路径及文件名
			for (int j = strFilePath.GetLength(); j>0; j--)
			{
				
				if (strFilePath.GetAt(j) == '\\')
				{
					strFileName = strFilePath.Right(strFilePath.GetLength()-j-1);
					strFileExt = strFilePath.Right(strFilePath.GetLength()-strFilePath.ReverseFind('.')-1);
					if (strFileExt == strExt)
					{
						vecFilePath.push_back(strFilePath);//文件名依次存入到文件名数组中
					}
					break;
				}
			}
		}
		else   if (ff.IsDots())   //如果是. ..，就继续。    
			continue;
		else    if (ff.IsDirectory())///*如果是文件夹，就递归进入此方法继续删除，你的操作就是递归进入此方法继续对比文件名，并缓存起来。    */
		{
			//禁用递归遍历
// 			path = ff.GetFilePath();
// 			FunSearchFile(path, strExt, vecFilePath);
		}
	}
	return true;
}
//生成文件目录
inline void FunCreateDir4Path(CString strPath)
{
	if(PathFileExists(strPath))	return ;

	char tmpPath[MAX_PATH];
//	strPath = FunGetFileFolder(strPath);
	const char* pCur = strPath;

	memset(tmpPath,0,sizeof(tmpPath));

	int pos=0;
	while(*pCur++!='\0')
	{
		tmpPath[pos++] = *(pCur-1);

		if(*pCur=='/' ||*pCur=='\\'|| *pCur=='\0')
		{
			if(!PathFileExists(tmpPath)&&strlen(tmpPath)>0)
			{
				CreateDirectory(tmpPath, NULL);
			}
		}
	}
	FunOutPutLogInfo("创建目录"+strPath+"成功");
}

#endif

inline CString DYUnitType2String(DYUNITTYPE& eType)
{
	switch(eType)
	{
	case PIX: return "pix";
	case UM:  return "um";
	case MM:  return "mm";
	case CM:  return "cm";
	case M:   return "m";
	default: return "pix";
	}
}
inline DYUNITTYPE DYString2UnitType(CString strType)
{
	if(strType.CompareNoCase("pix")==0) return PIX;
	if(strType.CompareNoCase("um")==0) return UM;else
	if(strType.CompareNoCase("mm")==0) return MM;else
	if(strType.CompareNoCase("cm")==0) return CM;else
	if(strType.CompareNoCase("m")==0) return M;else
		return PIX;
}

inline CString DYCoordinateFormat2String(DYCOORDINATEFORMAT& eFormat)
{
	switch(eFormat)
	{
	case LBH: return "LBH";
	case XYZ: return "XYZ";
	default: return "LBH";
	}
}
inline DYCOORDINATEFORMAT DYString2CoordinateFormat(CString strFormat)
{	
	if(strFormat.CompareNoCase("LBH")==0) return LBH;else
	if(strFormat.CompareNoCase("XYZ")==0) return XYZ;else
		return LBH;
}
//质检参数
enum DYQUALITYTYPE
{
	Q_OVERLAP_Y = 0,
	Q_OVERLAP_X = 1,
	Q_POS_ERR = 2,
	Q_PITCH = 3,
	Q_YAW = 4
};
inline CString DYAngleFormat2String(DYANGLEFORMAT& eFormat)
{
	switch(eFormat)
	{
	case DEG: return "deg";
	case RAD: return "rad";
	default:return"deg";
	}
}
inline DYANGLEFORMAT DYString2AngleFormat(CString strFormat)
{	
	if(strFormat.CompareNoCase("deg")==0) return DEG;else
	if(strFormat.CompareNoCase("rad")==0) return RAD;else
		return DEG;
}
//质检方法
enum DYFLYQUAMETHOD
{
	BY_POS = 0,
	BY_MATCH = 1,
	BY_ADJUSTMENT = 2
};
//重叠度方向
enum DYOVERLAPDIRECTION
{
	DIRECTION_X = 0,  //旁向重叠度
	DIRECTION_Y = 1   //航向重叠度
};
//相机参数数据结构
struct DYstuCmrInfo
{
	CString strLabel;
	double x0;
	double y0;
	double f;
	double fx;
	double fy;
	double pixsize;
	double k1;
	double k2;
	double p1;
	double p2;
	double a;
	double b;
	DYUNITTYPE eUnitx0y0;
	DYUNITTYPE eUnitf;
	DYUNITTYPE eUnitpixsize;
	DYstuCmrInfo()
	{
		strLabel = "-99999";
		x0=y0=InvalidValue;
		f=fx=fy=InvalidValue;
		pixsize=InvalidValue;
		k1=k2=InvalidValue;
		p1=p2=InvalidValue;
		a=b=InvalidValue;
		eUnitx0y0=MM;
		eUnitf=MM;
		eUnitpixsize=MM;
	}
	bool IsValid()
	{
		if( x0==InvalidValue||y0==InvalidValue||f==InvalidValue||fx==InvalidValue||
			fy==InvalidValue||pixsize==InvalidValue||k1==InvalidValue||k2==InvalidValue||
			p1==InvalidValue||p2==InvalidValue||a==InvalidValue||b==InvalidValue) 
			return false;
		else return true;
	}
	bool IsDistortion()
	{
		if(k1==0&&k2==0&&p1==0&&p2==0&&a==0&&b==0) return false;
		return true;
	}
	bool operator<(DYstuCmrInfo& cmr)
	{
		return strLabel<cmr.strLabel;
	}
	bool operator==(DYstuCmrInfo& cmr)
	{
		return strLabel==cmr.strLabel;
	}
};
//POS信息数据结构
struct DYstuPosInfo
{
	CString strLabel;
	double Xs;
	double Ys;
	double Zs;
	double phi;
	double omg;
	double kap;
	DYCOORDINATEFORMAT eFormatCoordinate;
	DYANGLEFORMAT eFormatAnglePOK;
	DYANGLEFORMAT eFormatAngleLBH;
	double R[9];
	DYstuPosInfo()
	{
		strLabel = "-99999";
		Xs=Ys=Zs=InvalidValue;
		phi=omg=kap=InvalidValue;
		eFormatCoordinate=LBH;
		eFormatAnglePOK = DEG;
		eFormatAngleLBH = DEG;
		for(int i = 0; i<9; i++) R[i]=InvalidValue;
		
	}
	bool IsValid()
	{
		if( Xs==InvalidValue||Ys==InvalidValue||Zs==InvalidValue||
			phi==InvalidValue||omg==InvalidValue||kap==InvalidValue) 
			return false;
		else return true;
	}
	bool operator<(DYstuPosInfo& DYPos)
	{
		return FunGetFileName(strLabel,false)<FunGetFileName(DYPos.strLabel,false);
	}
	bool operator==(DYstuPosInfo& DYPos)
	{
		return strLabel==DYPos.strLabel;
	}
	bool operator!=(DYstuPosInfo& DYPos)
	{
		return strLabel!=DYPos.strLabel;
	}
	DYstuPosInfo operator - (DYstuPosInfo& pos){
		DYstuPosInfo PosDif;
		PosDif.Xs = this->Xs-pos.Xs;
		PosDif.Ys = this->Ys-pos.Ys;
		PosDif.Zs = this->Zs-pos.Zs;
		PosDif.phi = this->phi-pos.phi;
		PosDif.omg = this->omg-pos.omg;
		PosDif.kap = this->kap-pos.kap;
		return PosDif;
	};
};

// 空三点数据结构
struct DYstuAdjPtInfo
{
	double X,Y,Z;                //三维坐标
	int r, g, b;				 //颜色
	int nOverlayCount;			 //重叠数
	vector<int> vecImgIdx;       //影像索引
	vector<Point2D> vecImgPt;    //像点坐标
	void Clear()
	{
		vector<int>().swap(vecImgIdx);
		vector<Point2D>().swap(vecImgPt);
	}
};
// 影像空三点信息
struct DYstuAdjPt4Img
{
	vector<int> vecAdjPtIdx;                    //空三点索引   
	vector<int> vecImgPtIdxInAdjPt;             //该影像所有空三点中像点索引
	vector<int> vecLinkImgIdx;                  //同名影像索
	vector<vector<int>> vvAdjPtIdx4EachLinkImg; //邻接影像空三点索引
	vector<vector<int>> vvImgPtIdxInAdjPt;      //邻接像对本影像像点索引 
	vector<vector<int>> vvLinkImgPtIdxInAdjPt;  //邻接像对邻接影像像点索引
	vector<double> vecImgPtResX;                //X残差
	vector<double> vecImgPtResY;                //Y残差
	DYstuAdjPt4Img(){};
	void Clear()
	{
		vector<int>().swap(vecAdjPtIdx);
		vector<int>().swap(vecImgPtIdxInAdjPt);
		vector<int>().swap(vecLinkImgIdx);
		vector<vector<int>>().swap(vvImgPtIdxInAdjPt);
		vector<vector<int>>().swap(vvAdjPtIdx4EachLinkImg);
		vector<vector<int>>().swap(vvLinkImgPtIdxInAdjPt);
		vector<double>().swap(vecImgPtResX);
		vector<double>().swap(vecImgPtResY);
	}
};

/// 定义回调函数，用于输出信息
typedef void (* FlyQuaCallBackFun)(const char *pInfoMain, float dPercentMain);
typedef void (* FlyQuaCallBackFunBundle)(const char *pInfoMain, float dPercentMain, const char *pInfoSub, float dPercentSub);
//空三参数
struct DYUAVMatchAndAdjPara
{
	int      nThreadNum;
	bool Sift_bDo;
	bool Sift_bCheckExist;       /// 如果存在则不做！
	int  Sift_nStartLayer;       /// 设置成 1， 2， 4， 8 这四个值，不能设其他值
	int  Sift_nWindow;           /// 窗口大小，设置成 1~100 之间
	bool Sift_bCreateThumbnail;  /// 创建缩略图
	bool     Match_bDo;
	int      Match_nMatrixFile;
	bool     Match_bCheckGeometry;
	float    Match_fMaxDistance;
	bool PickPoint_bDo;
	int  PickPoint_nGridXnum;          /// 默认挑点21*21个格子，每个格子5个点
	int  PickPoint_nGridYnum;
	int  PickPoint_nNumPerGrid;        /// 如果有任何一项等于0，则不挑点，保留全部点
	bool     Freenet_bDo;
	int      Freenet_nPair1;
	int      Freenet_nPair2;
	bool     Freenet_bFixedCamera;
	bool SBA_bDo;
	bool SBA_bFixedCamera;
	int  SBA_nCameraType;
	double SBA_GPS_Precise;
	double SBA_INS_Precise;
	double SBA_Ctrl_Precise;
	bool SBA_bTrustRegion;
	bool SBA_bWithPos;
	bool SBA_bWithCtrlPts;
	int  SBA_nMaxIterNum;
	FlyQuaCallBackFun pOutputFunc;
	FlyQuaCallBackFunBundle pOutputFuncBundle;
	CString pLogFilename;
	DYUAVMatchAndAdjPara()
	{
		nThreadNum = 8;
		Sift_bDo = true;
		Sift_bCheckExist = true; /// 如果存在则不做！
		Sift_nStartLayer = 1;    /// 设置成 1， 2， 4， 8 这四个值，不能设其他值
		Sift_nWindow = 50;       /// 窗口大小，设置成 1~100 之间
		Sift_bCreateThumbnail = false;
		Match_bDo = true;
		Match_nMatrixFile = 1;
		Match_bCheckGeometry = true;
		Match_fMaxDistance = 10.0f;
		PickPoint_bDo = true;
		PickPoint_nGridXnum = 21;          /// 默认挑点21*21个格子，每个格子5个点
		PickPoint_nGridYnum = 21;
		PickPoint_nNumPerGrid = 5;        /// 如果有任何一项等于0，则不挑点，保留全部点
		Freenet_bDo = true;
		Freenet_nPair1 = 0;
		Freenet_nPair2 = 1;
		Freenet_bFixedCamera = true;
		SBA_bDo = true;
		SBA_bFixedCamera = true;
		SBA_nCameraType = 4; /// Level_3.2
		SBA_GPS_Precise = 2.0;
		SBA_INS_Precise = 0.1;
		SBA_Ctrl_Precise = 0.01;
		SBA_bTrustRegion = true;
		SBA_bWithPos = true;
		SBA_bWithCtrlPts = true;
		SBA_nMaxIterNum = 100;
		pOutputFunc = NULL;
		pOutputFuncBundle=NULL;
		pLogFilename.Empty();
	}
};


#endif