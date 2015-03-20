#include "StdAfx.h"
#include "MyJob.h"
#include "ReceiveEmailDlg.h"


extern TCHAR __Main_Path__[MAX_PATH];

CMyJob::CMyJob(void)
{
	
}

CMyJob::CMyJob(MyJobParam* pMyJobParam)
:CFJobBase<MyJobParam*>(pMyJobParam)
{
	m_UidlData.clear();
}

CMyJob::~CMyJob(void)
{
	m_pop3.SetParent(NULL);
	m_smtp.SetCurrPath(NULL);
}

BOOL CMyJob::Initialize()
{
	//此处进行初始化，如果失败，则需要释放资源并且返回 FALSE
	return TRUE;
}

BOOL CMyJob::Run()
{
	//这是Job的主要循环，常用的方式是“循环调用 GetJobWaitType(INFINITE) 直到返回值不为 ftwtContinue 或Job完成
	FTL::FTLThreadWaitType waitType = FTL::ftwtError;
	if ( FTL::ftwtContinue == (waitType = GetJobWaitType(INFINITE))) 
		//如果 GetJobWaitType 的参数是 INFINITE， 则支持Pause操作；
		//如果不想支持Pause(比如网络传输)，则可以传0。
		//但需要注意：如果 ThreadPool::Pause 的话，这个函数的返回值会是 ftwtTimeOut
	{
		char chAbbreviation[64] = { 0 },chDebug[512] = { 0 };;
		CString csDebug;
		TCHAR szLogPath[MAX_PATH] = { 0 };
		char chFilePath[MAX_PATH] = { 0 }, chLogPath[MAX_PATH] = {0};
		CString csInfo;
		MailBoxInfo info;
		MongoDBInfo dbinfo;
		ForwardSet fdsinfo;
		string strUidl,strName,strPath;
		double dPercent = 0.0;
		DWORD dSleepTime(50);
		memset(&info, 0, sizeof(MailBoxInfo));
		memset(&dbinfo, 0, sizeof(MongoDBInfo));
		//m_JobParam->m_pDlg->GetMailBoxInfo(m_JobParam->m_lPos, csInfo,info);
		wsprintf(szLogPath, _T("%s\\Log\\%s.txt"), __Main_Path__, info.szName);
		WideCharToMultiByte(CP_ACP, 0, szLogPath, MAX_PATH, chLogPath, MAX_PATH, NULL, NULL);
		m_log.SetPath(szLogPath, lstrlen(szLogPath));
		if (info.bSendMail)
		{
			memset(&fdsinfo, 0, sizeof(ForwardSet));
			m_JobParam->m_pDlg->GetForwardInfo(fdsinfo);
			m_smtp.SetForwardInfo(fdsinfo);
			WideCharToMultiByte(CP_ACP, 0, info.szMailAdd, 128, chFilePath, MAX_PATH, NULL, NULL);
			m_smtp.SetReceiver(chFilePath);
			m_smtp.SetLogPath(chLogPath);
		}
		WideCharToMultiByte(CP_ACP, 0, info.szAbbreviation, 32, chAbbreviation, 64, NULL, NULL);
		strName = chAbbreviation;
		//m_JobParam->m_pDlg->GetDataBaseInfo(dbinfo);
		//m_sql.Connect(_T("OFFICE-PC\\SQLSERVER"), _T("ReportEmailDB"), _T("sa"), _T("test.123"));
		if (!csInfo.IsEmpty())
		{
			m_pop3.SetParent((void*)this);
			m_pop3.SetLogPath(chLogPath);
			m_pop3.SetInfo(csInfo, info, dbinfo,__Main_Path__, lstrlen(__Main_Path__));
			long lResult = m_pop3.Login(info.szServerAdd,info.lPort,csInfo,info.szPasswd);
			if (lResult >= 0)
			{
				lResult = m_pop3.GetMailCount();
				if (lResult > 0)
				{
					if (lResult <= 100)
						dSleepTime = 500;
					if (m_pop3.ConnectDataBase())
					{
						//Receive
						for (long i = 1; i < lResult + 1; i++)
						{
#ifdef _DEBUG
							csDebug.Format(_T("%s Count = %d\r\n"),info.szName,i);
							OutputDebugString(csDebug);
#endif
							if (m_pop3.GetStatus())
								break;
							if (FTL::ftwtContinue == (waitType = GetJobWaitType(INFINITE)))
							{
								strUidl.clear();
								strUidl = m_pop3.GetUIDL(i);
								if (strUidl.length()>0)
								{
									long lReturnvalue = m_pop3.CheckUIDL(strUidl,strName);
									if (lReturnvalue == MONGO_NOT_FOUND)
									{
										if (m_pop3.GetEMLFile(i, strUidl) == 0)
										{
											if (MailAnalysis(m_pop3,m_sql ,strUidl,info.szAbbreviation,1))
											{
												
												sprintf_s(chDebug, 512, "Analysis [%s] Error!", strUidl.c_str());
#ifdef _DEBUG
												OutputDebugStringA(chDebug);
												OutputDebugStringA("\r\n");
#endif
												m_log.Log(chDebug,strlen(chDebug));
											}
										}
										if (info.bSendMail)
										{
											m_smtp.SetCurrPath(m_pop3.GetCurrPath());
											m_smtp.AddAttachFileName(strUidl);
											SendEmail();
										}
									}
									else if (lReturnvalue == MONGO_DELETE)
										m_UidlData.push_back(strUidl);
									dPercent = (double)i / lResult * 100;
									//m_JobParam->m_pDlg->PostMessage(__umymessage__fres_hprogress__, (WPARAM)m_JobParam->m_lPos, (LPARAM)dPercent);
								}
							}
							else break;
							Sleep(dSleepTime);
						}
						m_sql.CloseDB();
						//Delete
						std::vector<string>::iterator ite = m_UidlData.begin();
						long n(0),lCount=m_UidlData.size();
						for (long i = 1; i < lResult + 1&&m_UidlData.size()>0; i++)
						{
#ifdef _DEBUG
							csDebug.Format(_T("%s Del-Count = %d\r\n"), info.szName, i);
							OutputDebugString(csDebug);
#endif
							if (FTL::ftwtContinue == (waitType = GetJobWaitType(INFINITE)))
							{
								strUidl.clear();
								strUidl = m_pop3.GetUIDL(i);
								if (strUidl.length()>0)
								{
									ite = std::find(m_UidlData.begin(), m_UidlData.end(), strUidl);
									if (ite != m_UidlData.end())
									{
										if (m_pop3.DelEmail(i,strUidl) == SUCCESS)
											m_UidlData.erase(ite);
										dPercent = (double)n++ / (double)lCount * 100;
										//m_JobParam->m_pDlg->PostMessage(__umymessage__delfres_hprogress__, (WPARAM)m_JobParam->m_lPos, (LPARAM)dPercent);
									}
								}
							}
							else break;
							Sleep(dSleepTime);
						}
						m_pop3.QuitDataBase();
					}
				}
			}
			else
			{
				TCHAR szError[256] = { 0 };
				GetErrorMessage(lResult, szError);
				csDebug.Format(_T("登陆 %s 出现问题[%s]"), info.szName, szError);
				m_log.Log(csDebug, csDebug.GetLength());
#ifdef DEBUG
				OutputDebugString(csDebug);
				OutputDebugString(_T("\r\n"));
#endif
			}
			m_pop3.Close();
		}
	}
	switch(waitType)
	{
	case FTL::ftwtStop: //用户停止了Job(单独停止或整个Pool)，一般用于保存中间结果
	case FTL::ftwtError://发生了错误(正常情况下应该不会发生这个问题，但如果强制关闭使用的 句柄 等时可能会发生)，可以采用类似 ftwtStop 的逻辑
		//因此，前面的 ftwtStop 后面没有 break
		break;
	case FTL::ftwtContinue:
		//本例子里，如果是正常结束的Job，会到这里来
		break;
	case FTL::ftwtTimeOut:
		//本例子里，等待时间采用的是 INFINITE，不会到这里。
	default:
		FTLASSERT(FALSE);
		break;
	}

	return TRUE;
}
VOID CMyJob::Finalize()
{
	//m_JobParam 是模版参数 MyJobParam*, 而且是 new 出来的，因此需要释放
	SAFE_DELETE(m_JobParam);

	//例子中所有的 CMyJob 都是 new 出来的，因此，最后需要 delete
	delete this;
}

