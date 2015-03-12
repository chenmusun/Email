#include "stdafx.h"
#include "POP3.h"
#include "../ReceiveEmail/public.h"
#include "MyJob.h"
#include "../include/json/json.h"

const char ENDOFCHAR[5] = { 0x0d, 0x0a, 0x2e, 0x0d, 0x0a };

POP3::POP3() :m_bConnect(FALSE), m_pParent(NULL), m_bFailed(FALSE)
{
	memset(m_CurrPath, 0, MAX_PATH);
	memset(m_chName, 0, 64);
	memset(&m_Info, 0, sizeof(MailBoxInfo));
}

POP3::~POP3()
{
	m_Socket.CloseMySocket();
}

long POP3::Login(LPCTSTR lpServer, long lPort, LPCTSTR lpUser, LPCTSTR lpPasswd)
{
	char chUserName[64] = { 0 }, chPasswd[128] = { 0 };
	long nValue(0), nError(0);
	string strCurrPos, strSize;
	DWORD nCount = 0;
	char chCommand[128] = { 0 }, chResult[256] = { 0 };
	char *pData = NULL;
	long lTotalSize(0), lRecSize(0);
	nValue=m_Socket.InitSocket(lpServer, lPort);
	if(nValue != SUCCESS)
		return nValue;
	WideCharToMultiByte(CP_ACP, 0, lpUser, 64, chUserName, 64, NULL, NULL);
	sprintf_s(chCommand, 128, "USER %s\r\n",chUserName);
	nValue = m_Socket.SendData(chCommand, strlen(chCommand));
	if (nValue <= 0)
	{
		return SEND_ERROR;
	}
	m_Socket.ReceiveData(chResult, 256);
	if (!StringProcess(chResult))
	{
		m_log.Log(chResult, strlen(chResult));
		return LOGIN_ERROR_LOCK;
	}
	WideCharToMultiByte(CP_ACP, 0, lpPasswd, 128, chPasswd, 128, NULL, NULL);
	sprintf_s(chCommand, 128, "PASS %s\r\n",chPasswd);
	nValue = m_Socket.SendData(chCommand, strlen(chCommand));
	if (nValue <= 0)
	{
		return SEND_ERROR;
	}
	memset(chResult, 0, 256);
	m_Socket.ReceiveData(chResult, 256);
	if (!StringProcess(chResult))
	{
		m_log.Log(chResult, strlen(chResult));
		return LOGIN_ERROR;
	}
	return SUCCESS;
}

long POP3::GetMailCount()
{
	string strCount, strTotalSize;
	int nValue(0),nError(0);
	char chCommand[128] = { 0 }, chResult[128] = { 0 };
	sprintf_s(chCommand, 128, "STAT\r\n");
	nValue = m_Socket.SendData(chCommand, strlen(chCommand));
	if (nValue < 0)
	{
		return SEND_ERROR;
	}
	nValue = m_Socket.ReceiveData(chResult, sizeof(chResult));
	if (nValue <= 0)
	{
		nError = WSAGetLastError();
		m_log.Log(nError);
		m_log.Log(chResult, strlen(chResult));
	}
	StringProcess(chResult, strCount, strTotalSize);
	return atoi(strCount.c_str());
}

BOOL POP3::Close()
{
	int nValue(0), nError(0);
	char chCommand[128] = { 0 }, chResult[128] = { 0 };
	do
	{
		sprintf_s(chCommand, 128, "QUIT\r\n");
		nValue = m_Socket.SendData(chCommand, strlen(chCommand));
		if (nValue <= 0)
			break;
		nValue = m_Socket.ReceiveData(chResult, sizeof(chResult));
		if (nValue <= 0)
			break;
		if (StringProcess(chResult))
			return TRUE;
	} while (0);
	m_Socket.CloseMySocket();
	return FALSE;
}


string POP3::GetUIDL(long lCurrPos)
{
	DWORD nCount = 0;
	int nValue(0), nError(0);
	char chCommand[128] = { 0 }, chResult[128] = { 0 };
	string strNum,strUIDL;
	long lMailCount(0);
	sprintf_s(chCommand, 128, "UIDL %d\r\n",lCurrPos);
	nValue = m_Socket.SendData(chCommand, strlen(chCommand));
	if (nValue < 0)
	{
		strUIDL.empty();
		return strUIDL;
	}
	nValue = m_Socket.ReceiveData(chResult, sizeof(chResult));
	if (nValue <= 0)
	{
		nError = WSAGetLastError();
		m_log.Log(nError);m_log.Log(chResult, strlen(chResult));
	}
	StringProcess(chResult, strNum, strUIDL);
	auto pos = strUIDL.find(":");
	while (pos != strUIDL.npos)
	{
		strUIDL.replace(pos, 1, "");
		pos = strUIDL.find(":",pos);
	}
	return strUIDL;
}

