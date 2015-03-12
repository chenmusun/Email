// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 LOG_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// LOG_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#pragma once
#ifdef LOG_EXPORTS
#define LOG_API __declspec(dllexport)
#else
#define LOG_API __declspec(dllimport)
#pragma	comment(lib,"../output/Log.lib")
#endif

#include <wchar.h>


// 此类是从 Log.dll 导出的
class LOG_API CLog {
public:
	CLog(void);
	~CLog();
	// TODO:  在此添加您的方法。
	void SetPath(LPCTSTR lpPath,long lLen);//设置Log日志路径
	void SetPath(const char*pPath);
	void Log(LPCTSTR lpText, size_t lLen);//写日志
	void Log(LPCSTR lpText, size_t lLen);
	void Log(long lCode);
	void GetCurrTime(char* pTime,long lLen);//获取当前时间
private:
	char m_chLogPath[MAX_PATH];
	FILE *m_fp;
	long m_lLevel;
protected:
};

extern LOG_API int nLog;

LOG_API int fnLog(void);
