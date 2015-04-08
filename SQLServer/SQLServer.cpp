// SQLServer.cpp : 定义 DLL 应用程序的导出函数。
//
//////////////////////////////////////////////////////////////////////
#include <math.h>


#include "stdafx.h"
#include "SQLServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ChunkSize 100
// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO:  更改错误代码以符合您的需要
			_tprintf(_T("错误:  MFC 初始化失败\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO:  在此处为应用程序的行为编写代码。
		}
	}
	else
	{
		// TODO:  更改错误代码以符合您的需要
		_tprintf(_T("错误:  GetModuleHandle 失败\n"));
		nRetCode = 1;
	}

	return nRetCode;
}



///////////////////////////////////////////////////////
//
// CADODatabase Class
//

DWORD CADODatabase::GetRecordCount(_RecordsetPtr m_pRs)
{
	DWORD numRows = 0;

	numRows = m_pRs->GetRecordCount();

	if (numRows == -1)
	{
		if (m_pRs->EndOfFile != VARIANT_TRUE)
			m_pRs->MoveFirst();

		while (m_pRs->EndOfFile != VARIANT_TRUE)
		{
			numRows++;
			m_pRs->MoveNext();
		}
		if (numRows > 0)
			m_pRs->MoveFirst();
	}
	return numRows;
}

BOOL CADODatabase::Open(LPCTSTR lpstrConnection, LPCTSTR lpstrUserID, LPCTSTR lpstrPassword)
{
	HRESULT hr = S_OK;

	if (IsOpen())
		Close();

	if (lstrcmp(lpstrConnection, _T("")) != 0)
		m_strConnection = lpstrConnection;

	ASSERT(!m_strConnection.IsEmpty());

	try
	{
		if (m_nConnectionTimeout != 0)
			m_pConnection->PutConnectionTimeout(m_nConnectionTimeout);
		hr = m_pConnection->Open(_bstr_t(m_strConnection), _bstr_t(lpstrUserID), _bstr_t(lpstrPassword), NULL);
		return hr == S_OK;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}

}

void CADODatabase::dump_com_error(_com_error &e)
{
	CString ErrorStr;

	_bstr_t bstrSource(e.Source());
	_bstr_t bstrDescription(e.Description());
	ErrorStr.Format(_T("CADODataBase Error\n\tCode = %08lx\n\tCode meaning = %s\n\tSource = %s\n\tDescription = %s\n"),
		e.Error(), e.ErrorMessage(), (LPCTSTR)bstrSource, (LPCTSTR)bstrDescription);
	m_strErrorDescription = (LPCTSTR)bstrDescription;
	m_strLastError = _T("Connection String = " + GetConnectionString() + '\n' + ErrorStr);
	m_dwLastError = e.Error();
//#ifdef _DEBUG
	//AfxMessageBox(ErrorStr, MB_OK | MB_ICONERROR);
//#endif
}

BOOL CADODatabase::IsOpen()
{
	if (m_pConnection)
		return m_pConnection->GetState() != adStateClosed;
	return FALSE;
}

void CADODatabase::Close()
{
	if (IsOpen())
		m_pConnection->Close();
}


///////////////////////////////////////////////////////
//
// CADORecordset Class
//

CADORecordset::CADORecordset()
{
	m_pRecordset = NULL;
	m_pCmd = NULL;
	m_strQuery = _T("");
	m_strLastError = _T("");
	m_dwLastError = 0;
	m_pRecordset.CreateInstance(__uuidof(Recordset));
	m_pCmd.CreateInstance(__uuidof(Command));
	m_nEditStatus = CADORecordset::dbEditNone;
	m_nSearchDirection = CADORecordset::searchForward;
}

CADORecordset::CADORecordset(CADODatabase* pAdoDatabase)
{
	m_pRecordset = NULL;
	m_pCmd = NULL;
	m_strQuery = _T("");
	m_strLastError = _T("");
	m_dwLastError = 0;
	m_pRecordset.CreateInstance(__uuidof(Recordset));
	m_pCmd.CreateInstance(__uuidof(Command));
	m_nEditStatus = CADORecordset::dbEditNone;
	m_nSearchDirection = CADORecordset::searchForward;

	m_pConnection = pAdoDatabase->GetActiveConnection();
}

BOOL CADORecordset::Open(_ConnectionPtr mpdb, LPCTSTR lpstrExec, int nOption)
{
	Close();

	if (lstrcmp(lpstrExec, _T("")) != 0)
		m_strQuery = lpstrExec;

	ASSERT(!m_strQuery.IsEmpty());

	if (m_pConnection == NULL)
		m_pConnection = mpdb;

	m_strQuery.TrimLeft();
	BOOL bIsSelect = m_strQuery.Mid(0, lstrlen(_T("Select "))).CompareNoCase(_T("select ")) == 0 && nOption == openUnknown;

	try
	{
		m_pRecordset->CursorType = adOpenStatic;
		m_pRecordset->CursorLocation = adUseClient;
		if (bIsSelect || nOption == openQuery || nOption == openUnknown)
			m_pRecordset->Open((LPCTSTR)m_strQuery, _variant_t((IDispatch*)mpdb, TRUE),
			adOpenStatic, adLockOptimistic, adCmdUnknown);
		else if (nOption == openTable)
			m_pRecordset->Open((LPCTSTR)m_strQuery, _variant_t((IDispatch*)mpdb, TRUE),
			adOpenKeyset, adLockOptimistic, adCmdTable);
		else if (nOption == openStoredProc)
		{
			m_pCmd->ActiveConnection = mpdb;
			m_pCmd->CommandText = _bstr_t(m_strQuery);
			m_pCmd->CommandType = adCmdStoredProc;
			m_pConnection->CursorLocation = adUseClient;

			m_pRecordset = m_pCmd->Execute(NULL, NULL, adCmdText);
		}
		else
		{
			TRACE("Unknown parameter. %d", nOption);
			return FALSE;
		}
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}

	return m_pRecordset != NULL && m_pRecordset->GetState() != adStateClosed;
}

BOOL CADORecordset::Open(LPCTSTR lpstrExec, int nOption)
{
	ASSERT(m_pConnection != NULL);
	ASSERT(m_pConnection->GetState() != adStateClosed);
	return Open(m_pConnection, lpstrExec, nOption);
}

