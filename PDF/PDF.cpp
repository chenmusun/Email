// PDF.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "PDF.h"
#include "pdf2text.h"


// 这是导出变量的一个示例
PDF_API int nPDF=0;

// 这是导出函数的一个示例。
PDF_API int fnPDF(void)
{
	return 42;
}

PDF_API int PDF2TXT(string inputFilePath, string outputFilePath, string& outputname, int& nPageNum, int& time)
{
	return PDF2Text(inputFilePath, outputFilePath, outputname, nPageNum, time);
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 PDF.h
CPDF::CPDF()
{
	return;
}


CPDF::~CPDF()
{

}