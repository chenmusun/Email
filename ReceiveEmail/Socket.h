#pragma once
class MailSocket
{
public:
	MailSocket();
	~MailSocket();
public:
	BOOL CloseMySocket();//关闭套接字
	long InitSocket(LPCTSTR lpAddr, UINT nHostPort);//初始化套接字连接(阻塞)
	long SendData(const void* Buf, size_t lBufLen);//发送数据
	long ReceiveData(void* Buf, size_t lBufLen);//接收数据
	u_long GetIOCtl();
private:
	SOCKET m_MySocket;
};

BOOL StringProcess(const char * const pStr, string& strData1, string& strData2);//返回字符串处理(按照空格将返回字符串拆分)
BOOL StringProcess(const char * const pStr);//返回字符串处理(只处理OK、ERROR)
BOOL StringProcess(const char* const pStr, long& lValue);