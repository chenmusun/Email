#include "stdafx.h"
#include "SendEmail.h"
#include "../ReceiveEmail/public.h"

#define min(a,b)            (((a) < (b)) ? (a) : (b))

static const char base64_table[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char week[][16] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
static const char month[][16] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };


std::string base64_encode(const string& decoded_string)
{
	const char base64_pad = '=';
	char end = '\0';
	string out;
	int inlen = decoded_string.size();
	int inlen1, inlen2 = inlen % 3;
	inlen1 = inlen - inlen2;
	int i;
	if (inlen1 > 0)
	for (i = 0; i < inlen1; i += 3)
	{
		out += base64_table[(decoded_string[i] & 0xFF) >> 2];

		out += base64_table[((decoded_string[i] & 0x3) << 4) | ((decoded_string[i + 1] & 0xF0) >> 4)];
		out += base64_table[((decoded_string[i + 1] & 0xF) << 2) | ((decoded_string[i + 2] & 0xC0) >> 6)];
		out += base64_table[decoded_string[i + 2] & 0x3f];
	}
	if (inlen2 > 0)
	{
		unsigned char fragment;
		out += base64_table[(decoded_string[inlen1] & 0xFF) >> 2];
		fragment = (decoded_string[inlen1] & 0x3) << 4;
		if (inlen2 > 1)
			fragment |= (decoded_string[inlen1 + 1] & 0xF0) >> 4;
		out += base64_table[fragment];
		out += (inlen2 < 2) ? base64_pad : base64_table[(decoded_string[inlen1 + 1] & 0xF) << 2];
		out += base64_pad;
	}
	out += end;
	return out;
}

SMTP::SMTP()
{
	memset(m_chBoundary,0,256);
	memset(&m_forwardinfo, 0, sizeof(ForwardSet));
	m_lsAttPath.clear();
	memset(m_chCurrPath, 0, MAX_PATH);
	m_lsReceiver.clear();
	memset(m_chPcName, 0, 128);
	memset(m_chLogPath, 0, MAX_PATH);
	memset(m_chSubBoundary, 0, 256);
}

SMTP::~SMTP()
{
	m_lsAttPath.clear();
	m_lsReceiver.clear();
}

