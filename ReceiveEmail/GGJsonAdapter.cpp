#include "stdafx.h"
#include "GGJsonAdapter.h"
#include <atlcomtime.h>
GGJsonAdapter::GGJsonAdapter(void):m_cur_indx(0),m_count(0)
{
}

GGJsonAdapter::~GGJsonAdapter(void)
{
}

BOOL GGJsonAdapter::Parse(std::string&str_content)
{
	m_str_content = str_content;
	if(m_js_read.parse(m_str_content,m_js_value,false))
	{
		if (m_js_value.isObject())
		{
			return TRUE;
		}
	}
	return FALSE;
}

void GGJsonAdapter::MoveNext()
{
	++m_cur_indx;
}

long GGJsonAdapter::ReadValueLongFromNum(LPCTSTR lpKey)
{
	if(m_js_value.isObject()&&lpKey)
	{
		char szkey[64]={0};
		FormatKey(lpKey,szkey);

		switch(m_js_value[szkey].type())
		{
		case Json::intValue:
		case Json::uintValue:
		case Json::realValue:
			return m_js_value[szkey].asInt();
		default:
			break;
		}
	}
	return -1;
}

long GGJsonAdapter::ReadValueLongFromStr(LPCTSTR lpKey)
{
	if(m_js_value.isObject()&&lpKey){ 
		char szkey[64]={0};
		FormatKey(lpKey,szkey);

		if (m_js_value[szkey].isString())
		{
			return atol(m_js_value[szkey].asCString());
		}

	}
	return -1;
}

long GGJsonAdapter::ReadVVLongFromNum(LPCTSTR lpValueKey,LPCTSTR lpKey)
{
	if(m_js_value.isObject()&&lpValueKey)
	{
		char szValuekey[64]={0};
		FormatKey(lpValueKey,szValuekey);
		if (m_js_value[szValuekey].isObject()&&lpKey)
		{
			char szkey[64]={0};
			FormatKey(lpKey,szkey);

			switch(m_js_value[szValuekey][szkey].type())
			{
			case Json::intValue:
			case Json::uintValue:
			case Json::realValue:
				return m_js_value[szValuekey][szkey].asInt();
			default:
				break;
			}
		}
	}
	return -1;
}

long GGJsonAdapter::ReadVVLongFromStr(LPCTSTR lpValueKey,LPCTSTR lpKey)
{
	if(m_js_value.isObject()&&lpValueKey)
	{
		char szValuekey[64]={0};
		FormatKey(lpValueKey,szValuekey);
		if (m_js_value[szValuekey].isObject()&&lpKey)
		{
			char szkey[64]={0};
			FormatKey(lpKey,szkey);

			if (m_js_value[szValuekey][szkey].isString())
			{
				return atol(m_js_value[szValuekey][szkey].asCString());
			}
		}
	}
	return -1;
}

double GGJsonAdapter::ReadVVDoubleFromStr(LPCTSTR lpValueKey,LPCTSTR lpKey)
{
	if(m_js_value.isObject()&&lpValueKey)
	{
		char szValuekey[64]={0};
		FormatKey(lpValueKey,szValuekey);
		if (m_js_value[szValuekey].isObject()&&lpKey)
		{
			char szkey[64]={0};
			FormatKey(lpKey,szkey);

			if (m_js_value[szValuekey][szkey].isString())
			{
				return atof(m_js_value[szValuekey][szkey].asCString());
			}
		}
	}
	return -1.0;
}

double GGJsonAdapter::ReadVVDoubleFromNum(LPCTSTR lpValueKey,LPCTSTR lpKey)
{
	if(m_js_value.isObject()&&lpValueKey)
	{
		char szValuekey[64]={0};
		FormatKey(lpValueKey,szValuekey);
		if (m_js_value[szValuekey].isObject()&&lpKey)
		{
			char szkey[64]={0};
			FormatKey(lpKey,szkey);

			if(m_js_value[szValuekey][szkey].type()==Json::realValue)
			{
				return m_js_value[szValuekey][szkey].asDouble();
			}
		}
	}
	return -1.0;
}

LPTSTR GGJsonAdapter::ReadVVString(LPCTSTR lpValueKey,LPCTSTR lpKey,LPTSTR lpStr,long maxSize)
{
	if(m_js_value.isObject()&&lpValueKey)
	{
		char szValuekey[64]={0};
		FormatKey(lpValueKey,szValuekey);
		if (m_js_value[szValuekey].isObject()&&lpKey)
		{
			char szkey[64]={0};
			FormatKey(lpKey,szkey);

			if (m_js_value[szValuekey][szkey].isString())
			{
				return FormatRetValue(m_js_value[szValuekey][szkey].asCString(),lpStr,maxSize);
			}
		}
	}

	return _T("");
}

