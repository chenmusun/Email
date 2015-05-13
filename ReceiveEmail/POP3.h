#pragma once
#include "../ReceiveEmail/public.h"
#include "../MailAnalysis/public.h"
#include "Socket.h"
#include "../Log/Log.h"
#include "../DataBase/DataBase.h"

class POP3
{
public:
	POP3();
	~POP3();
private:
protected:
public:
	long Login(LPCTSTR lpServer,long lPort,LPCTSTR lpUser,LPCTSTR lpPasswd);//��½POP3������
	long GetMailCount();//��ȡ�ʼ�����
	BOOL Close();//�Ƴ��ʼ�������
	string GetUIDL(long lCurrPos);//��ȡ�ʼ�Ψһ���
	long GetEMLFile(long lCurrPos,const string& strUIDL);//��ȡ�ʼ��ļ�
	//�����ʼ�������Ϣ
	void SetInfo(CString csName, const MailBoxInfo& info,LPCTSTR lpPath, long lLen);
	long DelEmail(long lCurrPos,const string& strUIDL);
	inline char* GetCurrPath(){ return m_CurrPath; }
	void SetLogPath(const char*pPath);
	inline BOOL GetStatus(){ return m_bFailed; }
	long CheckUIDL(const string& strUIDL,const string& strName,long lSaveDay=14);//���UIDL�Ƿ��Ѵ���MongoDB�������ڲ����н�������
	BOOL SaveFileToDB(EMAIL_ITEM& email);
	BOOL DeleteFromDB(EMAIL_ITEM& email);
	BOOL ConnectDataBase();
	void POP3::QuitDataBase();
	void SetDBinfo(const MongoDBInfo& dbinfo);
	BOOL DeleteFromDB(const string& strUIDL);
private:
	MailSocket m_Socket;//POP3�׽���
	char m_CurrPath[MAX_PATH];//��ǰ·��
	char m_chLogPath[MAX_PATH];
	char m_chName[64];//��������
	MailBoxInfo m_Info;//������Ϣ
	BOOL m_bConnect;
	CLog m_log;
	BOOL m_bFailed;
	CDataBase m_db;
protected:
};
