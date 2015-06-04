// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� PDF_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// PDF_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#pragma once
#ifdef PDF_EXPORTS
#define PDF_API __declspec(dllexport)
#else
#define PDF_API __declspec(dllimport)
#pragma	comment(lib,"../output/PDF.lib")
#endif

#include <string>
using namespace std;
// �����Ǵ� PDF.dll ������
class PDF_API CPDF {
public:
	CPDF(void);
	virtual ~CPDF();
	// TODO:  �ڴ�������ķ�����
};

extern PDF_API int nPDF;

PDF_API int fnPDF(void);
PDF_API int PDF2TXT(string inputFilePath, string outputFilePath, string& outputname, int& nPageNum, int& time);
PDF_API BOOL InitializeLibrary();
PDF_API void FinalizeLibrary();
PDF_API int RemovePasswd(string inputPDF, string outputPDF);
PDF_API int RemoveWaterMark(string inputPDF, string outputPDF);
PDF_API int PDF2TEXT(string inputPDF, string outputText);