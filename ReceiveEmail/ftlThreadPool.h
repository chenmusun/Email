///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file   ftlthread.h
/// @brief  Functional Template Library Base Header File.
/// @author fujie
/// @version 0.6 
/// @date 03/30/2008
/// @defgroup ftlThreadPool ftl thread pool function and class
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FTL_THREADPOOL_H
#define FTL_THREADPOOL_H
#pragma once

#ifndef FTL_BASE_H
#  error ftlThreadPool.h requires ftlbase.h to be included first
#endif

#include "ftlThread.h"
#include <set>
#include <map>
//#include "ftlSharePtr.h"
//#include "ftlFunctional.h"

namespace FTL
{
	//ǰ������
	//! ����ģ��������̳߳��࣬���ܿ��Է���Ľ��в��������⣬��ӵ�������ص㣺
	//  1.���Զ�����������̵߳Ķ����� ��С/��� �̸߳���֮�����
	//  2.�ܷ���ĶԵ����������ȡ������������δ�������ɿ�ܴ��봦���������Ѿ����У�����Ҫ JobBase ��������� GetJobWaitType �ķ���ֵ���д���
	//  3.�ܶ������̳߳ؽ��� ��ͣ��������ֹͣ ���� -- ��Ҫ JobBase ��������� GetJobWaitType �ķ���ֵ���д���
	//  4.֧�ֻص���ʽ�ķ���֪ͨ( Progress/Error ��)
	//  5.ʹ�õ���΢��Ļ���API����֧��WinXP��Vista��Win7�ȸ��ֲ���ϵͳ
	template <typename T> class CFThreadPool;  

	//enum FJobStatus
	//{
	//	jsWaiting,
	//	jsDoing,
	//	jsCancel,	//������������Ϊcancel, ���Job����֧��Cancel�Ļ��������� GetJobWaitType ���ú�������
	//	jsDone,
	//	jsError,	
	//};

	//! �����ʵ�ֶ�����в�ͬ�������͵�Job�����Խ�ģ����Ϊ DWORD_PTR �ȿ���ת��Ϊָ������ͼ���
	template <typename T>
	class CFJobBase
	{
		friend class CFThreadPool<T>;   //����Threadpool���� m_pThreadPool/m_nJobIndex ��ֵ
	public:
		FTLINLINE CFJobBase();
		FTLINLINE CFJobBase(T& t);
		FTLINLINE virtual ~CFJobBase();

		//! �Ƚ�Job�Ĵ�С������ȷ���� Waiting �����еĶ��У� ��������Ϊ Priority -> Index
		bool operator < (const CFJobBase & other) const;

		//! ��ȡ������Job�����ȼ�, ����ԽС�����ȼ�Խ��(�ڵȴ�����������Խǰ��)��ȱʡֵ�� 0
		//  ע�⣺���ȼ������ڷ��� Pool ǰ���ã������Ͳ����ٵ�����
		FTLINLINE LONG GetJobPriority() const { return m_nJobPriority; }
		FTLINLINE LONG SetJobPriority(LONG nNewPriority);

		FTLINLINE LONG GetJobIndex() const;
		//FTLINLINE FJobStatus GetJobStatus() const;

		T		m_JobParam;			//! Job��ʹ�õĲ������˴�Ϊ�˼򻯣�ֱ�Ӳ��ù��б����ķ�ʽ
		//FTLINLINE T& GetJobParam();
		//FTLINLINE const T& GetJobParam() const;

		//���Job�������й����б�ȡ����������������
		FTLINLINE BOOL RequestCancel();
	protected:
		//�����������һ��, ��������������Job�� if( Initialize ){ Run -> Finalize }
		virtual BOOL Initialize();
		// �����Run��ͨ����Ҫѭ�� ���� GetJobWaitType �������
		virtual BOOL Run() = 0;
		//! �����new�����ģ�ͨ����Ҫ�� Finalize �е��� delete this (������������������ڹ�������)
		virtual VOID Finalize() = 0;

		//�����������δ���е�Job(ֱ��ȡ�����̳߳�ֹͣ), ��������ڴ����Դ, �� delete this ��
		virtual VOID OnCancelJob() = 0;
	protected:
		FTLINLINE void _NotifyProgress(LONG64 nCurPos, LONG64 nTotalSize);
		FTLINLINE void _NotifyError(DWORD dwError, LPCTSTR pszDescription);
		FTLINLINE void _NotifyCancel();

