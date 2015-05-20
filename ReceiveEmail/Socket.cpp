#include "stdafx.h"
#include "Socket.h"
#include <winsock2.h>
#include <Ws2tcpip.h>
#include "../ReceiveEmail/public.h"

//#define _WINSOCK_DEPRECATED_NO_WARNINGS
MailSocket::MailSocket() :m_MySocket(INVALID_SOCKET)
{}

MailSocket::~MailSocket()
{
	if (m_MySocket != INVALID_SOCKET)
	{
		closesocket(m_MySocket);
		m_MySocket = INVALID_SOCKET;
	}
}

BOOL MailSocket::CloseMySocket()
{
	if (m_MySocket != INVALID_SOCKET)
	{
		closesocket(m_MySocket);
		m_MySocket = INVALID_SOCKET;
	}
	return TRUE;
}

long MailSocket::InitSocket(LPCTSTR lpAddr, UINT nHostPort)
{
	char chSrvAdd[64] = { 0 }, chResult[256] = { 0 }, chPort[32] = {0};
	int iResult(0), i(0);
	long noDelay(1);
	m_MySocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_MySocket == INVALID_SOCKET)
	{
		return SOCKETINIT_ERROR;
	}
	if (::setsockopt(m_MySocket, IPPROTO_TCP, TCP_NODELAY, (LPSTR)&noDelay, sizeof(long)) == SOCKETINIT_ERROR)
	{
		return SOCKETINIT_ERROR;
	}
	int TimeOut = 3000; //设置发送超时3秒
	if (::setsockopt(m_MySocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&TimeOut, sizeof(TimeOut)) == SOCKET_ERROR)
	{
		return SOCKETINIT_ERROR;
	}
	//设置接收超时3秒
	if (::setsockopt(m_MySocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&TimeOut, sizeof(TimeOut)) == SOCKET_ERROR)
	{
		return SOCKETINIT_ERROR;
	}
	WideCharToMultiByte(CP_ACP, 0, lpAddr, 64, chSrvAdd, 64, NULL, NULL);
		
	DWORD dwRetval(0);
	struct sockaddr_in  addr_sev;
	memset(&addr_sev, 0, sizeof(sockaddr_in));
	inet_pton(AF_INET, chSrvAdd, (PVOID)addr_sev.sin_addr.s_addr);
	BOOL bFound = FALSE;
	sprintf_s(chPort, 32, "%d", nHostPort);
	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	dwRetval = getaddrinfo(chSrvAdd, chPort, &hints, &result);
	if (dwRetval != 0)
	{
		return GETADDRINFO_ERROR;
	}
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		switch (ptr->ai_family)
		{
		case AF_INET:
			memcpy_s(&addr_sev, sizeof(sockaddr_in), (struct sockaddr_in *) ptr->ai_addr, sizeof(sockaddr_in));
#ifdef _DEBUG
			sprintf_s(chResult, 256, "\tIP Address #%d: %s\n", i, inet_ntoa(addr_sev.sin_addr));
#endif
			bFound = TRUE;
			break;
		default:
			break;
		}
	}
	if (dwRetval != 0 || !bFound)
	{
		freeaddrinfo(result);
		return NO_INTERNET;
	}
	freeaddrinfo(result);

	u_long iMode = 1;
	iResult = ::ioctlsocket(m_MySocket, FIONBIO, &iMode);//设置成非阻塞模式
	if (iResult != NO_ERROR)
		return SOCKETINIT_ERROR;
	iResult = ::connect(m_MySocket, (SOCKADDR*)&addr_sev, sizeof(addr_sev));

	timeval timeout;
	memset(&timeout, 0, sizeof(timeout));
	timeout.tv_sec = 5;

	fd_set set;
	FD_ZERO(&set);
	FD_SET(m_MySocket, &set);
	iResult = ::select(0, &set, NULL, NULL, &timeout);
	if (iResult == 0)
	{
		::closesocket(m_MySocket);
		m_MySocket = INVALID_SOCKET;
		return HOST_UNREACHABLE;
	}
	else
	{
		iMode = 0;
		iResult = ::ioctlsocket(m_MySocket, FIONBIO, &iMode);//设置成阻塞模式
		if (iResult != NO_ERROR)
		{
			::closesocket(m_MySocket);
			m_MySocket = INVALID_SOCKET;
			return SOCKETINIT_ERROR;
		}
	}

	if (iResult == SOCKET_ERROR)
	{
		::closesocket(m_MySocket);
		m_MySocket = INVALID_SOCKET;
		return SOCKETINIT_ERROR;
	}
	memset(chResult, 0, 256);
	ReceiveData(chResult, 256);
	switch (nHostPort)
	{
	case 110:
	{
				if (!StringProcess(chResult))
					return RETURN_FAIL;
	}
		break;
	case 25:
	{
			   long lVaule(0);
			   StringProcess(chResult, lVaule);
			   if (lVaule != 220)
				   return RETURN_FAIL;
	}
		break;
	default:
		break;
	}
	return SUCCESS;
}

