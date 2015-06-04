/**
 * Copyright (C) 2001-2015, Foxit Software Inc..
 * All Rights Reserved.
 *
 * http://www.foxitsoftware.com
 *
 * The following code is copyrighted and contains proprietary information and trade secrets of Foxit Software Inc..
 * You cannot distribute any part of Foxit PDF SDK to any third party or general public, 
 * unless there is a separate license agreement with Foxit Software Inc. which explicitly grants you such rights.
 *
 * This file is a demo to extract the text from PDF files and save them as text files.
 **/
#include "stdafx.h"
#include "fgsdk_common.h"
#include "pdf2text.h"

#if defined(FSDK_IOS_DEMO)
#define main pdf2text
#endif

//Covert a PDF file to a text file
CPDFTransToText::CPDFTransToText()
	:m_strDestFolder(""),
	m_strPageRange(""),
	m_strSrcFile("")
{

}

CPDFTransToText::~CPDFTransToText()
{

}

int CPDFTransToText::PDFToText(string fileName, string inputFilesPath, string outputFilesPath, string& outputname,int& nPage,int& time)
{
	FSDK_OutputLog("\r\n====== Convert %s to TXT ======\r\n", fileName.c_str());

	//Prepare for output file name and the format is xxxx.txt.
	//Abstract file name except the extension name
	if (fileName.length() <= 4)
	{
		FSDK_OutputLog("Failure: The file name is invalid. Please check it.\r\n");
		return -3;
	}
	
	//Open an input PDF file
	string strPdfFile = inputFilesPath + "/" + fileName;	
	CAutoDocument autoInputDoc(strPdfFile.c_str(), "rb+");

	//Load a PDF document object from a file
	if (FSCRT_ERRCODE_SUCCESS != autoInputDoc.LoadPDFDocument(NULL))
	{
		return -1;
	}
	int nCount = autoInputDoc.GetPagesCount();
	//Compose the output file
	string strTxtFile = outputFilesPath;
	strTxtFile += string(fileName.c_str(), fileName.length()-4) + ".txt";
	
	//Open an output txt file
	CAutoDocument autoOutPutDoc(strTxtFile.c_str(), "wb+");
	outputname = strTxtFile;

	//Mark the starting time
	unsigned int nStartTime = FSDK_GetClock();
	FS_RESULT ret = FSCRT_ERRCODE_ERROR;
	FS_INT32 nPageCount = 0;
	FS_INT32 nPageNum = 0;
	FS_INT32* nPageIndexArray = NULL;
	if (m_strPageRange != "")
	{
		if (FSDK_GetPageRange(m_strPageRange.c_str(), nPageIndexArray, &nPageNum))
		{
			//Get page count
			ret = FSPDF_Doc_CountPages(autoInputDoc.GetDocument(), &nPageCount);
			if (ret != FSCRT_ERRCODE_SUCCESS)
			{
				FSDK_OutputLog("Failure: Failed to get page count with error code %d.\r\n", ret);
				if (nPageIndexArray)
				{
					delete[] nPageIndexArray;
					nPageIndexArray = NULL;
				}
				outputname.empty();
				time = -1;
				return -1;
			}
		}
	}
	if (nPageNum < 1)
		nPage = nPageNum = nCount;

	//Parse and convert PDF page one by one into one text file 
	FS_INT32 nFinishPageCount = 0;
	for (int i=0; i < nPageNum; i++)
	{
		FS_INT32 pageIndex;
		if (m_strPageRange != "" && nPageIndexArray)
		{
			if (nPageIndexArray[i] >= nPageCount || nPageIndexArray[i] < 0)
			{
				FSDK_OutputLog("Failure: Page index %d is out of valid range!\r\n", nPageIndexArray[i]);
				continue;
			}
			pageIndex = nPageIndexArray[i];
		}
		else
		{
			pageIndex = i;
			FSDK_OutputLog("\r\n");
		}

		CAutoPDFPage autoPage;
		//Get a page based on the given page index
		ret = autoPage.LoadPDFPage(autoInputDoc.GetDocument(),pageIndex);
		if (FSCRT_ERRCODE_SUCCESS != ret)
		{
			continue;
		}

		//Before loading a text page from a PDF page, the PDF page should be pared successfully first.
		if (FSCRT_ERRCODE_SUCCESS != autoPage.ParsePage())
		{
			continue;
		}
		//Load a text page based on a PDF page
		ret = autoPage.LoadTextPage();
		if (FSCRT_ERRCODE_SUCCESS != ret)
		{
			continue;
		}

		//Export a text page to the given text file
		ret = FSPDF_TextPage_ExportToFile(autoPage.GetPDFTextPage(),autoOutPutDoc.GetFileHander());
		if(ret != FSCRT_ERRCODE_SUCCESS)
		{
			FSDK_OutputLog("Failure: In function FSPDF_TextPage_ExportToFile with error code %d.\r\n", ret);
			continue;
		}
		FSDK_OutputLog("Success: Convert to TXT form page with index %d.\r\n", i);
		nFinishPageCount++;
	}
	if (nPageIndexArray)
	{
		delete []nPageIndexArray;
		nPageIndexArray = NULL;
	}

	//Mark the end time
	unsigned int nEndTime = FSDK_GetClock();
	time = nEndTime - nStartTime;
	//Record the duration
	FSDK_OutputLog("\tfinish page count: %d\r\n\telapsed time: %dms\r\n", nFinishPageCount, nEndTime - nStartTime);
	return 0;
}