		//! ͨ���ú�������ȡ�̳߳ص�״̬(Stop/Pause)���Լ�Job�Լ���Stop, �÷�ͬ CFThread:GetThreadWaitType:
		//! �����֧����ͣ�������� INFINITE���粻��֧����ͣ(�����紫��)��������� 0
		FTLINLINE FTLThreadWaitType GetJobWaitType(DWORD dwMilliseconds = INFINITE) const;
	private:
		//����Ϊ˽�еı����ͷ�������ʹ������Ҳ��Ҫֱ�Ӹ��ģ���Pool���ý��п���
		LONG		m_nJobPriority;
		LONG		m_nJobIndex;
		HANDLE		m_hEventJobStop;					//ֹͣJob���¼����ñ�������Pool�������ͷ�(TODO:Pool�л���?)
		//FJobStatus	m_JobStatus;
		CFThreadPool<T>* m_pThreadPool;
	};

	typedef enum tagGetJobType
	{
		typeStop,
		typeSubtractThread,
		typeGetJob,
		typeError,		//����δ֪����(Ŀǰ�в����ʲô����»ᷢ��)
	}GetJobType;

	//�ص����� -- ͨ�� pJob->m_JobParam ���Է�������Ϊ T �� ����
	FTLEXPORT template <typename T>
	class IFThreadPoolCallBack
	{
	public:
		//��Job���������Ժ󣬻��� Pool ���� Begin �� End ��������
		FTLINLINE virtual void OnJobBegin(LONG nJobIndex, CFJobBase<T>* pJob )
		{
		} 
		FTLINLINE virtual void OnJobEnd(LONG nJobIndex, CFJobBase<T>* pJob)
		{
		}

		//�����δ��������״̬�ͱ�ȡ����Job������Pool�����������
		FTLINLINE virtual void OnJobCancel(LONG nJobIndex, CFJobBase<T>* pJob)
		{
		}

		//Progress �� Error �� JobBase �����༤��
		FTLINLINE virtual void OnJobProgress(LONG nJobIndex , CFJobBase<T>* pJob, LONG64 nCurPos, LONG64 nTotalSize)
		{
		}
		FTLINLINE virtual void OnJobError(LONG nJobIndex , CFJobBase<T>* pJob, DWORD dwError, LPCTSTR pszDescription)
		{
		}
	};

	FTLEXPORT template <typename T>  
	class CFThreadPool
	{
		//typedef CFSharePtr<CFJobBase< T> > CFJobBasePtr;
		//friend class CFJobBasePtr;
		friend class CFJobBase<T>;  //����Job�� GetJobWaitType �л�ȡ m_hEventStop/m_hEventContinue
	public:
		FTLINLINE CFThreadPool(IFThreadPoolCallBack<T>* pCallBack = NULL);
		FTLINLINE virtual ~CFThreadPool(void);

		//! ��ʼ�̳߳�,��ʱ�ᴴ�� nMinNumThreads ���̣߳�Ȼ�������������� nMinNumThreads -- nMaxNumThreads ֮�����е����̵߳ĸ���
		FTLINLINE BOOL Start(LONG nMinNumThreads, LONG nMaxNumThreads);

		//! ����ֹͣ�̳߳�
		//! ע�⣺
		//!   1.ֻ������StopEvent����ҪJob����GetJobWaitType���� 
		//!   2.���������ǰע��ĵ���δ���еĹ����������Ҫɾ������Ҫ����ClearUndoWork
		FTLINLINE BOOL Stop();

		FTLINLINE BOOL StopAndWait(DWORD dwTimeOut = FTL_MAX_THREAD_DEADLINE_CHECK);

		//! �ȴ������̶߳��������ͷ�Start�з�����߳���Դ
		FTLINLINE BOOL Wait(DWORD dwTimeOut = FTL_MAX_THREAD_DEADLINE_CHECK);

		//! �����ǰδ��ɵĹ�����
		FTLINLINE BOOL ClearUndoWork();

		//! ���̳߳���ע�Ṥ�� -- �����ǰû�п��е��̣߳����ҵ�ǰ�߳���С������߳���������Զ������µ��̣߳�
		//! �ɹ����ͨ�� outJobIndex ����Job�������ţ���ͨ����������λ��ȡ���ض���Job
		FTLINLINE BOOL SubmitJob(CFJobBase<T>* pJob, LONG* pOutJobIndex);

		//! ȡ��ָ����Job,
		//! TODO:���ȡ��Job���ͻ������ܵ����ߵõ�ָ��ʱ��Jobִ����� delete this�����ճ�Ұָ���쳣
		FTLINLINE BOOL CancelJob(LONG nJobIndex);

		//FTLINLINE BOOL PauseJob(LONG nJobIndex);
		//FTLINLINE BOOL ResumeJob(LONG nJobIndex);

		//! ������ͣ�̳߳صĲ���
		FTLINLINE BOOL Pause();

