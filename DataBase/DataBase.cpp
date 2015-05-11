// DataBase.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "DataBase.h"

static long lCount = 1;
// ���ǵ���������һ��ʾ��
DATABASE_API int nDataBase = 0;

// ���ǵ���������һ��ʾ����
DATABASE_API int fnDataBase(void)
{
	return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� DataBase.h
CDataBase::CDataBase(void) :m_bConnect(FALSE)
{
	memset(&m_dbinfo, 0, sizeof(MongoDBInfo));
	return;
}

CDataBase::~CDataBase()
{
	//connect.~DBClientConnection();
}

BOOL CDataBase::ConnectDataBase(string& strErr)
{
	string strWhat, strDBAdd(m_dbinfo.chDBAdd), strDBname(m_dbinfo.chDBName), strUser(m_dbinfo.chUserName), strPass(m_dbinfo.chPasswd);
	strErr.empty();//��������Ϣ���
	if (m_dbinfo.nUseDB != 1)
		return TRUE;
	if (m_dbinfo.chDBAdd && m_dbinfo.chDBAdd[0] != '\0')
	{
		try
		{
			mongo::client::initialize();
			if (connect.connect(strDBAdd, strErr))
			{
				connect.setSoTimeout(1000);
				if (!connect.auth(strDBname, strUser, strPass, strErr))//��½���ݿ���֤
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
	if (connect.isStillConnected() && m_bConnect)
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
	if (connect.isStillConnected() && m_bConnect)
	{
		connect.logout(m_dbinfo.chDBName, info);
		m_bConnect = FALSE;
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
	GfidFS ���������洢�ļ�����������Ҫ����DBClientConnectionʵ����ʹ�����ݿ�����
	storeFile���������ϴ�ָ��·�����ļ�������һ���ļ�·���������������ݿ�洢����
	write���������ļ����������ļ�����·��
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
		if (connect.isStillConnected() && m_bConnect)
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
	if (connect.isStillConnected() && m_bConnect)
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

BOOL CDataBase::GetFileFromMongoDB(const string& strFileName,const string&strSavePath,string& strErr)
{
	BOOL bRet = TRUE;
	string strTemp,strTempPath;
	int nStart(0);
	if (strFileName.length() <= 0)
		strErr = "FileName is empty!";
	gridfs_offset size(0), afsize(0);
	string strPath(strSavePath);
	
	if (connect.isStillConnected() && m_bConnect)
	{
		GridFS fs(connect, m_dbinfo.chDBName);
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
				bRet = bRet?TRUE:FALSE;
			else bRet = FALSE;
			pos = strFileName.find(";", nStart);
		}
	}
	return bRet;
}