long SMTP::Logon()
{
	SYSTEMTIME st = { 0 };
	long lValue(0),lCode(0);
	char chCommand[128] = { 0 }, chResult[256] = { 0 }, chDate[128] = { 0 };
	TCHAR szSrvadd[128] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, m_forwardinfo.srvadd, 64, szSrvadd, 128);
	lValue = m_SendSocket.InitSocket(szSrvadd, 25);
	if (lValue != 0)
	{
		m_log.Log(m_forwardinfo.srvadd, strlen(m_forwardinfo.srvadd));
		char chErr[128] = { 0 };
		GetErrorMessage(lValue, chErr, 128);
		m_log.Log(chErr,strlen(chErr));
		m_log.Log("InitSocket Error!", strlen("InitSocket Error!"));
#ifdef _DEBUG
		OutputDebugStringA("InitSocket Error!\r\n");
#endif
		return SOCKETINIT_ERROR;
	}
	
	if (0==::gethostname(m_chPcName,128))
		sprintf_s(chCommand, "HELO %s\r\n", m_chPcName);
	else
		sprintf_s(chCommand, "HELO %s\r\n","0.0.0.0");
	lValue = m_SendSocket.SendData(chCommand, strlen(chCommand));
	if (lValue < 0)
		return SEND_ERROR;
	m_SendSocket.ReceiveData(chResult, 256);
	StringProcess(chResult, lCode);
	if (lCode != 250)
	{
		m_log.Log(chResult, strlen(chResult));
		return LOGIN_ERROR;
	}

	memset(chResult, 0, 256);
	sprintf_s(chCommand, "AUTH LOGIN\r\n");
	lValue = m_SendSocket.SendData(chCommand, strlen(chCommand));
	if (lValue < 0)
		return SEND_ERROR;
	m_SendSocket.ReceiveData(chResult, 256);
	StringProcess(chResult, lCode);
	if (lCode != 334)
	{
		m_log.Log(chResult, strlen(chResult));
#ifdef _DEBUG
		OutputDebugStringA(chResult);
		OutputDebugStringA("\r\n");
#endif
		return LOGIN_ERROR;
	}

	char chUsername[128] = { 0 }, chPass[256] = { 0 }, chError[128] = {0};
	sprintf_s(chUsername, 128, "%s\r\n", base64_encode(m_forwardinfo.username).c_str());
	sprintf_s(chPass, 256, "%s\r\n", base64_encode(m_forwardinfo.pass).c_str());
	lValue = m_SendSocket.SendData(chUsername,strlen(chUsername));
	if (lValue < 0)
		return SEND_ERROR;
	memset(chResult, 0, 256);
	m_SendSocket.ReceiveData(chResult, 256);
	StringProcess(chResult, lCode);
	if (lCode != 334)
	{
		m_log.Log(chResult, strlen(chResult));
#ifdef _DEBUG
		OutputDebugStringA(chResult);
		OutputDebugStringA("\r\n");
#endif
		return LOGIN_ERROR;
	}
	lValue = m_SendSocket.SendData(chPass,strlen(chPass));
	if (lValue < 0)
	{
		int nErr = WSAGetLastError();
		GetErrorMessage(nErr, chError, 128);
		m_log.Log(chError, strlen(chError));
		m_log.Log("Send PASS Error!", strlen("Send PASS Error!"));
		return SEND_ERROR;
	}
	memset(chResult, 0, 256);
	m_SendSocket.ReceiveData(chResult, 256);
	StringProcess(chResult, lCode);
	if (lCode != 235)
	{
		m_log.Log(chResult, strlen(chResult));
#ifdef _DEBUG
		OutputDebugStringA(chResult);
		OutputDebugStringA("\r\n");
#endif
		return LOGIN_ERROR;
	}
	//使用计算机名称和当前日期编码base64作为boundary字段
	sprintf_s(chDate, 127, "%s, %2d %s %d %2d:%2d:%2d\r\n",
		week[st.wDayOfWeek], st.wDay, month[st.wMonth], st.wYear,
		st.wHour, st.wMinute, st.wSecond);
	sprintf_s(m_chBoundary, 256, "--0__=%s%s", base64_encode(m_chPcName).c_str(), base64_encode(chDate).c_str());
	return SUCCESS;
}

