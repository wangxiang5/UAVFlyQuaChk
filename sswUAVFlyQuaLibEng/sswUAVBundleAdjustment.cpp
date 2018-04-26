#include "StdAfx.h"
#include "sswUAVBundleAdjustment.h"
#include "ZGSiftEx.h"
#include "ZgSfmEx.h"
#include <io.h>
#include <omp.h>

CMemTxtFile::CMemTxtFile()
{

}
CMemTxtFile::~CMemTxtFile()
{
	CloseFile();
}
bool CMemTxtFile::OpenFile(CString strFilePath)
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_hFileMap = NULL;
	m_lpbFile = NULL;
	m_dwFileSize=0;
	if(!PathFileExists(strFilePath)) return false;
	m_hFile=CreateFile(strFilePath, //路径
		GENERIC_READ,//读
		FILE_SHARE_READ, //共享只读
		NULL,//安全特性
		OPEN_EXISTING,//文件必须存在
		FILE_FLAG_SEQUENTIAL_SCAN,//针对连续访问对文件缓冲进行优化
		NULL);  //如果不为0，则指定一个文件句柄
	if(INVALID_HANDLE_VALUE==m_hFile) return false;
	m_dwFileSize = ::GetFileSize(m_hFile,NULL);
	m_hFileMap=CreateFileMapping(m_hFile,
		NULL,//安全对象
		PAGE_READONLY,//只读方式打开映射
		0,
		0,//用磁盘文件实际长度
		NULL);//指定文件映射对象的名字
	if(NULL==m_hFileMap) return false;
	m_lpbFile=(char*)MapViewOfFile(
		m_hFileMap,
		FILE_MAP_READ,
		0,
		0,//映射整个文件映射对象
		0);//0表示允许Windows寻找地址
	if(NULL==m_lpbFile) return false;
	return true;
}
void CMemTxtFile::CloseFile()
{
	if(m_lpbFile!=NULL) UnmapViewOfFile(m_lpbFile);
	if(m_hFileMap!=NULL) CloseHandle(m_hFileMap);
	if(m_hFile!=INVALID_HANDLE_VALUE) CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
	m_hFileMap = NULL;
	m_lpbFile = NULL;
	m_dwFileSize=0;
}




CsswBundleAdjustment::CsswBundleAdjustment()
{
	ClearData();
}
CsswBundleAdjustment::~CsswBundleAdjustment()
{
	ClearData();
}

