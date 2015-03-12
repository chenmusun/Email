// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 SQLSERVER_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// SQLSERVER_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#pragma once
#ifdef SQLSERVER_EXPORTS
#define SQLSERVER_API __declspec(dllexport)
#else
#define SQLSERVER_API __declspec(dllimport)
#pragma	comment(lib,"../output/SQLServer.lib")
#endif

#import "C:\Program Files\Common Files\System\ado\msado25.tlb" rename("EOF", "EndOfFile")
//#import "C:\Program Files\Common Files\System\ado\MSJRO.DLL" no_namespace rename("ReplicaTypeEnum", "_ReplicaTypeEnum")
using namespace ADODB;
#include "../MailAnalysis/public.h"

// 此类是从 SQLServer.dll 导出的
class CADOCommand;
struct CADOFieldInfo
{
	TCHAR m_strName[30];
	short m_nType;
	long m_lSize;
	long m_lDefinedSize;
	long m_lAttributes;
	short m_nOrdinalPosition;
	BOOL m_bRequired;
	BOOL m_bAllowZeroLength;
	long m_lCollatingOrder;
	short m_nPrecision;
};

CString IntToStr(int nVal);

CString LongToStr(long lVal);

CString ULongToStr(unsigned long ulVal);

CString DblToStr(double dblVal, int ndigits = 20);

CString DblToStr(float fltVal);


class CADODatabase
{
public:
	enum cadoConnectModeEnum
	{
		connectModeUnknown = adModeUnknown,
		connectModeRead = adModeRead,
		connectModeWrite = adModeWrite,
		connectModeReadWrite = adModeReadWrite,
		connectModeShareDenyRead = adModeShareDenyRead,
		connectModeShareDenyWrite = adModeShareDenyWrite,
		connectModeShareExclusive = adModeShareExclusive,
		connectModeShareDenyNone = adModeShareDenyNone
	};

	enum cadoTransactionStatusEnum
	{
		transactionStatusUnknown = 0,
		transactionStatusBegin = 1,
		transactionStatusRollback = 2,
		transactionStatusCommit = 3
	};

	CADODatabase()
	{
		::CoInitialize(NULL);

		m_pConnection = NULL;
		m_strConnection = _T("");
		m_dwTransStatus = transactionStatusUnknown;
		m_strLastError = _T("");
		m_dwLastError = 0;
		m_pConnection.CreateInstance(__uuidof(Connection));
		m_nRecordsAffected = 0;
		m_nConnectionTimeout = 0;
	}

	virtual ~CADODatabase()
	{
		Close();
		m_pConnection.Release();
		m_pConnection = NULL;
		m_dwTransStatus = transactionStatusUnknown;
		m_strConnection = _T("");
		m_strLastError = _T("");
		m_dwLastError = 0;
		::CoUninitialize();
	}

	BOOL Open(LPCTSTR lpstrConnection = _T(""), LPCTSTR lpstrUserID = _T(""), LPCTSTR lpstrPassword = _T(""));
	_ConnectionPtr GetActiveConnection()
	{
		return m_pConnection;
	};
	BOOL Execute(LPCTSTR lpstrExec);
	int GetRecordsAffected()
	{
		return m_nRecordsAffected;
	};
	DWORD GetRecordCount(_RecordsetPtr m_pRs);
	long BeginTransaction();
	long CommitTransaction();
	long RollbackTransaction();
	DWORD GetTransStatus();
	BOOL IsOpen();
	void Close();
	void SetConnectionMode(cadoConnectModeEnum nMode)
	{
		m_pConnection->PutMode((enum ConnectModeEnum)nMode);
	};
	void SetConnectionString(LPCTSTR lpstrConnection)
	{
		m_strConnection = lpstrConnection;
	};
	CString GetConnectionString()
	{
		return m_strConnection;
	};
	CString GetLastErrorString()
	{
		return m_strLastError;
	};
	DWORD GetLastError()
	{
		return m_dwLastError;
	};
	CString GetErrorDescription()
	{
		return m_strErrorDescription;
	};
	void SetConnectionTimeout(long nConnectionTimeout = 30)
	{
		m_nConnectionTimeout = nConnectionTimeout;
	};

protected:
	void dump_com_error(_com_error &e);

public:
	_ConnectionPtr m_pConnection;

protected:
	CString m_strConnection;
	CString m_strLastError;
	CString m_strErrorDescription;
	DWORD m_dwLastError;
	DWORD m_dwTransStatus;
	int m_nRecordsAffected;
	long m_nConnectionTimeout;
};

