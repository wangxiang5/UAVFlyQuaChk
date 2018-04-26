#ifndef ZG_SIFT_EX_H_68013
#define ZG_SIFT_EX_H_68013

#  ifdef ZG_BUILD_DLL
#      define ZG_EXPORT  __declspec(dllexport)
#  else
#      define ZG_EXPORT  __declspec(dllimport)
#endif

/// 这个类没有任何定义，只是为了在VS2010的Class View中可以显示 ZGSiftEx类
/// 方便快速找到这个文件
class ZG_EXPORT ZGSiftEx
{
public:
    ZGSiftEx(void);
    ~ZGSiftEx(void);
};

/// 定义回调函数，用于输出信息
typedef void (CALLBACK* ZG_CallBack)(const char *pInfoMain, float nPercentMain, const char *pInfoSub, float nPercentSub);

/// 下面是一个示例，注释掉了，因为头文件中最好不要有函数定义。
//void CALLBACK SiftCallBack_example(const char *pInfoMain, float nPercentMain, const char *pInfoSub, float nPercentSub)
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

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// “特征提取”是自动空三的重要步骤！
#define IMAGE_LAYER_ORIGIN  0x1   /// 例如 5616*3744  -> 最高精度，最慢，做4D产品推荐！
                                  /// 缩小之后分别变成:
#define IMAGE_LAYER_S_2x2   0x2   ///     2808*1872  -> 精度中等，速度中等，各项指标均衡！
#define IMAGE_LAYER_S_4x4   0x4   ///     1404*936   -> 最高速度，精度低，快拼推荐！
#define IMAGE_LAYER_S_8x8   0x8   ///      702*468   -> 图片太小了，强烈不推荐！！！

/// 下面这个函数的作用是单幅影像的特征提取
/// 如果成功则返回0， 否则失败。
ZG_EXPORT int SiftFeatureDetector_SingleImage(
    bool  bEnglish,
    char  *strImageFilename,               /// 影像名
    char  *strKeypointPath,                /// 特征点的存放文件夹
    int   nThreadsNum = 8,                 /// 线程数，默认是8个线程
    int   nStartLayer = 1,                 /// 起始层数，1代表原图，2代表缩小2x2倍，4代表缩小4x4倍
    int   nNMS_Window = 50                 /// NMS局部非最大抑制，用于限制特征点的数量
    );

/// 下面这个函数的作用是多幅影像的特征提取，使用了线程优化，比上面的速度更快。
/// 如果成功则返回0， 否则失败。
ZG_EXPORT int SiftFeatureDetector_ImageList(
    bool  bEnglish,
    char  *strImageListFilename,           /// 影像列表
    char  *strImagePath,                   /// 影像的文件夹
    char  *strKeypointPath,                /// 特征点的存放文件夹
    char  *strThumbnailPath = NULL,        /// 缩略图保存路径
    int   nThreadsNum = 8,                 /// 线程数，默认是8个线程
    int   nStartLayer = 1,                 /// 起始层数，1代表原图，2代表缩小2x2倍，4代表缩小4x4倍
    int   nNMS_Window = 50,                /// NMS局部非最大抑制，用于限制特征点的数量
    bool  bDoNotDetectIfKeyExist = false,  /// 如果key文件存在，就不做特征提取
    ZG_CallBack outputInfo = NULL,         /// Callback
    char * strLogFilename = NULL           /// Log File
    );
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// “空三匹配串点”是自动空三的重要步骤！

/// 下面这个函数的作用是“空三匹配”
/// 如果成功则返回0， 否则失败。
ZG_EXPORT int SiftMatch_ImageList(
    bool  bEnglish,
    char  *strImageListFilename,           /// 影像列表
    char  *strKeypointPath,                /// 特征点的存放文件夹
    char  *strMatchMatrixFilename,         /// 匹配矩阵，txt文件，记录了N*N个1或0, 1代表匹配
    char  *strMatchResultFilename,         /// 匹配的结果文件
    int   nThreadsNum = 8,                 /// 线程数
    bool  bDoGeometryCheck = true,         /// 几何检查，通常是 F 矩阵
    float fTolerateDistance = 10.0f,       /// 容忍的 F 矩阵距离阈值，默认是10个像素，如果没有纠正畸变，则要设置大一点。
    ZG_CallBack outputInfo = NULL,         /// Callback
    char * strLogFilename = NULL           /// Log File
    );

/// 下面这个函数的作用是“串点”
/// 如果成功则返回0， 否则失败。
ZG_EXPORT int MatchResult_Choose_To_ZGOUT(
    bool  bEnglish,
    char  *strImageListFilename,           /// 影像列表
    char  *strKeypointPath,                /// 特征点的存放文件夹
    char  *strMatchResultFilename,         /// 匹配的结果文件
    char  *strZGOUT_MatchFilename,         /// ZG_OUT 格式的匹配结果文件
    int  nPickPointGridXnum = 21,          /// 默认挑点21*21个格子，每个格子5个点
    int  nPickPointGridYnum = 21,
    int  nPickPointNumPerGrid = 5         /// 如果有任何一项等于0，则不挑点，保留全部点
    );
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


#endif