VOID CMyJob::OnCancelJob()
{
	//尚未运行的Job，释放 m_JobParam 和 自身
	SAFE_DELETE(m_JobParam);
	delete this;
}

BOOL CMyJob::Wait()
{
	FTL::FTLThreadWaitType waitType = FTL::ftwtError;
	if (FTL::ftwtContinue == (waitType = GetJobWaitType(INFINITE)))
		return TRUE;
	return FALSE;
}

long CMyJob::SendEmail()
{
	if (m_smtp.Logon()!=0)
		return -1;
	if (m_smtp.SendHead() != 0)
		return -1;
	/*if (smtp.SendTextBody() != 0)
	return -1;*/
	if (m_smtp.SendFileBody() != 0)
		return -1;
	if (m_smtp.Quit())
		return 0;
	return 0;
}

long CMyJob::MailAnalysis(POP3& pop3, CSQLServer& sql,const string& strUIDL, LPCTSTR lpAbb, long lType)
{
	CString csUIDL(strUIDL.c_str()),csPath(pop3.GetCurrPath());
	CMailAnalysis ana;
	ana.SetAbbreviation(lpAbb);
	ana.LoadFile(csPath,csUIDL);
#ifdef _DEBUG
	DWORD dwTime(0);
	dwTime = GetTickCount();
#endif
	if (ana.AnalysisHead() < 0)
	{
		ana.Clear(1);
		return -1;
	}
	if (ana.AnalysisBody(ana.GetBoundry(), ana.GetHeadRowCount()) < 0)
	{
		ana.Clear(1);
		return -1;
	}
	if (ana.AnalysisBoundary(ana.GetBoundry(), ana.GetAttach()) < 0)
	{
		ana.Clear(1);
		return -1;
	}
	sql.SaveToDB(ana.GetEmailItem());
	pop3.SaveFileToDB(ana.GetEmailItem());
	ana.Clear(lType);
#ifdef _DEBUG
	dwTime = GetTickCount() - dwTime;
	CString csDebug;
	csDebug.Format(_T("Process Time = %d\tAnalysis [%s] Complete!"), dwTime / 1000, csUIDL);
	OutputDebugString(csDebug);
	OutputDebugString(_T("\r\n"));
#endif
	return 0;
}

