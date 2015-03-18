#include "stdafx.h"
#include "ggdataapi.h"
#include <process.h>
#include "../ReceiveEmail/public.h"
const char *RequestModeName[]={"GET","POST"};
string GGDataAPI::m_appHost="http://ggservice.sandbox.gofund.cn/";
string GGDataAPI::m_appKey="tvVfsVFAgtYWIGw";
string GGDataAPI::m_appSecret="7Ur1yME47G98NwK6ofiRUZuyENWh8eKo";
bool GGDataAPI::m_useProxy=false;
string GGDataAPI::m_proxyAddress="";
curl_proxytype GGDataAPI::m_proxyType=CURLPROXY_HTTP;


string GGDataAPI::UrlEncode(const string& str)
{
	string strTemp = "";
	size_t length = str.length();
	for (size_t i = 0; i < length; i++)
	{
		if (isalnum((unsigned char)str[i]) || (str[i] == '.') || (str[i] == '_') || (str[i] == '-')){
			strTemp += str[i];
		}else{
			strTemp += '%';
			strTemp += ToHex((unsigned char)str[i] >> 4);
			strTemp += ToHex((unsigned char)str[i] % 16);
		}
	}
	return strTemp;
}

string GGDataAPI::CodePayValue(const string& str)
{
	wstring wcs;
	Utf8::Decode(str,wcs);

	string strTemp = "";
	int nLen = wcs.size();
	for (int i=0;i<nLen;++i)
	{
		if ((L'0' <= wcs[i] && wcs[i]<= L'9') || (L'A' <= wcs[i] && wcs[i]<= L'Z') || (L'a' <= wcs[i] && wcs[i]<= L'z') || wcs[i] == L'!' || wcs[i] == L'*' || wcs[i] == L'(' || wcs[i] == L')'){
			strTemp += (char)wcs[i];
		}else{
			strTemp += '%';
			string strChUtf8;
			wstring wcsCh(&wcs[i],1);
			Utf8::Encode(wcsCh,strChUtf8);
			unsigned char ch = strChUtf8[strChUtf8.size()-1];
			strTemp += ToHex((ch%256) >> 4);
			strTemp += ToHex(ch%16);
		}
	}

	return strTemp;
}

void GGDataAPI::makeUrlParam()
{
	string strEncode(RequestModeName[m_requestMode]);
	string apiName = UrlEncode(m_apiName);
	string apiParam=m_apiParam;
	strEncode.append("&").append(apiName).append("&");

	if(!apiParam.empty()){
		apiParam.append("&");
	}
	apiParam.append("app_key=").append(m_appKey);

	time_t tt;
	time(&tt);
	char szTime[16];
	sprintf_s(szTime,16,"%d",tt);
	apiParam.append("&time_stamp=").append(szTime);

	map<string,string> mapParam;
	for(size_t i=0;i<apiParam.length();){
		string strSub;
		size_t nPos = apiParam.find('&',i);
		if(nPos != string::npos){
			strSub = apiParam.substr(i,nPos-i);
			size_t nSubPos = strSub.find('=',1);
			if(nSubPos != string::npos){
				string strKey = strSub.substr(0,nSubPos);
				string strValue = strSub.substr(nSubPos+1,string::npos);
				mapParam[strKey] = strValue;
			}
			i = nPos+1;
		}else{
			strSub = apiParam.substr(i,string::npos);
			size_t nSubPos = strSub.find('=',1);
			if(nSubPos != string::npos){
				string strKey = strSub.substr(0,nSubPos);
				string strValue = strSub.substr(nSubPos+1,string::npos);
				mapParam[strKey] = strValue;
			}
			break;
		}
	}

	string strParam;
	mapParam.erase("sign");
	for(map<string, string>::iterator iter=mapParam.begin();iter!=mapParam.end();++iter){
		if(iter!=mapParam.begin()){
			strParam.append("&");
		}
		strParam.append(iter->first).append("=").append(CodePayValue(iter->second));
	}
	strEncode.append(UrlEncode(strParam));

	BYTE byHmacSha1[20] ; 
	CHMAC_SHA1 HMAC_SHA1 ;
	HMAC_SHA1.HMAC_SHA1((BYTE*)strEncode.c_str(), strEncode.size(), (BYTE*)m_appSecret.c_str(), m_appSecret.size(), byHmacSha1);

	string strSign = base64_encode(byHmacSha1, sizeof(byHmacSha1));

	strParam = "";
	mapParam["sign"] = strSign;
	for(map<string, string>::iterator iter=mapParam.begin();iter!=mapParam.end();++iter){
		if(iter!=mapParam.begin()){
			strParam.append("&");
		}
		strParam.append(iter->first).append("=").append(UrlEncode(iter->second));
	}

	m_apiUrlParam=strParam;

}

