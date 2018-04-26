// 这段 MFC 示例源代码演示如何使用 MFC Microsoft Office Fluent 用户界面 
// (“Fluent UI”)。该示例仅供参考，
// 用以补充《Microsoft 基础类参考》和 
// MFC C++ 库软件随附的相关电子文档。
// 复制、使用或分发 Fluent UI 的许可条款是单独提供的。
// 若要了解有关 Fluent UI 许可计划的详细信息，请访问  
// http://msdn.microsoft.com/officeui。
//
// 版权所有(C) Microsoft Corporation
// 保留所有权利。

// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 从 Windows 头中排除极少使用的资料
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 某些 CString 构造函数将是显式的

// 关闭 MFC 对某些常见但经常可放心忽略的警告消息的隐藏
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展


#include <afxdisp.h>        // MFC 自动化类



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC 对 Internet Explorer 4 公共控件的支持
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持
#include <afxdisp.h>
#include <afxdisp.h>



//字符串标签
//#define SOFTWARE_LABEL "@2016 UAV Flight Quality Check System by SSW Group"
#define SOFTWARE_LABEL "@2016 UAV Flight Quality Check System"
//计时器标记
#define TIME_ID_SYSTIME 1
#define TIME_ID_IMGPOSMATCH 2
#define TIME_ID_IMGMATCHADJ 3
//自定义消息
#define ID_MSG_IMGPOSMATCH          (WM_USER+1000)
#define ID_MSG_UPDATE_STATUSLABEL   (WM_USER+1001)
#define ID_MSG_RESORT_STRIPS        (WM_USER+1002)
#define ID_MSG_FILL_WND_LOG         (WM_USER+1003)
#define ID_MSG_CREATE_TEXTURES      (WM_USER+1004)
#define ID_MSG_CREATE_ADJPTVBO      (WM_USER+1005)
#define ID_MSG_FILL_DATACHART       (WM_USER+1006)
#define ID_MSG_CREATE_MISSRENDER    (WM_USER+1007)
#define ID_MSG_FILL_REPRENDER       (WM_USER+1008)
#define ID_MSG_SAVE_REPORT          (WM_USER+1009)
#define ID_MSG_OPEN_REPORT          (WM_USER+1010)
#define ID_MSG_SHOW_PDF             (WM_USER+1011)
#define ID_MSG_WNDPROGRESSCTRL_POS  (WM_USER+1012)
#define ID_MSG_WNDPROGRESSCTRL_SHOW (WM_USER+1013)
#define ID_MSG_SET_IMGVALID_STATE   (WM_USER+1014)
#define ID_MSG_CREATE_SELECTRENDER  (WM_USER+1015)
#define SIZE_IMG_TEXTURE  256

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

//overlap color list
//0   250 240 230
//1   205 198 115
//2   255 255 0
//3   255 140 0
//4   50 205 50
//5   0 100 0
//11  0 0 205