long POP3::CheckUIDL(const string& strUIDL, const string& strName)
{
	long lFound = MONGO_NOT_FOUND;
	if (strUIDL.length() <= 0)
		return MONGO_FOUND;
	string strErr;
	lFound = m_db.ExecSQL(strUIDL, strErr,strName);
	if (strErr.length() > 0)
	{
		m_log.Log(strErr.c_str(), strErr.length());
	}
	return lFound;
}

void POP3::QuitDataBase()
{
	if (m_bConnect)
	{
		m_db.DisConnectDataBase();
		m_bConnect = FALSE;
	}
}

BOOL POP3::ConnectDataBase()
{
	string strErr;
	if (m_db.ConnectDataBase(strErr))
	{
		m_bConnect = TRUE;
		return TRUE;
	}
	else
	{
		m_log.Log(strErr.c_str(), strErr.length());
	}
	return FALSE;
}

long POP3::GetEMLFile(long lCurrPos,const string& strUIDL)
{
#ifdef _DEBUG
	CString csDebug;
#endif
	FILE *pFile;
	CMyJob* pMyJob = NULL;
	string strCurrPos, strSize,strEMail;
	char chCommand[128] = { 0 }, chResult[256] = { 0 }, chTemp[65536] = { 0 };
	int nValue(0), nError(0),nFailedCount(0);
	long lTotalSize(0), lRecSize(0),n(0),lLastDataSize(0),lSize(0);
	char chPath[MAX_PATH] = { 0 };
	m_bFailed = FALSE;
	if (strUIDL.length() <= 0)
		return RETURN_FAIL;
	sprintf_s(chCommand, 128, "LIST %d\r\n", lCurrPos);
	nValue = m_Socket.SendData(chCommand, strlen(chCommand));
	if (nValue <= 0)
	{
		return SEND_ERROR;
	}
	memset(chResult, 0, 256);
	m_Socket.ReceiveData(chResult, 256);
	if (!StringProcess(chResult, strCurrPos, strSize))
	{
		nError = WSAGetLastError();
		m_log.Log(nError);
		m_log.Log(chResult, strlen(chResult));
		return RETURN_FAIL;
	}
	lTotalSize = atoi(strSize.c_str());
	if (lTotalSize <= 0) return RECEIVE_ERROR;
	sprintf_s(chPath, MAX_PATH, "%s\\%s.eml", m_CurrPath, strUIDL.c_str());
	errno_t error = fopen_s(&pFile, chPath, "rb");
	if (error == 0)
	{
		fseek(pFile, SEEK_SET, SEEK_END);
		lSize = ftell(pFile);
		fclose(pFile);
	}
	else lSize = 0;
	if ((lSize<lTotalSize) || (GetFileAttributesA(chPath) == 0xFFFFFFFF))
	{
		memset(chCommand, 0, 128);
		sprintf_s(chCommand, 128, "RETR %d\r\n", lCurrPos);
		nValue = m_Socket.SendData(chCommand, strlen(chCommand));
		if (nValue < 0)
		{
			return SEND_ERROR;
		}
		pMyJob = (CMyJob*)m_pParent;
		do
		{
			n = 0;
			n = m_Socket.ReceiveData(chTemp, 65535);
			if (!pMyJob->Wait())
			{
#ifdef DEBUG
				OutputDebugString(_T("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n"));
#endif
				strEMail.clear();
				break;
			}
			if (n > 0)
			{
				strEMail.append(chTemp);
				memset(chTemp, 0, 65536);
				lRecSize += n;
				lLastDataSize = n;
#ifdef DEBUG
				csDebug.Format(_T("Bytes received: %d----Recv:%d----Total:%d\n"), n, lRecSize, lTotalSize);
				OutputDebugString(csDebug);
#endif
			}
			/*else if (n == 0)
			{
			CString csLog;
			csLog.Format(_T("Connection closed\n"));
			#ifdef DEBUG
			csDebug = csLog;
			OutputDebugString(csDebug);
			#endif
			nError = WSAGetLastError();
			m_log.Log(nError);
			m_log.Log(csLog, csLog.GetLength());
			}*/
			else if (n<=0)
			{
				CString csLog;
				if (n == 0)
				{
					csLog.Format(_T("Connection closed![%s]\n"), WSAGetLastError());
					m_log.Log(csLog, csLog.GetLength());
				}
				else csLog.Format(_T("GetLastError=%d\n"), WSAGetLastError());
				if (nFailedCount>5)//重试五次，否则中断接收
				{
					m_log.Log(csLog, csLog.GetLength());
					m_bFailed = TRUE;
					break;
				}
				if (lRecSize < lTotalSize)
				{
					nFailedCount++;
					continue;
				}
				break;
			}
			Sleep(10);
		} while (1);

		if (!pMyJob->Wait())
		{
			strEMail.clear();
		}
		if (strEMail.length()>0 && !m_bFailed)
		{
			error = fopen_s(&pFile, chPath, "w+b");
			if (error == 0)
			{
				fseek(pFile, 0, SEEK_SET);
				fwrite(strEMail.c_str(), lRecSize, 1, pFile);
				fclose(pFile);
			}
		}
		strEMail.clear();
	}
	else
		Sleep(50);
	char chDebug[256] = { 0 };
	sprintf_s(chDebug, 256, "Receive [%s] Complete!", strUIDL.c_str());
	//m_log.Log(chDebug, strlen(chDebug));
#ifdef _DEBUG
	OutputDebugStringA(chDebug);
	OutputDebugStringA("\r\n");
#endif
	if (m_bFailed)
		return RETURN_FAIL;
	return SUCCESS;
}


