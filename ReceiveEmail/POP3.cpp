#include "stdafx.h"
#include "POP3.h"
#include "../include/json/json.h"

const char ENDOFCHAR[5] = { 0x0d, 0x0a, 0x2e, 0x0d, 0x0a };

POP3::POP3() :m_bConnect(FALSE), m_bFailed(FALSE)
{
	memset(m_CurrPath, 0, MAX_PATH);
	memset(m_chName, 0, 64);
	memset(&m_Info, 0, sizeof(MailBoxInfo));
}

POP3::~POP3()
{
	string strErr;
	m_Socket.CloseMySocket(strErr);
}

long POP3::Login(LPCTSTR lpServer, long lPort, LPCTSTR lpUser, LPCTSTR lpPasswd)
{
	char chUserName[64] = { 0 }, chPasswd[128] = { 0 };
	long nValue(0), nError(0);
	string strCurrPos, strSize,strErr;
	DWORD nCount = 0;
	char chCommand[128] = { 0 }, chResult[256] = { 0 };
	char *pData = NULL;
	long lTotalSize(0), lRecSize(0);
	nValue = m_Socket.InitSocket(lpServer, lPort, strErr);
	if (nValue != SUCCESS)
	{
		m_log.Log(strErr.c_str(), strErr.length());
		return nValue;
	}
	m_bFailed = FALSE;
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
		return LOGIN_ERROR;
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
		char *pDest = NULL;
		pDest = strstr(chResult, "ERR maildrop already locked");
		if (pDest==NULL)
			m_log.Log(chResult, strlen(chResult));
		return LOGIN_ERROR;
	}
	return SUCCESS;
}

long POP3::GetMailCount()
{
	string strCount, strTotalSize;
	int nValue(0),nError(0);
	char chCommand[128] = { 0 }, chResult[128] = { 0 }, chError[128] = { 0 };
	sprintf_s(chCommand, 128, "STAT\r\n");
	nValue = m_Socket.SendData(chCommand, strlen(chCommand));
	if (nValue < 0)
	{
		nError = WSAGetLastError();
		GetErrorMessage(nError, chError, 128);
		m_log.Log(chError, strlen(chError));
		return SEND_ERROR;
	}
	nValue = m_Socket.ReceiveData(chResult, sizeof(chResult));
	if (nValue <= 0)
	{
		nError = WSAGetLastError();
		GetErrorMessage(nError, chError, 128);
		m_log.Log(chError,strlen(chError));
		m_log.Log(chResult, strlen(chResult));
	}
	StringProcess(chResult, strCount, strTotalSize);
	return atoi(strCount.c_str());
}

BOOL POP3::Close()
{
	string strErr;
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
		{
			m_Socket.CloseMySocket(strErr);
			return TRUE;
		}
	} while (0);
	m_Socket.CloseMySocket(strErr);
	return FALSE;
}

string POP3::GetUIDL(long lCurrPos)
{
	DWORD nCount = 0;
	int nValue(0), nError(0);
	char chCommand[128] = { 0 }, chResult[128] = { 0 }, chError[128] = {0};
	string strNum,strUIDL,strData;
	long lMailCount(0);
	sprintf_s(chCommand, 128, "UIDL %d\r\n",lCurrPos);
	nValue = m_Socket.SendData(chCommand, strlen(chCommand));
	if (nValue < 0)
	{
		strUIDL.clear();
		return strUIDL;
	}
	nValue = m_Socket.ReceiveData(chResult, sizeof(chResult));
	if (nValue <= 0)
	{
		nError = WSAGetLastError();
		GetErrorMessage(nError, chError, 128);
		m_log.Log(chError, strlen(chError));
		m_log.Log(chResult, strlen(chResult));
		strUIDL.clear();
		return strUIDL;
	}
	strData=chResult;
	auto pos = strData.find("\r\n+OK");
	if (pos > 0 && pos != strData.npos)
	{
		strData = strData.substr(pos + 2);
	}
	StringProcess(strData, strNum, strUIDL);
	nValue = atoi(strNum.c_str());
	if (nValue != lCurrPos)
	{
		strUIDL.clear();
		return strUIDL;
	}
	pos = strUIDL.find(":");
	while (pos != strUIDL.npos)
	{
		strUIDL.replace(pos, 1, "");
		pos = strUIDL.find(":",pos);
	}
	return strUIDL;
}

