// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 MAILANALYSIS_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// MAILANALYSIS_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#pragma once
#ifdef MAILANALYSIS_EXPORTS
#define MAILANALYSIS_API __declspec(dllexport)
#else
#define MAILANALYSIS_API __declspec(dllimport)
#pragma	comment(lib,"../output/MailAnalysis.lib")
#endif

#include <string>
#include <vector>
#include <list>
#include <map>
#include "../ReceiveEmail/public.h"
#include "../Log/Log.h"
using namespace std;
#include "pdf2text.h"

#include "public.h"


// 此类是从 MailAnalysis.dll 导出的
class MAILANALYSIS_API CMailAnalysis {
public:
	CMailAnalysis(void);
	// TODO:  在此添加您的方法。
	~CMailAnalysis();
	void Test();
	long LoadFile(LPCTSTR lpPath,LPCTSTR lpUIDL,long lType=0);//加载文件
	long AnalysisHead();//解析邮件头
	long AnalysisBody(const CString& csBoundry, long long lCurrentRow);//解析邮件体
	inline CString GetBoundry(){ return m_csBoundary; }//获得Boundary字段
	inline __int64 GetHeadRowCount(){ return m_lHeadRowCount; }//获取头信息行数
	inline long long GetCurrRow(){ return m_lCurrRow; }//获取当前处理行
	inline size_t GetAttachSize(){ return m_stAttachMent.size(); }//获取附件大小
	long AnalysisBoundary(const CString& csBoundary, vector<ATTACH>& vAttach);//解析
	long AnalysisBoundaryHead(list<CString>& lsHead, const CString& csBoundary, BOUNDARY_HEAD& info);
	inline vector<ATTACH>& GetAttach(){ return m_stAttachMent; }
	vector<ATTACH> AnalysisBody(const CString& csBody, const CString& csBoundry);//解析邮件体
	void Clear();
	void SetLogPath(const char*pPath);
	inline void SetAbbreviation(LPCTSTR lpAbb){ m_csAbbreviation.Format(_T("%s"), lpAbb); }
	inline EMAIL_ITEM& GetEmailItem(){ return m_stEmail; }
	friend void SaveAttachMent(CMailAnalysis* pana, ATTACH_FILE& attachfile, BOUNDARY_HEAD& stBouHead, vector<ATTACH>::iterator& ite);
	inline void SetClearType(long lType){ m_lClearType = lType; }
	long MailAnalysis(const CString& csFileName);
private:
	long GetContentInfo(const CString& csSrc, CString& csContent, CString& csExtra, long& lConttype);
	void GetDispositionInfo(const CString& csSrc, CString& csDis, CString& csExtra);
	void GetDescription(const CStringArray& csaSrc, CString& csDescription);
	inline void SetMainPath(LPCTSTR lpPath){ wsprintf(m_szMainPath, _T("%s"), lpPath); }//设置当前路径
	long SaveToFile(CString& csCode, LPCTSTR lpFileName, int nCharset, int nCodeType);//保存文件
	long SaveToFile(CString& csCode, LPCTSTR lpFileName, int nCodeType);//保存文件
private:
	CStringArray m_csText;//原文
	long long m_lHeadRowCount;//
	long long m_lCurrRow;
	CString m_csBoundary;//分隔符
	vector<ATTACH> m_stAttachMent;//附件信息
	vector<ATTACH> m_stSubAttachMent;
	
	CString m_cscsContentType;
	CString m_csEncoding;
	CString m_csUIDL;
	CString m_csMessageID;
	CString m_csFilePath;//EML文件路径
	CString m_csSavePath;//附件保存路径
	CString m_csSubSavePath;
	CString m_csCharset; //源字符集
	//list<CString> m_lsAttachFile;
	long m_lAttachmentCount; //附件数量
	TCHAR m_szMainPath[MAX_PATH];
	CLog m_log;
	EMAIL_ITEM m_stEmail;
	CString m_csAbbreviation;
	long m_lClearType;//1:删除本地文件；0：保留本地文件
};


BOOL StringProcess(const CString& csSrc, CString& csDest);
void StringProcess(const CString& csSrc, CString& csDest, CString& csExtra, int nFlag);
void StringProcess(const CString&csSrc, CString& csDest, int nType);
void CodeConvert(const CString& csSrc, CString&csDest, int nCharset = UTF8, int nCodetype = BASE64);
void CodeConvert(const CString& csSrc, char *pDest, int nCharset = UTF8, int nCodetype = BASE64);
long GetKeyWords(const CString&csSrc1, const CString& csSrc2, LPCTSTR lpKey, LPCTSTR lpEnd, CString& csDest,int nStart=0);


CString StringEncode(const CString& csSrc);

extern MAILANALYSIS_API int nMailAnalysis;

MAILANALYSIS_API int fnMailAnalysis(void);

void FormatFileName(CString& csFileName);

void SaveAttachMent(CMailAnalysis* pana, ATTACH_FILE& attachfile, BOUNDARY_HEAD& stBouHead,  vector<ATTACH>::iterator& ite);

MAILANALYSIS_API int PDF2TXT(string inputFilePath, string outputFilePath, string& outputname, int& nPageNum, int& time);