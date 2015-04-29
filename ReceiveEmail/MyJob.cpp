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

