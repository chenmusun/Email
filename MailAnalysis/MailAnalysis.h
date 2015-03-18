// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� MAILANALYSIS_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// MAILANALYSIS_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
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

#include "public.h"


// �����Ǵ� MailAnalysis.dll ������
class MAILANALYSIS_API CMailAnalysis {
public:
	CMailAnalysis(void);
	// TODO:  �ڴ�������ķ�����
	~CMailAnalysis();
	void Test();
	long LoadFile(LPCTSTR lpPath,LPCTSTR lpUIDL,long lType=0);//�����ļ�
	long AnalysisHead();//�����ʼ�ͷ
	long AnalysisBody(const CString& csBoundry, long long lCurrentRow);//�����ʼ���
	inline CString GetBoundry(){ return m_csBoundary; }//���Boundary�ֶ�
	inline __int64 GetHeadRowCount(){ return m_lHeadRowCount; }//��ȡͷ��Ϣ����
	inline long long GetCurrRow(){ return m_lCurrRow; }//��ȡ��ǰ������
	inline size_t GetAttachSize(){ return m_stAttachMent.size(); }//��ȡ������С
	long AnalysisBoundary(const CString& csBoundary, vector<ATTACH>& vAttach);//����
	long AnalysisBoundaryHead(list<CString>& lsHead, const CString& csBoundary, BOUNDARY_HEAD& info);
	inline vector<ATTACH>& GetAttach(){ return m_stAttachMent; }
	vector<ATTACH> AnalysisBody(const CString& csBody, const CString& csBoundry);//�����ʼ���
	void Clear(long lType=1);//lType----1:ɾ�������ļ���0�����������ļ�
	void SetLogPath(const char*pPath);
	inline void SetAbbreviation(LPCTSTR lpAbb){ m_csAbbreviation.Format(_T("%s"), lpAbb); }
	inline EMAIL_ITEM& GetEmailItem(){ return m_stEmail; }
	void GetContentType(long lContentType,CString& csContentType);
private:
	long GetContentInfo(const CString& csSrc, CString& csContent, CString& csExtra, long& lConttype);
	void GetDispositionInfo(const CString& csSrc, CString& csDis, CString& csExtra);
	void GetDescription(const CStringArray& csaSrc, CString& csDescription);
	inline void SetMainPath(LPCTSTR lpPath){ wsprintf(m_szMainPath, _T("%s"), lpPath); }//���õ�ǰ·��
	long SaveToFile(const CString& csCode, LPCTSTR lpFileName, int nCharset, int nCodeType);//�����ļ�
	long SaveToFile(const CString& csCode, LPCTSTR lpFileName, int nCodeType);//�����ļ�
private:
	CStringArray m_csText;//ԭ��
	long long m_lHeadRowCount;//
	long long m_lCurrRow;
	CString m_csBoundary;//�ָ���
	vector<ATTACH> m_stAttachMent;//������Ϣ
	vector<ATTACH> m_stSubAttachMent;
	
	CString m_cscsContentType;
	CString m_csEncoding;
	CString m_csUIDL;
	CString m_csMessageID;
	CString m_csFilePath;//EML�ļ�·��
	CString m_csSavePath;//��������·��
	CString m_csCharset; //Դ�ַ���
	//list<CString> m_lsAttachFile;
	long m_lAttachmentCount; //��������
	TCHAR m_szMainPath[MAX_PATH];
	CLog m_log;
	EMAIL_ITEM m_stEmail;
	CString m_csAbbreviation;
};


BOOL StringProcess(const CString& csSrc, CString& csDest);
void StringProcess(const CString& csSrc, CString& csDest, CString& csExtra, int nFlag);
void StringProcess(const CString&csSrc, CString& csDest, int nType);
void CodeConvert(const CString& csSrc, CString&csDest, int nCharset = UTF8, int nCodetype = BASE64);
void CodeConvert(const CString& csSrc, char *pDest, int nCharset = UTF8, int nCodetype = BASE64);
long GetKeyWords(const CString&csSrc1, const CString& csSrc2, LPCTSTR lpKey, LPCTSTR lpEnd, CString& csDest);


CString StringEncode(const CString& csSrc);

extern MAILANALYSIS_API int nMailAnalysis;

MAILANALYSIS_API int fnMailAnalysis(void);

void FormatFileName(CString& csFileName);
