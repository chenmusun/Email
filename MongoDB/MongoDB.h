// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� MONGODB_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// MONGODB_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#pragma once
#ifdef MONGODB_EXPORTS
#define MONGODB_API __declspec(dllexport)
#else
#define MONGODB_API __declspec(dllimport)
#pragma	comment(lib,"../output/MongoDB.lib")
#endif
#include <string>
using namespace std;
#include <winsock2.h>
#include <cstdlib>
#include "mongo/client/dbclient.h"
#include "mongo/bson/bson.h"
using namespace mongo;
#include "../ReceiveEmail/public.h"

// �����Ǵ� MongoDB.dll ������
class MONGODB_API CMongoDB {
public:
	CMongoDB(void);
	virtual ~CMongoDB();
	// TODO:  �ڴ�������ķ�����
public:
	BOOL ConnectDataBase(string& strErr);//�������ݿ⣨��ʽ�����ݿ�IP��ַ:�˿ڣ�������֮ǰ��Ҫ�趨�������û���������
	long CheckUIDLInMongoDB(const string& strUIDL, string& strErr, const string& strName, long lDay = 14);//���UIDL�Ƿ���ڣ��������ڲ���һ��������
	void DisConnectDataBase();//�Ͽ����ݿ����ӣ�Ŀǰʹ��MongoAPI�е�Logout��
	void SetDBInfo(const MongoDBInfo&dbinfo);//�������ݿ������Ϣ
	inline string GetDBName(){ return m_strDBName; }
	inline int GetUseDB(){ return m_nUseDB; }
	long SaveFileToMongoDB(string& remotename, string& strPath, string& strRtr);
	BOOL DelUIDL(const string& strUIDL, const string& strName);
	BOOL GetFileFromMongoDB(const string& strFileName, const string&strSavePath, string& strErr);
	BOOL DelUIDL(const string& strUIDL);
private:
	DBClientConnection connect;//Mongo����ʵ��
	BOOL m_bConnect;
	string m_strDBAdd;
	string m_strDBName;
	string m_strTable;
	string m_strUserName;
	string m_strPasswd;
	int m_nUseDB;
private:
	void GetCurrTime(string& strDate);
	long long GetTimeStamp();
protected:
};

extern MONGODB_API int nMongoDB;

MONGODB_API int fnMongoDB(void);