class CADORecordset
{
public:
	BOOL Clone(CADORecordset& pRs);

	enum cadoOpenEnum
	{
		openUnknown = 0,
		openQuery = 1,
		openTable = 2,
		openStoredProc = 3
	};

	enum cadoEditEnum
	{
		dbEditNone = 0,
		dbEditNew = 1,
		dbEdit = 2
	};

	enum cadoPositionEnum
	{

		positionUnknown = -1,
		positionBOF = -2,
		positionEOF = -3
	};

	enum cadoSearchEnum
	{
		searchForward = 1,
		searchBackward = -1
	};

	enum cadoDataType
	{
		typeEmpty = ADODB::adEmpty,
		typeTinyInt = ADODB::adTinyInt,
		typeSmallInt = ADODB::adSmallInt,
		typeInteger = ADODB::adInteger,
		typeBigInt = ADODB::adBigInt,
		typeUnsignedTinyInt = ADODB::adUnsignedTinyInt,
		typeUnsignedSmallInt = ADODB::adUnsignedSmallInt,
		typeUnsignedInt = ADODB::adUnsignedInt,
		typeUnsignedBigInt = ADODB::adUnsignedBigInt,
		typeSingle = ADODB::adSingle,
		typeDouble = ADODB::adDouble,
		typeCurrency = ADODB::adCurrency,
		typeDecimal = ADODB::adDecimal,
		typeNumeric = ADODB::adNumeric,
		typeBoolean = ADODB::adBoolean,
		typeError = ADODB::adError,
		typeUserDefined = ADODB::adUserDefined,
		typeVariant = ADODB::adVariant,
		typeIDispatch = ADODB::adIDispatch,
		typeIUnknown = ADODB::adIUnknown,
		typeGUID = ADODB::adGUID,
		typeDate = ADODB::adDate,
		typeDBDate = ADODB::adDBDate,
		typeDBTime = ADODB::adDBTime,
		typeDBTimeStamp = ADODB::adDBTimeStamp,
		typeBSTR = ADODB::adBSTR,
		typeChar = ADODB::adChar,
		typeVarChar = ADODB::adVarChar,
		typeLongVarChar = ADODB::adLongVarChar,
		typeWChar = ADODB::adWChar,
		typeVarWChar = ADODB::adVarWChar,
		typeLongVarWChar = ADODB::adLongVarWChar,
		typeBinary = ADODB::adBinary,
		typeVarBinary = ADODB::adVarBinary,
		typeLongVarBinary = ADODB::adLongVarBinary,
		typeChapter = ADODB::adChapter,
		typeFileTime = ADODB::adFileTime,
		typePropVariant = ADODB::adPropVariant,
		typeVarNumeric = ADODB::adVarNumeric,
		typeArray = ADODB::adVariant
	};

	enum cadoSchemaType
	{
		schemaSpecific = adSchemaProviderSpecific,
		schemaAsserts = adSchemaAsserts,
		schemaCatalog = adSchemaCatalogs,
		schemaCharacterSet = adSchemaCharacterSets,
		schemaCollections = adSchemaCollations,
		schemaColumns = adSchemaColumns,
		schemaConstraints = adSchemaCheckConstraints,
		schemaConstraintColumnUsage = adSchemaConstraintColumnUsage,
		schemaConstraintTableUsage = adSchemaConstraintTableUsage,
		shemaKeyColumnUsage = adSchemaKeyColumnUsage,
		schemaTableConstraints = adSchemaTableConstraints,
		schemaColumnsDomainUsage = adSchemaColumnsDomainUsage,
		schemaIndexes = adSchemaIndexes,
		schemaColumnPrivileges = adSchemaColumnPrivileges,
		schemaTablePrivileges = adSchemaTablePrivileges,
		schemaUsagePrivileges = adSchemaUsagePrivileges,
		schemaProcedures = adSchemaProcedures,
		schemaTables = adSchemaTables,
		schemaProviderTypes = adSchemaProviderTypes,
		schemaViews = adSchemaViews,
		schemaViewTableUsage = adSchemaViewTableUsage,
		schemaProcedureParameters = adSchemaProcedureParameters,
		schemaForeignKeys = adSchemaForeignKeys,
		schemaPrimaryKeys = adSchemaPrimaryKeys,
		schemaProcedureColumns = adSchemaProcedureColumns,
		schemaDBInfoKeywords = adSchemaDBInfoKeywords,
		schemaDBInfoLiterals = adSchemaDBInfoLiterals,
		schemaCubes = adSchemaCubes,
		schemaDimensions = adSchemaDimensions,
		schemaHierarchies = adSchemaHierarchies,
		schemaLevels = adSchemaLevels,
		schemaMeasures = adSchemaMeasures,
		schemaProperties = adSchemaProperties,
		schemaMembers = adSchemaMembers,
	};

