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
	strErr.empty();//��������Ϣ���
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
				if (!connect.auth(m_dbinfo.chDBName, m_dbinfo.chUserName, m_dbinfo.chPasswd, strErr))//��½���ݿ���֤
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

long CDataBase::ExecSQL(const string& strUIDL, string& strErr,const string& strName)
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
		BSONObj cmd, obj = BSON("UIDL" << strUIDL << "MD5" << "MD5test" << "DATE" << DATENOW<<"TO"<<strName<<"STATUS"<<3)
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
				long long lSrvDate = date.asInt64(), lLocalDate(0), lDay(1209600);
				lSrvDate = lSrvDate / 1000;
				lLocalDate = GetTimeStamp();
				if (lLocalDate - lSrvDate > lDay)
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

void CDataBase::SaveFileToMongoDB(string& strRtr)
{
	/*
	GfidFS ���������洢�ļ�����������Ҫ����DBClientConnectionʵ����ʹ�����ݿ�����
	storeFile���������ϴ�ָ��·�����ļ�������һ���ļ�·���������������ݿ�洢����
	write���������ļ����������ļ�����·��
	*/
	string remotename("test11.pdf"),strerr;
	GridFS fs(connect,m_dbinfo.chDBName);
	BSONObj obj=fs.storeFile("E:\\Mail\\MailTest\\ReceiveEmail\\ReceiveEmail\\Email\\912340\\2.pdf", remotename);
	BSONElement em = obj.getField("md5");
	strRtr = em.toString();
}