LPTSTR GGJsonAdapter::ReadVVString(LPCTSTR lpValueKey,LPCTSTR lpKey,CString& str)
{
	if(m_js_value.isObject()&&lpValueKey)
	{
		char szValuekey[64]={0};
		FormatKey(lpValueKey,szValuekey);
		if (m_js_value[szValuekey].isObject()&&lpKey)
		{
			char szkey[64]={0};
			FormatKey(lpKey,szkey);

			if (m_js_value[szValuekey][szkey].isString())
			{
				return FormatRetValue(m_js_value[szValuekey][szkey].asCString(),str);
			}
		}
	}

	return _T("");
}


LPTSTR GGJsonAdapter::ReadValueString(LPCTSTR lpKey,CString& str)
{
	if(m_js_value.isObject()&&lpKey)
	{
		char szkey[64]={0};
		FormatKey(lpKey,szkey);

		if (m_js_value[szkey].isString())
		{
			return FormatRetValue(m_js_value[szkey].asCString(),str);
		}	
	}

	return _T("");
}

LPTSTR GGJsonAdapter::ReadValueString(LPCTSTR lpKey,LPTSTR lpStr,long maxSize)
{
	if(m_js_value.isObject()&&lpKey&&lpStr)
	{
		char szkey[64]={0};
		FormatKey(lpKey,szkey);

		if (m_js_value[szkey].isString())
		{
			return FormatRetValue(m_js_value[szkey].asCString(),lpStr,maxSize);
		}		
	}
	return _T("");
}

long GGJsonAdapter::GetDataSet(LPCTSTR lpKey)
{
	if(m_js_value.isObject()&&lpKey)
	{
		char szkey[64]={0};
		FormatKey(lpKey,szkey);
		m_js_array = m_js_value[szkey];

		if(m_js_array.isArray())
		{
			m_cur_indx=0;
			return m_js_array.size();
		}		
	}
	return -1;
}
long GGJsonAdapter::ReadArrayLongFromStr(LPCTSTR lpKey) const
{
	if(m_js_array[m_cur_indx].isObject()&&lpKey){
		char szkey[64]={0};
		FormatKey(lpKey,szkey);

		if (m_js_array[m_cur_indx][szkey].isString())
		{
			return atol(m_js_array[m_cur_indx][szkey].asCString());
		}

	}
	return 0;
}

long GGJsonAdapter::ReadArrayLongFromNum(LPCTSTR lpKey) const
{
	if(m_js_array[m_cur_indx].isObject()&&lpKey)
	{
		char szkey[64]={0};
		FormatKey(lpKey,szkey);

		switch(m_js_array[m_cur_indx][szkey].type())
		{
		case Json::intValue:
		case Json::uintValue:
		case Json::realValue:
		case Json::booleanValue:
			return m_js_array[m_cur_indx][szkey].asInt();
		default:
			break;
		}
	}
	return 0;
}

LPTSTR GGJsonAdapter::ReadArrayString(LPCTSTR lpKey,CString& str) const
{
	if(m_js_array[m_cur_indx].isObject()&&lpKey)
	{
		char szkey[64]={0};
		FormatKey(lpKey,szkey);

		if (m_js_array[m_cur_indx][szkey].isString())
		{
			return FormatRetValue(m_js_array[m_cur_indx][szkey].asCString(),str);
		}		
	}	
	return _T("");
}

LPTSTR GGJsonAdapter::ReadArrayString(LPCTSTR lpKey,LPTSTR lpStr,long maxSize)const
{
	if(m_js_array[m_cur_indx].isObject()&&lpKey&&lpStr)
	{
		char szkey[64]={0};
		FormatKey(lpKey,szkey);

		if (m_js_array[m_cur_indx][szkey].isString())
		{		
			return FormatRetValue(m_js_array[m_cur_indx][szkey].asCString(),lpStr,maxSize);
		}
		else if (m_js_array[m_cur_indx][szkey].isArray())
		{
			if (m_js_array[m_cur_indx][szkey][0].isNull())
			{
				return _T("");
			}
			return FormatRetValue(m_js_array[m_cur_indx][szkey][0].asCString(),lpStr,maxSize);
		}

	}
	return _T("");
}

