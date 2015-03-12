#pragma once	//系统工具
/////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef GGDATAAPI_EXPORTS
#define GGDATA_API __declspec(dllexport)
#else
#define GGDATA_API __declspec(dllimport)
#pragma	comment(lib,"../output/GGDataAPI.lib")
#endif
#include <algorithm>
#include <map>
#include <ctime>
#include <string>

#include "StdAfx.h"
#include "curl/curl.h"
#include "GGDataApi.h"
#include "utf8.hpp"
#include "base64.h"
#include "HMAC_SHA1.h"

typedef void (*RecvApiDataProc)(DWORD req_type,std::string&strContent,void*pObject);

using namespace std;
using std::string;
using std::wstring;
using std::map;

enum RequestMode{GET,POST};

class GGDATA_API GGDataAPI{
public:
	GGDataAPI():m_pCallback(NULL),m_pUserData(NULL),m_pCurl(NULL),m_curlCode(CURL_LAST),m_hReflectWnd(NULL),m_dwTimeCost(0)
		,m_dwReqMask(0)
	{
		memset(&m_fnRecvApiData,0,sizeof(CALLBACK_INFO));
		m_pCurl = curl_easy_init();
	}

	~GGDataAPI()
	{
		if(m_pCurl != NULL){
			curl_easy_cleanup(m_pCurl);
		}
	}
	//==================================================================
	//函数名： getSyncRequest
	//作者：   chenmusheng
	//日期：   2014-3-12
	//功能：   执行GET方式的同步请求
	//输入参数：apiName[in] 要执行的API名称 
	//			apiParam[in] 要传给API的参数
	//			apiExeTime[out] API执行时间
	//			retContent[out] 执行API返回的页面内容
	//返回值：  curl_easy_perform执行后的状态码
	//修改记录：
	//==================================================================
	static int getSyncRequest(const string& apiName,const string& apiParam,long& apiExeTime,string& retContent);
	//==================================================================
	//函数名： postSyncRequest
	//作者：   chenmusheng
	//日期：   2014-3-12
	//功能：   执行POST方式的同步请求
	//输入参数：apiName[in] 要执行的API名称 
	//			apiParam[in] 要传给API的参数
	//			apiExeTime[out] API执行时间
	//			retContent[out] 执行API返回的页面内容
	//返回值：  curl_easy_perform执行后的状态码
	//修改记录：
	//==================================================================
	static int postSyncRequest(const string& apiName,const string& apiParam,long& apiExeTime,string& retContent);
	string getRetContent(){return m_retContent;}
	static void	SetProxy(string proxyAddress,curl_proxytype proxyType){
		m_useProxy=true;
		m_proxyAddress=proxyAddress;
		m_proxyType=proxyType;}
	static void SetAppInfo(string appHost,string appKey,string appSecret){
		m_appHost=appHost;
		m_appKey=appKey;
		m_appSecret=appSecret;
	}
	//hWnd 接收返回消息的窗口 ，dwType：请求后原样返回，不做改变 对应OnAPICommand 中的req_type
	static int PostAsyncRequest(const string& apiName,const string& apiParam,HWND hwnd,DWORD dwType=0);
	static int GetAsyncRequest(const string& apiName,const string& apiParam,HWND hwnd,DWORD dwType=0);

	//无窗口环境下，采用回调函数实现请求异步
	static int PostAsyncRequest(const string& apiName,const string& apiParam,RecvApiDataProc pfnRecvFunc,void*pObject,DWORD dwType=0);
	
	void SetReflectWnd(HWND hWnd){m_hReflectWnd = hWnd;}
	HWND GetReflectWnd(void)const{return m_hReflectWnd;}

	void SetReqType(DWORD dwType){m_dwReqMask = dwType;}
	DWORD GetReqType(void){return m_dwReqMask;}
private:
	
	static int AsyncRequest(const string& apiName,const string& apiParam,const RequestMode& requestMode,GGDataAPI* pHttp,HWND hWnd,RecvApiDataProc pfnRecvDataProc,void*pObject,DWORD dwType);
	static int SyncRequest(const string& apiName,const string& apiParam,const RequestMode& requestMode,GGDataAPI* pHttp,long& apiExeTime,string& retContent);
	static void AnsiToUTF8(const string& ansi,string& utf8);
	static void UTF8ToAnsi(const string& utf8,string& ansi);
	void setApiName(const string& apiName){m_apiName=apiName;}
	void setApiParam(const string& apiParam);//{m_apiParam=apiParam;}
	void setRequestMode(const RequestMode& requestMode){m_requestMode=requestMode;}	
	void makeUrlParam();
	void setCurlOpt();

	inline void SetRecvDataFunc(RecvApiDataProc pfn){m_fnRecvApiData.pfnRecvApiData = pfn;}
	inline void SetRecvDataObject(void* pObject){m_fnRecvApiData.pObject = pObject;}

	static unsigned char ToHex(unsigned char x) 
	{ 
		return  x > 9 ? x + 55 : x + 48; 
	}
	static string UrlEncode(const string& str);
	static string CodePayValue(const string& str);
	static size_t ResponseProc(void* pBuff, size_t nSize, size_t nCount, GGDataAPI* pHttp);
	static unsigned int __stdcall AsyncThread(GGDataAPI* pHttp);
	static	string m_appHost;
	static	string m_appKey;
	static	string m_appSecret;
	static	bool m_useProxy;
	static	string m_proxyAddress;
	string m_apiName;
	string m_apiParam;
	string m_apiUrlParam;
//	long m_apiExeTime;
	string m_apiUrl;
	string m_retContent;
	RequestMode m_requestMode;
	void* m_pCallback;
	void* m_pUserData;
	CURL* m_pCurl;
	CURLcode m_curlCode;
	static curl_proxytype m_proxyType;//代理类型
	HWND m_hReflectWnd;
	long m_dwTimeCost;
	DWORD m_dwReqMask;
	RecvApiDataProc m_pfnRecvApiData;

	typedef struct tagCallbackInfo
	{
		void* pObject;
		RecvApiDataProc pfnRecvApiData;
	}CALLBACK_INFO,*PCALLBACK_INFO;

	CALLBACK_INFO m_fnRecvApiData;
};

void GGDATA_API ReleaseApiData(char*&pData);