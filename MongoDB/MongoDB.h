// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 MONGODB_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// MONGODB_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
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

// 此类是从 MongoDB.dll 导出的
class MONGODB_API CMongoDB {
public:
	CMongoDB(void);
	~CMongoDB();
	// TODO:  在此添加您的方法。
public:
	BOOL ConnectDataBase(string& strErr);//连接数据库（格式：数据库IP地址:端口），连接之前需要设定表明、用户名、密码
	long CheckUIDLInMongoDB(const string& strUIDL, string& strErr, const string& strName, long lDay = 14);//检查UIDL是否存在，若不存在插入一条新数据
	void DisConnectDataBase();//断开数据库连接（目前使用MongoAPI中的Logout）
	void SetDBInfo(const MongoDBInfo&dbinfo);//设置数据库相关信息
	long SaveFileToMongoDB(string& remotename, string& strPath, string& strRtr);
	BOOL DelUIDL(const string& strUIDL, const string& strName);
	BOOL GetFileFromMongoDB(const string& strFileName, const string&strSavePath, string& strErr);
	BOOL DelUIDL(const string& strUIDL);
private:
	DBClientConnection connect;//Mongo连接实例
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