double GGJsonAdapter::ReadArrayDoubleFromStr(LPCTSTR lpKey) const
{
	if(m_js_array[m_cur_indx].isObject()&&lpKey){
		char szkey[64]={0};
		FormatKey(lpKey,szkey);
		if (m_js_array[m_cur_indx][szkey].isString())
		{
			return atof(m_js_array[m_cur_indx][szkey].asCString());
		}
	}
	return 0;
}

double GGJsonAdapter::ReadArrayDoubleFromNum(LPCTSTR lpKey) const
{
	if(m_js_array[m_cur_indx].isObject()&&lpKey)
	{
		char szkey[64]={0};
		FormatKey(lpKey,szkey);
		switch(m_js_array[m_cur_indx][szkey].type())
		{
		case Json::intValue:
		case Json::uintValue:
		case Json::realValue:
			return m_js_array[m_cur_indx][szkey].asDouble();
		default:
			break;
		}

	}
	return 0;
}

long GGJsonAdapter::ReadArrayDate(LPCTSTR lpKey) const
{
	string str_date;
	if(m_js_array[m_cur_indx].isObject()&&lpKey)
	{
		char szkey[64]={0};
		FormatKey(lpKey,szkey);
		if (m_js_array[m_cur_indx][szkey].isString())
		{
			str_date = m_js_array[m_cur_indx][szkey].asString();
		}
	}

	ATL::COleDateTime ole_date;
	ole_date.ParseDateTime(CString(str_date.c_str()).GetBuffer());
	long date=ole_date.GetYear()*10000+ole_date.GetMonth()*100+ole_date.GetDay();
	return date;
}

long * GGJsonAdapter::ReadArrayLongArr(LPCTSTR lpKey,long * arr,long arrSize) const
{
	if(lpKey&&arr&&arrSize&&m_js_array[m_cur_indx].isObject())
	{
		char szkey[64]={0};
		FormatKey(lpKey,szkey);

		long type=m_js_array[m_cur_indx][szkey].type();
		if (m_js_array[m_cur_indx][szkey].isString())
		{
			string str=m_js_array[m_cur_indx][szkey].asString();
			string::size_type start=0,end;
			long i=0;
			while((end=str.find(',',start))!=string::npos&&i<(arrSize-1))
			{
				arr[i]=atol(str.substr(start,(end-start)).c_str());
				i++;
				start=end+1;
			}
			
			arr[i]=atol(str.substr(start).c_str());
		}

	}
	return arr;
}


char * GGJsonAdapter::FormatKey(LPCTSTR lpKey,char(&key)[64])
{
	memset(key,0,64);
#ifdef UNICODE
	WideCharToMultiByte(CP_ACP,0,lpKey,-1,key,64,NULL,NULL);
#else
	strcpy(key,lpKey);
#endif
	return key;
}

LPTSTR GGJsonAdapter::FormatRetValue(const char * ansiStr,LPTSTR lpDestStr,long maxSize)
{
	memset(lpDestStr,0,maxSize);
#ifdef UNICODE
	long ansiLen=strlen(ansiStr);
	int wcsLen = ::MultiByteToWideChar(CP_ACP, 0, ansiStr,ansiLen, NULL, 0);
	if (wcsLen<maxSize)
	{
		::MultiByteToWideChar(CP_ACP, 0, ansiStr,ansiLen, lpDestStr, wcsLen);
	}
#else
	strcpy_s(lpDestStr,maxSize,ansiStr);
#endif
	return lpDestStr;
}

LPTSTR GGJsonAdapter::FormatRetValue(const char * ansiStr,CString& cstrDestStr)
{
	long ansiLen= strlen(ansiStr);
#ifdef UNICODE
	int wcsLen = ::MultiByteToWideChar(CP_ACP, 0, ansiStr,ansiLen, NULL, 0);
	LPTSTR pBuffer=cstrDestStr.GetBuffer(wcsLen+1);
	::MultiByteToWideChar(CP_ACP, 0, ansiStr,ansiLen,pBuffer, wcsLen);
	pBuffer[wcsLen]=L'\0';
#else
	LPTSTR pBuffer=cstrDestStr.GetBuffer(ansiLen+1);
	strcpy_s(pBuffer,ansiLen+1,ansiStr);
	pBuffer[ansiLen]='\0';
#endif
	cstrDestStr.ReleaseBuffer();
	return pBuffer;
}