BOOL CADORecordset::OpenSchema(int nSchema, LPCTSTR SchemaID /*= _T("")*/)
{
	try
	{
		_variant_t vtSchemaID = vtMissing;

		if (lstrlen(SchemaID) != 0)
		{
			vtSchemaID = SchemaID;
			nSchema = adSchemaProviderSpecific;
		}

		m_pRecordset = m_pConnection->OpenSchema((enum SchemaEnum)nSchema, vtMissing, vtSchemaID);
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::Requery()
{
	if (IsOpen())
	{
		try
		{
			m_pRecordset->Requery(2048);
		}
		catch (_com_error &e)
		{
			dump_com_error(e);
			return FALSE;
		}
	}
	return TRUE;
}


BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, double& dbValue)
{
	double val = (double)NULL;
	_variant_t vtFld;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		switch (vtFld.vt)
		{
		case VT_R4:
			val = vtFld.fltVal;
			break;
		case VT_R8:
			val = vtFld.dblVal;
			break;
		case VT_DECIMAL:
			//Corrected by Jos?Carlos Mart韓ez Gal醤
			val = vtFld.decVal.Lo32;
			val *= (vtFld.decVal.sign == 128) ? -1 : 1;
			val /= pow((double)10, (int)vtFld.decVal.scale);
			break;
		case VT_UI1:
			val = vtFld.iVal;
			break;
		case VT_I2:
		case VT_I4:
			val = vtFld.lVal;
			break;
		case VT_INT:
			val = vtFld.intVal;
			break;
		case VT_CY:   //Added by John Andy Johnson!!!!
			vtFld.ChangeType(VT_R8);
			val = vtFld.dblVal;
			break;
		case VT_NULL:
		case VT_EMPTY:
			val = 0;
			break;
		default:
			val = vtFld.dblVal;
		}
		dbValue = val;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}


BOOL CADORecordset::GetFieldValue(int nIndex, double& dbValue)
{
	double val = (double)NULL;
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		switch (vtFld.vt)
		{
		case VT_R4:
			val = vtFld.fltVal;
			break;
		case VT_R8:
			val = vtFld.dblVal;
			break;
		case VT_DECIMAL:
			//Corrected by Jos?Carlos Mart韓ez Gal醤
			val = vtFld.decVal.Lo32;
			val *= (vtFld.decVal.sign == 128) ? -1 : 1;
			val /= pow((double)10, (int)vtFld.decVal.scale);
			break;
		case VT_UI1:
			val = vtFld.iVal;
			break;
		case VT_I2:
		case VT_I4:
			val = vtFld.lVal;
			break;
		case VT_INT:
			val = vtFld.intVal;
			break;
		case VT_CY:   //Added by John Andy Johnson!!!!
			vtFld.ChangeType(VT_R8);
			val = vtFld.dblVal;
			break;
		case VT_NULL:
		case VT_EMPTY:
			val = 0;
			break;
		default:
			val = 0;
		}
		dbValue = val;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(int nIndex, INT64& i64Value)
{
	//double val = (double)NULL;
	INT64  i64val = 0;
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		switch (vtFld.vt)
		{
		case VT_R4:
			i64val = (INT64)vtFld.fltVal;
			break;
		case VT_R8:
			i64val = (INT64)vtFld.dblVal;
			break;
		case VT_DECIMAL:
			//Corrected by Jos?Carlos Mart韓ez Gal醤
			i64val = vtFld.decVal.Lo64;
			break;
		case VT_UI1:
			i64val = vtFld.iVal;
			break;
		case VT_I2:
		case VT_I4:
			i64val = vtFld.lVal;
			break;
		case VT_INT:
			i64val = vtFld.intVal;
			break;
		case VT_CY:   //Added by John Andy Johnson!!!!
			vtFld.ChangeType(VT_R8);
			i64val = (INT64)vtFld.dblVal;
			break;
		case VT_NULL:
		case VT_EMPTY:
			i64val = 0;
			break;
		default:
			i64val = 0;
		}
		i64Value = i64val;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, long& lValue)
{
	long val = (long)NULL;
	_variant_t vtFld;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		if (vtFld.vt != VT_NULL && vtFld.vt != VT_EMPTY)
			val = vtFld.lVal;
		lValue = val;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(int nIndex, long& lValue)
{
	long val = (long)NULL;
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		if (vtFld.vt != VT_NULL && vtFld.vt != VT_EMPTY)
			val = vtFld.lVal;
		lValue = val;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}


BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, unsigned long& ulValue)
{
	long val = (long)NULL;
	_variant_t vtFld;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		if (vtFld.vt != VT_NULL && vtFld.vt != VT_EMPTY)
			val = vtFld.ulVal;
		ulValue = val;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(int nIndex, unsigned long& ulValue)
{
	long val = (long)NULL;
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		if (vtFld.vt != VT_NULL && vtFld.vt != VT_EMPTY)
			val = vtFld.ulVal;
		ulValue = val;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, int& nValue)
{
	int val = NULL;
	_variant_t vtFld;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		switch (vtFld.vt)
		{
		case VT_BOOL:
			val = vtFld.boolVal;
			break;
		case VT_I2:
		case VT_UI1:
			val = vtFld.iVal;
			break;
		case VT_INT:
			val = vtFld.intVal;
			break;
		case VT_NULL:
		case VT_EMPTY:
			val = 0;
			break;
		default:
			val = vtFld.iVal;
		}
		nValue = val;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(int nIndex, int& nValue)
{
	int val = (int)NULL;
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		switch (vtFld.vt)
		{
		case VT_BOOL:
			val = vtFld.boolVal;
			break;
		case VT_I2:
		case VT_UI1:
			val = vtFld.iVal;
			break;
		case VT_INT:
			val = vtFld.intVal;
			break;
		case VT_NULL:
		case VT_EMPTY:
			val = 0;
			break;
		default:
			val = vtFld.iVal;
		}
		nValue = val;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, CString& strValue, CString strDateFormat)
{
	CString str;
	_variant_t vtFld;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		switch (vtFld.vt)
		{
		case VT_R4:
			str = DblToStr(vtFld.fltVal);
			break;
		case VT_R8:
			str = DblToStr(vtFld.dblVal);
			break;
		case VT_BSTR:
			str = vtFld.bstrVal;
			break;
		case VT_I2:
		case VT_UI1:
			str = IntToStr(vtFld.iVal);
			break;
		case VT_INT:
			str = IntToStr(vtFld.intVal);
			break;
		case VT_I4:
			str = LongToStr(vtFld.lVal);
			break;
		case VT_UI4:
			str = ULongToStr(vtFld.ulVal);
			break;
		case VT_DECIMAL:
		{
						   //Corrected by Jos?Carlos Mart韓ez Gal醤
						   double val = vtFld.decVal.Lo32;
						   val *= (vtFld.decVal.sign == 128) ? -1 : 1;
						   val /= pow((double)10, (int)vtFld.decVal.scale);
						   str = DblToStr(val);
		}
			break;
		case VT_DATE:
		{
						COleDateTime dt(vtFld);

						if (strDateFormat.IsEmpty())
							strDateFormat = _T("%Y-%m-%d %H:%M:%S");
						str = dt.Format(strDateFormat);
		}
			break;
		case VT_CY:		//Added by John Andy Johnson!!!
		{
							vtFld.ChangeType(VT_R8);

							CString str;
							str.Format(_T("%f"), vtFld.dblVal);

							_TCHAR pszFormattedNumber[64];

							//	LOCALE_USER_DEFAULT
							if (GetCurrencyFormat(
								LOCALE_USER_DEFAULT,	// locale for which string is to be formatted 
								0,						// bit flag that controls the function's operation
								str,					// pointer to input number string
								NULL,					// pointer to a formatting information structure
								//	NULL = machine default locale settings
								pszFormattedNumber,		// pointer to output buffer
								63))					// size of output buffer
							{
								str = pszFormattedNumber;
							}
		}
			break;
		case VT_EMPTY:
		case VT_NULL:
			str.Empty();
			break;
		case VT_BOOL:
			str = vtFld.boolVal == VARIANT_TRUE ? 'T' : 'F';
			break;
		default:
			str.Empty();
			return FALSE;
		}
		strValue = str;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(int nIndex, CString& strValue, CString strDateFormat)
{
	CString str;
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		switch (vtFld.vt)
		{
		case VT_R4:
			str = DblToStr(vtFld.fltVal);
			break;
		case VT_R8:
			str = DblToStr(vtFld.dblVal);
			break;
		case VT_BSTR:
			str = vtFld.bstrVal;
			break;
		case VT_I2:
		case VT_UI1:
			str = IntToStr(vtFld.iVal);
			break;
		case VT_INT:
			str = IntToStr(vtFld.intVal);
			break;
		case VT_I4:
			str = LongToStr(vtFld.lVal);
			break;
		case VT_UI4:
			str = ULongToStr(vtFld.ulVal);
			break;
		case VT_DECIMAL:
		{
						   //Corrected by Jos?Carlos Mart韓ez Gal醤
						   double val = vtFld.decVal.Lo32;
						   val *= (vtFld.decVal.sign == 128) ? -1 : 1;
						   val /= pow((double)10, (int)vtFld.decVal.scale);
						   str = DblToStr(val);
		}
			break;
		case VT_DATE:
		{
						COleDateTime dt(vtFld);

						if (strDateFormat.IsEmpty())
							strDateFormat = _T("%Y-%m-%d %H:%M:%S");
						str = dt.Format(strDateFormat);
		}
			break;
		case VT_CY:		//Added by John Andy Johnson!!!
		{
							vtFld.ChangeType(VT_R8);

							CString str;
							str.Format(_T("%f"), vtFld.dblVal);

							_TCHAR pszFormattedNumber[64];

							//	LOCALE_USER_DEFAULT
							if (GetCurrencyFormat(
								LOCALE_USER_DEFAULT,	// locale for which string is to be formatted 
								0,						// bit flag that controls the function's operation
								str,					// pointer to input number string
								NULL,					// pointer to a formatting information structure
								//	NULL = machine default locale settings
								pszFormattedNumber,		// pointer to output buffer
								63))					// size of output buffer
							{
								str = pszFormattedNumber;
							}
		}
			break;
		case VT_BOOL:
			str = vtFld.boolVal == VARIANT_TRUE ? 'T' : 'F';
			break;
		case VT_EMPTY:
		case VT_NULL:
			str.Empty();
			break;
		default:
			str.Empty();
			return FALSE;
		}
		strValue = str;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, COleDateTime& time)
{
	_variant_t vtFld;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		switch (vtFld.vt)
		{
		case VT_DATE:
		{
						COleDateTime dt(vtFld);
						time = dt;
		}
			break;
		case VT_EMPTY:
		case VT_NULL:
			time.SetStatus(COleDateTime::null);
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(int nIndex, COleDateTime& time)
{
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		switch (vtFld.vt)
		{
		case VT_DATE:
		{
						COleDateTime dt(vtFld);
						time = dt;
		}
			break;
		case VT_EMPTY:
		case VT_NULL:
			time.SetStatus(COleDateTime::null);
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, bool& bValue)
{
	_variant_t vtFld;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		switch (vtFld.vt)
		{
		case VT_BOOL:
			bValue = vtFld.boolVal == VARIANT_TRUE ? true : false;
			break;
		case VT_EMPTY:
		case VT_NULL:
			bValue = false;
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(int nIndex, bool& bValue)
{
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		switch (vtFld.vt)
		{
		case VT_BOOL:
			bValue = vtFld.boolVal == VARIANT_TRUE ? true : false;
			break;
		case VT_EMPTY:
		case VT_NULL:
			bValue = false;
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, COleCurrency& cyValue)
{
	_variant_t vtFld;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		switch (vtFld.vt)
		{
		case VT_CY:
			cyValue = (CURRENCY)vtFld.cyVal;
			break;
		case VT_EMPTY:
		case VT_NULL:
		{
						cyValue = COleCurrency();
						cyValue.m_status = COleCurrency::null;
		}
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(int nIndex, COleCurrency& cyValue)
{
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		switch (vtFld.vt)
		{
		case VT_CY:
			cyValue = (CURRENCY)vtFld.cyVal;
			break;
		case VT_EMPTY:
		case VT_NULL:
		{
						cyValue = COleCurrency();
						cyValue.m_status = COleCurrency::null;
		}
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(LPCTSTR lpFieldName, _variant_t& vtValue)
{
	try
	{
		vtValue = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::GetFieldValue(int nIndex, _variant_t& vtValue)
{
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	try
	{
		vtValue = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::IsFieldNull(LPCTSTR lpFieldName)
{
	_variant_t vtFld;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		return vtFld.vt == VT_NULL;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::IsFieldNull(int nIndex)
{
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		return vtFld.vt == VT_NULL;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::IsFieldEmpty(LPCTSTR lpFieldName)
{
	_variant_t vtFld;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
		return vtFld.vt == VT_EMPTY || vtFld.vt == VT_NULL;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::IsFieldEmpty(int nIndex)
{
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	try
	{
		vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
		return vtFld.vt == VT_EMPTY || vtFld.vt == VT_NULL;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::SetFieldEmpty(LPCTSTR lpFieldName)
{
	_variant_t vtFld;
	vtFld.vt = VT_EMPTY;

	return PutFieldValue(lpFieldName, vtFld);
}

BOOL CADORecordset::SetFieldEmpty(int nIndex)
{
	_variant_t vtFld;
	vtFld.vt = VT_EMPTY;

	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	return PutFieldValue(vtIndex, vtFld);
}


DWORD CADORecordset::GetRecordCount()
{
	DWORD nRows = 0;

	nRows = m_pRecordset->GetRecordCount();

	if (nRows == -1)
	{
		nRows = 0;
		if (m_pRecordset->EndOfFile != VARIANT_TRUE)
			m_pRecordset->MoveFirst();

		while (m_pRecordset->EndOfFile != VARIANT_TRUE)
		{
			nRows++;
			m_pRecordset->MoveNext();
		}
		if (nRows > 0)
			m_pRecordset->MoveFirst();
	}

	return nRows;
}

BOOL CADORecordset::IsOpen()
{
	if (m_pRecordset != NULL && IsConnectionOpen())
		return m_pRecordset->GetState() != adStateClosed;
	return FALSE;
}

void CADORecordset::Close()
{
	if (IsOpen())
	{
		if (m_nEditStatus != dbEditNone)
			CancelUpdate();

		m_pRecordset->PutSort(_T(""));
		m_pRecordset->Close();
	}
}


BOOL CADODatabase::Execute(LPCTSTR lpstrExec)
{
	ASSERT(m_pConnection != NULL);
	ASSERT(lstrcmp(lpstrExec, _T("")) != 0);
	_variant_t vRecords;

	m_nRecordsAffected = 0;

	try
	{
		m_pConnection->CursorLocation = adUseClient;
		m_pConnection->Execute(_bstr_t(lpstrExec), &vRecords, adExecuteNoRecords);
		m_nRecordsAffected = vRecords.iVal;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

long CADODatabase::BeginTransaction()
{
	if (m_dwTransStatus == transactionStatusBegin) return 1;

	long lRet = m_pConnection->BeginTrans();
	m_dwTransStatus = transactionStatusBegin;
	return lRet;
}

long CADODatabase::CommitTransaction()
{
	if (m_dwTransStatus != transactionStatusBegin) return 1;

	long lRet = m_pConnection->CommitTrans();
	m_dwTransStatus = transactionStatusCommit;
	return lRet;
}

long CADODatabase::RollbackTransaction()
{
	if (m_dwTransStatus != transactionStatusBegin) return 1;

	long lRet = m_pConnection->RollbackTrans();
	m_dwTransStatus = transactionStatusRollback;
	return lRet;
}

DWORD CADODatabase::GetTransStatus()
{
	return m_dwTransStatus;
}

BOOL CADORecordset::GetFieldInfo(LPCTSTR lpFieldName, CADOFieldInfo* fldInfo)
{
	FieldPtr pField = m_pRecordset->Fields->GetItem(lpFieldName);

	return GetFieldInfo(pField, fldInfo);
}

BOOL CADORecordset::GetFieldInfo(int nIndex, CADOFieldInfo* fldInfo)
{
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	FieldPtr pField = m_pRecordset->Fields->GetItem(vtIndex);

	return GetFieldInfo(pField, fldInfo);
}

BOOL CADORecordset::GetFieldInfo(FieldPtr pField, CADOFieldInfo* fldInfo)
{
	memset(fldInfo, 0, sizeof(CADOFieldInfo));

	lstrcpy(fldInfo->m_strName, (LPCTSTR)pField->GetName());
	fldInfo->m_lDefinedSize = pField->GetDefinedSize();
	fldInfo->m_nType = pField->GetType();
	fldInfo->m_lAttributes = pField->GetAttributes();
	fldInfo->m_nPrecision = pField->GetPrecision();
	if (!IsEof())
		fldInfo->m_lSize = pField->GetActualSize();
	return TRUE;
}

BOOL CADORecordset::GetChunk(LPCTSTR lpFieldName, CString& strValue)
{
	FieldPtr pField = m_pRecordset->Fields->GetItem(lpFieldName);

	return GetChunk(pField, strValue);
}

BOOL CADORecordset::GetChunk(int nIndex, CString& strValue)
{
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	FieldPtr pField = m_pRecordset->Fields->GetItem(vtIndex);

	return GetChunk(pField, strValue);
}


BOOL CADORecordset::GetChunk(FieldPtr pField, CString& strValue)
{
	CString str;
	long lngSize, lngOffSet = 0;
	_variant_t varChunk;

	lngSize = pField->ActualSize;

	str.Empty();
	while (lngOffSet < lngSize)
	{
		try
		{
			varChunk = pField->GetChunk(ChunkSize);

			str += varChunk.bstrVal;
			lngOffSet += ChunkSize;
		}
		catch (_com_error &e)
		{
			dump_com_error(e);
			return FALSE;
		}
	}

	lngOffSet = 0;
	strValue = str;
	return TRUE;
}

BOOL CADORecordset::GetChunk(LPCTSTR lpFieldName, LPVOID lpData)
{
	FieldPtr pField = m_pRecordset->Fields->GetItem(lpFieldName);

	return GetChunk(pField, lpData);
}

BOOL CADORecordset::GetChunk(int nIndex, LPVOID lpData)
{
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	FieldPtr pField = m_pRecordset->Fields->GetItem(vtIndex);

	return GetChunk(pField, lpData);
}

BOOL CADORecordset::GetChunk(FieldPtr pField, LPVOID lpData)
{
	long lngSize, lngOffSet = 0;
	_variant_t varChunk;
	UCHAR chData;
	HRESULT hr;
	long lBytesCopied = 0;

	lngSize = pField->ActualSize;

	while (lngOffSet < lngSize)
	{
		try
		{
			varChunk = pField->GetChunk(ChunkSize);

			//Copy the data only upto the Actual Size of Field.  
			for (long lIndex = 0; lIndex <= (ChunkSize - 1); lIndex++)
			{
				hr = SafeArrayGetElement(varChunk.parray, &lIndex, &chData);
				if (SUCCEEDED(hr))
				{
					//Take BYTE by BYTE and advance Memory Location
					//hr = SafeArrayPutElement((SAFEARRAY FAR*)lpData, &lBytesCopied ,&chData); 
					((UCHAR*)lpData)[lBytesCopied] = chData;
					lBytesCopied++;
				}
				else
					break;
			}
			lngOffSet += ChunkSize;
		}
		catch (_com_error &e)
		{
			dump_com_error(e);
			return FALSE;
		}
	}

	lngOffSet = 0;
	return TRUE;
}

BOOL CADORecordset::AppendChunk(LPCTSTR lpFieldName, LPVOID lpData, UINT nBytes)
{

	FieldPtr pField = m_pRecordset->Fields->GetItem(lpFieldName);

	return AppendChunk(pField, lpData, nBytes);
}


BOOL CADORecordset::AppendChunk(int nIndex, LPVOID lpData, UINT nBytes)
{
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	FieldPtr pField = m_pRecordset->Fields->GetItem(vtIndex);

	return AppendChunk(pField, lpData, nBytes);
}

BOOL CADORecordset::AppendChunk(FieldPtr pField, LPVOID lpData, UINT nBytes)
{
	if (m_nEditStatus == dbEditNone)
		return FALSE;

	HRESULT hr;
	_variant_t varChunk;
	long lngOffset = 0;
	UCHAR chData;
	SAFEARRAY FAR *psa = NULL;
	SAFEARRAYBOUND rgsabound[1];

	try
	{
		//Create a safe array to store the array of BYTES 
		rgsabound[0].lLbound = 0;
		rgsabound[0].cElements = nBytes;
		psa = SafeArrayCreate(VT_UI1, 1, rgsabound);

		while (lngOffset < (long)nBytes)
		{
			chData = ((UCHAR*)lpData)[lngOffset];
			hr = SafeArrayPutElement(psa, &lngOffset, &chData);

			if (FAILED(hr))
				return FALSE;

			lngOffset++;
		}
		lngOffset = 0;

		//Assign the Safe array  to a variant. 
		varChunk.Clear();
		varChunk.vt = VT_ARRAY | VT_UI1;
		varChunk.parray = psa;

		hr = pField->AppendChunk(varChunk);

		if (SUCCEEDED(hr)) return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}

	return FALSE;
}

CString CADORecordset::GetString(LPCTSTR lpCols, LPCTSTR lpRows, LPCTSTR lpNull, long numRows)
{
	_bstr_t varOutput;
	_bstr_t varNull(_T(""));
	_bstr_t varCols(_T("\t"));
	_bstr_t varRows(_T("\r"));

	if (lstrlen(lpCols) != 0)
		varCols = _bstr_t(lpCols);

	if (lstrlen(lpRows) != 0)
		varRows = _bstr_t(lpRows);

	if (numRows == 0)
		numRows = (long)GetRecordCount();

	varOutput = m_pRecordset->GetString(adClipString, numRows, varCols, varRows, varNull);

	return (LPCTSTR)varOutput;
}

CString IntToStr(int nVal)
{
	CString strRet;
	TCHAR buff[10];

#ifdef _UNICODE
	_itow_s(nVal, buff, lstrlen(buff), 10);
#else
	_itoa(nVal, buff, lstrlen(buff));
#endif

	strRet = buff;
	return strRet;
}

CString LongToStr(long lVal)
{
	CString strRet;
	TCHAR buff[20];

#ifdef _UNICODE
	_ltow_s(lVal, buff, lstrlen(buff), 10);
#else
	_ltoa(lVal, buff, lstrlen(buff));
#endif
	strRet = buff;
	return strRet;
}

CString ULongToStr(unsigned long ulVal)
{
	CString strRet;
	TCHAR buff[20];

#ifdef _UNICODE
	_ultow_s(ulVal, buff, lstrlen(buff), 10);
#else
	_ultoa(ulVal, buff, lstrlen(buff));
#endif
	strRet = buff;
	return strRet;

}


CString DblToStr(double dblVal, int ndigits)
{
	CString strRet;
	char buff[50];
	_gcvt_s(buff, 50, dblVal,ndigits);
	//sprintf_s(buff,50,"%")
#ifdef _UNICODE
	::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buff, -1, strRet.GetBuffer(0), 50);
#else
	strRet = buff;
#endif
	return strRet;
}

CString DblToStr(float fltVal)
{
	return DblToStr(fltVal, 10);
}

void CADORecordset::Edit()
{
	m_nEditStatus = dbEdit;
}

BOOL CADORecordset::AddNew()
{
	m_nEditStatus = dbEditNone;
	if (m_pRecordset->AddNew() != S_OK)
		return FALSE;

	m_nEditStatus = dbEditNew;
	return TRUE;
}

BOOL CADORecordset::Update()
{
	BOOL bret = TRUE;

	if (m_nEditStatus != dbEditNone)
	{

		try
		{
			if (m_pRecordset->Update() != S_OK)
				bret = FALSE;
		}
		catch (_com_error &e)
		{
			dump_com_error(e);
			bret = FALSE;
		}

		if (!bret)
			m_pRecordset->CancelUpdate();
		m_nEditStatus = dbEditNone;
	}
	return bret;
}

void CADORecordset::CancelUpdate()
{
	m_pRecordset->CancelUpdate();
	m_nEditStatus = dbEditNone;
}

BOOL CADORecordset::SetFieldValue(int nIndex, CString strValue)
{
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	if (!strValue.IsEmpty())
		vtFld.vt = VT_BSTR;
	else
		vtFld.vt = VT_NULL;

	//Corrected by Giles Forster 10/03/2001
	vtFld.bstrVal = strValue.AllocSysString();

	BOOL bret = PutFieldValue(vtIndex, vtFld);

	//Corrected by Flemming27 at CodeProject.com
	SysFreeString(vtFld.bstrVal);
	return bret;
}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, CString strValue)
{
	_variant_t vtFld;

	if (!strValue.IsEmpty())
		vtFld.vt = VT_BSTR;
	else
		vtFld.vt = VT_NULL;

	//Corrected by Giles Forster 10/03/2001
	vtFld.bstrVal = strValue.AllocSysString();

	BOOL bret = PutFieldValue(lpFieldName, vtFld);
	//Corrected by Flemming27 at CodeProject.com
	SysFreeString(vtFld.bstrVal);
	return bret;
}

BOOL CADORecordset::SetFieldValue(int nIndex, int nValue)
{
	_variant_t vtFld;

	vtFld.vt = VT_I2;
	vtFld.iVal = nValue;

	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	return PutFieldValue(vtIndex, vtFld);
}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, int nValue)
{
	_variant_t vtFld;

	vtFld.vt = VT_I2;
	vtFld.iVal = nValue;


	return PutFieldValue(lpFieldName, vtFld);
}

BOOL CADORecordset::SetFieldValue(int nIndex, long lValue)
{
	_variant_t vtFld;
	vtFld.vt = VT_I4;
	vtFld.lVal = lValue;

	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	return PutFieldValue(vtIndex, vtFld);

}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, long lValue)
{
	_variant_t vtFld;
	vtFld.vt = VT_I4;
	vtFld.lVal = lValue;

	return PutFieldValue(lpFieldName, vtFld);
}

BOOL CADORecordset::SetFieldValue(int nIndex, unsigned long ulValue)
{
	_variant_t vtFld;
	vtFld.vt = VT_UI4;
	vtFld.ulVal = ulValue;

	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	return PutFieldValue(vtIndex, vtFld);

}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, unsigned long ulValue)
{
	_variant_t vtFld;
	vtFld.vt = VT_UI4;
	vtFld.ulVal = ulValue;

	return PutFieldValue(lpFieldName, vtFld);
}

BOOL CADORecordset::SetFieldValue(int nIndex, double dblValue)
{
	_variant_t vtFld;
	vtFld.vt = VT_R8;
	vtFld.dblVal = dblValue;

	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	return PutFieldValue(vtIndex, vtFld);
}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, double dblValue)
{
	_variant_t vtFld;
	vtFld.vt = VT_R8;
	vtFld.dblVal = dblValue;

	return PutFieldValue(lpFieldName, vtFld);
}

BOOL CADORecordset::SetFieldValue(int nIndex, COleDateTime time)
{
	_variant_t vtFld;
	vtFld.vt = VT_DATE;
	vtFld.date = time;

	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	return PutFieldValue(vtIndex, vtFld);
}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, COleDateTime time)
{
	_variant_t vtFld;
	vtFld.vt = VT_DATE;
	vtFld.date = time;

	return PutFieldValue(lpFieldName, vtFld);
}



BOOL CADORecordset::SetFieldValue(int nIndex, bool bValue)
{
	_variant_t vtFld;
	vtFld.vt = VT_BOOL;
	vtFld.boolVal = bValue;

	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	return PutFieldValue(vtIndex, vtFld);
}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, bool bValue)
{
	_variant_t vtFld;
	vtFld.vt = VT_BOOL;
	vtFld.boolVal = bValue;

	return PutFieldValue(lpFieldName, vtFld);
}


BOOL CADORecordset::SetFieldValue(int nIndex, COleCurrency cyValue)
{
	if (cyValue.m_status == COleCurrency::invalid)
		return FALSE;

	_variant_t vtFld;

	vtFld.vt = VT_CY;
	vtFld.cyVal = cyValue.m_cur;

	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	return PutFieldValue(vtIndex, vtFld);
}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, COleCurrency cyValue)
{
	if (cyValue.m_status == COleCurrency::invalid)
		return FALSE;

	_variant_t vtFld;

	vtFld.vt = VT_CY;
	vtFld.cyVal = cyValue.m_cur;

	return PutFieldValue(lpFieldName, vtFld);
}

BOOL CADORecordset::SetFieldValue(int nIndex, _variant_t vtValue)
{
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	return PutFieldValue(vtIndex, vtValue);
}

BOOL CADORecordset::SetFieldValue(LPCTSTR lpFieldName, _variant_t vtValue)
{
	return PutFieldValue(lpFieldName, vtValue);
}


BOOL CADORecordset::SetBookmark()
{
	if (m_varBookmark.vt != VT_EMPTY)
	{
		m_pRecordset->Bookmark = m_varBookmark;
		return TRUE;
	}
	return FALSE;
}

BOOL CADORecordset::Delete()
{
	if (m_pRecordset->Delete(adAffectCurrent) != S_OK)
		return FALSE;

	if (m_pRecordset->Update() != S_OK)
		return FALSE;

	m_nEditStatus = dbEditNone;
	return TRUE;
}

BOOL CADORecordset::Find(LPCTSTR lpFind, int nSearchDirection)
{

	m_strFind = lpFind;
	m_nSearchDirection = nSearchDirection;

	ASSERT(!m_strFind.IsEmpty());

	if (m_nSearchDirection == searchForward)
	{
		m_pRecordset->Find(_bstr_t(m_strFind), 0, adSearchForward, "");
		if (!IsEof())
		{
			m_varBookFind = m_pRecordset->Bookmark;
			return TRUE;
		}
	}
	else if (m_nSearchDirection == searchBackward)
	{
		m_pRecordset->Find(_bstr_t(m_strFind), 0, adSearchBackward, "");
		if (!IsBof())
		{
			m_varBookFind = m_pRecordset->Bookmark;
			return TRUE;
		}
	}
	else
	{
		TRACE("Unknown parameter. %d", nSearchDirection);
		m_nSearchDirection = searchForward;
	}
	return FALSE;
}

BOOL CADORecordset::FindFirst(LPCTSTR lpFind)
{
	m_pRecordset->MoveFirst();
	return Find(lpFind);
}

BOOL CADORecordset::FindNext()
{
	if (m_nSearchDirection == searchForward)
	{
		m_pRecordset->Find(_bstr_t(m_strFind), 1, adSearchForward, m_varBookFind);
		if (!IsEof())
		{
			m_varBookFind = m_pRecordset->Bookmark;
			return TRUE;
		}
	}
	else
	{
		m_pRecordset->Find(_bstr_t(m_strFind), 1, adSearchBackward, m_varBookFind);
		if (!IsBof())
		{
			m_varBookFind = m_pRecordset->Bookmark;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CADORecordset::PutFieldValue(LPCTSTR lpFieldName, _variant_t vtFld)
{
	if (m_nEditStatus == dbEditNone)
		return FALSE;

	try
	{
		m_pRecordset->Fields->GetItem(lpFieldName)->Value = vtFld;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}


BOOL CADORecordset::PutFieldValue(_variant_t vtIndex, _variant_t vtFld)
{
	if (m_nEditStatus == dbEditNone)
		return FALSE;

	try
	{
		m_pRecordset->Fields->GetItem(vtIndex)->Value = vtFld;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::Clone(CADORecordset &pRs)
{
	try
	{
		pRs.m_pRecordset = m_pRecordset->Clone(adLockUnspecified);
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::SetFilter(LPCTSTR strFilter)
{
	ASSERT(IsOpen());

	try
	{
		m_pRecordset->PutFilter(strFilter);
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::SetSort(LPCTSTR strCriteria)
{
	ASSERT(IsOpen());

	try
	{
		m_pRecordset->PutSort(strCriteria);
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::SaveAsXML(LPCTSTR lpstrXMLFile)
{
	HRESULT hr;

	ASSERT(IsOpen());

	try
	{
		hr = m_pRecordset->Save(lpstrXMLFile, adPersistXML);
		return hr == S_OK;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
	return TRUE;
}

BOOL CADORecordset::OpenXML(LPCTSTR lpstrXMLFile)
{
	HRESULT hr = S_OK;

	if (IsOpen())
		Close();

	try
	{
		hr = m_pRecordset->Open(lpstrXMLFile, "Provider=MSPersist;", adOpenForwardOnly, adLockOptimistic, adCmdFile);
		return hr == S_OK;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADORecordset::Execute(CADOCommand* pAdoCommand)
{
	if (IsOpen())
		Close();

	ASSERT(!pAdoCommand->GetText().IsEmpty());
	try
	{
		m_pConnection->CursorLocation = adUseClient;
		m_pRecordset = pAdoCommand->GetCommand()->Execute(NULL, NULL, pAdoCommand->GetType());
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

void CADORecordset::dump_com_error(_com_error &e)
{
	CString ErrorStr;


	_bstr_t bstrSource(e.Source());
	_bstr_t bstrDescription(e.Description());
	ErrorStr.Format(_T("CADORecordset Error\n\tCode = %08lx\n\tCode meaning = %s\n\tSource = %s\n\tDescription = %s\n"),
		e.Error(), e.ErrorMessage(), (LPCTSTR)bstrSource, (LPCTSTR)bstrDescription);
	m_strLastError = _T("Query = " + GetQuery() + '\n' + ErrorStr);
	m_dwLastError = e.Error();
#ifdef _DEBUG
	//AfxMessageBox(ErrorStr, MB_OK | MB_ICONERROR);
#endif	
	// throw CADOException(e.Error(), e.Description());
}


///////////////////////////////////////////////////////
//
// CADOCommad Class
//

CADOCommand::CADOCommand(CADODatabase* pAdoDatabase, CString strCommandText, int nCommandType)
{
	m_pCommand = NULL;
	m_pCommand.CreateInstance(__uuidof(Command));
	m_strCommandText = strCommandText;
	m_pCommand->CommandText = m_strCommandText.AllocSysString();
	m_nCommandType = nCommandType;
	m_pCommand->CommandType = (CommandTypeEnum)m_nCommandType;
	m_pCommand->ActiveConnection = pAdoDatabase->GetActiveConnection();
	m_nRecordsAffected = 0;
}

BOOL CADOCommand::AddParameter(CADOParameter* pAdoParameter)
{
	ASSERT(pAdoParameter->GetParameter() != NULL);

	try
	{
		m_pCommand->Parameters->Append(pAdoParameter->GetParameter());
		return TRUE;
	}
	catch (_com_error& e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADOCommand::AddParameter(CString strName, int nType, int nDirection, long lSize, int nValue)
{

	_variant_t vtValue;

	vtValue.vt = VT_I2;
	vtValue.iVal = nValue;

	return AddParameter(strName, nType, nDirection, lSize, vtValue);
}

BOOL CADOCommand::AddParameter(CString strName, int nType, int nDirection, long lSize, long lValue)
{

	_variant_t vtValue;

	vtValue.vt = VT_I4;
	vtValue.lVal = lValue;

	return AddParameter(strName, nType, nDirection, lSize, vtValue);
}

BOOL CADOCommand::AddParameter(CString strName, int nType, int nDirection, long lSize, double dblValue, int nPrecision, int nScale)
{

	_variant_t vtValue;

	vtValue.vt = VT_R8;
	vtValue.dblVal = dblValue;

	return AddParameter(strName, nType, nDirection, lSize, vtValue, nPrecision, nScale);
}

BOOL CADOCommand::AddParameter(CString strName, int nType, int nDirection, long lSize, CString strValue)
{

	_variant_t vtValue;

	vtValue.vt = VT_BSTR;
	vtValue.bstrVal = strValue.AllocSysString();

	return AddParameter(strName, nType, nDirection, lSize, vtValue);
}

BOOL CADOCommand::AddParameter(CString strName, int nType, int nDirection, long lSize, COleDateTime time)
{

	_variant_t vtValue;

	vtValue.vt = VT_DATE;
	vtValue.date = time;

	return AddParameter(strName, nType, nDirection, lSize, vtValue);
}

BOOL CADOCommand::AddParameter(CString strName, int nType, int nDirection, long lSize, _variant_t vtValue, int nPrecision, int nScale)
{
	try
	{
		_ParameterPtr pParam = m_pCommand->CreateParameter(strName.AllocSysString(), (DataTypeEnum)nType, (ParameterDirectionEnum)nDirection, lSize, vtValue);
		pParam->PutPrecision(nPrecision);
		pParam->PutNumericScale(nScale);
		m_pCommand->Parameters->Append(pParam);

		return TRUE;
	}
	catch (_com_error& e)
	{
		dump_com_error(e);
		return FALSE;
	}
}


BOOL CADOCommand::AddChunkParam(CString strName, int nType, int nDirection, long lSize, LPVOID lpData)
{
	HRESULT hr;
	_variant_t varChunk;
	long lngOffset = 0;
	UCHAR chData;
	SAFEARRAY FAR *psa = NULL;
	SAFEARRAYBOUND rgsabound[1];

	try
	{
		//Create a safe array to store the array of BYTES 
		rgsabound[0].lLbound = 0;
		rgsabound[0].cElements = lSize;
		psa = SafeArrayCreate(VT_UI1, 1, rgsabound);

		while (lngOffset < (long)lSize)
		{
			chData = ((UCHAR*)lpData)[lngOffset];
			hr = SafeArrayPutElement(psa, &lngOffset, &chData);

			if (FAILED(hr))
				return FALSE;

			lngOffset++;
		}
		lngOffset = 0;

		//Assign the Safe array  to a variant. 
		varChunk.Clear();
		varChunk.vt = VT_ARRAY | VT_UI1;
		varChunk.parray = psa;

		_ParameterPtr pParam = m_pCommand->CreateParameter(strName.AllocSysString(), (DataTypeEnum)nType, (ParameterDirectionEnum)nDirection, lSize, varChunk);
		m_pCommand->Parameters->Append(pParam);
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}

	return FALSE;
}

void CADOCommand::SetText(CString strCommandText)
{
	ASSERT(!strCommandText.IsEmpty());

	m_strCommandText = strCommandText;
	m_pCommand->CommandText = m_strCommandText.AllocSysString();
}

void CADOCommand::SetType(int nCommandType)
{
	m_nCommandType = nCommandType;
	m_pCommand->CommandType = (CommandTypeEnum)m_nCommandType;
}

BOOL CADOCommand::Execute(int nCommandType /*= typeCmdStoredProc*/)
{
	_variant_t vRecords;
	m_nRecordsAffected = 0;
	try
	{
		m_nCommandType = nCommandType;
		m_pCommand->Execute(&vRecords, NULL, nCommandType);
		m_nRecordsAffected = vRecords.iVal;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

void CADOCommand::dump_com_error(_com_error &e)
{
	CString ErrorStr;


	_bstr_t bstrSource(e.Source());
	_bstr_t bstrDescription(e.Description());
	ErrorStr.Format(_T("CADOCommand Error\n\tCode = %08lx\n\tCode meaning = %s\n\tSource = %s\n\tDescription = %s\n"),
		e.Error(), e.ErrorMessage(), (LPCTSTR)bstrSource, (LPCTSTR)bstrDescription);
	m_strLastError = ErrorStr;
	m_dwLastError = e.Error();
#ifdef _DEBUG
	//AfxMessageBox(ErrorStr, MB_OK | MB_ICONERROR);
#endif	
	// throw CADOException(e.Error(), e.Description());
}


///////////////////////////////////////////////////////
//
// CADOParameter Class
//

CADOParameter::CADOParameter(int nType, long lSize, int nDirection, CString strName)
{
	m_pParameter = NULL;
	m_pParameter.CreateInstance(__uuidof(Parameter));
	m_strName = _T("");
	m_pParameter->Direction = (ParameterDirectionEnum)nDirection;
	m_strName = strName;
	m_pParameter->Name = m_strName.AllocSysString();
	m_pParameter->Type = (DataTypeEnum)nType;
	m_pParameter->Size = lSize;
	m_nType = nType;
}

BOOL CADOParameter::SetValue(int nValue)
{
	_variant_t vtVal;

	ASSERT(m_pParameter != NULL);

	vtVal.vt = VT_I2;
	vtVal.iVal = nValue;

	try
	{
		if (m_pParameter->Size == 0)
			m_pParameter->Size = sizeof(int);

		m_pParameter->Value = vtVal;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}


BOOL CADOParameter::SetValue(long lValue)
{
	_variant_t vtVal;

	ASSERT(m_pParameter != NULL);

	vtVal.vt = VT_I4;
	vtVal.lVal = lValue;

	try
	{
		if (m_pParameter->Size == 0)
			m_pParameter->Size = sizeof(long);

		m_pParameter->Value = vtVal;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADOParameter::SetValue(double dblValue)
{
	_variant_t vtVal;

	ASSERT(m_pParameter != NULL);

	vtVal.vt = VT_R8;
	vtVal.dblVal = dblValue;

	try
	{
		if (m_pParameter->Size == 0)
			m_pParameter->Size = sizeof(double);

		m_pParameter->Value = vtVal;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADOParameter::SetValue(CString strValue)
{
	_variant_t vtVal;

	ASSERT(m_pParameter != NULL);

	if (!strValue.IsEmpty())
		vtVal.vt = VT_BSTR;
	else
		vtVal.vt = VT_NULL;

	//Corrected by Giles Forster 10/03/2001
	vtVal.bstrVal = strValue.AllocSysString();
	//vtVal.SetString(strValue.GetBuffer(0));

	try
	{
		if (m_pParameter->Size == 0)
			m_pParameter->Size = sizeof(TCHAR)* strValue.GetLength();

		m_pParameter->Value = vtVal;
		::SysFreeString(vtVal.bstrVal);
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		::SysFreeString(vtVal.bstrVal);
		return FALSE;
	}
}

BOOL CADOParameter::SetValue(COleDateTime time)
{
	_variant_t vtVal;

	ASSERT(m_pParameter != NULL);

	vtVal.vt = VT_DATE;
	vtVal.date = time;

	try
	{
		if (m_pParameter->Size == 0)
			m_pParameter->Size = sizeof(DATE);

		m_pParameter->Value = vtVal;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADOParameter::SetValue(_variant_t vtValue)
{

	ASSERT(m_pParameter != NULL);

	try
	{
		if (m_pParameter->Size == 0)
			m_pParameter->Size = sizeof(VARIANT);

		m_pParameter->Value = vtValue;
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}
}


BOOL CADOParameter::SetChunkValue(LPVOID lpData, long lSize)
{
	HRESULT hr;
	_variant_t varChunk;
	long lngOffset = 0;
	UCHAR chData;
	SAFEARRAY FAR *psa = NULL;
	SAFEARRAYBOUND rgsabound[1];

	try
	{
		//Create a safe array to store the array of BYTES 
		rgsabound[0].lLbound = 0;
		rgsabound[0].cElements = lSize;
		psa = SafeArrayCreate(VT_UI1, 1, rgsabound);

		while (lngOffset < (long)lSize)
		{
			chData = ((UCHAR*)lpData)[lngOffset];
			hr = SafeArrayPutElement(psa, &lngOffset, &chData);

			if (FAILED(hr))
				return FALSE;

			lngOffset++;
		}
		lngOffset = 0;

		//Assign the Safe array  to a variant. 
		varChunk.Clear();
		varChunk.vt = VT_ARRAY | VT_UI1;
		varChunk.parray = psa;

		SetValue(varChunk);
		return TRUE;
	}
	catch (_com_error &e)
	{
		dump_com_error(e);
		return FALSE;
	}

	return FALSE;
}

BOOL CADOParameter::GetValue(int& nValue)
{
	_variant_t vtVal;
	int nVal = 0;

	try
	{
		vtVal = m_pParameter->Value;

		switch (vtVal.vt)
		{
		case VT_BOOL:
			nVal = vtVal.boolVal;
			break;
		case VT_I2:
		case VT_UI1:
			nVal = vtVal.iVal;
			break;
		case VT_INT:
			nVal = vtVal.intVal;
			break;
		case VT_NULL:
		case VT_EMPTY:
			nVal = 0;
			break;
		default:
			nVal = vtVal.iVal;
		}
		nValue = nVal;
		return TRUE;
	}
	catch (_com_error& e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADOParameter::GetValue(long& lValue)
{
	_variant_t vtVal;
	long lVal = 0;

	try
	{
		vtVal = m_pParameter->Value;
		if (vtVal.vt != VT_NULL && vtVal.vt != VT_EMPTY)
			lVal = vtVal.lVal;
		lValue = lVal;
		return TRUE;
	}
	catch (_com_error& e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADOParameter::GetValue(double& dbValue)
{
	_variant_t vtVal;
	double dblVal;
	try
	{
		vtVal = m_pParameter->Value;
		switch (vtVal.vt)
		{
		case VT_R4:
			dblVal = vtVal.fltVal;
			break;
		case VT_R8:
			dblVal = vtVal.dblVal;
			break;
		case VT_DECIMAL:
			//Corrected by Jos?Carlos Mart韓ez Gal醤
			dblVal = vtVal.decVal.Lo32;
			dblVal *= (vtVal.decVal.sign == 128) ? -1 : 1;
			dblVal /= pow((double)10, (int)vtVal.decVal.scale);
			break;
		case VT_UI1:
			dblVal = vtVal.iVal;
			break;
		case VT_I2:
		case VT_I4:
			dblVal = vtVal.lVal;
			break;
		case VT_INT:
			dblVal = vtVal.intVal;
			break;
		case VT_NULL:
		case VT_EMPTY:
			dblVal = 0;
			break;
		default:
			dblVal = 0;
		}
		dbValue = dblVal;
		return TRUE;
	}
	catch (_com_error& e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADOParameter::GetValue(CString& strValue, CString strDateFormat)
{
	_variant_t vtVal;
	CString strVal;

	try
	{
		vtVal = m_pParameter->Value;
		switch (vtVal.vt)
		{
		case VT_R4:
			strVal = DblToStr(vtVal.fltVal);
			break;
		case VT_R8:
			strVal = DblToStr(vtVal.dblVal);
			break;
		case VT_BSTR:
			strVal = vtVal.bstrVal;
			break;
		case VT_I2:
		case VT_UI1:
			strVal = IntToStr(vtVal.iVal);
			break;
		case VT_INT:
			strVal = IntToStr(vtVal.intVal);
			break;
		case VT_I4:
			strVal = LongToStr(vtVal.lVal);
			break;
		case VT_DECIMAL:
		{
						   //Corrected by Jos?Carlos Mart韓ez Gal醤
						   double val = vtVal.decVal.Lo32;
						   val *= (vtVal.decVal.sign == 128) ? -1 : 1;
						   val /= pow((double)10, (int)vtVal.decVal.scale);
						   strVal = DblToStr(val);
		}
			break;
		case VT_DATE:
		{
						COleDateTime dt(vtVal);

						if (strDateFormat.IsEmpty())
							strDateFormat = _T("%Y-%m-%d %H:%M:%S");
						strVal = dt.Format(strDateFormat);
		}
			break;
		case VT_EMPTY:
		case VT_NULL:
			strVal.Empty();
			break;
		default:
			strVal.Empty();
			return FALSE;
		}
		strValue = strVal;
		return TRUE;
	}
	catch (_com_error& e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADOParameter::GetValue(COleDateTime& time)
{
	_variant_t vtVal;

	try
	{
		vtVal = m_pParameter->Value;
		switch (vtVal.vt)
		{
		case VT_DATE:
		{
						COleDateTime dt(vtVal);
						time = dt;
		}
			break;
		case VT_EMPTY:
		case VT_NULL:
			time.SetStatus(COleDateTime::null);
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
	catch (_com_error& e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

BOOL CADOParameter::GetValue(_variant_t& vtValue)
{
	try
	{
		vtValue = m_pParameter->Value;
		return TRUE;
	}
	catch (_com_error& e)
	{
		dump_com_error(e);
		return FALSE;
	}
}

void CADOParameter::dump_com_error(_com_error &e)
{
	CString ErrorStr;
	_bstr_t bstrSource(e.Source());
	_bstr_t bstrDescription(e.Description());
	ErrorStr.Format(_T("CADOParameter Error\n\tCode = %08lx\n\tCode meaning = %s\n\tSource = %s\n\tDescription = %s\n"),
		e.Error(), e.ErrorMessage(), (LPCTSTR)bstrSource, (LPCTSTR)bstrDescription);
	m_strLastError = ErrorStr;
	m_dwLastError = e.Error();

#ifdef _DEBUG
	//AfxMessageBox(ErrorStr, MB_OK | MB_ICONERROR);
#endif	
	// throw CADOException(e.Error(), e.Description());
}


///////////////////////////////////////////////////////
CSQLServer::CSQLServer() :m_nType(0), m_pEMRs(NULL), m_pGuidRs(NULL), m_pSubjectRs(NULL)
{
	m_pEMRs = new CADORecordset(&m_db);
	m_pGuidRs = new CADORecordset(&m_db);
	m_pSubjectRs = new CADORecordset(&m_db);
}

CSQLServer::~CSQLServer()
{
	if (m_pEMRs != NULL)
	{
		m_pEMRs->Close();
		delete m_pEMRs;
		m_pEMRs = NULL;
	}

	if (m_pSubjectRs != NULL)
	{
		m_pSubjectRs->Close();
		delete m_pSubjectRs;
		m_pSubjectRs = NULL;
	}

	if (m_pGuidRs != NULL)
	{
		m_pGuidRs->Close();
		delete m_pGuidRs;
		m_pGuidRs = NULL;
	}
}


long CSQLServer::SaveToDB(EMAIL_ITEM& email)
{
	BOOL bFailed = FALSE;
	if (IsExist(email))
		return 0;
	DWORD size = 0;
	COleDateTime oledt = COleDateTime::GetCurrentTime();
	CString csTemp, csPath,csLog;
	try
	{
		if (m_db.IsOpen())
		{
			m_db.BeginTransaction();
			CADOCommand * pEMCmd = new CADOCommand(&m_db);
			if (pEMCmd == NULL)
				return -1;

			TCHAR szCmdText[512] = _T("INSERT INTO [ReportEmailDB].[dbo].[T_REPORT_EMAIL](EmailUIDL, EmailSubject,EmailFrom, EmailTo , EmailDate,ContentType) VALUES(?, ?, ?, ?, ?, ?)");
			auto pos = email.csUIDL.Find(_T("\\"));
			if (pos > 0)
				email.csUIDL = email.csUIDL.Mid(pos + 1);
			pEMCmd->AddParameter(_T("EmailUIDL"), adVarChar, CADOParameter::paramInput, email.csUIDL.GetLength()*sizeof(TCHAR), _bstr_t(email.csUIDL.GetBuffer(0)));
			if (email.csSubject.IsEmpty())
				email.csSubject.Format(_T("无主题"));
			pEMCmd->AddParameter(_T("EmailSubject"), adVarWChar, CADOParameter::paramInput, email.csSubject.GetLength()*sizeof(TCHAR), _bstr_t(email.csSubject.GetBuffer(0)));

			if (email.csFrom.IsEmpty())
				email.csFrom.Format(_T("unknow"));
			pEMCmd->AddParameter(_T("EmailFrom"), adVarChar, CADOParameter::paramInput, email.csFrom.GetLength()*sizeof(TCHAR), _bstr_t(email.csFrom.GetBuffer(0)));
			if (email.csTo.IsEmpty())
				email.csTo.Format(_T("unknow"));
			pEMCmd->AddParameter(_T("EmailTo"), adVarWChar, CADOParameter::paramInput, email.csTo.GetLength()*sizeof(TCHAR), _bstr_t(email.csTo.GetBuffer(0)));

			TCHAR szDate[32] = { 0 };
			TCHAR szTime[32] = { 0 };
			oledt.ParseDateTime(email.csDate);
			wsprintf(szDate, _T("%04d-%02d-%02d %02d:%02d:%02d"), oledt.GetYear(), oledt.GetMonth(), oledt.GetDay()
				, oledt.GetHour(), oledt.GetMinute(), oledt.GetSecond());
			pEMCmd->AddParameter(_T("EmailDate"), adVarChar, CADOParameter::paramInput, lstrlen(szDate), _bstr_t(szDate));
			if (email.csContentType.IsEmpty())
				email.csContentType.Format(_T("unknow"));
			pEMCmd->AddParameter(_T("ContentType"), adVarWChar, CADOParameter::paramInput, email.csContentType.GetLength()*sizeof(TCHAR), _bstr_t(email.csContentType.GetBuffer(0)));
			pEMCmd->SetText(szCmdText);
			if (pEMCmd->Execute(adCmdText))
			{
#ifdef _DEBUG
				OutputDebugString(_T("Insert to SQL Success!\r\n"));
#endif
			}
			else
			{
				csLog.Format(_T("[%s]Insert to SQL Failed!"),email.csUIDL);
				Log(csLog,csLog.GetLength());
				csLog.Append(_T("\r\n"));
				OutputDebugString(csLog);
				delete pEMCmd;
				return -1;
			}
			delete pEMCmd;
			TCHAR szQuerySQL[512] = { 0 };
			swprintf_s(szQuerySQL,
				_T("SELECT EMailID, ContentType, EmailContent FROM [ReportEmailDB].[dbo].[T_REPORT_EMAIL] WHERE EMailUidl COLLATE Latin1_General_CS_AS ='%s' AND EMailTo='%s' AND EMailDate=CONVERT(datetime,'%s',121)"),
				email.csUIDL, email.csTo, szDate);
			if (m_pEMRs->Open(szQuerySQL, CADORecordset::openQuery))
			{
				m_pEMRs->Edit();
				CFile file;
				char * pContentData = NULL;
				csPath = email.csEmailContent;
				if (csPath.IsEmpty())
					csPath.Format(_T("%s"), email.csEmailContentHTML);
				if (!csPath.IsEmpty())
				{
					size = 0;
					if (file.Open(csPath, CFile::modeRead))
					{
						size = file.GetLength();
						pContentData = new char[size + 1];
						memset(pContentData, 0, size + 1);
						for (DWORD index = 0; index < size; index)
						{
							index += file.Read(pContentData + index, size - index);
						}
						file.Close();
						if (pContentData != NULL && size > 0)
							m_pEMRs->AppendChunk(_T("EmailContent"), pContentData, size + 1);
					}
				}
				if (pContentData != NULL&& size > 0)
				{
					delete  pContentData;
					pContentData = NULL;
				}
				m_pEMRs->Update();
				m_pEMRs->GetFieldValue(_T("EMailID"), email.lSn);
#ifdef _DEBUG
				CString csDebug;
				csDebug.Format(_T("EMailID = %d\r\n"), email.lSn);
				OutputDebugString(csDebug);
#endif
			}
			vector<ATTACH_FILE>::iterator ite = email.vecAttachFiles.begin();
			while (ite != email.vecAttachFiles.end())
			{
				if ((*ite).lType == 0)
				{
					ite++;
					continue;
				}
				if (SaveAttachment((*ite), email.lSn) < 0)
				{
					bFailed = TRUE;
					break;
				}
				ite++;
			}
			m_db.CommitTransaction();
		}
		else
		{
			csLog.Format(_T("DataBase is closed!"));
			Log(csLog, csLog.GetLength());
			csLog.Append(_T("\r\n"));
			OutputDebugString(csLog);
		}
	}
	catch (_com_error& e)
	{
		m_db.RollbackTransaction();
		csLog.Format(_T("SaveToDB\r\n%s\r\n%s"), (TCHAR*)e.Description(), (TCHAR*)e.ErrorMessage());
		Log(csLog, csLog.GetLength());
		csLog.Append(_T("\r\n"));
		OutputDebugString(csLog);
	}
	if (bFailed)
		return -1;
	return 0;
}

void CSQLServer::GetGUID(CString &guid)
{
	guid.Empty();
	if (m_db.IsOpen() && m_pGuidRs)
	{
		if (m_pGuidRs->Open(_T("SELECT newid() as GUID")) == FALSE) return;
		if (m_pGuidRs->GetFieldValue(_T("GUID"), guid) == FALSE) return;
		m_pGuidRs->Close();
	}
}

BOOL CSQLServer::Connect(SQLDBInfo& sqlinfo, int nType)
{
	::CoInitialize(NULL);
	CString csCommand,csLog;
	m_csServer.Empty();
	m_csDatabase.Empty();
	m_csUser.Empty();
	m_csPass.Empty();
	m_nType = nType;
	if (m_db.IsOpen() == TRUE)
		return TRUE;
	m_csServer.Format(_T("%s"),sqlinfo.szDBAdd);
	m_csDatabase.Format(_T("%s"), sqlinfo.szDBName);
	m_csUser.Format(_T("%s"), sqlinfo.szUserName);
	m_csPass.Format(_T("%s"), sqlinfo.szPasswd);
	if (m_csServer.IsEmpty() || m_csDatabase.IsEmpty())
		return FALSE;
	try
	{
		switch (m_nType)
		{
		case 0://混合验证模式
		{
				   csCommand.Format(_T("Provider=sqloledb;Data Source=%s;Database=%s;User ID=%s;Password=%s")
					   , sqlinfo.szDBAdd, sqlinfo.szDBName, sqlinfo.szUserName, sqlinfo.szPasswd);
		}
			break;
		case 1://Windows验证模式
		{
				   csCommand.Format(_T("Provider=sqloledb;Data Source=%s;Database=%s;Trusted_Connection=yes")
					   , sqlinfo.szDBAdd, sqlinfo.szDBName);
		}
			break;
		default:
			break;
		}
		m_db.SetConnectionTimeout(180);
		if (!m_db.Open(csCommand))
		{
			csLog.Format(_T("ConnectError:[%s]-[%s]"),m_csServer,m_csDatabase);
			Log(csLog, csLog.GetLength());
			return FALSE;
		}
	}
	catch (_com_error&e)
	{
		csLog.Format(_T("Connect:[%s]-[%s]\r\n[%s]\r\n%s")
			, m_csServer, m_csDatabase, (TCHAR*)e.Description(), (TCHAR*)e.ErrorMessage());
		Log(csLog, csLog.GetLength());
		//#ifdef _DEBUG
		OutputDebugString(csLog);
		OutputDebugString(_T("\r\n"));
		//#endif
		return FALSE;
	}
	return TRUE;
}

BOOL CSQLServer::IsConnect()
{
	try
	{
		if (m_db.IsOpen() == FALSE)
			return FALSE;
		if (m_pEMRs->Open(_T("SELECT 1")) == FALSE)
			return FALSE;
		if (m_pEMRs->GetRecordCount() > 0)
			return TRUE;
	}
	catch (_com_error &e)
	{
		CString csErr;
		csErr.Format(_T("IsConnect:[%s]-[%s]\r\n[%s]\r\n[%s]")
			, m_csServer, m_csDatabase, (TCHAR*)e.Description(), (TCHAR*)e.ErrorMessage());
		//#ifdef _DEBUG
		OutputDebugString(csErr);
		OutputDebugString(_T("\r\n"));
		//#endif
		Log(csErr, csErr.GetLength());
	}
	return FALSE;
}


BOOL CSQLServer::ReConnect()
{
	CString csCommand;
	if (m_csServer.IsEmpty() || m_csDatabase.IsEmpty() || m_csUser.IsEmpty())
		return FALSE;
	try
	{
		if (m_db.IsOpen())
			m_db.Close();

		switch (m_nType)
		{
		case 0://混合验证模式
		{
				   csCommand.Format(_T("Provider=sqloledb;Data Source=%s;Database=%s;User ID=%s;Password=%s")
					   , m_csServer, m_csDatabase, m_csUser, m_csPass);
		}
			break;
		case 1://Windows验证模式
		{
				   csCommand.Format(_T("Provider=sqloledb;Data Source=%s;Database=%s;Trusted_Connection=yes")
					   , m_csServer, m_csDatabase);
		}
			break;
		default:
			break;
		}
		if (!m_db.Open(csCommand))
			return FALSE;
	}
	catch (_com_error&e)
	{
		csCommand.Format(_T("ReConnect:[%s]-[%s]\r\n[%s]\r\n[%s]")
			, m_csServer, m_csDatabase, (TCHAR*)e.Description(), (TCHAR*)e.ErrorMessage());
		//#ifdef _DEBUG
		OutputDebugString(csCommand);
		OutputDebugString(_T("\r\n"));
		//#endif
		Log(csCommand, csCommand.GetLength());
		return FALSE;
	}
	return TRUE;
}

BOOL CSQLServer::SQLExec(LPCTSTR lpSql)
{
	CString csCmd(lpSql);
	if (csCmd.IsEmpty())
		return FALSE;
	try
	{
		CADOCommand * pEMCmd = new CADOCommand(&m_db);
		pEMCmd->SetText(csCmd);
		pEMCmd->Execute(adCmdText);
		delete pEMCmd;
		return TRUE;
	}
	catch (_com_error&e)
	{
		csCmd.Format(_T("SqlExec:[%s]Err\r\n[%s]\r\n[%s]"), lpSql, (TCHAR*)e.Description(), (TCHAR*)e.ErrorMessage());
		//#ifdef _DEBUG
		OutputDebugString(csCmd);
		OutputDebugString(_T("\r\n"));
		//#endif
		Log(csCmd, csCmd.GetLength());
	}
	return FALSE;
}

BOOL CSQLServer::CloseDB()
{
	::CoUninitialize();
	if (m_db.IsOpen())
	{
		m_db.Close();
		return TRUE;
	}
	return FALSE;
}

BOOL CSQLServer::IsExist(EMAIL_ITEM& email)
{
	COleDateTime oledt = COleDateTime::GetCurrentTime();
	CString csSQL, csLog;
	TCHAR szDate[32] = { 0 };
	try
	{
		auto pos = email.csUIDL.Find(_T("\\"));
		if (pos > 0)
			email.csUIDL = email.csUIDL.Mid(pos + 1);
		oledt.ParseDateTime(email.csDate);
		wsprintf(szDate, _T("%04d-%02d-%02d %02d:%02d:%02d"), oledt.GetYear(), oledt.GetMonth(), oledt.GetDay()
			, oledt.GetHour(), oledt.GetMinute(), oledt.GetSecond());
		csSQL.Format(_T("SELECT top 1 EMailID from [ReportEmailDB].[dbo].[T_REPORT_EMAIL] where  EmailUIDL='%s' and EMailTo='%s'and EMailDate='%s'"),
			email.csUIDL, email.csTo, szDate);
		if (m_pEMRs->Open(csSQL, CADORecordset::openQuery) == FALSE) return FALSE;
		if (m_pEMRs->GetRecordCount() > 0)
		{
			m_pEMRs->GetFieldValue(_T("EMailID"), email.lSn);
			return TRUE;
		}
	}
	catch (const _com_error& e)
	{
		TRACE(_T("\r\n%s"), (TCHAR*)e.ErrorMessage());
		TRACE(_T("\r\n%s"), (TCHAR*)e.Description());
		csLog.Format(_T("IsExistError:[%s]\r\n[%s]\r\n[%s]"), csSQL, (TCHAR*)e.ErrorMessage(), (TCHAR*)e.Description());
		Log(csLog, csLog.GetLength());
		csLog.Append(_T("\r\n"));
		OutputDebugString(csLog);
		return FALSE;
	}
	return FALSE;
}

long CSQLServer::SaveAttachment(ATTACH_FILE& attach, long lEmailID)
{
	BOOL bRet = FALSE;
	CFile file;
	CString csGuid,csLog;
	GetGUID(csGuid);
	if (csGuid.IsEmpty() || attach.csLocalFileName.IsEmpty())
		return -1;
	attach.csGUID = csGuid;
	auto pos = attach.csLocalFileName.ReverseFind(_T('.'));
	if (pos > 0)
	{
		attach.csRemoteName.Format(_T("%s%s"), csGuid, attach.csLocalFileName.Mid(pos));
		attach.csRemoteName.Replace(_T("{"), _T(""));
		attach.csRemoteName.Replace(_T("}"), _T(""));
	}
	try
	{
		if (file.Open(attach.csFilePath, CFile::modeRead))
		{
			attach.lSize = file.GetLength();
			file.Close();
			if (attach.lSize <= 0)
			{
				attach.csRemoteName.Empty();
				return -1;
			}
		}
		else
		{
			csLog.Format(_T("Open[%s]Error"), attach.csFilePath);
			Log(csLog, csLog.GetLength());
			csLog.Append(_T("\r\n"));
			OutputDebugString(csLog);
			attach.csRemoteName.Empty();
			return -1;
		}
		CADOCommand * pEMAttachCmd = new CADOCommand(&m_db);
		if (pEMAttachCmd == NULL) throw;
		TCHAR szAttachCmdText[512] = _T("INSERT INTO [ReportEmailDB].[dbo].[T_REPORT_FILES](GUID, FileName, FileSize,AffixType,EmailID) VALUES(CONVERT(UNIQUEIDENTIFIER, ?), ?, ?,?,?)");
		pEMAttachCmd->AddParameter(_T("GUID"), adGUID, CADOParameter::paramInput, csGuid.GetLength()*sizeof(TCHAR), _bstr_t(csGuid.GetBuffer(0)));
		if (attach.csFileName.IsEmpty())
			attach.csFileName.Format(_T("unknow"));
		pEMAttachCmd->AddParameter(_T("FileName"), adVarChar, CADOParameter::paramInput, attach.csFileName.GetLength()*sizeof(TCHAR), _bstr_t(attach.csFileName.GetBuffer(0)));
		pEMAttachCmd->AddParameter(_T("FileSize"), adInteger, CADOParameter::paramInput, sizeof(long), attach.lSize);
		if (attach.csAffixType.IsEmpty())
			attach.csAffixType.Format(_T("unknow"));
		pEMAttachCmd->AddParameter(_T("AffixType"), adVarChar, CADOParameter::paramInput, attach.csAffixType.GetLength()*sizeof(TCHAR), _bstr_t(attach.csAffixType.GetBuffer(0)));
		pEMAttachCmd->AddParameter(_T("EMailID"), adInteger, CADOParameter::paramInput, sizeof(long), lEmailID);
		pEMAttachCmd->SetText(szAttachCmdText);
		if (!pEMAttachCmd->Execute(adCmdText))
		{
			csLog.Format(_T("Insert to [T_REPORT_FILES] Error![%s]"),attach.csFilePath);
			Log(csLog, csLog.GetLength());
			csLog.Append(_T("\r\n"));
			OutputDebugString(csLog);
			bRet = TRUE;
		}
		delete pEMAttachCmd;
		if (bRet)
			return -1;
	}
	catch (const _com_error&e)
	{
		TRACE(_T("\r\n%s"), (TCHAR*)e.ErrorMessage());
		TRACE(_T("\r\n%s"), (TCHAR*)e.Description());
		csLog.Format(_T("Insert to [T_REPORT_FILES] Error![%s]\r\n%s\r\n%s"), attach.csFilePath
			, (TCHAR*)e.ErrorMessage(), (TCHAR*)e.Description());
		Log(csLog, csLog.GetLength());
		csLog.Append(_T("\r\n"));
		OutputDebugString(csLog);
		return -1;
	}
	try
	{
		CADOCommand * pMapCmd = new CADOCommand(&m_db);
		if (pMapCmd == NULL) throw;
		TCHAR szMapCmdText[512] = _T("INSERT INTO [ReportEmailDB].[dbo].[T_FILE_AND_REPORT](GUID, EmailID) VALUES(CONVERT(UNIQUEIDENTIFIER, ?), ?)");
		pMapCmd->AddParameter(_T("GUID"), adGUID, CADOParameter::paramInput, csGuid.GetLength(), _bstr_t(csGuid.GetBuffer(0)));
		pMapCmd->AddParameter(_T("EMailID"), adInteger, CADOParameter::paramInput, sizeof(long), lEmailID);
		pMapCmd->SetText(szMapCmdText);
		if (!pMapCmd->Execute(adCmdText))
		{
			csLog.Format(_T("Insert to [T_FILE_AND_REPORT] Error![%s]"), attach.csFilePath);
			Log(csLog, csLog.GetLength());
			csLog.Append(_T("\r\n"));
			OutputDebugString(csLog);
			bRet = TRUE;
		}
		delete pMapCmd;
		if (bRet)
			return -1;
	}
	catch (const _com_error& e)
	{
		TRACE(_T("\r\n%s"), (TCHAR*)e.ErrorMessage());
		TRACE(_T("\r\n%s"), (TCHAR*)e.Description());
		csLog.Format(_T("Insert to [T_FILE_AND_REPORT] Error![%s]\r\n%s\r\n%s"), attach.csFilePath
			, (TCHAR*)e.ErrorMessage(), (TCHAR*)e.Description());
		Log(csLog, csLog.GetLength());
		csLog.Append(_T("\r\n"));
		OutputDebugString(csLog);
		return -1;
	}
	return 0;
}

void CSQLServer::SetLogPath(const char*pPath)
{
	if (pPath && pPath[0] != '\0')
		sprintf_s(m_chLogPath, MAX_PATH, "%s", pPath);
}

void CSQLServer::GetCurrTime(char* pTime, long lLen)
{
	SYSTEMTIME systm;
	GetLocalTime(&systm);
	sprintf_s(pTime, lLen, "%4d-%.2d-%.2d %.2d:%.2d:%.2d-",
		systm.wYear, systm.wMonth, systm.wDay,
		systm.wHour, systm.wMinute, systm.wSecond);
}

void CSQLServer::Log(LPCTSTR lpText, int nLen)
{
	if (strlen(m_chLogPath) <= 0)
		return;
	if ((lpText && nLen > 0))
	{
		char chTime[64] = { 0 };
		string strText;
		char *pText = NULL;
		pText = new char[nLen * 2 + 1];
		memset(pText, 0, nLen * 2 + 1);
		WideCharToMultiByte(CP_ACP, 0, lpText, (int)nLen, pText, (int)nLen * 2, NULL, NULL);
		GetCurrTime(chTime, 64);
		strText.append(chTime);
		strText.append(pText);
		strText.append("\r\n");

		if (fopen_s(&m_fp, m_chLogPath, "a+") == 0)
		{
			fseek(m_fp, 0, SEEK_END);
			fwrite(strText.c_str(), strText.length(), 1, m_fp);
			fclose(m_fp);
		}
		if (pText)
		{
			delete[] pText;
			pText = NULL;
		}
	}
}

BOOL CSQLServer::DeleteFromSQL(EMAIL_ITEM& email)
{
	BOOL bValue(TRUE);
	CADOCommand * pCmd = new CADOCommand(&m_db);
	CString csSql;
	do 
	{
		csSql.Format(_T("DELETE [dbo].[T_REPORT_EMAIL] WHERE EmailID = %d and EmailUIDL = '%s'")
			, email.lSn, email.csUIDL);
		if (!SQLExec(csSql))
		{
			bValue = FALSE;
			break;
		}
		csSql.Format(_T("DELETE [dbo].[T_FILE_AND_REPORT] WHERE EmailID =%d"), email.lSn);
		if (!SQLExec(csSql))
		{
			bValue = FALSE;
			break;
		}
		csSql.Format(_T("DELETE [dbo].[T_REPORT_FILES] WHERE EmailID = %d"), email.lSn);
		if (!SQLExec(csSql))
		{
			bValue = FALSE;
			break;
		}
	} while (0);

	return bValue;
}
///////////////////////////////////////////////////////