long SMTP::SendHead()
{
	long lValue(0), lCode;
	char chCommand[1024] = { 0 }, chResult[1024] = { 0 }, chSubject[128] = { 0 },
		chDate[128] = { 0 }, chReceiver[128] = { 0 }, chTimeZone[8] = {0};
	sprintf_s(chCommand, 1024, "MAIL FROM:<%s>\r\n",m_forwardinfo.from);
	lValue = m_SendSocket.SendData(chCommand, strlen(chCommand));
	if (lValue < 0)
		return SEND_ERROR;
	m_SendSocket.ReceiveData(chResult, 1024);
	StringProcess(chResult, lCode);
	//MAIL FROM的邮箱必须在本邮件系统可用，否则lCode会返回失败
	if (lCode == 250)
	{
		if (m_lsReceiver.size()>0)
		{
			list<string>::iterator ite = m_lsReceiver.begin();
			while (ite != m_lsReceiver.end())
			{
				sprintf_s(chReceiver, 127, "%s", (*ite).c_str());
#ifdef _DEBUG
				char chTo[128] = { 0 };
				sprintf_s(chTo, 128, "Send To:%s\r\n", chReceiver);
				OutputDebugStringA(chTo);
#endif
				sprintf_s(chCommand, 1024, "RCPT TO:<%s>\r\n", chReceiver);
				lValue = m_SendSocket.SendData(chCommand, strlen(chCommand));
				if (lValue < 0)
					return SEND_ERROR;
				memset(chResult, 0, 256);
				m_SendSocket.ReceiveData(chResult, 1024);
				StringProcess(chResult, lCode);
				if (lCode != 250)
				{
					m_log.Log(chResult, strlen(chResult));
					return LOGIN_ERROR;
				}
				ite++;
			}
			if (lCode == 250)
			{
				//开始发送邮件数据部分
				sprintf_s(chCommand, 1024, "DATA\r\n");
				lValue = m_SendSocket.SendData(chCommand, strlen(chCommand));
				if (lValue < 0)
					return SEND_ERROR;
				SYSTEMTIME st = { 0 };
				TIME_ZONE_INFORMATION TimeZoneInformation;
				GetLocalTime(&st);
				GetTimeZoneInformation(&TimeZoneInformation);
				int n = TimeZoneInformation.Bias / -60;
				if (n>0)
				{
					if (n > 9)
						sprintf_s(chTimeZone, 8, "+%d00", n);
					else sprintf_s(chTimeZone, 8, "+0%d00",n);
				}
				else
				{
					if (n < -9)
						sprintf_s(chTimeZone, 8, "-%d00", n);
					else sprintf_s(chTimeZone, 8, "-0%d00", n);
				}
				sprintf_s(chDate, 127, "Date: %s, %2d %s %d %2d:%2d:%2d %s\r\n",
					week[st.wDayOfWeek], st.wDay, month[st.wMonth], st.wYear,
					st.wHour, st.wMinute, st.wSecond,chTimeZone);
				lValue = m_SendSocket.SendData(chDate, strlen(chDate));
				if (lValue < 0)
				{
					m_log.Log(chDate, strlen(chDate));
					return SEND_ERROR;
				}

				sprintf_s(chCommand, 1024, "From: \"=?GB2312?B?%s?=\"<%s>\r\n", base64_encode(m_forwardinfo.username).c_str(), m_forwardinfo.username);
				lValue = m_SendSocket.SendData(chCommand, strlen(chCommand));
				if (lValue < 0)
					return SEND_ERROR;
				ite = m_lsReceiver.begin();
				memset(chCommand, 0, 1024);

				while (ite != m_lsReceiver.end())
				{
					if (ite == m_lsReceiver.begin())
						sprintf_s(chCommand, 1024, "To: <%s>", (*ite).c_str());
					else
					{
						strcat_s(chCommand, 1024, ",\r\n\t<");
						strcat_s(chCommand, 1024, (*ite).c_str());
						strcat_s(chCommand, 1024, ">");
					}
					ite++;
				}
				strcat_s(chCommand, 1024, "\r\n");
				lValue = m_SendSocket.SendData(chCommand, strlen(chCommand));
				if (lValue < 0)
					return SEND_ERROR;

				if (m_lsAttPath.size()>0)
				{
					list<string>::iterator ite = m_lsAttPath.begin();
					sprintf_s(chSubject, 127, "%s", (*ite).c_str());
				}
				else sprintf_s(chSubject, 127, "%s", chDate);
				sprintf_s(chCommand, 1024,
					"Subject: =?GB2312?b?%s?=\r\nMIME-Version: 1.0\r\nContent-type: multipart/mixed;\r\n\tboundary=\"%s\"\r\n\r\n",
					base64_encode(chSubject).c_str(), m_chBoundary);

				lValue = m_SendSocket.SendData(chCommand, strlen(chCommand));
				if (lValue < 0)
					return SEND_ERROR;
				memset(chResult, 0, 256);
				m_SendSocket.ReceiveData(chResult, 1024);
			}
			else
			{
				m_log.Log(chResult, strlen(chResult));
				return LOGIN_ERROR;
			}
		}
		else
		{
			m_log.Log("ReceiveList is empty!", strlen("ReceiveList is empty!"));
			return RETURN_FAIL;
		}
	}
	else
	{
		m_log.Log(chResult, strlen(chResult));
		return RETURN_FAIL;
	}
	return SUCCESS;
}