	BOOL SetFieldValue(int nIndex, int nValue);
	BOOL SetFieldValue(LPCTSTR lpFieldName, int nValue);
	BOOL SetFieldValue(int nIndex, long lValue);
	BOOL SetFieldValue(LPCTSTR lpFieldName, long lValue);
	BOOL SetFieldValue(int nIndex, unsigned long lValue);
	BOOL SetFieldValue(LPCTSTR lpFieldName, unsigned long lValue);
	BOOL SetFieldValue(int nIndex, double dblValue);
	BOOL SetFieldValue(LPCTSTR lpFieldName, double dblValue);
	BOOL SetFieldValue(int nIndex, CString strValue);
	BOOL SetFieldValue(LPCTSTR lpFieldName, CString strValue);
	BOOL SetFieldValue(int nIndex, COleDateTime time);
	BOOL SetFieldValue(LPCTSTR lpFieldName, COleDateTime time);
	BOOL SetFieldValue(int nIndex, bool bValue);
	BOOL SetFieldValue(LPCTSTR lpFieldName, bool bValue);
	BOOL SetFieldValue(int nIndex, COleCurrency cyValue);
	BOOL SetFieldValue(LPCTSTR lpFieldName, COleCurrency cyValue);
	BOOL SetFieldValue(int nIndex, _variant_t vtValue);
	BOOL SetFieldValue(LPCTSTR lpFieldName, _variant_t vtValue);

	BOOL SetFieldEmpty(int nIndex);
	BOOL SetFieldEmpty(LPCTSTR lpFieldName);

	void CancelUpdate();
	BOOL Update();
	void Edit();
	BOOL AddNew();

	BOOL Find(LPCTSTR lpFind, int nSearchDirection = CADORecordset::searchForward);
	BOOL FindFirst(LPCTSTR lpFind);
	BOOL FindNext();

	CADORecordset();

	CADORecordset(CADODatabase* pAdoDatabase);

	virtual ~CADORecordset()
	{
		Close();
		if (m_pRecordset)
			m_pRecordset.Release();
		if (m_pCmd)
			m_pCmd.Release();
		m_pRecordset = NULL;
		m_pCmd = NULL;
		m_strQuery = _T("");
		m_strLastError = _T("");
		m_dwLastError = 0;
		m_nEditStatus = dbEditNone;
	}

