// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� LOG_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// LOG_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#pragma once
#ifdef LOG_EXPORTS
#define LOG_API __declspec(dllexport)
#else
#define LOG_API __declspec(dllimport)
#pragma	comment(lib,"../output/Log.lib")
#endif

#include <wchar.h>


// �����Ǵ� Log.dll ������
class LOG_API CLog {
public:
	CLog(void);
	~CLog();
	// TODO:  �ڴ�������ķ�����
	void SetPath(LPCTSTR lpPath,long lLen);//����Log��־·��
	void SetPath(const char*pPath,long lLen);
	void Log(LPCTSTR lpText, size_t lLen);//д��־
	void Log(LPCSTR lpText, size_t lLen);
	void Log(long lCode);
	void GetCurrTime(char* pTime,long lLen);//��ȡ��ǰʱ��
private:
	char m_chLogPath[MAX_PATH];
	FILE *m_fp;
	long m_lLevel;
protected:
};

extern LOG_API int nLog;

LOG_API int fnLog(void);