void POP3::SetInfo(CString csName, const MailBoxInfo& info,const MongoDBInfo& dbinfo,LPCTSTR lpPath, long lLen)
{
	WideCharToMultiByte(CP_ACP, 0, csName.GetBuffer(), csName.GetLength(), m_chName, 64, NULL, NULL);
	memcpy_s(&m_Info, sizeof(MailBoxInfo), &info, sizeof(MailBoxInfo));
	m_db.SetDBInfo(dbinfo);
	if (lpPath)
	{
		memset(m_CurrPath, 0, MAX_PATH);
		WideCharToMultiByte(CP_ACP, 0, lpPath, lLen, m_CurrPath, MAX_PATH, NULL, NULL);
		char chName[64] = { 0 };
		WideCharToMultiByte(CP_ACP, 0, m_Info.szAbbreviation, lstrlen(m_Info.szAbbreviation), chName, 64, NULL, NULL);
		if (*m_CurrPath != '\0' && *chName != '\0')
		{
			sprintf_s(m_CurrPath, MAX_PATH, "%s\\Mail\\%s", m_CurrPath, chName);
			if ((GetFileAttributesA(m_CurrPath) == 0xFFFFFFFF))
				CreateDirectoryA(m_CurrPath, NULL);
		}
	}
}

long POP3::DelEmail(long lCurrPos,const string& strUIDL)
{
	int nValue(0), nError(0);
	char chCommand[128] = { 0 }, chResult[256] = { 0 }, chTemp[65537] = { 0 };
	sprintf_s(chCommand, 128, "DELE %d\r\n", lCurrPos);
	nValue = m_Socket.SendData(chCommand, strlen(chCommand));
	if (nValue <= 0)
	{
		return SEND_ERROR;
	}
	nValue = m_Socket.ReceiveData(chResult, 256);
	if (!StringProcess(chResult))
		return RETURN_FAIL;
	//DeleteFile()
	sprintf_s(chTemp, 65537, "%s\\%s.eml", m_CurrPath, strUIDL.c_str());
	if(remove(chTemp)>=0)
		return SUCCESS;
	return SUCCESS;
}

void POP3::SetLogPath(const char*pPath)
{
	if (pPath && pPath[0] != '\0')
	{
		m_log.SetPath(pPath);
	}
}

void POP3::SaveFileToDB()
{
	string str, strmd5;
	Json::Value js_value;
	Json::Reader js_reader;
	m_db.SaveFileToMongoDB(str);
	js_reader.parse(str, js_value);
	strmd5 = /*js_value["md5"].asString()*/str;
	auto pos1 = strmd5.find("\"");
	if (pos1 > 0 && pos1 != string::npos)
	{
		auto pos2 = strmd5.find("\"",pos1+1);
		if (pos2 > 0 && pos2 != string::npos)
		{
			str = strmd5.substr(pos1+1, pos2 - pos1 - 1);
		}
	}
}
/////////////////////////////////////////////////////////////////////
