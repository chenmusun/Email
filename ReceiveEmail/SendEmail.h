#pragma once
#include <vector>
#include <list>
using namespace std;
#include "Socket.h"
#include "../Log/Log.h"


class SMTP
{
public:
	SMTP();
	~SMTP();
	MailSocket m_SendSocket;
	long Logon();//��¼
	long SendHead();//�����ʼ�ͷ��Ϣ
	long SendTextBody(const string& strtext);//�������Ĳ���
	long SendFileBody();//�����ʼ�����
	BOOL Quit();//�˳�
	void SetForwardInfo(ForwardSet& info);//����ת����Ϣ
	void SetReceiver(const char*pStr);//�����ʼ�������
	void AddAttachFileName(const string& strName,long lType=0);//��Ӹ���,0Ĭ��eml�ļ�
	inline void SetCurrPath(const char*pPath){if(pPath) sprintf_s(m_chCurrPath, MAX_PATH, "%s", pPath); }//���õ�ǰ��ȡ·��
	void SetLogPath(const char*pPath);//����Log��־·��
	BOOL DeleteEMLFile();
	void InitSMTPPro();
private:
	char m_chBoundary[256];//boundary�ֶ�
	char m_chSubBoundary[256];
	ForwardSet m_forwardinfo;//ת����Ϣ
	list<string> m_lsAttPath;//����·��
	char m_chCurrPath[MAX_PATH];//��ǰ·��
	list<string> m_lsReceiver;//�������б����ʮ��
	char m_chPcName[128];//���������
	CLog m_log;
	char m_chLogPath[MAX_PATH];//Log·��
};

std::string base64_encode(const string& src);

