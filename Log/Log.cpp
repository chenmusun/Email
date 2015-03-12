// Log.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "Log.h"
#include <stdio.h>
#include <string>
using namespace std;


// 这是导出变量的一个示例
LOG_API int nLog=0;

// 这是导出函数的一个示例。
LOG_API int fnLog(void)
{
	return 42;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 Log.h
CLog::CLog() :m_fp(NULL), m_lLevel(0)
{
	memset(m_chLogPath, 0, MAX_PATH);
}

CLog::~CLog()
{

}


void CLog::SetPath(LPCTSTR lpPath,long lLen)
{
	if (lpPath && lLen>0)
		WideCharToMultiByte(CP_ACP, 0, lpPath, lLen, m_chLogPath, MAX_PATH, NULL, NULL);
}

void CLog::SetPath(const char*pPath)
{
	if (pPath && pPath[0] != '\0')
		sprintf_s(m_chLogPath, MAX_PATH, "%s", pPath);
}

void CLog::Log(LPCTSTR lpText, size_t lLen)
{
	if (strlen(m_chLogPath) <= 0)
		return;
	if ((lpText && lLen > 0))
	{
		char chTime[64] = { 0 };
		string strText;
		char *pText = NULL;
		pText = new char[lLen*2 + 1];
		memset(pText, 0, lLen*2 + 1);
		WideCharToMultiByte(CP_ACP, 0, lpText, (int)lLen, pText, (int)lLen * 2, NULL, NULL);
		GetCurrTime(chTime,64);
		strText.append(chTime);
		strText.append(pText);
		strText.append("\r\n");
		
		if (fopen_s(&m_fp, m_chLogPath, "a+") == 0)
		{
			fseek(m_fp, 0, SEEK_END);
			fwrite(strText.c_str(), strText.length(), 1, m_fp);
			fclose(m_fp);
		}
		if (pText)
		{
			delete[] pText;
			pText = NULL;
		}
	}
}

void CLog::Log(LPCSTR lpText,size_t lLen)
{
	if (strlen(m_chLogPath) <= 0)
		return;
	if (lpText && *lpText != '\0')
	{
		char *pText = NULL;
		char chTime[64] = { 0 };
		pText = new char[64 + lLen * 2 + 1];
		memset(pText, 0, 64 + lLen * 2 + 1);
		GetCurrTime(chTime, 64);
		sprintf_s(pText, 64 + lLen * 2, "%s%s\r\n",
			chTime, lpText);
		if (fopen_s(&m_fp, m_chLogPath, "a+") == 0)
		{
			fseek(m_fp, 0, SEEK_END);
			fwrite(pText, strlen(pText), 1, m_fp);
			fclose(m_fp);
		}
	
		if (pText)
		{
			delete[] pText;
			pText = NULL;
		}
	}
}

void CLog::Log(long lCode)
{
	if (strlen(m_chLogPath) == 0)
		return;
	char chText[128] = { 0 }, chTime[64] = { 0 };
	GetCurrTime(chTime, 64);
	sprintf_s(chText, "%s[%d]\r\n", chTime,lCode);
	if (chText[0] != '\0')
	{
		if (fopen_s(&m_fp, m_chLogPath, "a+") == 0)
		{
			fseek(m_fp, 0, SEEK_END);
			fwrite(chText, strlen(chText), 1, m_fp);
			fclose(m_fp);
		}
	}
}

void CLog::GetCurrTime(char* pTime,long lLen)
{
	SYSTEMTIME systm;
	GetLocalTime(&systm);
	sprintf_s(pTime, lLen, "%4d-%.2d-%.2d %.2d:%.2d:%.2d-",
		systm.wYear, systm.wMonth, systm.wDay,
		systm.wHour, systm.wMinute, systm.wSecond);
}

