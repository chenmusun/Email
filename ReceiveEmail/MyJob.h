#pragma once

#include "POP3.h"
#include "../Log/Log.h"
#include "SendEmail.h"
#include "../MailAnalysis/MailAnalysis.h"
class CReceiveEmailDlg;

struct MyJobParam
{
	LONG		m_nIntParam;
	CString		m_nStrParam;
	CReceiveEmailDlg* m_pDlg;
	long		m_lPos;
};


class CMyJob : public FTL::CFJobBase<MyJobParam*>
{
	DISABLE_COPY_AND_ASSIGNMENT(CMyJob);
public:
	CMyJob(MyJobParam* pMyJobParam);
	CMyJob();
	~CMyJob();
	//需要重载的Job函数

	virtual BOOL Initialize();
	virtual BOOL Run();
	virtual VOID Finalize();
	virtual void OnCancelJob();
	BOOL Wait();
	CLog m_log;
	long SendEmail();
	long MailAnalysis(POP3& pop3, const string& strUIDL, long lType = 0);
private:
	POP3 m_pop3;
	SMTP m_smtp;
	vector<string> m_UidlData;
};