		//! ��������̳߳صĲ���
		FTLINLINE BOOL Resume();

		//! �Ƿ��Ѿ���������ͣ�̳߳�
		FTLINLINE BOOL HadRequestPause() const;

		//! �Ƿ��Ѿ�������ֹͣ�̳߳�
		FTLINLINE BOOL HadRequestStop() const;

		FTLINLINE void GetWaitJobNum(size_t& lCount) const;
	protected:
		//! �������е��߳�,��� ��ǰ�߳��� + nThreadNum <= m_nMaxNumThreads ʱ ��ɹ�ִ��
		FTLINLINE BOOL _AddJobThread(LONG nThreadNum);
		FTLINLINE void _DestroyPool();
		FTLINLINE void _DoJobs();

		FTLINLINE GetJobType _GetJob(CFJobBase<T>** ppJob);

		FTLINLINE void _NotifyJobBegin(CFJobBase<T>* pJob);
		FTLINLINE void _NotifyJobEnd(CFJobBase<T>* pJob);
		FTLINLINE void _NotifyJobCancel(CFJobBase<T>* pJob);

		FTLINLINE void _NotifyJobProgress(CFJobBase<T>* pJob, LONG64 nCurPos, LONG64 nTotalSize);
		FTLINLINE void _NotifyJobError(CFJobBase<T>* pJob, DWORD dwError, LPCTSTR pszDescription); 

	protected:
		LONG m_nMinNumThreads;					//! �̳߳������ٵ��̸߳���
		LONG m_nMaxNumThreads;					//! �̳߳��������̸߳���
		IFThreadPoolCallBack<T>* m_pCallBack;	//! �ص��ӿ�
		LONG m_nJobIndex;						//! Job��������ÿ SubmitJob һ�Σ������1

		LONG m_nRunningJobNumber;				//! ��ǰ�������е�Job����

		//TODO: �������ͳһ��
		LONG m_nCurNumThreads;                  //! ��ǰ���̸߳���(��Ҫ����ά�� m_pJobThreadHandles ����)
		LONG m_nRunningThreadNum;				//! ��ǰ�����ŵ��̸߳���(���������е��߳̽���ʱ���� Complete �¼�)

		HANDLE* m_pJobThreadHandles;            //! �����߳̾��������
		DWORD*  m_pJobThreadIds;                //! �����߳� Id ������(Ϊ�����߳̽�������������е�λ��)

		//HANDLE	m_hMgrThread;					//! Pool�����̵߳ľ��

		//! ����ȴ�Job����Ϣ�����������ȼ������⣬����һ���Ǵ���ǰ�濪ʼȡ����˱���� set��
		//! ��֤���ȼ��ߡ�JobIndexС(ͬ���ȼ�ʱFIFO) ��Job����ǰ��
		typedef typename UnreferenceLess< CFJobBase<T> * >	JobBaseUnreferenceLess;
		typedef std::set<CFJobBase<T>*, JobBaseUnreferenceLess > WaitingJobContainer;
		WaitingJobContainer		m_WaitingJobs;	//! �ȴ����е�Job

		//! ��������Job����Ϣ�� ���ڻ�Ƶ�����롢ɾ��������Ҫ����JobIndex���ң���˱���� map
		typedef std::map<LONG, CFJobBase<T>* >	DoingJobContainer;
		DoingJobContainer		m_DoingJobs;	//! �������е�Job

		HANDLE m_hEventStop;                    //! ֹͣPool���¼�
		HANDLE m_hEventAllThreadComplete;		//! ���е��̶߳�����ʱ��������¼�
		HANDLE m_hEventContinue;				//! ����Pool�������е��¼�
		HANDLE m_hSemaphoreJobToDo;             //! ���滹�ж��ٸ�Job���ź���,ÿSubmitһ��Job,������һ��
		HANDLE m_hSemaphoreSubtractThread;      //! ���ڼ����̸߳���ʱ���ź���,��ʼʱ����Ϊ0,ÿҪ�ͷ�һ����������һ����

		CFCriticalSection m_lockDoingJobs;		//���� m_DoingJobs ʱ����
		CFCriticalSection m_lockWaitingJobs;    //���� m_WaitingJobs ʱ����
		CFCriticalSection m_lockThreads;        //���� m_pJobThreadHandles/m_pJobThreadIds ʱ����

		static unsigned int CALLBACK JobThreadProc(void *pParam);    //! �����̵߳�ִ�к���
		//static unsigned int CALLBACK MgrThreadProc(void* pParam);	 //! �����̵߳�ִ�к���
	};
}

#endif //FTL_THREADPOOL_H

#ifndef USE_EXPORT
#  include "ftlThreadPool.hpp"
#endif