// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� DATABASE_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// DATABASE_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#pragma once
#ifdef DATABASE_EXPORTS
#define DATABASE_API __declspec(dllexport)
#else
#define DATABASE_API __declspec(dllimport)
#pragma	comment(lib,"../output/Database.lib")
#endif
#include <string>
using namespace std;
#include <winsock2.h>
#include <cstdlib>
#include "mongo/client/dbclient.h"
#include "mongo/bson/bson.h"
using namespace mongo;
#include "../ReceiveEmail/public.h"

//#include <windows.h>
// �����Ǵ� DataBase.dll ������
class DATABASE_API CDataBase {
public:
	CDataBase(void);
	virtual ~CDataBase();
	// TODO:  �ڴ�������ķ�����
public:
	BOOL ConnectDataBase(string& strErr);//�������ݿ⣨��ʽ�����ݿ�IP��ַ:�˿ڣ�������֮ǰ��Ҫ�趨�������û���������
	long CheckUIDLInMongoDB(const string& strUIDL, string& strErr,const string& strName,long lDay=14);//���UIDL�Ƿ���ڣ��������ڲ���һ��������
	void DisConnectDataBase();//�Ͽ����ݿ����ӣ�Ŀǰʹ��MongoAPI�е�Logout��
	void SetDBInfo(const MongoDBInfo&dbinfo);//�������ݿ������Ϣ
	inline string GetDBName(){ string strName(m_dbinfo.chDBName); return strName; }
	void GetCurrTime(string& strDate);
	long long GetTimeStamp();
	inline int GetUseDB(){ return m_dbinfo.nUseDB; }
	long SaveFileToMongoDB(string& remotename,string& strPath,string& strRtr);
	BOOL DelUIDL(const string& strUIDL,const string& strName);
	BOOL GetFileFromMongoDB(const string& strFileName,const string&strSavePath,string& strErr);
private:
	DBClientConnection connect;//Mongo����ʵ��
	MongoDBInfo m_dbinfo;
	BOOL m_bConnect;
private:
protected:
};

extern DATABASE_API int nDataBase;

DATABASE_API int fnDataBase(void);
