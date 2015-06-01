// MongoDB.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "MongoDB.h"
static long lCount = 1;

// 这是导出变量的一个示例
MONGODB_API int nMongoDB=0;

// 这是导出函数的一个示例。
MONGODB_API int fnMongoDB(void)
{
	return 42;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 MongoDB.h
CMongoDB::CMongoDB() :m_bConnect(FALSE), m_nUseDB(1)
{
	return;
}

CMongoDB::~CMongoDB()
{
	//connect.~DBClientConnection();
}

BOOL CMongoDB::ConnectDataBase(string& strErr)
{
	string strWhat;
	strErr.empty();//将错误信息清空
	if (m_nUseDB != 1)
		return TRUE;
	if (m_strDBAdd.length()>0)
	{
		try
		{
			mongo::client::initialize();
			if (connect.connect(m_strDBAdd, strErr))
			{
				connect.setSoTimeout(1000);
				if (connect.auth(m_strDBName, m_strUserName, m_strPasswd, strErr))//登陆数据库验证
				{
#ifdef _DEBUG
					char chDebug[128] = { 0 };
					sprintf_s(chDebug, 128, "Connect [%s]-[%s] MongoDB Success!\r\n", m_strDBAdd.c_str(), m_strDBName.c_str());
					OutputDebugStringA(chDebug);
#endif
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
	else
	{
		return FALSE;
	}
	m_bConnect = TRUE;
	return TRUE;
}

long CMongoDB::CheckUIDLInMongoDB(const string& strUIDL, string& strErr, const string& strName, long lDay)
{
	//COleDateTime date;
	strErr.empty();
	long lFound = MONGO_FOUND;
	if (m_nUseDB != 1)
	{
		lFound = MONGO_NOT_FOUND;
		return lFound;
	}
	string strIndexName(m_strDBName);
	strIndexName.append(".");
	strIndexName.append(m_strTable);
	if (connect.isStillConnected() && m_bConnect)
	{
		BSONObj cmd, obj = BSON("UIDL" << strUIDL << "DATE" << DATENOW << "TO" << strName)
			, bsoReturnValue, bsoQuery = BSON("UIDL" << strUIDL), bsoValue;
		try
		{
			bsoReturnValue = connect.findOne(strIndexName, bsoQuery, &bsoValue);
			if (bsoReturnValue.isEmpty())
			{
#ifdef _DEBUG
				char chDebugInfo[256] = { 0 };
				sprintf_s(chDebugInfo, 256, "MongoDB InsertCount = %d\r\n", lCount++);
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
				if (lSaveDay >= lDay)
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


void CMongoDB::DisConnectDataBase()
{
	BSONObj info;
	if (m_nUseDB != 1) return;
	if (connect.isStillConnected() && m_bConnect)
	{
		connect.logout(m_strDBName, info);
		m_bConnect = FALSE;
	}
}

void CMongoDB::SetDBInfo(const MongoDBInfo&dbinfo)
{
	m_strDBAdd = dbinfo.chDBAdd;
	m_strDBName = dbinfo.chDBName;
	m_strTable = dbinfo.chTable;
	m_strUserName = dbinfo.chUserName;
	m_strPasswd = dbinfo.chPasswd;
	m_nUseDB = dbinfo.nUseDB;

}

void CMongoDB::GetCurrTime(string& strDate)
{
	char chDate[128] = { 0 };
	SYSTEMTIME systm;
	GetLocalTime(&systm);
	sprintf_s(chDate, 128, "%4d-%.2d-%.2d %.2d:%.2d:%.2d",
		systm.wYear, systm.wMonth, systm.wDay,
		systm.wHour, systm.wMinute, systm.wSecond);
	strDate = chDate;
}

long long CMongoDB::GetTimeStamp()
{
	long long llTime = 0;
	time_t rawtime;
	time(&rawtime);
	llTime = (long long)rawtime;
	return llTime;
}

long CMongoDB::SaveFileToMongoDB(string& remotename, string& strPath, string& strRtr)
{
	/*
	GfidFS 对象用来存储文件，构造是需要传入DBClientConnection实例，使用数据库名称
	storeFile函数用来上传指定路径的文件，参数一：文件路径，参数二：数据库存储名称
	write函数下载文件，参数：文件保存路径
	*/
	if (m_nUseDB != 1)
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
		if (connect.isStillConnected() && m_bConnect)
		{
			GridFS fs(connect, m_strDBName);
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

BOOL CMongoDB::DelUIDL(const string& strUIDL, const string& strName)
{
	if (m_nUseDB != 1)
		return TRUE;
	if (strUIDL.length() <= 0)
		return FALSE;
	string strIndexName(m_strDBName), strValue;
	strIndexName.append(".");
	strIndexName.append(m_strTable);
	BSONObj cmd, obj = BSON("UIDL" << strUIDL << "TO" << strName)
		, bsoReturnValue, bsoValue;
	if (connect.isStillConnected() && m_bConnect)
	{
		bsoReturnValue = connect.findAndRemove(strIndexName, obj);
		if (bsoReturnValue.isEmpty())
			return FALSE;
		else
			strValue = bsoReturnValue.toString();
	}
	else return FALSE;
	--lCount;
	return TRUE;
}

BOOL CMongoDB::GetFileFromMongoDB(const string& strFileName, const string&strSavePath, string& strErr)
{
	BOOL bRet = TRUE;
	string strTemp, strTempPath;
	int nStart(0);
	if (strFileName.length() <= 0)
		strErr = "FileName is empty!";
	gridfs_offset size(0), afsize(0);
	string strPath(strSavePath);

	if (connect.isStillConnected() && m_bConnect)
	{
		GridFS fs(connect, m_strDBName);
		auto pos = strFileName.find(";", nStart);
		while ((pos > 0) && (pos != strFileName.npos))
		{
			strTemp = strFileName.substr(nStart, pos - nStart);
			nStart = pos + 1;
			if (strTemp.length() > 0)
			{
				GridFile file = fs.findFileByName(strTemp);
				if (file.exists())
				{
					strPath = strSavePath;
					strPath.append("\\");
					strPath.append(strTemp);
					size = file.getContentLength();
					afsize = file.write(strPath);
				}
				else strErr = "File not exists!";
			}
			if (size > 0 && size == afsize)
				bRet = bRet ? TRUE : FALSE;
			else bRet = FALSE;
			pos = strFileName.find(";", nStart);
		}
	}
	return bRet;
}

BOOL CMongoDB::DelUIDL(const string& strUIDL)
{
	if (m_nUseDB != 1)
		return TRUE;
	if (strUIDL.length() <= 0)
		return FALSE;
	string strIndexName(m_strDBName), strValue;
	strIndexName.append(".");
	strIndexName.append(m_strTable);
	BSONObj cmd, bsoReturnValue, bsoQuery = BSON("UIDL" << strUIDL), bsoValue;
	if (connect.isStillConnected() && m_bConnect)
	{
		bsoReturnValue = connect.findAndRemove(strIndexName, bsoQuery);
		if (bsoReturnValue.isEmpty())
			return FALSE;
		else
			strValue = bsoReturnValue.toString();
	}
	else return FALSE;
	--lCount;
	return TRUE;
}
