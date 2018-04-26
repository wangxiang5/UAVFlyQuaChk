#ifndef _SSW_IMAGE_20160622_
#define _SSW_IMAGE_20160622_


/*************************************************************************************************/
/* Version1.0  时间：20160701															         */
/* 说明: 对JPG文件暂不支持创建和写入,仅能读取数据													 */
/* 时间20160706修改1：修复析构函数BUG																 */
/*************************************************************************************************/

/*************************************************************************************************/
/* Tiff的创建选项：																				 */
/* TFW=YES:	创建ESRI World File(.tfw)														     */
/* RPB=YES:	创建RPB文件描述RPC模型															     */
/* INTERLEAVE=[BAND,PIXEL]:	波段存储方式														     */
/* TILED=YES: 	创建文件为分块儿存储																 */
/* BLOCKXSIZE=n 设置块儿X方向的大小,默认为256														 */
/* BLOCKYSIZE=n 设置块儿Y方向的大小,默认为256														 */
/* NBITS=n 设置每个像素所占的Bit位数,可以小于8(比如以NBITS=1存储掩膜文件)							 */
/* COMPRESS=[JPEG/LZW/PACKBITS/DEFLATE/CCITTRLE//CCITTFAX3/CCITTFAX4/LZMA/NONE],文件压缩方式		 */
/* BIGTIFF=[YES/NO/IF_NEEDED/IF_SAFER] 是否创建BigTiff文件										 */
/*************************************************************************************************/

/*************************************************************************************************/
/* IMG的创建选项：																				 */
/* BLOCKSIZE=n 切片大小(32-2048), 默认为64														 */
/* USE_SPILL=YES 强制创建溢出文件,可创建大于2GB的IMG文件											 */
/* COMPRESSED=YES 创建文件为压缩格式																 */
/* AUX=YES 创建附带的AUX文件																		 */
/* NBITS=1/2/4 创建有特殊Bit数的影像																 */
/*************************************************************************************************/

#include <string>

using namespace std;
#ifdef  SSWIMAGE_EXPORTS
#define SSWIMAGE_API __declspec(dllexport)
#else
#define SSWIMAGE_API __declspec(dllimport)
#endif

#ifndef SSWIMAGE_EXPORTS
#pragma comment(lib, "sswImage.lib")
#pragma message("Automatically Linking With sswImage.dll")
#endif 

static int SSWDefaltBandMap[12] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

static int SSWDefaltOverviewList[20] = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65535, 131072, 262144, 524288, 1048576 };


class SSWIMAGE_API CSSWImage
{
public:
	enum SSWDATATYPE
	{
		SSWUnknown  = 0,
		SSWByte     = 1,
		SSWUInt16   = 2,
		SSWInt16	= 3,
		SSWUInt32   = 4,
		SSWInt32    = 5,
		SSWFloat32  = 6,
		SSWFloat64  = 7,
		SSWCInt16   = 8,
		SSWCInt32   = 9,
		SSWCFloat32 = 10,
		SSWCFloat64 = 11
	};

	enum SSWRESAMPLE
	{
		SSWNearest  = 1,
		SSWGauss    = 2,
		SSWCubic    = 3,
		SSWAverage  = 4
	};

	static int GetPixelLength(SSWDATATYPE dt);

public:

	CSSWImage();

   virtual ~CSSWImage();

public:
	/************************************************************************/
	/* 文件的创建/打开/关闭                                                    */
	/*																		*/
	/* strCreateOption可选项有:												*/			
	/************************************************************************/
	bool Create(const string& filename, int iCols, int iRows, int iBandCount, SSWDATATYPE dt, char** strCreateOption);

	bool Open(const string& filename, bool bReadOnly);

	void Close();


	/************************************************************************/
	/* 影像基本信息接口                                                       */
	/************************************************************************/

	SSWDATATYPE GetDataType();

	int GetCols();
	
	int GetRows();

	int GetBandCount();

	/************************************************************************/
	/* 获取影像属性                                                           */
	/************************************************************************/

	bool IsTiled();

	bool IsGeoImage();

	void GetTileSize(int* xblocksize, int* yblocksize);

	/************************************************************************/
	/* 地理信息相关接口                                                       */
	/* 计算公式:															    */
	/* GeoX = pGeo[0] + pGeo[1] * PixX + pGeo[2] * PixY				        */
	/* GeoY = pGeo[3] + pGeo[4] * PixX + pGeo[5] * PixY					    */
	/* 使用示例:																*/
	/* double GeoTrans[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };				*/
	/* sswimage.GetGeoTransform(GeoTrans);									*/
	/************************************************************************/

	void GetGeoTransform(double* pGeo);

	void SetGeoTransform(double* pGeo);


	/************************************************************************/
	/* 金字塔相关接口                                                         */
	/* iLevels 层级,从0开始,0代表第一级										*/
	/* pOverviewList  金字塔层级缩放比例,例如{2,4,8,16,32}                     */
	/************************************************************************/
	bool BuildOverviews(SSWRESAMPLE resampletype, int iLevels, int* pOverviewList = SSWDefaltOverviewList);

	int  GetOverviewCount();

	void GetOverviewList(int* pList, int iOverviewCount);

	int  GetOverviewCols(int iLevel);

	int  GetOverviewRows(int iLevel);

	
	/*********************************************************************************/
	/* 影像IO相关接口																	*/
	/* iStartCol, iStartRow, iCols, iRows 起始列,起始行,列数,行数						    */
	/* pData 数据内存指针, iBand 读取的波段数, pBandMap 读取的波段列表					    */
	/* iPixelSpace  每像素在单波段所占空间	0代表当前SSWDATATYPE所占字节数				*/
	/* iLineSpace   每行像素所占的空间	    0代表iCols * iPixelSpace个像素所占字节		*/
	/* iBandSpace   每波段所占的空间		    0代表iRows * iCols * iPixelSpace像素所占字节数*/
	/*********************************************************************************/

	bool ReadImage(int iStartCol, int iStartRow, int iCols, int iRows, void* pData, int iBands, int* pBandMap = SSWDefaltBandMap,  int iPixelSpace = 0, int iLineSpace = 0, int iBandSpace = 0);

	bool WriteImage(int iStartCol, int iStartRow, int iCols, int iRows, void* pData, int iBands, int* pBandMap = SSWDefaltBandMap, int iPixelSpace = 0, int iLineSpace = 0, int iBandSpace = 0);

	bool ReadPyramidImage(int iLevel, int iStartCol, int iStartRow, int iCols, int iRows, void* pData, int iBands, int* pBandMap = SSWDefaltBandMap, int iPixelSpace = 0, int iLineSpace = 0, int iBandSpace = 0);

	bool WritePyramidImage(int iLevel, int iStartCol, int iStartRow, int iCols, int iRows, void* pData, int iBands, int* pBandMap = SSWDefaltBandMap, int iPixelSpace = 0, int iLineSpace = 0, int iBandSpace = 0);

	void FlushCache();


	/************************************************************************/
	/* 函数说明对影像进行切片                                                 */
	/************************************************************************/
	static bool TileImage(const string& srcfilename, const string& dstfilname);

	/************************************************************************/
	/* 采用LZW方式对影像进行压缩(仅限于tif影像)                               */
	/************************************************************************/
	static bool LZWCompress(const string& srcfilename, const string& dstfilename);

private:
	void*  m_pDataSet;
	int	   m_OverviewList[32];
};



#endif