void GGDataAPI::AnsiToUTF8(const string& ansi,string& utf8)
{
	// ANSI TO UNICODE
	const char* szAnsi = ansi.c_str();
	int wcsLen = ::MultiByteToWideChar(CP_ACP, 0, szAnsi, strlen(szAnsi), NULL, 0);
	wchar_t* wszString = new wchar_t[wcsLen + 1];
	::MultiByteToWideChar(CP_ACP, 0, szAnsi, strlen(szAnsi), wszString, wcsLen);
	wszString[wcsLen] = L'\0';
	//UNICODE TO UTF8
	int utf8Len = ::WideCharToMultiByte(CP_UTF8, NULL, wszString, wcslen(wszString), NULL, 0, NULL, NULL);
	char* szUTF8 = new char[utf8Len + 1];
	::WideCharToMultiByte(CP_UTF8, NULL, wszString, wcslen(wszString), szUTF8, utf8Len, NULL, NULL);
	szUTF8[utf8Len] = '\0';

	utf8=szUTF8;
	delete[] wszString;
	delete[] szUTF8;
}

void GGDataAPI::UTF8ToAnsi(const string& utf8,string& ansi)
{
	// UTF8 TO UNICODE
	const char* szUtf8 = utf8.c_str();
	int wcsLen = ::MultiByteToWideChar(CP_UTF8, 0, szUtf8, strlen(szUtf8), NULL, 0);
	wchar_t* wszString = new wchar_t[wcsLen + 1];
	::MultiByteToWideChar(CP_UTF8, 0, szUtf8, strlen(szUtf8), wszString, wcsLen);
	wszString[wcsLen] = L'\0';
	//UNICODE TO ANSI
	int ansiLen = ::WideCharToMultiByte(CP_ACP, NULL, wszString, wcslen(wszString), NULL, 0, NULL, NULL);
	char* szAnsi = new char[ansiLen + 1];
	::WideCharToMultiByte(CP_ACP, NULL, wszString, wcslen(wszString), szAnsi, ansiLen, NULL, NULL);
	szAnsi[ansiLen] = '\0';

	ansi=szAnsi;
	delete[] wszString;
	delete[] szAnsi;
}

void GGDataAPI::setApiParam(const string& apiParam)
{
	AnsiToUTF8(apiParam,m_apiParam);
}

void GGDataAPI::setCurlOpt()
{
	if(m_requestMode==GET){
		curl_easy_setopt( m_pCurl, CURLOPT_HTTPGET, 1 );
		makeUrlParam();
		m_apiUrl= "";
		m_apiUrl.append(m_appHost).append(m_apiName).append("?").append(m_apiUrlParam);
		curl_easy_setopt(m_pCurl, CURLOPT_URL, m_apiUrl.c_str());
	}else if(m_requestMode == POST){
		curl_easy_setopt(m_pCurl, CURLOPT_POST,1);
		m_apiUrl = "";
		m_apiUrl.append(m_appHost).append(m_apiName);
		curl_easy_setopt(m_pCurl, CURLOPT_URL, m_apiUrl.c_str());
		makeUrlParam();
		curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, m_apiUrlParam.c_str());
		curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDSIZE, m_apiUrlParam.length());		
	}

	if (m_useProxy)
	{
		curl_easy_setopt(m_pCurl,CURLOPT_PROXYTYPE,m_proxyType);
		curl_easy_setopt(m_pCurl,CURLOPT_PROXY,m_proxyAddress.c_str());		
	}
	curl_easy_setopt(m_pCurl, CURLOPT_ENCODING, "gzip");
	curl_easy_setopt(m_pCurl, CURLOPT_TIMEOUT_MS, 10000); 
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, &ResponseProc);
	curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, this); 
}

size_t GGDataAPI::ResponseProc(void* pBuff, size_t nSize, size_t nCount, GGDataAPI* pHttp)
{
	unsigned int nLen = nSize*nCount;
	char* pBuf = new char[nLen + 1];
	if(pBuf != NULL){
		pBuf[nLen] = '\0';
		memcpy(pBuf,pBuff,nLen);
		pHttp->m_retContent.append(pBuf);
		delete[] pBuf;
		return (nLen);
	}
	return 0;
}

int GGDataAPI::getSyncRequest(const string& apiName,const string& apiParam,long& apiExeTime,string& retContent)
{
	GGDataAPI* pHttp = new GGDataAPI;
	if(pHttp == NULL){
		return false;
	}
	if(pHttp->m_pCurl == NULL){
		delete pHttp;
		return false;
	}
    
	int curlCode=SyncRequest(apiName,apiParam,GET,pHttp,apiExeTime,retContent);
	delete pHttp;
	return curlCode;
}

int GGDataAPI::postSyncRequest(const string& apiName,const string& apiParam,long& apiExeTime,string& retContent)
{
	GGDataAPI* pHttp = new GGDataAPI;
	if(pHttp == NULL){
		return false;
	}
	if(pHttp->m_pCurl == NULL){
		delete pHttp;
		return false;
	}

	int curlCode=SyncRequest(apiName,apiParam,POST,pHttp,apiExeTime,retContent);
	delete pHttp;
	return curlCode;
}