long SMTP::SendTextBody(const string& strtext)
{
	long lValue(0);
	char *pCommand=NULL;
	long lSize = strtext.length();
	if (lSize%3==0)
		lSize = lSize / 3 * 4 + 136;
	else lSize = (lSize / 3 + 1) * 4 + 136;
	pCommand = new char[lSize];
	memset(pCommand, 0, lSize);
	sprintf_s(pCommand, lSize,
		"--%s\r\nContent-transfer-encoding: base64\r\nContent-Type: text/plain; charset=\"gb2312\"\r\n\r\n%s\r\n\r\n",
		m_chBoundary, base64_encode(strtext).c_str());
	lValue = m_SendSocket.SendData(pCommand, strlen(pCommand));
	if (pCommand)
	{
		delete pCommand;
		pCommand = NULL;
	}
	if (lValue < 0)
		return SEND_ERROR;
	return SUCCESS;
}

long SMTP::SendFileBody()
{
	BOOL bRet = TRUE;
	FILE *pFile;
	long lValue(0),lLen(0),lPt(0),lSend(0),lSendCount(0);
	char chCommand[1024] = { 0 }, chResult[1024] = { 0 }, chFileName[128] = { 0 },
		chTextTemp[65536] = { 0 }, chPath[MAX_PATH] = { 0 }, chEnCode[65536] = {0};
	char*pText = NULL;
	string strEncode;
	list<string>::iterator ite = m_lsAttPath.begin();
	while (ite!=m_lsAttPath.end())
	{
		sprintf_s(chFileName, 127, "%s", (*ite).c_str());
		if (chFileName && chFileName[0]!='\0')
		{
			if (m_chCurrPath && m_chCurrPath[0] != '\0')
			{
				sprintf_s(chPath,MAX_PATH,"%s\\%s",m_chCurrPath,chFileName);
				if (fopen_s(&pFile, chPath, "r+b") == 0)//打开成功
				{
					fseek(pFile, 0, SEEK_END);
					lLen = ftell(pFile);
					fseek(pFile, 0, SEEK_SET);
					pText = new char[lLen + 1];
					memset(pText, 0, lLen + 1);
					fread_s(pText, lLen, lLen, 1, pFile);
					fclose(pFile);

					sprintf_s(chCommand, 1024,
						"--%s\r\nContent-Type: application/octet-stream;\r\n  name=\"=?GB2312?B?%s?=\"\r\nContent-Transfer-Encoding: base64\r\nContent-Disposition: attachment;\r\n  filename=\"=?GB2312?B?%s?=\"\r\n\r\n",
						m_chBoundary, base64_encode(chFileName).c_str(), base64_encode(chFileName).c_str());
					lValue = m_SendSocket.SendData(chCommand, strlen(chCommand));
					if (lValue < 0)
						return SEND_ERROR;
					//发送邮件附件内容
					while (lPt < lLen)
					{
						memset(chTextTemp, 0, 65536);
						memcpy_s(chTextTemp, 65535, &(pText[lPt]), min(lLen - lPt, 3072));
						sprintf_s(chEnCode, 65535, "%s", base64_encode(chTextTemp).c_str());
						lSend = m_SendSocket.SendData(chEnCode, strlen(chEnCode));
						if (lSend != strlen(chEnCode))
						{
							bRet = FALSE;
							break;
						}
						lSendCount += lSend;
						lPt += min(lLen - lPt, 3072);
#ifdef _DEBUG
						CString csDebug;
						csDebug.Format(_T("Bytes send: %d----EnSend: %d----SendCount: %d----Total:%d\n"), strlen(chTextTemp), lSend,lSendCount,lLen);
						OutputDebugString(csDebug);
						//m_log.Log(csDebug, csDebug.GetLength());
#endif
					}
					sprintf_s(chCommand, 1024, "\r\n");
					m_SendSocket.SendData(chCommand, strlen(chCommand));
					lSendCount = 0;
				}
				
				if (pText)
				{
					delete pText;
					pText = NULL;
				}
			}
			else
			{
				m_log.Log("CurrentPath Error!", strlen("CurrentPath Error!"));
			}
		}
		else
		{
			m_log.Log("No FileName!", strlen("No FileName!"));
		}
		lPt = 0;
		lSend = 0;
		lSendCount = 0;
		ite++;
	}
	if (bRet)
		return SUCCESS;
	else return RETURN_FAIL;
}

