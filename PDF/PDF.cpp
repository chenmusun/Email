// PDF.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "PDF.h"
#include "pdf2text.h"


// ���ǵ���������һ��ʾ��
PDF_API int nPDF=0;

// ���ǵ���������һ��ʾ����
PDF_API int fnPDF(void)
{
	return 42;
}

PDF_API int PDF2TXT(string inputFilePath, string outputFilePath, string& outputname, int& nPageNum, int& time)
{
	return PDF2Text(inputFilePath, outputFilePath, outputname, nPageNum, time);
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� PDF.h
CPDF::CPDF()
{
	return;
}


CPDF::~CPDF()
{

}