	CString GetQuery()
	{
		return m_strQuery;
	};
	void SetQuery(LPCSTR strQuery)
	{
		m_strQuery = strQuery;
	};
	DWORD GetRecordCount();
	BOOL IsOpen();
	void Close();
	BOOL Open(_ConnectionPtr mpdb, LPCTSTR lpstrExec = _T(""), int nOption = CADORecordset::openUnknown);
	BOOL Open(LPCTSTR lpstrExec = _T(""), int nOption = CADORecordset::openUnknown);
	BOOL OpenSchema(int nSchema, LPCTSTR SchemaID = _T(""));
	long GetFieldCount()
	{
		return m_pRecordset->Fields->GetCount();
	};
	BOOL GetFieldValue(LPCTSTR lpFieldName, int& nValue);
	BOOL GetFieldValue(int nIndex, int& nValue);
	BOOL GetFieldValue(LPCTSTR lpFieldName, long& lValue);
	BOOL GetFieldValue(int nIndex, long& lValue);
	BOOL GetFieldValue(LPCTSTR lpFieldName, unsigned long& ulValue);
	BOOL GetFieldValue(int nIndex, unsigned long& ulValue);
	BOOL GetFieldValue(LPCTSTR lpFieldName, double& dbValue);
	BOOL GetFieldValue(int nIndex, double& dbValue);
	BOOL GetFieldValue(LPCTSTR lpFieldName, CString& strValue, CString strDateFormat = _T(""));
	BOOL GetFieldValue(int nIndex, CString& strValue, CString strDateFormat = _T(""));
	BOOL GetFieldValue(LPCTSTR lpFieldName, COleDateTime& time);
	BOOL GetFieldValue(int nIndex, COleDateTime& time);
	BOOL GetFieldValue(int nIndex, bool& bValue);
	BOOL GetFieldValue(LPCTSTR lpFieldName, bool& bValue);
	BOOL GetFieldValue(int nIndex, COleCurrency& cyValue);
	BOOL GetFieldValue(LPCTSTR lpFieldName, COleCurrency& cyValue);
	BOOL GetFieldValue(int nIndex, _variant_t& vtValue);
	BOOL GetFieldValue(LPCTSTR lpFieldName, _variant_t& vtValue);

	BOOL GetFieldValue(int nIndex, INT64& i64Value);

	BOOL IsFieldNull(LPCTSTR lpFieldName);
	BOOL IsFieldNull(int nIndex);
	BOOL IsFieldEmpty(LPCTSTR lpFieldName);
	BOOL IsFieldEmpty(int nIndex);
	BOOL IsEof()
	{
		return m_pRecordset->EndOfFile == VARIANT_TRUE;
	};
	BOOL IsEOF()
	{
		return m_pRecordset->EndOfFile == VARIANT_TRUE;
	};
	BOOL IsBof()
	{
		return m_pRecordset->BOF == VARIANT_TRUE;
	};
	BOOL IsBOF()
	{
		return m_pRecordset->BOF == VARIANT_TRUE;
	};
	void MoveFirst()
	{
		m_pRecordset->MoveFirst();
	};
	void MoveNext()
	{
		m_pRecordset->MoveNext();
	};
	void MovePrevious()
	{
		m_pRecordset->MovePrevious();
	};
	void MoveLast()
	{
		m_pRecordset->MoveLast();
	};
	long GetAbsolutePage()
	{
		return m_pRecordset->GetAbsolutePage();
	};
	void SetAbsolutePage(int nPage)
	{
		m_pRecordset->PutAbsolutePage((enum PositionEnum)nPage);
	};
	long GetPageCount()
	{
		return m_pRecordset->GetPageCount();
	};
	long GetPageSize()
	{
		return m_pRecordset->GetPageSize();
	};
	void SetPageSize(int nSize)
	{
		m_pRecordset->PutPageSize(nSize);
	};
	long GetAbsolutePosition()
	{
		return m_pRecordset->GetAbsolutePosition();
	};
	void SetAbsolutePosition(int nPosition)
	{
		m_pRecordset->PutAbsolutePosition((enum PositionEnum)nPosition);
	};
	BOOL GetFieldInfo(LPCTSTR lpFieldName, CADOFieldInfo* fldInfo);
	BOOL GetFieldInfo(int nIndex, CADOFieldInfo* fldInfo);
	BOOL AppendChunk(LPCTSTR lpFieldName, LPVOID lpData, UINT nBytes);
	BOOL AppendChunk(int nIndex, LPVOID lpData, UINT nBytes);

	BOOL GetChunk(LPCTSTR lpFieldName, CString& strValue);
	BOOL GetChunk(int nIndex, CString& strValue);

	BOOL GetChunk(LPCTSTR lpFieldName, LPVOID pData);
	BOOL GetChunk(int nIndex, LPVOID pData);

