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
	long Logon();//登录
	long SendHead();//发送邮件头信息
	long SendTextBody();//发送正文部分
	long SendFileBody();//发送邮件附件
	BOOL Quit();//退出
	void SetForwardInfo(ForwardSet& info);//设置转发信息
	void SetReceiver(const char*pStr);//设置邮件接收人
	void AddAttachFileName(const string& strName,long lType=0);//添加附件,0默认eml文件
	inline void SetCurrPath(const char*pPath){if(pPath) sprintf_s(m_chCurrPath, MAX_PATH, "%s", pPath); }//设置当前读取路径
	void SetLogPath(const char*pPath);//设置Log日志路径
	BOOL DeleteEMLFile();
	void InitSMTPPro();
private:
	char m_chBoundary[256];//boundary字段
	char m_chSubBoundary[256];
	ForwardSet m_forwardinfo;//转发信息
	list<string> m_lsAttPath;//附件路径
	char m_chCurrPath[MAX_PATH];//当前路径
	list<string> m_lsReceiver;//接收人列表，最大十个
	char m_chPcName[128];//计算机名称
	CLog m_log;
	char m_chLogPath[MAX_PATH];//Log路径
};

std::string base64_encode(const string& src);