void  CPDFTransToText::BrowseFolder(void* folder, string inputFilesPath, string outputFilesPath)
{
	FSDK_OutputLog("Enter the input folder\r\n");
	string strPath;
	int nTime = 0,nPageNum=0;
	do
	{
		FS_BOOL bIsDir = FALSE;
		string strFile = FSDK_GetNextFile(folder, &bIsDir);
		if(!bIsDir)
		{
			// Check whether a file is PDF or not
			if(FSDK_CompareExtName(strFile.c_str(), ".pdf")) 
				PDFToText(strFile, inputFilesPath, outputFilesPath, strPath, nPageNum, nTime);
		}
		if (strFile.size() < 1) break;

	} while (TRUE);
}

int PDF2Text(string inputFilePath, string outputFilePath, string& outputname, int& nPageNum, int& time)
{
	int nVal = -1;
	outputname.empty();
	time=-1;
	nPageNum = 0;
	CPDFTransToText Pdf2Text;
	Pdf2Text.m_strDestFolder = outputFilePath;
	Pdf2Text.m_strSrcFile = inputFilePath;

	string logPath;
	logPath = Pdf2Text.m_strDestFolder;
	logPath += "/log.txt";
	FSDK_OpenLog(logPath.c_str());
	FSDK_OutputLog("Foxit PDF SDK example: pdf2text\r\n\r\n");
	/*FS_BOOL bRet = FSDK_InitializeLibray(TRUE);
	if (!bRet)
	{
	FSDK_CloseLog();
	return -2;
	}
	if (!FSDK_PDFModule_Initialize())
	{
	FSDK_FinalizeLibrary();
	FSDK_CloseLog();
	return -1;
	}*/
	string inputFolder, pdfName;

	if (Pdf2Text.m_strSrcFile != "")
	{
		pdfName = FSDK_GetFileName(Pdf2Text.m_strSrcFile.c_str());
		inputFolder = FSDK_GetCustomInputPath(Pdf2Text.m_strSrcFile.c_str());
		FSDK_OutputLog("Input from folder: %s\r\n", inputFolder.c_str());
		FSDK_OutputLog("Output to folder: %s\r\n", Pdf2Text.m_strDestFolder.c_str());
		nVal = Pdf2Text.PDFToText(pdfName, inputFolder, Pdf2Text.m_strDestFolder, outputname, nPageNum,time);
	}
	////Finalize PDF module.
	//FSDK_PDFModule_Finalize();
	////Finalize SDK library.
	//FSDK_FinalizeLibrary();
	////Close log file.
	FSDK_CloseLog();
	return nVal;
}