	CString GetString(LPCTSTR lpCols, LPCTSTR lpRows, LPCTSTR lpNull, long numRows = 0);
	CString GetLastErrorString()
	{
		return m_strLastError;
	};
	DWORD GetLastError()
	{
		return m_dwLastError;
	};
	void GetBookmark()
	{
		m_varBookmark = m_pRecordset->Bookmark;
	};
	BOOL SetBookmark();
	BOOL Delete();
	BOOL IsConnectionOpen()
	{
		return m_pConnection != NULL && m_pConnection->GetState() != adStateClosed;
	};
	_RecordsetPtr GetRecordset()
	{
		return m_pRecordset;
	};
	_ConnectionPtr GetActiveConnection()
	{
		return m_pConnection;
	};

	BOOL SetFilter(LPCTSTR strFilter);
	BOOL SetSort(LPCTSTR lpstrCriteria);
	BOOL SaveAsXML(LPCTSTR lpstrXMLFile);
	BOOL OpenXML(LPCTSTR lpstrXMLFile);
	BOOL Execute(CADOCommand* pCommand);
	BOOL Requery();

public:
	_RecordsetPtr m_pRecordset;
	_CommandPtr m_pCmd;

protected:
	_ConnectionPtr m_pConnection;
	int m_nSearchDirection;
	CString m_strFind;
	_variant_t m_varBookFind;
	_variant_t m_varBookmark;
	int m_nEditStatus;
	CString m_strLastError;
	DWORD m_dwLastError;
	void dump_com_error(_com_error &e);
	CString m_strQuery;

protected:
	BOOL PutFieldValue(LPCTSTR lpFieldName, _variant_t vtFld);
	BOOL PutFieldValue(_variant_t vtIndex, _variant_t vtFld);
	BOOL GetFieldInfo(FieldPtr pField, CADOFieldInfo* fldInfo);
	BOOL GetChunk(FieldPtr pField, CString& strValue);
	BOOL GetChunk(FieldPtr pField, LPVOID lpData);
	BOOL AppendChunk(FieldPtr pField, LPVOID lpData, UINT nBytes);

};

class CADOParameter
{
public:

	enum cadoParameterDirection
	{
		paramUnknown = adParamUnknown,
		paramInput = adParamInput,
		paramOutput = adParamOutput,
		paramInputOutput = adParamInputOutput,
		paramReturnValue = adParamReturnValue
	};

	CADOParameter(int nType, long lSize = 0, int nDirection = paramInput, CString strName = _T(""));

	virtual ~CADOParameter()
	{
		m_pParameter.Release();
		m_pParameter = NULL;
		m_strName = _T("");
	}

	BOOL SetValue(int nValue);
	BOOL SetValue(long lValue);
	BOOL SetValue(double dbValue);
	BOOL SetValue(CString strValue);
	BOOL SetValue(COleDateTime time);
	BOOL SetValue(_variant_t vtValue);
	BOOL SetChunkValue(LPVOID lpData, long lSize);
	BOOL GetValue(int& nValue);
	BOOL GetValue(long& lValue);
	BOOL GetValue(double& dbValue);
	BOOL GetValue(CString& strValue, CString strDateFormat = _T(""));
	BOOL GetValue(COleDateTime& time);
	BOOL GetValue(_variant_t& vtValue);
	void SetPrecision(int nPrecision)
	{
		m_pParameter->PutPrecision(nPrecision);
	};
	void SetScale(int nScale)
	{
		m_pParameter->PutNumericScale(nScale);
	};

	void SetName(CString strName)
	{
		m_strName = strName;
	};
	CString GetName()
	{
		return m_strName;
	};
	int GetType()
	{
		return m_nType;
	};
	_ParameterPtr GetParameter()
	{
		return m_pParameter;
	};

protected:
	void dump_com_error(_com_error &e);

protected:
	_ParameterPtr m_pParameter;
	CString m_strName;
	int m_nType;
	CString m_strLastError;
	DWORD m_dwLastError;
};

class CADOCommand
{
public:
	enum cadoCommandType
	{
		typeCmdText = adCmdText,
		typeCmdTable = adCmdTable,
		typeCmdTableDirect = adCmdTableDirect,
		typeCmdStoredProc = adCmdStoredProc,
		typeCmdUnknown = adCmdUnknown,
		typeCmdFile = adCmdFile
	};

	CADOCommand(CADODatabase* pAdoDatabase, CString strCommandText = _T(""), int nCommandType = typeCmdStoredProc);

