
// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件

#pragma once

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









#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif
#include <afxsock.h>

#include "ftlFake.h"
#include "ftlThread.h"
#include "ftlThreadPool.h"

using namespace FTL;

#include <list>
#include <string>
#include <set>
#include <map>
#include <vector>
using namespace std;
#include <time.h>
#include "public.h"


inline LPTSTR GetErrorMessage(DWORD error, LPTSTR lpMessage)
{
	switch (error)
	{
	case SOCKETINIT_ERROR:
		lstrcpy(lpMessage, _T("初始化套接字错误！"));
		break;
	case SEND_ERROR:
		lstrcpy(lpMessage, _T("发送错误！"));
		break;
	case RECEIVE_ERROR:
		lstrcpy(lpMessage, _T("接收错误！"));
		break;
	case LOGIN_ERROR:
		lstrcpy(lpMessage, _T("登录失败！"));
		break;
	case NO_INTERNET:
		lstrcpy(lpMessage, _T("没有连接网络！"));
		break;
	case LOGIN_ERROR_LOCK:
		lstrcpy(lpMessage, _T("登陆失败-邮箱已经锁定！"));
		break;
	case RETURN_FAIL:
		lstrcpy(lpMessage, _T("返回错误！"));
		break;
	case HOST_UNREACHABLE:
		lstrcpy(lpMessage, _T("目标主机不可达！"));
		break;
	case GETADDRINFO_ERROR:
		lstrcpy(lpMessage, _T("获取地址信息失败！"));
		break;
	default:
		break;
	}
	return lpMessage;
}

inline void GetErrorMessage(int error, char* pMessage,int nLen=MAX_PATH)
{
	switch (error)
	{
	case SOCKETINIT_ERROR:
		sprintf_s(pMessage, nLen, "初始化套接字错误！");
		break;
	case SEND_ERROR:
		sprintf_s(pMessage, nLen, "发送错误！");
		break;
	case RECEIVE_ERROR:
		sprintf_s(pMessage, nLen, "接收错误！");
		break;
	case LOGIN_ERROR:
		sprintf_s(pMessage, nLen, "登录失败！");
		break;
	case NO_INTERNET:
		sprintf_s(pMessage, nLen, "没有连接网络！");
		break;
	case LOGIN_ERROR_LOCK:
		sprintf_s(pMessage, nLen, "登陆失败-邮箱已经锁定！");
		break;
	case RETURN_FAIL:
		sprintf_s(pMessage, nLen, "返回错误！");
		break;
	case HOST_UNREACHABLE:
		sprintf_s(pMessage, nLen, "目标主机不可达！");
		break;
	case 10051:
		sprintf_s(pMessage, nLen, "向一个无法连接的网络尝试了一个套接字操作");
		break;
	case 10052:
		sprintf_s(pMessage, nLen, "当该操作在进行中，由于保持活动的操作检测到一个 故障，该连接中断");
		break;
	case 10053:
		sprintf_s(pMessage, nLen, "您的主机中的软件放弃了一个已建立的连接");
		break;
	case 10054:
		sprintf_s(pMessage, nLen, "远程主机强迫关闭了一个现有的连接");
		break;
	case 10060:
		sprintf_s(pMessage,nLen,"由于连接方在一段时间后没有正确的答复或连接的主机没有反应，连接尝试失败");
		break;
	default:
		break;
	}
}