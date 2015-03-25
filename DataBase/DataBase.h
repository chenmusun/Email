// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 DATABASE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// DATABASE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef DATABASE_EXPORTS
#define DATABASE_API __declspec(dllexport)
#else
#define DATABASE_API __declspec(dllimport)
#pragma	comment(lib,"../output/Database.lib")
#endif
#include <string>
using namespace std;
#include <winsock2.h>
#include "mongo/client/dbclient.h"
#include "mongo/bson/bson.h"
using namespace mongo;
#include "../ReceiveEmail/public.h"

//#include <windows.h>
// 此类是从 DataBase.dll 导出的
class DATABASE_API CDataBase {
public:
	CDataBase(void);
	~CDataBase();
	// TODO:  在此添加您的方法。
public:
	BOOL ConnectDataBase(string& strErr);//连接数据库（格式：数据库IP地址:端口），连接之前需要设定表明、用户名、密码
	long ExecSQL(const string& strUIDL, string& strErr,const string& strName);//检查UIDL是否存在，若不存在插入一条新数据
	void DisConnectDataBase();//断开数据库连接（目前使用MongoAPI中的Logout）
	void SetDBInfo(const MongoDBInfo&dbinfo);//设置数据库相关信息
	inline string GetDBName(){ string strName(m_dbinfo.chDBName); return strName; }
	void GetCurrTime(string& strDate);
	long long GetTimeStamp();
	inline int GetUseDB(){ return m_dbinfo.nUseDB; }
	long SaveFileToMongoDB(string& remotename,string& strPath,string& strRtr);
	BOOL DelUIDL(const string& strUIDL,const string& strName);
private:
	DBClientConnection connect;//Mongo连接实例
	MongoDBInfo m_dbinfo;
	//GridFS m_gridfs;
private:
protected:
};

extern DATABASE_API int nDataBase;

DATABASE_API int fnDataBase(void);
