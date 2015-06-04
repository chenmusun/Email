// PDF.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "PDF.h"
#include "pdf2text.h"
#include "fgsdk_common.h"
#include "fadk/fadk.h"
#include "fgsdk/fsdk.h"

std::wstring	utf8_to_unicode(const std::string& str)
{
	std::wstring result;
	int size = ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), 0, 0);
	if (size <= 0) {
		return result;
	}

	result.assign(size, 0);
	::MultiByteToWideChar(CP_UTF8, NULL, str.data(), str.size(), (wchar_t*)result.data(), size);
	return result;
}

bool	Test_DeletePDFObject(void* clientData, FADK_PDF_OBJECT_INFO* objectInfo)
{
	if (NULL == objectInfo)
	{
		return false;
	}

	//水印-此文件只供reports@lordabbettchina.com使用.pdf
	if (*(int*)clientData == 1)
	{
		if (objectInfo->textContentLength != 0)
		{
			bool bfind = false;

			std::string s(objectInfo->textContent, objectInfo->textContentLength);
			std::wstring ws = utf8_to_unicode(s);
			bfind = ws.find(L"此文件只供reports@lordabbettchina.com使用") != string::npos;

			if (bfind)
				return true;
		}
	}

	//水印-本研究报告谨呈baogao@vanfon.com.cn阅读
	if (*(int*)clientData == 2)
	{
		if (objectInfo->textContentLength != 0)
		{
			bool bfind = false;

			std::string s(objectInfo->textContent, objectInfo->textContentLength);
			std::wstring ws = utf8_to_unicode(s);
			bfind = ws.find(L"baogao@vanfon.com.cn") != string::npos;
			FSPDF_TEXTSTATE* textState = (FSPDF_TEXTSTATE*)objectInfo->textState;
			if (bfind && textState->fontSize == 8)
			{
				if (objectInfo->rcObject.left > 20 && objectInfo->rcObject.right < 173
					&& objectInfo->rcObject.top>6 && objectInfo->rcObject.bottom > -2)
				{
					return true;
				}
			}

			//::OutputDebugStringW((LPCWSTR)ws.c_str());
			//::OutputDebugStringW(L"\r\n");
		}
	}

	//水印-中投证券版权所有,发送给兴业全球基金管理有限公司
	if (*(int*)clientData == 3)
	{
		if (objectInfo->textContentLength != 0)
		{
			bool bfind = false;

			std::string s(objectInfo->textContent, objectInfo->textContentLength);
			std::wstring ws = utf8_to_unicode(s);
			bfind = ws.find(L"中投证券版权所有,发送给兴业全球基金管理有限公司") != string::npos;
			if (bfind)
			{
				return true;
			}

			bfind = ws.find(L"中投证券") != string::npos;
			FSPDF_TEXTSTATE* textState = (FSPDF_TEXTSTATE*)objectInfo->textState;
			if (bfind && textState->fontSize == 60)
			{
				return true;
			}

		}
	}

	return false;
}

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

PDF_API BOOL InitializeLibrary()
{
	FS_BOOL bRet = FSDK_InitializeLibray(TRUE);
	if (!bRet)
	{
		FSDK_CloseLog();
		return FALSE;
	}
	if (!FSDK_PDFModule_Initialize())
	{
		FSDK_FinalizeLibrary();
		FSDK_CloseLog();
		return FALSE;
	}
	return TRUE;
}

PDF_API void FinalizeLibrary()
{
	//Finalize PDF module.
	FSDK_PDFModule_Finalize();
	//Finalize SDK library.
	FSDK_FinalizeLibrary();
	//Close log file.
	FSDK_CloseLog();
}

PDF_API int RemovePasswd(string inputPDF, string outputPDF)
{
	return FSADK_PDF_RemovePassword((char*)outputPDF.c_str(), (char*)inputPDF.c_str());
}

PDF_API int RemoveWaterMark(string inputPDF, string outputPDF)
{
	int wClientData = 0;

	FADK_WATERMARK_HANDLER handler;
	handler.DeletePDFObject = Test_DeletePDFObject;
	FSADK_PDF_RemoveWatermark(&handler, (char*)outputPDF.c_str(), (char*)inputPDF.c_str());
	return 0;
}

PDF_API int PDF2TEXT(string inputPDF, string outputText)
{
	return FSADK_PDF_ExtractText((char*)outputText.c_str(), (char*)inputPDF.c_str());
}
////////////////////////////////////////////////////////////////////////////////////
// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 PDF.h
CPDF::CPDF()
{
	return;
}


CPDF::~CPDF()
{

}