long POP3::CheckUIDL(const string& strUIDL, const string& strName, long lSaveDay)
{
	long lFound = MONGO_NOT_FOUND;
	if (strUIDL.length() <= 0)
		return MONGO_FOUND;
	string strErr;
	if (lSaveDay == 0)
		lSaveDay = 14;
	lFound = m_db.CheckUIDLInMongoDB(strUIDL, strErr, strName, lSaveDay);
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
//#ifdef _DEBUG
	DWORD dwTime(GetTickCount64());
	CString csDebug;
//#endif
	FILE *pFile;
	string strCurrPos, strSize,strEMail;
	char chCommand[128] = { 0 }, chResult[256] = { 0 }, chTemp[65536] = { 0 }, chError[128] = {0};
	int nValue(0), nError(0),nFailedCount(0);
	long lTotalSize(0), lRecSize(0),n(0),lLastDataSize(0),lSize(0);
	char chPath[MAX_PATH] = { 0 };
	m_bFailed = FALSE;
	if (strUIDL.length() <= 0)
		return RETURN_FAIL;
	sprintf_s(chCommand, 128, "LIST %d\r\n", lCurrPos);

	do 
	{
		nValue = m_Socket.SendData(chCommand, strlen(chCommand));
		if (nValue>0)
			break;
		nFailedCount++;
		nError = WSAGetLastError();
		memset(chError, 0, 128);
		GetErrorMessage(nError, chError, 128);
		m_log.Log(chError, strlen(chError));
		m_log.Log(chResult, strlen(chResult));
	} while (nFailedCount<2);
	if (nFailedCount>1)
	{
		m_bFailed = TRUE;
		return SEND_ERROR;
	}

	nFailedCount = 0;
	do 
	{
		memset(chResult, 0, 256);
		m_Socket.ReceiveData(chResult, 256);
		if (StringProcess(chResult, strCurrPos, strSize))
			break;
		nFailedCount++;
		nError = WSAGetLastError();
		memset(chError, 0, 128);
		GetErrorMessage(nError, chError, 128);
		m_log.Log(chError, strlen(chError));
		m_log.Log(chResult, strlen(chResult));
	} while (nFailedCount<2);
	if (nFailedCount>1)
	{
		m_bFailed = TRUE;
		return RETURN_FAIL;
	}

	nFailedCount = 0;
	lTotalSize = atoi(strSize.c_str());
	if (lTotalSize <= 0) return RECEIVE_ERROR;
	strEMail.reserve(lTotalSize);
	sprintf_s(chPath, MAX_PATH, "%s\\%s.eml", m_CurrPath, strUIDL.c_str());
	errno_t error = fopen_s(&pFile, chPath, "rb");
	if (error == 0)
	{
		fseek(pFile, SEEK_SET, SEEK_END);
		lSize = ftell(pFile);
		fclose(pFile);
	}
	else lSize = 0;
	if ((lSize < lTotalSize) || (GetFileAttributesA(chPath) == 0xFFFFFFFF))
	{
		memset(chCommand, 0, 128);
		sprintf_s(chCommand, 128, "RETR %d\r\n", lCurrPos);

		do
		{
			nValue = m_Socket.SendData(chCommand, strlen(chCommand));
			if (nValue>0)
				break;
			m_bFailed = TRUE;
			return SEND_ERROR;
		} while (nFailedCount < 2);
		do
		{
			n = 0;
			n = m_Socket.ReceiveData(chTemp, 65535);
			if (n > 0)
			{
				nFailedCount = 0;
				strEMail.append(chTemp);
				memset(chTemp, 0, 65536);
				lRecSize += n;
				lLastDataSize = n;
#ifdef DEBUG
				csDebug.Format(_T("Bytes received: %d----Recv:%d----Total:%d\n"), n, lRecSize, lTotalSize);
				OutputDebugString(csDebug);
#endif
			}
			else if (n <= 0)
			{
				CString csLog;
				GetErrorMessage(WSAGetLastError(), chError, 128);
				if (n == 0)
				{
					csLog.Format(_T("Connection closed![%s]\n"), chError);
					m_log.Log(csLog, csLog.GetLength());
				}
				else csLog.Format(_T("GetLastError=%s\n"), chError);
				if (nFailedCount > 5)//重试五次，否则中断接收
				{
					m_log.Log(csLog, csLog.GetLength());
					m_bFailed = TRUE;
					break;
				}
				if (lRecSize < lTotalSize)
				{
					OutputDebugStringA("*************Received Faild!*************\r\n");
					nFailedCount++;
					Sleep(500);
					continue;
				}
				break;
			}
			Sleep(10);
		} while (1);

		if (strEMail.length() > 0 && !m_bFailed)
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
	if (m_bFailed)
	{
		sprintf_s(chDebug, 256, "Receive Time = %d\tReceive [%s] Failed!\t[%d-%d]", dwTime / 1000, strUIDL.c_str(), lSize, lTotalSize);
		m_log.Log(chDebug, strlen(chDebug));
	}
	else
	{
		dwTime = GetTickCount64() - dwTime;
		sprintf_s(chDebug, 256, "Receive Time = %d\tReceive [%s] Complete!", dwTime / 1000, strUIDL.c_str());
	}
	//m_log.Log(chDebug, strlen(chDebug));
	OutputDebugStringA(chDebug);
	OutputDebugStringA("\r\n");
	if (m_bFailed)
		return RETURN_FAIL;
	return SUCCESS;
}


void POP3::SetInfo(CString csName, const MailBoxInfo& info,LPCTSTR lpPath, long lLen)
{
	WideCharToMultiByte(CP_ACP, 0, csName.GetBuffer(), csName.GetLength(), m_chName, 64, NULL, NULL);
	memcpy_s(&m_Info, sizeof(MailBoxInfo), &info, sizeof(MailBoxInfo));
	if (lpPath&&lLen>0)
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
	string strSUIDL;
	int nValue(0), nError(0);
	char chCommand[128] = { 0 }, chResult[256] = { 0 }, chTemp[65537] = { 0 };
	strSUIDL = GetUIDL(lCurrPos);
	if (strSUIDL == strUIDL)
	{
		sprintf_s(chCommand, 128, "DELE %d\r\n", lCurrPos);
		nValue = m_Socket.SendData(chCommand, strlen(chCommand));
		if (nValue <= 0)
		{
			return SEND_ERROR;
		}
		nValue = m_Socket.ReceiveData(chResult, 256);
		if (StringProcess(chResult))
		{
			//DeleteFile()
			sprintf_s(chTemp, 65537, "%s\\%s.eml", m_CurrPath, strUIDL.c_str());
			if (remove(chTemp) >= 0)
				return SUCCESS;
		}
	}
	return RETURN_FAIL;
}

void POP3::SetLogPath(const char*pPath,long lLen)
{
	if (pPath&&lLen>0)
	{
		m_log.SetPath(pPath,lLen);
	}
}

BOOL POP3::SaveFileToDB(EMAIL_ITEM& email)
{
	long lCount(0);
	char chGUID[128] = { 0 }, chFilePath[512] = {0};
	string strRemote, strPath, strRtr;
	vector<ATTACH_FILE>::iterator ite = email.vecAttachFiles.begin();
	while (ite!=email.vecAttachFiles.end())
	{
		memset(&chGUID, 0, 128);
		memset(&chFilePath, 0,512);
		if ((*ite).lType == 0)
		{
			ite++;
			continue;
		}
		WideCharToMultiByte(CP_ACP, 0, (*ite).csRemoteName.GetBuffer(), (*ite).csRemoteName.GetLength(), chGUID, 128, NULL, NULL);
		strRemote = chGUID;
		WideCharToMultiByte(CP_ACP, 0, (*ite).csFilePath.GetBuffer(), (*ite).csFilePath.GetLength(), chFilePath, 512, NULL, NULL);
		strPath = chFilePath;
		//strPath = "D:\\20150315_既要谋势，又要做活_(王涵_高群山_卢燕津_贾潇君_王连庆_王轶君_唐跃)_兴业宏观中国周报.pdf";
		if (m_db.SaveFileToMongoDB(strRemote, strPath, strRtr) < 0)
		{
			m_log.Log(strRtr.c_str(), strRtr.length());
			lCount++;
		}
		else (*ite).csMD5 = strRtr.c_str();
		ite++;
	}
	if (lCount>0)
		return FALSE;
	return TRUE;
}

BOOL POP3::DeleteFromDB(EMAIL_ITEM& email)
{
	char chTemp[MAX_PATH] = { 0 };
	string strUIDL, strTo;
	WideCharToMultiByte(CP_ACP, 0, email.csUIDL, email.csUIDL.GetLength(), chTemp, MAX_PATH, NULL, NULL);
	strUIDL = chTemp;
	memset(&chTemp, 0, MAX_PATH);
	WideCharToMultiByte(CP_ACP, 0, email.csTo, email.csTo.GetLength(), chTemp, MAX_PATH, NULL, NULL);
	strTo = chTemp;
	if(m_db.DelUIDL(strUIDL,strTo))
		return TRUE;
	return FALSE;
}

void POP3::SetDBinfo(const MongoDBInfo& dbinfo)
{
	m_db.SetDBInfo(dbinfo);
}

BOOL POP3::DeleteFromDB(const string& strUIDL)
{
	return m_db.DelUIDL(strUIDL);
}

BOOL POP3::GetUDILs(map<long, string>& mapUIDLs, long lTotal)
{
	string strUIDLs,strTemp,strUIDL;
	long lSn(0);
	int nValue(0), n(0), nStart(0), nFailedCount(0);
	char chCommand[128] = { 0 }, chResult[256] = { 0 }, chBuffer[65536] = { 0 };
	sprintf_s(chCommand, 128, "UIDL\r\n");
	nValue = m_Socket.SendData(chCommand, strlen(chCommand));
	if (nValue <= 0)
	{
		return FALSE;
	}
	do 
	{
		n = m_Socket.ReceiveData(chBuffer, 65535);
		if (n > 0)
		{
			strUIDLs.append(chBuffer);
#ifdef _DEBUG
			OutputDebugStringA(chBuffer);
#endif
			memset(chBuffer, 0, 65536);
		}
		else if (n <= 0)
		{
			if (nFailedCount > 5)//重试五次，否则中断接收
			{
				break;
			}
			nFailedCount++;
			Sleep(500);
			continue;
		}
		Sleep(10);
	} while (n>0);
	auto pos = strUIDLs.find("+OK\r\n");
	if (pos>= 0 && pos != strUIDLs.npos)
	{
		strUIDLs = strUIDLs.substr(pos + 5);
		do 
		{
			strTemp.clear();
			strUIDL.clear();
			pos = strUIDLs.find("\r\n", nStart);
			if (pos >= 0 && pos != strUIDLs.npos)
			{
				strTemp = strUIDLs.substr(nStart, pos - nStart);
				nStart = pos + 2;
				if (StringProcess(strTemp, lSn, strUIDL))
				{
					mapUIDLs.insert(make_pair(lSn, strUIDL));
				}
			}
			else break;
		} while (1);
		return TRUE;
	}
	return FALSE;
}
/////////////////////////////////////////////////////////////////////
