#pragma once
class MailSocket
{
public:
	MailSocket();
	~MailSocket();
public:
	BOOL CloseMySocket();//�ر��׽���
	long InitSocket(LPCTSTR lpAddr, UINT nHostPort);//��ʼ���׽�������(����)
	long SendData(const void* Buf, size_t lBufLen);//��������
	long ReceiveData(void* Buf, size_t lBufLen);//��������
	u_long GetIOCtl();
private:
	SOCKET m_MySocket;
};

BOOL StringProcess(const char * const pStr, string& strData1, string& strData2);//�����ַ�������(���տո񽫷����ַ������)
BOOL StringProcess(const char * const pStr);//�����ַ�������(ֻ����OK��ERROR)
BOOL StringProcess(const char* const pStr, long& lValue);
BOOL StringProcess(const string& strSrc, string& strData1, string& strData2);
BOOL StringProcess(const string& strSrc, long& lValue, string& strData);