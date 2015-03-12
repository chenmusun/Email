#pragma once
#include <string>
#include "../include/json/json.h"

class GGJsonAdapter
{
public:
	GGJsonAdapter(void);
	~GGJsonAdapter(void);

	BOOL Parse(std::string&str_content);
	long ReadValueLongFromStr(LPCTSTR lpKey);
	long ReadValueLongFromNum(LPCTSTR lpKey);
	long ReadVVLongFromNum(LPCTSTR lpValueKey,LPCTSTR lpKey);
	double ReadVVDoubleFromStr(LPCTSTR lpValueKey,LPCTSTR lpKey);
	double ReadVVDoubleFromNum(LPCTSTR lpValueKey,LPCTSTR lpKey);
	long ReadVVLongFromStr(LPCTSTR lpValueKey,LPCTSTR lpKey);
	LPTSTR ReadVVString(LPCTSTR lpValueKey,LPCTSTR lpKey,CString& str);
	LPTSTR ReadVVString(LPCTSTR lpValueKey,LPCTSTR lpKey,LPTSTR lpStr,long maxSize);
	LPTSTR ReadValueString(LPCTSTR lpKey,CString& str);
	LPTSTR ReadValueString(LPCTSTR lpKey,LPTSTR lpStr,long maxSize);

	void MoveNext(void);
	long GetDataSet(LPCTSTR lpKey);
	long ReadArrayLongFromStr(LPCTSTR lpKey)const;
	long ReadArrayLongFromNum(LPCTSTR lpKey)const;
	LPTSTR ReadArrayString(LPCTSTR lpKey,CString& str)const;
	LPTSTR ReadArrayString(LPCTSTR lpKey,LPTSTR lpStr,long maxSize)const;
	double ReadArrayDoubleFromStr(LPCTSTR lpKey) const;
	double ReadArrayDoubleFromNum(LPCTSTR lpKey) const;
	long	ReadArrayDate(LPCTSTR lpKey) const;
	long * ReadArrayLongArr(LPCTSTR lpKey,long * arr,long arrSize=6) const;
private:
static	char * FormatKey(LPCTSTR lpKey,char(&key)[64]);
static	LPTSTR FormatRetValue(const char * ansiStr,LPTSTR lpDestStr,long maxSize);
static	LPTSTR FormatRetValue(const char * ansiStr,CString& cstrDestStr);
	std::string m_js_key;
	long m_count;
	long m_cur_indx;		//当前所在行
	Json::Reader m_js_read;
	Json::Value	 m_js_value,m_js_array;
	std::string  m_str_content;
};
