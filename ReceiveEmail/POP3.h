#pragma once
#include "../ReceiveEmail/public.h"
#include "../MailAnalysis/public.h"
#include "Socket.h"
#include "../Log/Log.h"

class POP3
{
public:
	POP3();
	~POP3();
private:
protected:
public:
	long Login(LPCTSTR lpServer,long lPort,LPCTSTR lpUser,LPCTSTR lpPasswd);//登陆POP3服务器
	long GetMailCount();//获取邮件总数
	BOOL Close();//推出邮件服务器
	string GetUIDL(long lCurrPos);//获取邮件唯一编号
	//long CheckUIDL(const string& strUIDL,const string& strName,long lSaveDay=14);//检测UIDL是否已存在MongoDB，若存在不进行接收跳过
	long GetEMLFile(long lCurrPos,const string& strUIDL);//获取邮件文件
	//设置邮件接收信息
	void SetInfo(CString csName, const MailBoxInfo& info,LPCTSTR lpPath, long lLen);
	long DelEmail(long lCurrPos,const string& strUIDL);
	inline char* GetCurrPath(){ return m_CurrPath; }
	void SetLogPath(const char*pPath);
	//BOOL SaveFileToDB(EMAIL_ITEM& email);
	inline BOOL GetStatus(){ return m_bFailed; }
	//BOOL DeleteFromDB(EMAIL_ITEM& email);
private:
	MailSocket m_Socket;//POP3套接字
	char m_CurrPath[MAX_PATH];//当前路径
	char m_chLogPath[MAX_PATH];
	char m_chName[64];//中文名称
	MailBoxInfo m_Info;//邮箱信息
	BOOL m_bConnect;
	CLog m_log;
	BOOL m_bFailed;
protected:
};
