// PDF.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "PDF.h"
#include "pdf2text.h"
#include "fgsdk_common.h"
#include "fadk/fadk.h"
#include "fgsdk/fsdk.h"

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
	GetPathInfo(inputPDF,stroutpath,1);
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

void GetPathInfo(const string&strSrc, string&stroutputPath,int nType)
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
		sprintf_s(chTemp, 512, "%s\\%s.txt", strinputPath.c_str(),strFileName.c_str());
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
	default:
		break;
	}
}

//void CPDFConvertDemoDlg::OnBnClickedButtonConvert()
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
//	if (fxtInitLibrary("E:\\Email\\Officeת��_demo20150515\\PDFConvert\\pdftools\\ftlkey.txt", 0) == 0) {
//
//		int ret = fxtOfficeToPDFRun(chCommend, NULL, NULL);
//
//		fxtDestoryLibrary();
//		if (ret == 0)
//			OutputDebugStringA("Success!");
//	}
//}


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