	virtual ~CADOCommand()
	{
		::SysFreeString(m_pCommand->CommandText);
		m_pCommand.Release();
		m_pCommand = NULL;
		m_strCommandText = _T("");
	}

	void SetTimeout(long nTimeOut)
	{
		m_pCommand->PutCommandTimeout(nTimeOut);
	};
	void SetText(CString strCommandText);
	void SetType(int nCommandType);
	int GetType()
	{
		return m_nCommandType;
	};
	BOOL AddParameter(CADOParameter* pAdoParameter);
	BOOL AddParameter(CString strName, int nType, int nDirection, long lSize, int nValue);
	BOOL AddParameter(CString strName, int nType, int nDirection, long lSize, long lValue);
	BOOL AddParameter(CString strName, int nType, int nDirection, long lSize, double dblValue, int nPrecision = 0, int nScale = 0);
	BOOL AddParameter(CString strName, int nType, int nDirection, long lSize, CString strValue);
	BOOL AddParameter(CString strName, int nType, int nDirection, long lSize, COleDateTime time);
	BOOL AddParameter(CString strName, int nType, int nDirection, long lSize, _variant_t vtValue, int nPrecision = 0, int nScale = 0);
	BOOL AddChunkParam(CString strName, int nType, int nDirection, long lSize, LPVOID lpData);
	CString GetText()
	{
		return m_strCommandText;
	};
	BOOL Execute(int nCommandType = typeCmdStoredProc);
	int GetRecordsAffected()
	{
		return m_nRecordsAffected;
	};
	_CommandPtr GetCommand()
	{
		return m_pCommand;
	};

protected:
	void dump_com_error(_com_error &e);

protected:
	_CommandPtr m_pCommand;
	int m_nCommandType;
	int m_nRecordsAffected;
	CString m_strCommandText;
	CString m_strLastError;
	DWORD m_dwLastError;
};

/////////////////////////////////////////////////////////////////////
//
//		CADOException Class
//

class CADOException
{
public:
	CADOException() :
		m_lErrorCode(0),
		m_strError(_T(""))
	{
	}

	CADOException(long lErrorCode) :
		m_lErrorCode(lErrorCode),
		m_strError(_T(""))
	{
	}

	CADOException(long lErrorCode, const CString& strError) :
		m_lErrorCode(lErrorCode),
		m_strError(strError)
	{
	}

	CADOException(const CString& strError) :
		m_lErrorCode(0),
		m_strError(strError)
	{
	}

	CADOException(long lErrorCode, const char* szError) :
		m_lErrorCode(lErrorCode),
		m_strError(szError)
	{
	}

	CADOException(const char* szError) :
		m_lErrorCode(0),
		m_strError(szError)
	{
	}

	virtual ~CADOException()
	{
	}

	CString GetErrorMessage() const
	{
		return m_strError;
	};
	void SetErrorMessage(LPCTSTR lpstrError = _T(""))
	{
		m_strError = lpstrError;
	};
	long GetError()
	{
		return m_lErrorCode;
	};
	void SetError(long lErrorCode = 0)
	{
		m_lErrorCode = lErrorCode;
	};

protected:
	CString m_strError;
	long m_lErrorCode;
};




class SQLSERVER_API CSQLServer {
public:
	CSQLServer(void);
	~CSQLServer();
	// TODO:  在此添加您的方法。
	void Test();
	void GetGUID(CString &guid);
	BOOL Connect(LPCTSTR lpServer,LPCTSTR lpDatabase,LPCTSTR lpUser,LPCTSTR lpPass,int nType=0);//0:混合验证；1:Windows验证
	BOOL IsConnect();
	BOOL ReConnect();
	BOOL SQLExec(LPCTSTR lpSql);
	BOOL CloseDB();
	BOOL IsExist();
	BOOL SaveToDB(const ATTACH_FILE& item);
private:
	CString m_csServer;
	CString m_csDatabase;
	CString m_csUser;
	CString m_csPass;
	int m_nType;
	CADODatabase m_db;
	CADORecordset *	m_pEMRs;
	CADORecordset *	m_pGuidRs;
	CADORecordset *	m_pSubjectRs;
public:
};

extern SQLSERVER_API int nSQLServer;

SQLSERVER_API int fnSQLServer(void);