void CsswBundleAdjustment::ClearData()
{
	m_bDataLoad=false;
//	m_bDataPrePro=false;
	m_bIsEnglishVersion = false;
// 	vector<string>().swap(m_vecImgPath);
// 	vector<SSWstuPosInfo>().swap( m_vecPosInfo);
// 	vector<int>().swap( m_vecImgIdx2PosIdx);
// 	vector<int>().swap( m_vecMatchMatrix);
	m_strMatchAndAdjFolder.Empty();
	m_strImgListFile.Empty();
	m_strPosListFile.Empty();
	m_strMatchMatrixFile.Empty();

//	m_nImgPosed = 0;
}
void CsswBundleAdjustment::UpdateParamDo( sswUAVMatchAndAdjPara &param )
{
	CString path = m_strMatchAndAdjFolder;
	CString imgPath = m_strImgFolder;
	CString sbaPath = path + "BundleAdjustment\\";
	CString imgListFilename = m_strImgListFile;
	CString keyPath = path + "Keypoints\\";
	CString thumbnailPath = path + "Thumbnail\\";
	CString matchMatrixName = m_strMatchMatrixFile;
	CString matchResult = keyPath + "MatchResult.txt";

	if( PathFileExists(matchResult))
	{
		param.Sift_bDo  = false;
		param.Match_bDo = false;
	}
	else
	{
		param.Sift_bDo  = true;
		param.Match_bDo = true;
	}

	CString match_ZGout = sbaPath + "Freenet_Match.zgout";

	if( PathFileExists( match_ZGout))
	{
		param.PickPoint_bDo  = false;
	}
	else
	{
		param.PickPoint_bDo  = true;
	}

	CString Freenet_Result_init  = sbaPath + "Freenet_Init.zgout";
	CString Freenet_Result       = sbaPath + "Freenet.zgout";
	CString Freenet_Result_Final = sbaPath + "Freenet_SBA.zgout";
	CString Freenet_Result_Old   = sbaPath + "Freenet_Old.zgout";

	if( PathFileExists( Freenet_Result))
	{
		param.Freenet_bDo  = false;
	}
	else
	{
		param.Freenet_bDo  = true;
	}

	CString GPS_init = m_strPosListFile;
	if( PathFileExists( GPS_init))
	{
		param.SBA_bWithPos  = true;
	}
	else
	{
		param.SBA_bWithPos  = false;
	}
	CString Control_Point = sbaPath + "CtrlPts.txt";
	if( PathFileExists( Control_Point))
	{
		param.SBA_bWithCtrlPts  = true;
	}
	else
	{
		param.SBA_bWithCtrlPts  = false;
	}
	CString SBA_Report = sbaPath + "SBA_Report.txt";

}
bool CsswBundleAdjustment::OneKey( sswUAVMatchAndAdjPara& param)
{
// 	if( param.pOutputFunc )
// 	{
// 		param.pOutputFunc("匹配平差处理开始...", 0, NULL, 0,true);
// 	}
	clock_t t1, t2; double dt; CString strInfo;
	CString path = m_strMatchAndAdjFolder;
	CString imgPath = m_strImgFolder+"\\";	
	CString sbaPath = path + "\\BundleAdjustment\\"; CreateDirectory(sbaPath,NULL);
	CString imgListFilename = m_strImgListFile;
	CString keyPath = path + "\\Keypoints\\";CreateDirectory(keyPath,NULL);
	CString logPath = sbaPath+"SmartAT_Log.txt";
	FILE *fp_out = fopen(logPath,"w");
	fclose(fp_out); fp_out = NULL;
	param.pLogFilename = logPath;
	

	if( param.Sift_bDo )
	{
		t1 = clock();
		if( param.pOutputFunc )
		{
			param.pOutputFunc("Detecting feature points, please wait...", -1, NULL, 0,true);
		}
		if( SiftFeatureDetector_ImageList(
			m_bIsEnglishVersion,
			(LPSTR)(LPCSTR)(imgListFilename),			/// 影像列表
			(LPSTR)(LPCSTR)(imgPath),					/// 影像的文件夹
			(LPSTR)(LPCSTR)(keyPath),					/// 特征点的存放文件夹
			NULL,									/// 缩略图保存路径
			param.nThreadNum,						/// 线程数，默认是8个线程
			param.Sift_nStartLayer,                 /// 起始层数，1代表原图，2代表缩小2x2倍，4代表缩小4x4倍
			param.Sift_nWindow,						/// NMS局部非最大抑制，用于限制特征点的数量
			param.Sift_bCheckExist,		/// 如果key文件存在，就不做特征提取
			param.pOutputFuncBundle,			/// Callback
		(LPSTR)(LPCSTR)(param.pLogFilename)		/// Log File
			) != 0)
		{
			if( param.pOutputFunc )
			{
				if( m_bIsEnglishVersion )
				{
					param.pOutputFunc("Failed to detect features, please check!", 100, NULL, 0,true);
				}
				else
				{
					param.pOutputFunc("Feature detecting error!", -1, NULL, 0,true);
				}
			}
			else
			{
				if( m_bIsEnglishVersion )
				{
					FunOutPutLogInfo("Failed to detect features, please check!");
				}
				else
				{
					FunOutPutLogInfo("Feature detecting error!");
				}
			}
			return false;
		}
		t2 = clock();
		dt = (double)(t2-t1)/CLOCKS_PER_SEC;
		strInfo.Format("Time %.3lfs",dt);
		if (param.pOutputFunc)
			param.pOutputFunc("Detecting finished: "+strInfo,-1,NULL,0,true);
	}

	//AfxMessageBox("特征提取完成！");


	CString matchMatrixName = m_strMatchMatrixFile;
	CString matchResult = keyPath + "MatchResult.txt";
	CString match_ZGout = sbaPath + "Freenet_Match.zgout";

	if( param.Match_bDo )
	{	
			t1 = clock();
		if( param.pOutputFunc )
		{
			param.pOutputFunc("Point matching, please wait...", -1, NULL, 0,true);
		}
		if(SiftMatch_ImageList(
			m_bIsEnglishVersion,
			(LPSTR)(LPCSTR)(imgListFilename),         /// 影像列表
			(LPSTR)(LPCSTR)(keyPath),					/// 特征点的存放文件夹
			(LPSTR)(LPCSTR)(matchMatrixName),         /// 匹配矩阵，txt文件，记录了N*N个1或0, 1代表匹配
			(LPSTR)(LPCSTR)(matchResult),				/// 匹配的结果文件
			param.nThreadNum,						/// 线程数
			param.Match_bCheckGeometry,				/// 几何检查，通常是 F 矩阵
			param.Match_fMaxDistance,				/// 容忍的 F 矩阵距离阈值，默认是10个像素，如果没有纠正畸变，则要设置大一点。
			param.pOutputFuncBundle,				/// Callback
			(LPSTR)(LPCSTR)(param.pLogFilename)				/// Log File
			) != 0)
		{
			if( param.pOutputFunc )
			{
				if( m_bIsEnglishVersion )
				{
					param.pOutputFunc("Failed to match, please check!", 100, NULL, 0,true);
				}
				else
				{
					param.pOutputFunc("Matching error!", -1, NULL, 0,true);
				}
			}
			else
			{
				if( m_bIsEnglishVersion )
				{
					FunOutPutLogInfo("Failed to match, please check!");
				}
				else
				{
					FunOutPutLogInfo("Matching error!");
				}
			}
			return false;
		}
		t2 = clock();
		dt = (double)(t2-t1)/CLOCKS_PER_SEC;
		strInfo.Format("Time %.3lfs",dt);
		if (param.pOutputFunc)
			param.pOutputFunc("Matching finished: "+strInfo,-1,NULL,0,true);

		int M = 21, N = 21, K = 0;
		if( param.PickPoint_bDo )
		{
			M = param.PickPoint_nGridXnum;
			N = param.PickPoint_nGridYnum;
			K = param.PickPoint_nNumPerGrid;
		}
		t1 = clock();
		if( param.pOutputFunc )
		{
			param.pOutputFunc("Match checking, please wait...", -1, NULL, 0,true);
		}

		if(MatchResult_Choose_To_ZGOUT(
			m_bIsEnglishVersion,
			(LPSTR)(LPCSTR)(imgListFilename),           /// 影像列表
			(LPSTR)(LPCSTR)(keyPath),            /// 特征点的存放文件夹
			(LPSTR)(LPCSTR)(matchResult),        /// 匹配结果文件
			(LPSTR)(LPCSTR)(match_ZGout),        /// 匹配串点文件
			M,        /// 默认挑点21*21个格子，每个格子5个点
			N,
			K         /// 如果有任何一项等于0，则不挑点，保留全部点
			) != 0)
		{
			if( param.pOutputFunc )
			{
				if( m_bIsEnglishVersion )
				{
					param.pOutputFunc("Failed to pick points, please check!", 100, NULL, 0,true);
				}
				else
				{
					param.pOutputFunc("Checking error!", -1, NULL, 0,true);
				}
			}
			else
			{
				if( m_bIsEnglishVersion )
				{
					FunOutPutLogInfo("Failed to pick points, please check!");
				}
				else
				{
					FunOutPutLogInfo("Checking error!");
				}
			}
			return false;
		}
		t2 = clock();
		dt = (double)(t2-t1)/CLOCKS_PER_SEC;
		strInfo.Format("Time %.3lfs",dt);
		if (param.pOutputFunc)
			param.pOutputFunc("Checking finished: "+strInfo,-1,NULL,0,true);
	}
	else if(param.PickPoint_bDo)  //不进行匹配时的串点
	{
		t1 = clock();
		if( param.pOutputFunc )
		{
			param.pOutputFunc("Match checking, please wait...", -1, NULL, 0,true);
		}
		int M = 21, N = 21, K = 0;
		if( param.PickPoint_bDo )
		{
			M = param.PickPoint_nGridXnum;
			N = param.PickPoint_nGridYnum;
			K = param.PickPoint_nNumPerGrid;
		}

		if( param.pOutputFunc )
		{
			if( m_bIsEnglishVersion )
			{
				param.pOutputFunc("Picking points, please wait!", -1, NULL, 0,true);
			}
			else
			{
				param.pOutputFunc("Match checking, please wait...", -1, NULL, 0,true);
			}
		}

		if(MatchResult_Choose_To_ZGOUT(
			m_bIsEnglishVersion,
			(LPSTR)(LPCSTR)(imgListFilename),           /// 影像列表
			(LPSTR)(LPCSTR)(keyPath),            /// 特征点的存放文件夹
			(LPSTR)(LPCSTR)(matchResult),        /// 匹配结果文件
			(LPSTR)(LPCSTR)(match_ZGout),        /// 匹配串点文件
			M,        /// 默认挑点21*21个格子，每个格子5个点
			N,
			K         /// 如果有任何一项等于0，则不挑点，保留全部点
			) != 0)
		{
			if( param.pOutputFunc )
			{
				if( m_bIsEnglishVersion )
				{
					param.pOutputFunc("Failed to pick points, please check!", -1, NULL, 0,true);
				}
				else
				{
					param.pOutputFunc("Checking error!", -1, NULL, 0,true);
				}
			}
			else
			{
				if( m_bIsEnglishVersion )
				{
					FunOutPutLogInfo("Failed to pick points, please check!");
				}
				else
				{
					FunOutPutLogInfo("Checking error!");
				}
			}
			return false;
		}
		t2 = clock();
		dt = (double)(t2-t1)/CLOCKS_PER_SEC;
		strInfo.Format("Time %.3lfs",dt);
		if (param.pOutputFunc)
			param.pOutputFunc("Checking finished: "+strInfo,-1,NULL,0,true);
	}
	CString Freenet_Result_init  = sbaPath + "Freenet_Init.zgout";
	CString Freenet_Result       = sbaPath + "Freenet.zgout";
	CString Freenet_Result_Final = sbaPath + "Freenet_SBA.zgout";
	CString Freenet_Result_Old   = sbaPath + "Freenet_Old.zgout";
	CString GPS_init = m_strPosListFile;
	if( !param.SBA_bWithPos )
	{
		GPS_init = "";
	}
	CString Control_Point = sbaPath + "CtrlPts.txt";
	if( !param.SBA_bWithCtrlPts )
	{
		Control_Point = "";
	}
	CString SBA_Report = sbaPath + "SBA_Report.txt";
	if( param.Freenet_bDo )
	{
		
		t1 = clock();
		if( param.pOutputFunc )
		{
			param.pOutputFunc("Starting freenet adjustment, please wait...", -1, NULL, 0,true);
		}
		if( Sfm_ImageList(
			m_bIsEnglishVersion,
			(LPSTR)(LPCSTR)(imgListFilename),		/// *\imageList.txt
			(LPSTR)(LPCSTR)(match_ZGout),			/// *\Freenet_Match.zgout，匹配串点文件，可以是自动的，也可以是手动追加的
			(LPSTR)(LPCSTR)(Freenet_Result_init),      /// BundleAdjustment\*; *\Freenet.zgout
			(LPSTR)(LPCSTR)(GPS_init),		/// GPS曝光点的文件，用于辅助平差
			param.nThreadNum,				/// 线程数
			param.Freenet_nPair1,			/// 初始像对设为-1，意味着自动选择初始像对
			param.Freenet_nPair2,
			param.Freenet_bFixedCamera,		/// 固定相机内参数
			param.pOutputFuncBundle,				/// 回调函数，用于输出信息
			(LPSTR)(LPCSTR)(Freenet_Result_Old),  /// *\*.zgout文件，旧的结果，如果存在旧的结果，则读入
			(LPSTR)(LPCSTR)(param.pLogFilename)				/// Log File
			) != 0 )
		{
			if( param.pOutputFunc )
			{
				if( m_bIsEnglishVersion )
				{
					param.pOutputFunc("Failed to build freenet, please check!", -1, NULL, 0,true);
				}
				else
				{
					param.pOutputFunc("Freenet build error!", -1, NULL, 0,true);
				}
			}
			else
			{
				if( m_bIsEnglishVersion )
				{
					FunOutPutLogInfo("Failed to build freenet, please check!");
				}
				else
				{
					FunOutPutLogInfo("Freenet build error!");
				}
			}
			return false;
		}

		if( CSba_zgout(
			m_bIsEnglishVersion,
			(LPSTR)(LPCSTR)imgListFilename,   /// *\imageList.txt
			(LPSTR)(LPCSTR)Freenet_Result_init,    /// *\Freenet.zgout
			(LPSTR)(LPCSTR)Freenet_Result,   /// *\Freenet_SBA.zgout
			NULL,  /// *\ControlPoints
			NULL,  /// *\POS, GPS曝光点的文件，用于辅助平差
			param.nThreadNum,         /// 线程数
			param.Freenet_bFixedCamera,  /// 固定相机内参数
			Cam_Level3_1,
			param.SBA_GPS_Precise,
			param.SBA_INS_Precise,
			param.SBA_Ctrl_Precise,
			true,
			100,
			param.pOutputFuncBundle,       /// 回调函数，用于输出信息
			(LPSTR)(LPCSTR)param.pLogFilename,         /// Log File
			3.0
			) != 0)
		{
			if( param.pOutputFunc )
			{
				if( m_bIsEnglishVersion )
				{
					param.pOutputFunc("Failed to bundle adjustment freenet, please check!", 100, NULL, 0,true);
				}
				else
				{
					param.pOutputFunc("Freenet Bundle adjustment error!", -1, NULL, 0,true);
				}
			}
			else
			{
				if( m_bIsEnglishVersion )
				{
					FunOutPutLogInfo("Failed to bundle adjustment freenet, please check!");
				}
				else
				{
					FunOutPutLogInfo("Freenet bundle adjustment error!");
				}
			}
			return false;
		}
		t2 = clock();
		dt = (double)(t2-t1)/CLOCKS_PER_SEC;
		strInfo.Format("Time %.3lfs",dt);
		if (param.pOutputFunc)
			param.pOutputFunc("Freenet adjustment finished: "+strInfo,-1,NULL,0,true);
	}

	if( param.SBA_bDo )	
	{
		t1 = clock();
		if( param.pOutputFunc )
		{
			param.pOutputFunc("Starting adjustment with POS, please wait...", -1, NULL, 0,true);
		}
		if( CSba_zgout(
			m_bIsEnglishVersion,
			(LPSTR)(LPCSTR)imgListFilename,   /// *\imageList.txt
			(LPSTR)(LPCSTR)Freenet_Result,    /// *\Freenet.zgout
			(LPSTR)(LPCSTR)Freenet_Result_Final,   /// *\Freenet_SBA.zgout
			(LPSTR)(LPCSTR)Control_Point,  /// *\ControlPoints
			(LPSTR)(LPCSTR)GPS_init,  /// *\POS, GPS曝光点的文件，用于辅助平差
			param.nThreadNum,         /// 线程数
			param.SBA_bFixedCamera,  /// 固定相机内参数
			param.SBA_nCameraType,
			param.SBA_GPS_Precise,   /// GPS的先验精度
			param.SBA_INS_Precise,
			param.SBA_Ctrl_Precise,
			param.SBA_bTrustRegion,
			param.SBA_nMaxIterNum,
			param.pOutputFuncBundle,       /// 回调函数，用于输出信息
		(LPSTR)(LPCSTR)param.pLogFilename,         /// Log File
			3.0
			) != 0)
		{
			if( param.pOutputFunc )
			{
				if( m_bIsEnglishVersion )
				{
					param.pOutputFunc("Failed to bundle adjustment, please check!", 100, NULL, 0,true);
				}
				else
				{
					param.pOutputFunc("Bundle adjustment(With POS) error!", -1, NULL, 0,true);
				}
			}
			else
			{
				if( m_bIsEnglishVersion )
				{
					FunOutPutLogInfo("Failed to bundle adjustment, please check!");
				}
				else
				{
					FunOutPutLogInfo("Bundle adjustment(With POS) error!");
				}
			}
			return false;
		}
		t2 = clock();
		dt = (double)(t2-t1)/CLOCKS_PER_SEC;
		strInfo.Format("Time %.3lfs",dt);
		if (param.pOutputFunc)
			param.pOutputFunc("Adjustment finshed: "+strInfo,-1,NULL,0,true);
	
		if( param.pOutputFunc )
		{
			if( m_bIsEnglishVersion )
			{
				param.pOutputFunc("Writing precise report !", -1, NULL, 0,true);
			}
			else
			{
				param.pOutputFunc("Writing precise report!", -1, NULL, 0,true);
			}
		}
		Output_ControlPoint_Report(
			m_bIsEnglishVersion,
			(LPSTR)(LPCSTR)(imgListFilename),   /// *\imageList.txt
			(LPSTR)(LPCSTR)(Freenet_Result_Final),   /// *\Freenet_SBA.zgout
			(LPSTR)(LPCSTR)(Control_Point),  /// *\ControlPoints
			(LPSTR)(LPCSTR)(SBA_Report),        /// *\Report.txt
			param.SBA_nCameraType
			);

		if( param.pOutputFunc )
		{
			if( m_bIsEnglishVersion )
			{
				param.pOutputFunc("Writing results, please wait !", -1, NULL, 0,true);
			}
			else
			{
				param.pOutputFunc("Writing results, please wait!", -1, NULL, 0,true);
			}

		}
	}

	/// 更新状态
	UpdateParamDo( param );

	if( param.pOutputFunc )
	{
		if(param.Freenet_bDo) param.pOutputFunc("====  Matching and adjustment are finished  ====", -1, NULL, 0,false);
		else if(param.Match_bDo)param.pOutputFunc("====  Matching is finished  ====", -1, NULL, 0,false);
	}
	return true;
}
bool CsswBundleAdjustment::LoadData(CString strImgFolder, CString strImgListPath, CString strPosListPath, CString strMatchMatrixPathPath)
{
	if(!PathFileExists(strImgFolder)) 
	{
		FunOutPutLogInfo("Image folder error, adjustment failed!");
		m_bDataLoad= false;
		return false;
	}
	if(!PathFileExists(strImgListPath)) 
	{
		FunOutPutLogInfo("Image list file path error, adjustment failed!");
		m_bDataLoad= false;
		return false;
	}
	if(!PathFileExists(strPosListPath))
	{
		FunOutPutLogInfo("POS list file path error, adjustment failed!");
		m_bDataLoad=false;
		return false;
	}
	if(!PathFileExists(strMatchMatrixPathPath))
	{
		FunOutPutLogInfo("Matching list file path error, adjustment failed!");
		m_bDataLoad=false;
		return false;
	}
	m_strImgFolder = strImgFolder;
	m_strImgListFile = strImgListPath;
	m_strPosListFile = strPosListPath;
	m_strMatchMatrixFile=strMatchMatrixPathPath;
	m_bDataLoad = true;
	return true;
}
bool CsswBundleAdjustment::RunOneKeyMatchAndAdj(CString strMatchAndAdjFolder, sswUAVMatchAndAdjPara& para, bool bRunAdj /* = true */)
{
	m_strMatchAndAdjFolder=strMatchAndAdjFolder;
	if(!m_bDataLoad)
	{
		if(para.pOutputFunc) para.pOutputFunc("Data loading error, matching and adjustment failed!",-1,NULL,0,true);
		else FunOutPutLogInfo("Data loading error, matching and adjustment failed!");
		return false;
	}
	if(!OneKey(para)) return false ;
	return true;
}