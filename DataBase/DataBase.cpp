// DataBase.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "DataBase.h"

static long lCount = 1;
// 这是导出变量的一个示例
DATABASE_API int nDataBase = 0;

// 这是导出函数的一个示例。
DATABASE_API int fnDataBase(void)
{
	return 42;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 DataBase.h
CDataBase::CDataBase(void)
{
	memset(&m_dbinfo, 0, sizeof(MongoDBInfo));
	return;
}

CDataBase::~CDataBase()
{

}

BOOL CDataBase::ConnectDataBase(string& strErr)
{
	string strWhat;
	strErr.empty();//将错误信息清空
	if (m_dbinfo.nUseDB != 1)
		return TRUE;
	if (m_dbinfo.chDBAdd && m_dbinfo.chDBAdd[0] != '\0')
	{
		try
		{
			mongo::client::initialize();
			if (connect.connect(m_dbinfo.chDBAdd, strErr))
			{
				connect.setSoTimeout(1000);
				if (!connect.auth(m_dbinfo.chDBName, m_dbinfo.chUserName, m_dbinfo.chPasswd, strErr))//登陆数据库验证
				{
#ifdef _DEBUG
					OutputDebugStringA(strErr.c_str());
					OutputDebugStringA("\r\n");
#endif
					return FALSE;
				}
			}
			else
			{
#ifdef _DEBUG
				OutputDebugStringA(strErr.c_str());
				OutputDebugStringA("\r\n");
#endif
				return FALSE;
			}
		}
		catch (const mongo::DBException &e)
		{

			strWhat = e.what();
			strWhat.append("\r\n");
			strErr.append(strWhat);
#ifdef _DEBUG
			OutputDebugStringA(strErr.c_str());
#endif
			return FALSE;
		}
	}
	return TRUE;
}

long CDataBase::CheckUIDLInMongoDB(const string& strUIDL, string& strErr, const string& strName, long lDay)
{
	//COleDateTime date;
	strErr.empty();
	long lFound = MONGO_FOUND;
	if (m_dbinfo.nUseDB != 1)
	{
		lFound = MONGO_NOT_FOUND;
		return lFound;
	}
	string strIndexName(m_dbinfo.chDBName);
	strIndexName.append(".");
	strIndexName.append(m_dbinfo.chTable);
	if (connect.isStillConnected())
	{
		BSONObj cmd, obj = BSON("UIDL" << strUIDL << "DATE" << DATENOW<<"TO"<<strName)
			, bsoReturnValue, bsoQuery = BSON("UIDL" << strUIDL), bsoValue;
		try
		{
			bsoReturnValue = connect.findOne(strIndexName, bsoQuery, &bsoValue);
			if (bsoReturnValue.isEmpty())
			{
#ifdef _DEBUG
				char chDebugInfo[256] = { 0 };
				sprintf_s(chDebugInfo, 256, "InsertCount = %d\r\n", lCount++);
				OutputDebugStringA(chDebugInfo);
#endif
				connect.insert(strIndexName, obj);
				lFound = MONGO_NOT_FOUND;
			}
			else
			{
				string strName("DATE"), strValue;
				strValue = bsoReturnValue.toString();
				mongo::BSONElement obj = bsoReturnValue.getField("DATE");
				mongo::Date_t date = obj.Date();
				long long lSrvDate = date.asInt64(), lLocalDate(0), lSaveDay(15);
				lSrvDate = lSrvDate / 1000;
				lLocalDate = GetTimeStamp();
				lSaveDay = (lLocalDate - lSrvDate) / 86400;
				if (lSaveDay>=lDay)
					lFound = MONGO_DELETE;
			}

		}
		catch (const mongo::DBException &e)
		{
			strErr = e.what();
		}
	}
	return lFound;
}


void CDataBase::DisConnectDataBase()
{
	BSONObj info;
	if (m_dbinfo.nUseDB != 1) return;
	if (connect.isStillConnected())
	{
		connect.logout(m_dbinfo.chDBName, info);
	}
}

void CDataBase::SetDBInfo(const MongoDBInfo&dbinfo)
{
	memset(&m_dbinfo, 0, sizeof(MongoDBInfo));
	memcpy_s(&m_dbinfo, sizeof(MongoDBInfo), &dbinfo, sizeof(MongoDBInfo));
}

void CDataBase::GetCurrTime(string& strDate)
{
	char chDate[128] = { 0 };
	SYSTEMTIME systm;
	GetLocalTime(&systm);
	sprintf_s(chDate, 128, "%4d-%.2d-%.2d %.2d:%.2d:%.2d",
		systm.wYear, systm.wMonth, systm.wDay,
		systm.wHour, systm.wMinute, systm.wSecond);
	strDate = chDate;
}

long long CDataBase::GetTimeStamp()
{
	long long llTime = 0;
	time_t rawtime;
	time(&rawtime);
	llTime = (long long)rawtime;
	return llTime;
}

long CDataBase::SaveFileToMongoDB(string& remotename, string& strPath, string& strRtr)
{
	/*
	GfidFS 对象用来存储文件，构造是需要传入DBClientConnection实例，使用数据库名称
	storeFile函数用来上传指定路径的文件，参数一：文件路径，参数二：数据库存储名称
	write函数下载文件，参数：文件保存路径
	*/
	if (m_dbinfo.nUseDB != 1)
	{
		strRtr = "No use Database!";
		return 0;
	}
	if (remotename.length() <= 0)
	{
		strRtr = "RemoteName is Empty!";
		return -1;
	}
	if (strPath.length() <= 0)
	{
		strRtr = "Path is Empty!";
		return -1;
	}
	try
	{
		string strerr;
		if (connect.isStillConnected())
		{
			GridFS fs(connect, m_dbinfo.chDBName);
			//fs.storeFile(strPath, remotename);
			BSONObj obj = fs.storeFile(strPath, remotename);
			if (!obj.isEmpty())
			{
				BSONElement em = obj.getField("md5");
				strRtr = em.toString();
			}
			else return -1;
		}
		else return -1;
		return 0;
	}
	catch (...)
	{
		return -1;
	}
}

BOOL CDataBase::DelUIDL(const string& strUIDL, const string& strName)
{
	if (strUIDL.length() <= 0)
		return FALSE;
	string strIndexName(m_dbinfo.chDBName),strValue;
	strIndexName.append(".");
	strIndexName.append(m_dbinfo.chTable);
	BSONObj cmd, obj = BSON("UIDL" << strUIDL << "TO" << strName)
		, bsoReturnValue, bsoQuery = BSON("UIDL" << strUIDL), bsoValue;
	if (connect.isStillConnected())
	{
		bsoReturnValue = connect.findAndRemove(strIndexName, bsoQuery);
		if (bsoReturnValue.isEmpty())
			return FALSE;
		else
			strValue = bsoReturnValue.toString();
	}
	else return FALSE;
	return TRUE;
}