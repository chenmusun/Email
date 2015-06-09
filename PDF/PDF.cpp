// PDF.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "PDF.h"
#include "pdf2text.h"
#include "fgsdk_common.h"
#include "fadk/fadk.h"
#include "fgsdk/fsdk.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


typedef int(*FXT_InitLibrary) (const char* key, int screenFlag);
typedef void(*FXT_DestoryLibrary) ();

typedef char*(*FXT_CallbackFun)(void* userData, int mode, char* msg, bool* isStop);
typedef int(*FXT_OfficeToPDFRun) (const char* commandline, FXT_CallbackFun callback, void* userData);

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

// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO:  更改错误代码以符合您的需要
			_tprintf(_T("错误:  MFC 初始化失败\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO:  在此处为应用程序的行为编写代码。
		}
	}
	else
	{
		// TODO:  更改错误代码以符合您的需要
		_tprintf(_T("错误:  GetModuleHandle 失败\n"));
		nRetCode = 1;
	}

	return nRetCode;
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

PDF_API int RemovePasswd(string inputPDF)
{
	string stroutpath;
	GetPathInfo(inputPDF, stroutpath, 1);
	return FSADK_PDF_RemovePassword((char*)stroutpath.c_str(), (char*)inputPDF.c_str());
}

PDF_API int RemoveWaterMark(string inputPDF)
{
	int wClientData = 0;
	string outputPDF;
	GetPathInfo(inputPDF, outputPDF, 2);
	FADK_WATERMARK_HANDLER handler;

	wClientData = 3;
	handler.clientData = (void*)&wClientData;
	handler.DeletePDFObject = Test_DeletePDFObject;
	FSADK_PDF_RemoveWatermark(&handler, (char*)outputPDF.c_str(), (char*)inputPDF.c_str());
	return 0;
}

PDF_API int PDF2TEXT(string inputPDF)
{
	string outputText;
	GetPathInfo(inputPDF, outputText);
	return FSADK_PDF_ExtractText((char*)outputText.c_str(), (char*)inputPDF.c_str());
}

void GetPathInfo(const string&strSrc, string&stroutputPath, int nType)
{
	char chTemp[512] = { 0 };
	string strTemp, strinputPath, strFileName;
	auto pos = strSrc.find_last_of("\\");
	strTemp = strSrc.substr(0, pos);
	strinputPath = strTemp;
	auto pos2 = strSrc.find_last_of(".");
	strFileName = strSrc.substr(pos + 1, pos2 - pos - 1);
	switch (nType)
	{
	case 0:
		sprintf_s(chTemp, 512, "%s\\%s.txt", strinputPath.c_str(), strFileName.c_str());
		stroutputPath = chTemp;
		break;
	case 1:
		sprintf_s(chTemp, 512, "%s\\No_Permission_%s.pdf", strinputPath.c_str(), strFileName.c_str());
		stroutputPath = chTemp;
		break;
	case 2:
		sprintf_s(chTemp, 512, "%s\\No_Watermark_%s.pdf", strinputPath.c_str(), strFileName.c_str());
		stroutputPath = chTemp;
		break;
	case 3:
		sprintf_s(chTemp, 512, "%s\\%s.pdf", strinputPath.c_str(), strFileName.c_str());
		stroutputPath = chTemp;
		break;
	default:
		break;
	}
}

//void OnBnClickedButtonConvert()
//{
//	// TODO: Add your control notification handler code here
//	char chCommend[512] = { 0 }, chPath[MAX_PATH] = { 0 }, chiPath[MAX_PATH] = { 0 }, choPath[MAX_PATH] = { 0 }, chFileName[MAX_PATH] = { 0 };
//	CString csFileName, csiPath;
//	HMODULE hModule = LoadLibrary(_T("pdftools.dll"));
//
//	if (NULL == hModule) {
//		int ret = GetLastError();
//		return;
//	}
//
//	FXT_InitLibrary fxtInitLibrary = (FXT_InitLibrary)GetProcAddress(hModule, "FXT_InitLibrary");
//	FXT_DestoryLibrary fxtDestoryLibrary = (FXT_DestoryLibrary)GetProcAddress(hModule, "FXT_DestoryLibrary");
//	FXT_OfficeToPDFRun fxtOfficeToPDFRun = (FXT_OfficeToPDFRun)GetProcAddress(hModule, "FXT_OfficeToPDFRun");
//	WideCharToMultiByte(CP_ACP, 0, m_csFilePath, m_csFilePath.GetLength(), chPath, MAX_PATH, NULL, NULL);
//	auto pos = m_csFilePath.ReverseFind(_T('\\'));
//	csiPath = m_csFilePath.Left(pos);
//	WideCharToMultiByte(CP_ACP, 0, csiPath, csiPath.GetLength(), chiPath, MAX_PATH, NULL, NULL);
//	auto pos2 = m_csFilePath.ReverseFind(_T('.'));
//	csFileName = m_csFilePath.Mid(pos + 1, pos2 - pos - 1);
//	WideCharToMultiByte(CP_ACP, 0, csFileName, csFileName.GetLength(), chFileName, MAX_PATH, NULL, NULL);
//	sprintf_s(choPath, MAX_PATH, "%s\\%s.pdf", chiPath, chFileName);
//	sprintf_s(chCommend, 512, "-i %s -o %s", chPath, choPath);
//	if (fxtInitLibrary("E:\\Email\\Office转换_demo20150515\\PDFConvert\\pdftools\\ftlkey.txt", 0) == 0) {
//
//		int ret = fxtOfficeToPDFRun(chCommend, NULL, NULL);
//
//		fxtDestoryLibrary();
//		if (ret == 0)
//			OutputDebugStringA("Success!");
//	}
//}

PDF_API int OFFICE2PDF(const CString& csinputPath)
{
	int ret(-1);
	string stroutputPath, inputPath(ConvertPath(csinputPath));
	char chCommend[512] = { 0 };
	GetPathInfo(inputPath, stroutputPath, 3);
	HMODULE hModule = LoadLibrary(_T("pdftools64.dll"));

	if (NULL == hModule)
	{
		int ret = GetLastError();
		return -1;
	}
	FXT_InitLibrary fxtInitLibrary = (FXT_InitLibrary)GetProcAddress(hModule, "FXT_InitLibrary");
	FXT_DestoryLibrary fxtDestoryLibrary = (FXT_DestoryLibrary)GetProcAddress(hModule, "FXT_DestoryLibrary");
	FXT_OfficeToPDFRun fxtOfficeToPDFRun = (FXT_OfficeToPDFRun)GetProcAddress(hModule, "FXT_OfficeToPDFRun");
	sprintf_s(chCommend, 512, "-i %s -o %s", inputPath.c_str(), stroutputPath.c_str());
	if (fxtInitLibrary("ftlkey.txt", 0) == 0) 
	{

		ret = fxtOfficeToPDFRun(chCommend, NULL, NULL);

		fxtDestoryLibrary();
	}
	return ret;
}

string ConvertPath(const CString& csPath)
{
	string strPath;
	char*p = NULL;
	int nLen = ::WideCharToMultiByte(CP_ACP, NULL, csPath, csPath.GetLength(), NULL, 0, NULL, NULL);
	if (nLen <= 0)
		return strPath;
	p = new char[nLen + 1];
	memset(p, 0, nLen + 1);
	::WideCharToMultiByte(CP_ACP, NULL, csPath, csPath.GetLength(), p, nLen, NULL, NULL);
	strPath = p;
	if (p&& nLen > 0)
	{
		delete p;
		p = NULL;
	}
	return strPath;
}