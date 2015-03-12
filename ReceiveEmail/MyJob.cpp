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
	//�˴����г�ʼ�������ʧ�ܣ�����Ҫ�ͷ���Դ���ҷ��� FALSE
	return TRUE;
}

BOOL CMyJob::Run()
{
	//����Job����Ҫѭ�������õķ�ʽ�ǡ�ѭ������ GetJobWaitType(INFINITE) ֱ������ֵ��Ϊ ftwtContinue ��Job���
	FTL::FTLThreadWaitType waitType = FTL::ftwtError;
	if ( FTL::ftwtContinue == (waitType = GetJobWaitType(INFINITE))) 
		//��� GetJobWaitType �Ĳ����� INFINITE�� ��֧��Pause������
		//�������֧��Pause(�������紫��)������Դ�0��
		//����Ҫע�⣺��� ThreadPool::Pause �Ļ�����������ķ���ֵ���� ftwtTimeOut
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
		m_JobParam->m_pDlg->GetMailBoxInfo(m_JobParam->m_lPos, csInfo,info);
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
		m_JobParam->m_pDlg->GetDataBaseInfo(dbinfo);
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
											if (MailAnalysis(m_pop3, strUidl,info.szAbbreviation))
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
									m_JobParam->m_pDlg->PostMessage(__umymessage__fres_hprogress__, (WPARAM)m_JobParam->m_lPos, (LPARAM)dPercent);
								}
							}
							else break;
							Sleep(dSleepTime);
						}
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
										m_JobParam->m_pDlg->PostMessage(__umymessage__delfres_hprogress__, (WPARAM)m_JobParam->m_lPos, (LPARAM)dPercent);
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
				csDebug.Format(_T("��½ %s ��������[%s]"), info.szName, szError);
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
	case FTL::ftwtStop: //�û�ֹͣ��Job(����ֹͣ������Pool)��һ�����ڱ����м���
	case FTL::ftwtError://�����˴���(���������Ӧ�ò��ᷢ��������⣬�����ǿ�ƹر�ʹ�õ� ��� ��ʱ���ܻᷢ��)�����Բ������� ftwtStop ���߼�
		//��ˣ�ǰ��� ftwtStop ����û�� break
		break;
	case FTL::ftwtContinue:
		//����������������������Job���ᵽ������
		break;
	case FTL::ftwtTimeOut:
		//��������ȴ�ʱ����õ��� INFINITE�����ᵽ���
	default:
		FTLASSERT(FALSE);
		break;
	}

	return TRUE;
}
VOID CMyJob::Finalize()
{
	//m_JobParam ��ģ����� MyJobParam*, ������ new �����ģ������Ҫ�ͷ�
	SAFE_DELETE(m_JobParam);

	//���������е� CMyJob ���� new �����ģ���ˣ������Ҫ delete
	delete this;
}

VOID CMyJob::OnCancelJob()
{
	//��δ���е�Job���ͷ� m_JobParam �� ����
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

long CMyJob::MailAnalysis(POP3& pop3, const string& strUIDL, LPCTSTR lpAbb,long lType)
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
	//pop3.SaveFileToDB();
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