BOOL SMTP::Quit()
{
	long lValue(0);
	char chCommand[1024] = { 0 }, chResult[1024] = { 0 },chSendInfo[MAX_PATH] = { 0 };
	sprintf_s(chCommand, "--%s--\r\n.\r\n",m_chBoundary);
	m_SendSocket.SendData(chCommand, 1024);
	sprintf_s(chCommand, "QUIT");
	m_SendSocket.SendData(chCommand, 1024);
	m_SendSocket.ReceiveData(chResult,1024);
	list<string>::iterator ite = m_lsAttPath.begin();
	while (ite!=m_lsAttPath.end())
	{
		sprintf_s(chSendInfo, MAX_PATH, "Send [%s] Complete!", (*ite).c_str());
#ifdef _DEBUG
		m_log.Log(chSendInfo, strlen(chSendInfo));
		OutputDebugStringA(chSendInfo);
		OutputDebugStringA("\r\n");
#endif
		ite++;
	}
	return TRUE;
}

void SMTP::SetForwardInfo(ForwardSet& info)
{
	sprintf_s(m_forwardinfo.srvadd, 64, "%s", info.srvadd);
	sprintf_s(m_forwardinfo.username, 64, "%s", info.username);
	sprintf_s(m_forwardinfo.pass, 128, "%s", info.pass);
	sprintf_s(m_forwardinfo.to, 512, "%s", info.to);
	sprintf_s(m_forwardinfo.from, 128, "%s", info.from);
}

void SMTP::SetReceiver(const char*pStr)
{
	BOOL bFull = FALSE;
	string strReceiver(pStr),strTemp;
	int nStart(0);
	if (strReceiver.length() == 0)
	{
		strReceiver.clear();
		strReceiver = m_forwardinfo.to;
	}
	auto pos = strReceiver.find(";",nStart);
	while ((pos>0)&&(pos!=strReceiver.npos))
	{
		strTemp = strReceiver.substr(nStart, pos - nStart);
		nStart = pos + 1;
		if (strTemp.length() > 0)
		{
			m_lsReceiver.push_back(strTemp);
			if (m_lsReceiver.size() > 10)
			{
				bFull = TRUE;
				break;
			}
		}
		pos = strReceiver.find(";", nStart);
	}
	if (!bFull)
	{
		strTemp = strReceiver.substr(nStart);
		if (strTemp.length() > 0)
			m_lsReceiver.push_back(strTemp);
	}
}

void SMTP::AddAttachFileName(const string& strName, long lType)
{
	if (strName.length() < 0)
		return;
	if (lType == 0)
	{
		string strEmlName(strName);
		strEmlName.append(".eml");
		m_lsAttPath.push_back(strEmlName);
	}
	else
		m_lsAttPath.push_back(strName);
}

void SMTP::SetLogPath(const char*pPath)
{
	if (pPath && pPath[0] != '\0')
	{
		m_log.SetPath(pPath);
	}
}

BOOL SMTP::DeleteEMLFile()
{
	char chPath[MAX_PATH] = { 0 };
	long lSize = m_lsAttPath.size(),lCount(0);
	list<string>::iterator ite = m_lsAttPath.begin();
	while (ite!=m_lsAttPath.end())
	{
		sprintf_s(chPath, MAX_PATH, "%s\\%s", m_chCurrPath, (*ite).c_str());
		if (0 == remove(chPath))
			lCount++;
		ite++;
	}
	m_lsAttPath.clear();
	if (lCount == lSize)
		return TRUE;
	else return FALSE;
}

void SMTP::InitSMTPPro()
{
	m_lsReceiver.clear();
	m_lsAttPath.clear();
	memset(m_chBoundary, 0, 256);
	memset(m_chSubBoundary, 0, 256);
	memset(m_chCurrPath, 0, MAX_PATH); 
	memset(m_chPcName, 0, 128);
}