long MailSocket::SendData(const void *Buf, size_t lBufLen)
{
	int nSendSize(0);
	if (Buf && lBufLen>0)
	{
		nSendSize = ::send(m_MySocket, (char*)Buf, (int)lBufLen, 0);
		if (nSendSize < 0)
		{
			int nErr = WSAGetLastError();
#ifdef _DEBUG
			char chDebug[64] = { 0 };
			sprintf_s(chDebug, 64, "Send LastError = %d\r\n", nErr);
			OutputDebugStringA(chDebug);
#endif
		}
	}
//#ifdef _DEBUG
//	OutputDebugStringA((char*)Buf);
//#endif
	return nSendSize;
}

long MailSocket::ReceiveData(void * Buf, size_t lBufLen)
{
	int nRecvSize(0);
	if (Buf&& lBufLen>0)
	{
		nRecvSize = ::recv(m_MySocket, (char*)Buf, (int)lBufLen, 0);
		if (nRecvSize < 0)
		{
			int nErr = WSAGetLastError();
#ifdef _DEBUG
			char chDebug[64] = { 0 };
			sprintf_s(chDebug, 64, "Recv LastError = %d\r\n", nErr);
			OutputDebugStringA(chDebug);
#endif
		}
	}
//#ifdef _DEBUG
//	OutputDebugStringA((char*)Buf);
//#endif
	return nRecvSize;
}

u_long MailSocket::GetIOCtl()
{
	u_long lSize(0);
	if (m_MySocket != INVALID_SOCKET)
		::ioctlsocket(m_MySocket, FIONREAD, &lSize);
	return lSize;
}
/////////////////////////////////////////////////////////////////////
BOOL StringProcess(const char * const pStr, string& strData1, string& strData2)
{
	string strSrc, strTemp;
	strData1.empty();
	strData2.empty();
	if (pStr && *pStr != '\0')
	{
		strSrc = pStr;
		do
		{
			auto pos = strSrc.find("+OK");
			if (pos < 0) break;
			if (pos == strSrc.npos) break;
			pos = strSrc.find_first_of(" ", pos + 1);
			if (pos >= 0 && pos != strSrc.npos)
			{
				auto pos1 = strSrc.find_first_of(" ", pos + 1);
				if (pos1>pos && pos1 != strSrc.npos)
				{
					strData1 = strSrc.substr(pos + 1, pos1 - pos - 1);
					strData2 = strSrc.substr(pos1 + 1, strSrc.length() - pos1);
					auto pos2 = strData2.find("\r\n");
					if (pos2 >= 0 && pos2 != strData2.npos)
						strData2.replace(pos2, 4, "");
				}
				else
				{
					strData1 = strSrc.substr(pos, strSrc.length() - pos);
				}
			}
			return TRUE;
		} while (0);
	}
	return FALSE;
}

BOOL StringProcess(const char * const pStr)
{
	if (pStr && *pStr != '\0')
	{
		string strSrc(pStr), strTemp;
		auto pos = strSrc.find("+OK");
		if (pos >= 0 && pos != strSrc.npos)
			return TRUE;
	}
	return FALSE;
}

BOOL StringProcess(const char* const pStr, long& lValue)
{
	if (pStr && *pStr != '\0')
	{
		CString csSrc(pStr), csTemp;
		auto pos = csSrc.Find(_T(" "));
		if (pos > 0)
		{
			csTemp = csSrc.Left(pos);
			lValue = _ttol(csTemp);
		}
		else lValue = -1;
	}
	return TRUE;
}

BOOL StringProcess(const string& strSrc, string& strData1, string& strData2)
{
	string strTemp;
	strData1.empty();
	strData2.empty();
	if (strSrc.length()>0)
	{
		do
		{
			auto pos = strSrc.find("+OK");
			if (pos < 0) break;
			if (pos == strSrc.npos) break;
			pos = strSrc.find_first_of(" ", pos + 1);
			if (pos >= 0 && pos != strSrc.npos)
			{
				auto pos1 = strSrc.find_first_of(" ", pos + 1);
				if (pos1>pos && pos1 != strSrc.npos)
				{
					strData1 = strSrc.substr(pos + 1, pos1 - pos - 1);
					strData2 = strSrc.substr(pos1 + 1, strSrc.length() - pos1);
					auto pos2 = strData2.find("\r\n");
					if (pos2 >= 0 && pos2 != strData2.npos)
						strData2.replace(pos2, 4, "");
				}
				else
				{
					strData1 = strSrc.substr(pos, strSrc.length() - pos);
				}
			}
			return TRUE;
		} while (0);
	}
	return FALSE;
}

BOOL StringProcess(const string& strSrc, long& lValye, string& strData)
{
	auto pos = strSrc.find(" ");
	string strTemp;
	strTemp = strSrc.substr(0, pos);
	if (strTemp.length() > 0)
		lValye = atoi(strTemp.c_str());
	strData = strSrc.substr(pos + 1);
	pos = strData.find(":");
	while (pos != strData.npos)
	{
		strData.replace(pos, 1, "");
		pos = strData.find(":", pos);
	}
	if (strData.length() > 0 && lValye > 0)
		return TRUE;
	else
	{
		lValye = 0;
		strData.clear();
	}
	return FALSE;
}