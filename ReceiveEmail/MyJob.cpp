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
	//m_pop3.SetParent(NULL);
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

