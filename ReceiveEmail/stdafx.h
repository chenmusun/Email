
// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // �� Windows ͷ���ų�����ʹ�õ�����
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // ĳЩ CString ���캯��������ʽ��

// �ر� MFC ��ĳЩ�����������ɷ��ĺ��Եľ�����Ϣ������
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ


#include <afxdisp.h>        // MFC �Զ�����



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC �� Internet Explorer 4 �����ؼ���֧��
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // �������Ϳؼ����� MFC ֧��









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
		lstrcpy(lpMessage, _T("��ʼ���׽��ִ���"));
		break;
	case SEND_ERROR:
		lstrcpy(lpMessage, _T("���ʹ���"));
		break;
	case RECEIVE_ERROR:
		lstrcpy(lpMessage, _T("���մ���"));
		break;
	case LOGIN_ERROR:
		lstrcpy(lpMessage, _T("��¼ʧ�ܣ�"));
		break;
	case NO_INTERNET:
		lstrcpy(lpMessage, _T("û���������磡"));
		break;
	case LOGIN_ERROR_LOCK:
		lstrcpy(lpMessage, _T("��½ʧ��-�����Ѿ�������"));
		break;
	case RETURN_FAIL:
		lstrcpy(lpMessage, _T("���ش���"));
		break;
	case HOST_UNREACHABLE:
		lstrcpy(lpMessage, _T("Ŀ���������ɴ"));
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
		sprintf_s(pMessage, nLen, "��ʼ���׽��ִ���");
		break;
	case SEND_ERROR:
		sprintf_s(pMessage, nLen, "���ʹ���");
		break;
	case RECEIVE_ERROR:
		sprintf_s(pMessage, nLen, "���մ���");
		break;
	case LOGIN_ERROR:
		sprintf_s(pMessage, nLen, "��¼ʧ�ܣ�");
		break;
	case NO_INTERNET:
		sprintf_s(pMessage, nLen, "û���������磡");
		break;
	case LOGIN_ERROR_LOCK:
		sprintf_s(pMessage, nLen, "��½ʧ��-�����Ѿ�������");
		break;
	case RETURN_FAIL:
		sprintf_s(pMessage, nLen, "���ش���");
		break;
	case HOST_UNREACHABLE:
		sprintf_s(pMessage, nLen, "Ŀ���������ɴ");
		break;
	default:
		break;
	}
}