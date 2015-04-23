#pragma once
#include "../DataBase/DataBase.h"
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
	long Login(LPCTSTR lpServer,long lPort,LPCTSTR lpUser,LPCTSTR lpPasswd);//��½POP3������
	long GetMailCount();//��ȡ�ʼ�����
	BOOL Close();//�Ƴ��ʼ�������
	string GetUIDL(long lCurrPos);//��ȡ�ʼ�Ψһ���
	long CheckUIDL(const string& strUIDL,const string& strName,long lSaveDay=14);//���UIDL�Ƿ��Ѵ���MongoDB�������ڲ����н�������
	long GetEMLFile(long lCurrPos,const string& strUIDL);//��ȡ�ʼ��ļ�
	//�����ʼ�������Ϣ
	void SetInfo(CString csName, const MailBoxInfo& info, const MongoDBInfo& dbinfo,LPCTSTR lpPath, long lLen);
	void QuitDataBase();//�˳����ݿ�
	BOOL ConnectDataBase();
	long DelEmail(long lCurrPos,const string& strUIDL);
	inline char* GetCurrPath(){ return m_CurrPath; }
	void SetLogPath(const char*pPath);
	inline void SetParent(void* pParent){ if (pParent) m_pParent = pParent; }
	BOOL SaveFileToDB(EMAIL_ITEM& email);
	inline BOOL GetStatus(){ return m_bFailed; }
	BOOL DeleteFromDB(EMAIL_ITEM& email);
private:
	CDataBase m_db;//���ݿ����ʵ��
	MailSocket m_Socket;//POP3�׽���
	char m_CurrPath[MAX_PATH];//��ǰ·��
	char m_chLogPath[MAX_PATH];
	char m_chName[64];//��������
	MailBoxInfo m_Info;//������Ϣ
	BOOL m_bConnect;
	CLog m_log;
	void *m_pParent;
	BOOL m_bFailed;
protected:
};