int GGDataAPI::SyncRequest(const string& apiName,const string& apiParam,const RequestMode& requestMode,GGDataAPI* pHttp,long& apiExeTime,string& retContent)
{
	pHttp->setApiName(apiName);
	pHttp->setApiParam(apiParam);
	pHttp->setRequestMode(requestMode);
	pHttp->setCurlOpt();
	long requestStartTime= (long)GetTickCount64();
	CURLcode curlCode = curl_easy_perform(pHttp->m_pCurl);
	apiExeTime = (long)GetTickCount64() - requestStartTime;
	UTF8ToAnsi(pHttp->getRetContent(),retContent);
	return curlCode;
}


int GGDataAPI::PostAsyncRequest(const string& apiName,const string& apiParam,HWND hwnd,DWORD dwType)
{
	GGDataAPI* pHttp = new GGDataAPI;
	if(pHttp == NULL){
		return false;
	}
	if(pHttp->m_pCurl == NULL){
		delete pHttp;
		return false;
	}

	int curlCode=AsyncRequest(apiName,apiParam,POST,pHttp,hwnd,NULL,NULL,dwType);
	return curlCode;
}

int GGDataAPI::GetAsyncRequest(const string& apiName,const string& apiParam,HWND hwnd,DWORD dwType)
{
	GGDataAPI* pHttp = new GGDataAPI;
	if(pHttp == NULL){
		return false;
	}
	if(pHttp->m_pCurl == NULL){
		delete pHttp;
		return false;
	}

	int curlCode=AsyncRequest(apiName,apiParam,GET,pHttp,hwnd,NULL,NULL,dwType);
	return curlCode;
}

int GGDataAPI::AsyncRequest(const string& apiName,const string& apiParam,const RequestMode& requestMode
							,GGDataAPI* pHttp,HWND hWnd,RecvApiDataProc pfnRecvDataProc,void*pObjcet,DWORD dwType)
{
	pHttp->setApiName(apiName);
	pHttp->setApiParam(apiParam);
	pHttp->setRequestMode(requestMode);
	pHttp->setCurlOpt();
	pHttp->SetReflectWnd(hWnd);
	pHttp->SetReqType(dwType);
	pHttp->SetRecvDataFunc(pfnRecvDataProc);
	pHttp->SetRecvDataObject(pObjcet);
	
	unsigned int nThreadID = 0;
	if(-1L == _beginthreadex(NULL, 0, (unsigned int (__stdcall *)(void *))AsyncThread, pHttp, 0, &nThreadID)){
		delete pHttp;
		return false;
	}
	return true;
}

unsigned int __stdcall GGDataAPI::AsyncThread(GGDataAPI* pHttp)
{
	long nCount(0);

	CURLcode curlCode = CURL_LAST;
	pHttp->m_dwTimeCost = (long)GetTickCount64();
	do 
	{
		CURLcode curlCode = curl_easy_perform(pHttp->m_pCurl);
		if(curlCode ==  CURLE_OK) break;
	} while (++nCount<4);   //如果网络异常，重复3次
	
	pHttp->m_dwTimeCost = (long)GetTickCount64() - pHttp->m_dwTimeCost;
	std::string strContent;
	UTF8ToAnsi(pHttp->getRetContent(),strContent);

	DWORD dwReqType = pHttp->GetReqType();
	if(NULL == pHttp->m_fnRecvApiData.pfnRecvApiData)
	{
		char* pData = NULL;
		long lSize = strContent.length();
		pData = new char[lSize*2];
		memset(pData, 0, lSize * 2);
		if (pData)
		{
			sprintf_s(pData, lSize*2, "%s", strContent.c_str());

			::PostMessage(pHttp->GetReflectWnd() ? pHttp->GetReflectWnd() : NULL
				, __umymessage_api_netcommand__, (WPARAM)MAKELONG(curlCode, dwReqType), (LPARAM)pData);
			/*::SendMessage(pHttp->GetReflectWnd() ? pHttp->GetReflectWnd() : NULL
				, __umymessage_api_netcommand__, (WPARAM)MAKELONG(curlCode, dwReqType), (LPARAM)pData);*/
		}
	}
	else
	{
		pHttp->m_fnRecvApiData.pfnRecvApiData(MAKELONG(curlCode,dwReqType),strContent,pHttp->m_fnRecvApiData.pObject);
	}
	
	delete pHttp;
	return 0;
}


int GGDataAPI::PostAsyncRequest(const string& apiName,const string& apiParam,RecvApiDataProc pfnRecvFunc,void*pObject,DWORD dwType)
{
	GGDataAPI* pHttp = new GGDataAPI;
	if(pHttp == NULL){
		return false;
	}
	if(pHttp->m_pCurl == NULL){
		delete pHttp;
		return false;
	}

	int curlCode=AsyncRequest(apiName,apiParam,POST,pHttp,NULL,pfnRecvFunc,pObject,dwType);
	return curlCode;
}