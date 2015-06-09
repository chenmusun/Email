// PDF.cpp : ���� DLL Ӧ�ó���ĵ���������
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

	//ˮӡ-���ļ�ֻ��reports@lordabbettchina.comʹ��.pdf
	if (*(int*)clientData == 1)
	{
		if (objectInfo->textContentLength != 0)
		{
			bool bfind = false;

			std::string s(objectInfo->textContent, objectInfo->textContentLength);
			std::wstring ws = utf8_to_unicode(s);
			bfind = ws.find(L"���ļ�ֻ��reports@lordabbettchina.comʹ��") != string::npos;

			if (bfind)
				return true;
		}
	}

	//ˮӡ-���о��������baogao@vanfon.com.cn�Ķ�
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

	//ˮӡ-��Ͷ֤ȯ��Ȩ����,���͸���ҵȫ�����������޹�˾
	if (*(int*)clientData == 3)
	{
		if (objectInfo->textContentLength != 0)
		{
			bool bfind = false;

			std::string s(objectInfo->textContent, objectInfo->textContentLength);
			std::wstring ws = utf8_to_unicode(s);
			bfind = ws.find(L"��Ͷ֤ȯ��Ȩ����,���͸���ҵȫ�����������޹�˾") != string::npos;
			if (bfind)
			{
				return true;
			}

			bfind = ws.find(L"��Ͷ֤ȯ") != string::npos;
			FSPDF_TEXTSTATE* textState = (FSPDF_TEXTSTATE*)objectInfo->textState;
			if (bfind && textState->fontSize == 60)
			{
				return true;
			}

		}
	}

	return false;
}

// Ψһ��Ӧ�ó������

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// ��ʼ�� MFC ����ʧ��ʱ��ʾ����
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO:  ���Ĵ�������Է���������Ҫ
			_tprintf(_T("����:  MFC ��ʼ��ʧ��\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO:  �ڴ˴�ΪӦ�ó������Ϊ��д���롣
		}
	}
	else
	{
		// TODO:  ���Ĵ�������Է���������Ҫ
		_tprintf(_T("����:  GetModuleHandle ʧ��\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
////////////////////////////////////////////////////////////////////////////////////
// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� PDF.h
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

PDF_API int RemovePasswd(const string& inputPDF)
{
	string stroutpath;
	GetPathInfo(inputPDF, stroutpath, 1);
	return FSADK_PDF_RemovePassword((char*)stroutpath.c_str(), (char*)inputPDF.c_str());
}

PDF_API int RemoveWaterMark(const string& inputPDF)
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

PDF_API int PDF2TEXT(const string& inputPDF)
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

PDF_API int OFFICE2PDF(const string& inputPDF)
{
	int ret(-1);
	string stroutputPath;
	char chCommend[512] = { 0 };
	GetPathInfo(inputPDF, stroutputPath, 3);
	HMODULE hModule = LoadLibrary(_T("pdftools64.dll"));

	if (NULL == hModule)
	{
		int ret = GetLastError();
		return -1;
	}
	FXT_InitLibrary fxtInitLibrary = (FXT_InitLibrary)GetProcAddress(hModule, "FXT_InitLibrary");
	FXT_DestoryLibrary fxtDestoryLibrary = (FXT_DestoryLibrary)GetProcAddress(hModule, "FXT_DestoryLibrary");
	FXT_OfficeToPDFRun fxtOfficeToPDFRun = (FXT_OfficeToPDFRun)GetProcAddress(hModule, "FXT_OfficeToPDFRun");
	sprintf_s(chCommend, 512, "-i %s -o %s", inputPDF.c_str(), stroutputPath.c_str());
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