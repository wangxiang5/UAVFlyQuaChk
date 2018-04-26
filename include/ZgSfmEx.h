#ifndef  ZG_SFM_EX_H_19274
#define  ZG_SFM_EX_H_19274

#  ifdef ZG_BUILD_DLL
#      define ZG_EXPORT  __declspec(dllexport)
#  else
#      define ZG_EXPORT  __declspec(dllimport)
#endif

/// 这个类没有任何定义，只是为了在VS2010的Class View中可以显示 ZGSiftEx类
/// 方便快速找到这个文件
class ZgSfmEx
{
public:
    ZgSfmEx(void);
    ~ZgSfmEx(void);
};

/// 定义回调函数，用于输出信息
typedef void (CALLBACK* ZG_CallBack)(const char *pInfoMain, float nPercentMain, const char *pInfoSub, float nPercentSub);

/// 下面是一个示例，注释掉了，因为头文件中最好不要有函数定义。
//void CALLBACK SfmCallBack_example(const char *pInfoMain, float nPercentMain, const char *pInfoSub, float nPercentSub)
//{
//    // ZhongGuan Note : you can put information into your CListBox or CProgressBar.
//    // We promise that nPercentMain and nPercentSub belong in [0.00 ~ 100.00].
//    if (pInfoMain)
//    {
//        printf("主进度：%.2f%%  %s\n",
//            nPercentMain, pInfoMain);
//    }
//    if (pInfoSub)
//    {
//        printf("子进度：%.2f%%  %s\n",
//            nPercentSub, pInfoSub);
//    }
//    // 注意：不要在这里添加任何只能在界面线程中调用的函数，如Invalidate(FALSE)！
//    // 如果需要刷新界面，请使用::PostMessage函数发送消息！
//}
#ifndef CAM_LEVEL_DEFINE_ZG
#define CAM_LEVEL_DEFINE_ZG
#define Cam_Level0     0
#define Cam_Level1     1
#define Cam_Level2     2
#define Cam_Level3_1   3
#define Cam_Level3_2   4
#define Cam_Level4     5
#define Cam_Level5_1   6
#define Cam_Level5_2   7
#define Cam_Level6     8

#define CamParamNum_Level0     1
#define CamParamNum_Level1     3
#define CamParamNum_Level2     5
#define CamParamNum_Level3_1   6
#define CamParamNum_Level3_2   7
#define CamParamNum_Level4     8
#define CamParamNum_Level5_1   9
#define CamParamNum_Level5_2   10
#define CamParamNum_Level6     11
#endif
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/// 返回0意味着成功，否则失败！
ZG_EXPORT int Sfm_ImageList(
    bool  bEnglish,
    char  *strImageListFilename,   /// *\imageList.txt
    char  *strMatchZgoutFilename,  /// *\Freenet_Match.zgout，匹配串点文件，可以是自动的，也可以是手动追加的
    char  *strResultFilename,      /// BundleAdjustment\*; *\Freenet.zgout
    char  *strGPSFilename = NULL,  /// GPS曝光点的文件，用于辅助平差
    int    nThreadsNum = 8,        /// 线程数
    int    nInitPair1 = -1,        /// 初始像对设为-1，意味着自动选择初始像对
    int    nInitPair2 = -1,
    bool  bFixedCameraInnerPara = true,  /// 固定相机内参数
    ZG_CallBack outputInfo = NULL,       /// 回调函数，用于输出信息
    char  *strOldResultFilename = NULL,  /// *\*.zgout文件，旧的结果，如果存在旧的结果，则读入
    char * strLogFilename = NULL         /// Log File
    );

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//  Return 0 if succeed, and return other value if failed.
ZG_EXPORT int CSba_zgout(
    bool  bEnglish,
    char  *strImageListFilename,   /// *\imageList.txt
    char  *strSBA_InitFilename,    /// *\Freenet.zgout
    char  *strSBA_FinalFilename,   /// *\Freenet_SBA.zgout
    char  *strCtrlPointsFilename,  /// *\ControlPoints
    char  *strGPSFilename = NULL,  /// *\POS, GPS曝光点的文件，用于辅助平差
    int   nThreadsNum = 8,         /// 线程数
    bool  bFixedCameraInnerPara = true,  /// 固定相机内参数
    int   nCameraInnerParaLevel = Cam_Level5_2,
    double dGPS_Precise = 1.0,       /// GPS的先验精度，单位是米
    double dINS_Precise = 0.1,       /// INS的先验精度，单位是度
    double dControl_Precise = 0.01,  /// 控制点的先验精度，单位是米
    bool  bTrustRegion = true,
    int   nMaxIterNum  = 100,
    ZG_CallBack outputInfo = NULL,       /// 回调函数，用于输出信息
    char * strLogFilename = NULL,         /// Log 
    double IgnoreGPS_LargeThan_ThisRatio = 3.0  /// 忽略GPS中大于这个比例的
    );
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
ZG_EXPORT void  Output_ControlPoint_Report(
    bool  bEnglish,
    char  *strImageListFilename,   /// *\imageList.txt
    char  *strSBA_FinalFilename,   /// *\Freenet_SBA.zgout
    char  *strCtrlPointsFilename,  /// *\ControlPoints
    char  *strOutputReport,        /// *\Report.txt
    int   nCameraInnerParaLevel = Cam_Level5_2
    );



#endif
