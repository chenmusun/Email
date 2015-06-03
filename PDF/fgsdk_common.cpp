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
 * This file is a common source to implement some general functions which are used by examples.
 */

//Include common header file.
#include "stdafx.h"
#include "fgsdk_common.h"

/*******************************************************************************/
/*				auto Document class										   */
/*******************************************************************************/
CAutoDocument::CAutoDocument(const char* fileName, const char* fileMode, BOOL useDef)
			: m_Doc(NULL)
			, m_file(NULL)
			, m_bPDFDoc(FALSE)
			,m_attachments(NULL)
			,m_pAttachment(NULL)
			,m_pAttachmentFileName(NULL)
			,m_AttachmentFileCount(0)
			,m_attachment(NULL)
{
	m_file = FSDK_OpenFile(fileName, fileMode);
	if (!m_file)
	{
		FSDK_OutputLog("Failure: Fail to open file.\r\n");
	}
	else
		FSDK_OutputLog("Success: Open file successfully .\r\n");

}

CAutoDocument::CAutoDocument()
			: m_Doc(NULL)
			, m_file(NULL)
			, m_bPDFDoc(FALSE)
			,m_attachments(NULL)
			,m_pAttachment(NULL)
			,m_pAttachmentFileName(NULL)
			,m_AttachmentFileCount(0)
			,m_attachment(NULL)
{

}

 CAutoDocument::~CAutoDocument()
{
	if (m_file)
	{
		FSCRT_File_Release(m_file);
		m_file = NULL;
	}

	if (m_pAttachment)
	{
		delete [] m_pAttachment;
		m_pAttachment = NULL;
	}

	if (m_pAttachmentFileName)
	{
		for (FS_INT32 i = 0; i < m_AttachmentFileCount; i++)
			FSCRT_BStr_Clear(&m_pAttachmentFileName[i]);
		delete [] m_pAttachmentFileName;
		m_pAttachmentFileName = NULL;
	}

	if (m_attachment)
	{
		FSPDF_Attachment_Release(m_Doc, m_attachment);
	}

	if (m_attachments) 
	{
		FSPDF_Attachments_Release(m_attachments);
		m_attachments = NULL;
	}
	CloseDocument();
}

void	 CAutoDocument::CloseDocument()
{
	if(!m_Doc)
		return;
	if(m_bPDFDoc)
		FSPDF_Doc_Close(m_Doc);
	else
		FSFDF_Doc_Close(m_Doc);
	m_Doc = NULL;

}

FS_RESULT CAutoDocument::LoadPDFDocument(FSCRT_BSTR* password, FS_BOOL bProgress)
{
	FS_RESULT ret = FSCRT_ERRCODE_PARAM;
	if (NULL == m_file)
		return ret;
	CloseDocument();

	FSCRT_PROGRESS docProgress = NULL;
	if (!bProgress)
	{
		ret = FSPDF_Doc_StartLoad(m_file, password, &m_Doc, NULL);
		if (FSCRT_ERRCODE_SUCCESS != ret)
		{
			FSDK_OutputLog("Failure: Fail to load PDF document, and the error code %d.\r\n", ret);
			return ret;
		}
	}
	else
	{
		ret = FSPDF_Doc_StartLoad(m_file, password, &m_Doc, &docProgress);
		if (FSCRT_ERRCODE_SUCCESS != ret)
		{
			FSDK_OutputLog("Failure: Fail to load PDF document, and the error code %d.\r\n", ret);
			return ret;
		}
		ret = FSCRT_ERRCODE_TOBECONTINUED;
		while (FSCRT_ERRCODE_TOBECONTINUED == ret)
			ret = FSCRT_Progress_Continue(docProgress, NULL);

		FSCRT_Progress_Release(docProgress);
	}

	FSDK_OutputLog("Success: Load PDF document successfully.\r\n");
	m_bPDFDoc = TRUE;
	return ret;
}

FS_BOOL	 CAutoDocument::LoadFDFDocument()
{
	if (NULL == m_file)
		return FALSE;

	CloseDocument();
	FS_RESULT ret = FSFDF_Doc_Load(m_file, &m_Doc);

	if (FSCRT_ERRCODE_SUCCESS != ret)
	{
		FSDK_OutputLog("Failure: Fail to load FDF document, and the error code %d.\r\n", ret);
		return FALSE;
	}

	FSDK_OutputLog("Success: Load FDF document.\r\n");
	m_bPDFDoc = FALSE;
	return TRUE;
}

FS_RESULT CAutoDocument::CreatePDFDocument()
{
	CloseDocument();
	FS_RESULT ret = FSPDF_Doc_Create(&m_Doc);
	if (FSCRT_ERRCODE_SUCCESS != ret)
	{
		FSDK_OutputLog("Failure: Fail to Create PDF document, and the error code %d.\r\n", ret);
	}
	else
	{
		FSDK_OutputLog("Success: Create PDF document.\r\n");
		m_bPDFDoc = TRUE;
	}
	
	return ret;
}

FS_RESULT CAutoDocument::CreateFDFDocument(FS_INT32  nformat)
{	
	CloseDocument();
	FS_RESULT ret = FSFDF_Doc_Create(nformat,&m_Doc);
	if (FSCRT_ERRCODE_SUCCESS != ret)
	{
		FSDK_OutputLog("Failure: Fail to Create FDF document, and the error code %d.\r\n", ret);
	}else
	{
		FSDK_OutputLog("Success: Create FDF document.\r\n");
		m_bPDFDoc = FALSE;
	}
	return ret;
}

FSCRT_DOCUMENT CAutoDocument::GetDocument()
{
	return m_Doc;
}

FSCRT_FILE	CAutoDocument::GetFileHander()
{
	return m_file;
}

FS_RESULT CAutoDocument::LoadAttachments()
{
	FS_RESULT ret = FSPDF_Doc_LoadAttachments(m_Doc,&m_attachments);
	if (FSCRT_ERRCODE_SUCCESS != ret)
	{
		FSDK_OutputLog("Failure: In function FSPDF_Doc_LoadAttachments with error code %d.\r\n", ret);
	}
	else
	{
		FSDK_OutputLog("Success: Load Attachments from pdf file.\r\n");
	}
	return ret;
}

FSPDF_ATTACHMENTS CAutoDocument::GetAttachments()
{
	return m_attachments;
}

FSPDF_ATTACHMENT* CAutoDocument::GetAllAttachment()
{
	//get attachments count
	FS_INT32 count = 0;
	FS_RESULT ret = FSPDF_Attachments_CountAttachment(m_attachments, &count);
	if (FSCRT_ERRCODE_SUCCESS != ret)
	{
		FSDK_OutputLog("Failure: In function FSPDF_Attachments_CountAttachment with error code %d.\r\n", ret);
		return NULL;
	}

	m_pAttachment = new FSPDF_ATTACHMENT[count];
	for (int i = 0;i<count;i++)
	{
		ret = FSPDF_Attachments_GetAttachment(m_attachments, i, &m_pAttachment[i]);
		if (ret != FSCRT_ERRCODE_SUCCESS)
		{
			FSDK_OutputLog("Failure: In function FSPDF_Attachments_GetAttachment with error code %d.\r\n", ret);
			return NULL;
		}
	}

	FSDK_OutputLog("Success: Get a attachment from attachments.\r\n");
	return m_pAttachment;
}

FSCRT_BSTR*	CAutoDocument::GetAllAttachmentFileName()
{
	//get attachments count
	FS_RESULT ret = FSPDF_Attachments_CountAttachment(m_attachments, &m_AttachmentFileCount);
	if (FSCRT_ERRCODE_SUCCESS != ret)
	{
		FSDK_OutputLog("Failure: In function FSPDF_Attachments_CountAttachment with error code %d.\r\n", ret);
		return NULL;
	}
	m_pAttachmentFileName = new FSCRT_BSTR[m_AttachmentFileCount];

	for (int j=0;j<m_AttachmentFileCount;j++)
	{
		FSCRT_BStr_Init(&m_pAttachmentFileName[j]);
		ret = FSPDF_Attachment_GetFileName(m_pAttachment[j], &m_pAttachmentFileName[j]);
		if (ret != FSCRT_ERRCODE_SUCCESS)
		{
			FSDK_OutputLog("Failure: In function FSPDF_Attachment_GetFileName with error code %d.\r\n", ret);
			return NULL;
		}
	}

	FSDK_OutputLog("Success: Get file name from attachment.\r\n");
	return m_pAttachmentFileName;
}

void CAutoDocument::SaveAttachmentByFileByName(string path)
{
	//get attachments count
	size_t   i;
	FS_INT32 count = 0;
	FS_RESULT ret = FSPDF_Attachments_CountAttachment(m_attachments, &count);
	if (FSCRT_ERRCODE_SUCCESS != ret)
	{
		FSDK_OutputLog("Failure: In function FSPDF_Attachments_CountAttachment with error code %d.\r\n", ret);
		return;
	}

	//save attachments by filename
	for (int k =0; k<count; k++)
	{
		string savePath = path + '/';
#if defined(_WIN64)||defined(_WIN32)
		FS_DWORD length = 0;
		ret = FSCRT_UTF8_ToUTF16LE(&m_pAttachmentFileName[k], NULL, &length);
		FS_WORD* buffer = new FS_WORD[length+1];
		ret = FSCRT_UTF8_ToUTF16LE(&m_pAttachmentFileName[k], buffer, &length);
		buffer[length] = '\0';
		char* curLocale = setlocale(LC_ALL, NULL);
		setlocale(LC_ALL, "");
		size_t charSize = wcslen((wchar_t*)buffer)*2;
		char* str = new char[charSize];
		memset(str,0,charSize);
		//wcstombs(str,(wchar_t*)buffer,charSize);
		wcstombs_s(&i,str,charSize,(wchar_t*)buffer,(FS_DWORD)length);
		savePath.append(str);

		delete []str;
		delete []buffer;
		str = NULL;
		buffer = NULL;
#else
		savePath += m_pAttachmentFileName[k].str;
#endif
		FSCRT_FILE saveFile = FSDK_OpenFile(savePath.c_str(), "wb");
		if (!saveFile) return;

		ret = FSPDF_Attachment_WriteToFile(m_pAttachment[k], saveFile);
		if (ret != FSCRT_ERRCODE_SUCCESS)
		{
			FSDK_OutputLog("Failure: In function FSPDF_Attachment_WriteToFile with error code %d.\r\n", ret);	
		}
		else
		{
			string filename = FSDK_GetFileName(savePath.c_str());
			FSDK_OutputLog("Success: Save attachment to %s\r\n", filename.c_str());
		}
		FSCRT_File_Release(saveFile);
		return;
	}

}

FS_RESULT CAutoDocument::CreateAttachment()
{
	FS_RESULT ret = FSPDF_Attachment_Create(m_Doc, &m_attachment);
	if (FSCRT_ERRCODE_SUCCESS != ret)
	{
		FSDK_OutputLog("Failure: In function FSPDF_Attachment_Create with error code %d.\r\n", ret);
	}
	else
	{
		FSDK_OutputLog("Success: Create a attachment.\r\n");
	}
	return ret;
}

FSPDF_ATTACHMENT CAutoDocument::GetAttachment()
{
	return m_attachment;
}

FS_INT32 CAutoDocument::GetPagesCount()
{
	FS_INT32 count = 0;
	FS_RESULT ret = FSPDF_Doc_CountPages(m_Doc, &count);
	if (FSCRT_ERRCODE_SUCCESS != ret)
		FSDK_OutputLog("Failure: In function FSPDF_Attachment_Create with error code %d.\r\n", ret);

	return count;
}

/*******************************************************************************/
/*				auto page class											   */
/*******************************************************************************/
CAutoPDFPage::CAutoPDFPage()
	:m_pdfPage(NULL)
	,m_textPage(NULL)
	,m_bIsLoadAnnots(FALSE)
{
}

CAutoPDFPage::~CAutoPDFPage()
{
	if (m_bIsLoadAnnots)
	{
		FSPDF_Page_UnloadAnnots(m_pdfPage);
	}
	ClearPage();
}

void CAutoPDFPage::ClearPage()
{
	if (m_textPage)
	{
		FSPDF_TextPage_Release(m_textPage);
		m_textPage = NULL;
	}
	if (m_pdfPage)
	{
		FSPDF_Page_Clear(m_pdfPage);
		m_pdfPage = NULL;
	}
}

FS_RESULT CAutoPDFPage::LoadPDFPage(FSCRT_DOCUMENT pdfDoc,FS_INT32 nIndex)
{
	ClearPage();
	FS_RESULT ret = FSPDF_Doc_GetPage(pdfDoc, nIndex, &m_pdfPage);
	if (FSCRT_ERRCODE_SUCCESS != ret)
	{
		FSDK_OutputLog("Failure: In function FSPDF_Doc_GetPage(pdfDoc, %d, pdfPage) with error code %d.\r\n", nIndex, ret);
	} else {
		FSDK_OutputLog("Success: Load the PDF page with index %d.\r\n", nIndex);
	}
	return ret;
}

FS_RESULT CAutoPDFPage::CreatePDFPage(FSCRT_DOCUMENT pdfDoc,FS_INT32 nIndex)
{
	ClearPage();
	FS_RESULT ret = FSPDF_Page_Create(pdfDoc, nIndex, &m_pdfPage);
	if (FSCRT_ERRCODE_SUCCESS != ret)
	{
		FSDK_OutputLog("Failure: In function FSPDF_Page_Create(pdfDoc, %d, pdfPage) with error code %d.\r\n", nIndex, ret);
	} else {
		FSDK_OutputLog("Success: Create the PDF page with index %d.\r\n", nIndex);
	}
	return ret;
}

FSCRT_PAGE	CAutoPDFPage::GetPDFPage()
{
	return m_pdfPage;
}

FSPDF_TEXTPAGE	CAutoPDFPage::GetPDFTextPage()
{
	return m_textPage;
}

FS_RESULT	CAutoPDFPage::ParsePage()
{
	FSCRT_PROGRESS progress;
	FS_RESULT ret = FSPDF_Page_StartParse(m_pdfPage, FSPDF_PAGEPARSEFLAG_NORMAL, &progress);
	if (FSCRT_ERRCODE_SUCCESS != ret)
	{
		FSDK_OutputLog("Failure: In function FSPDF_Page_StartParse with error code %d.\r\n", ret);
		return ret;
	}

	//Continue to parse page, without pausing.
	ret = FSCRT_Progress_Continue(progress, NULL);
	if (FSCRT_ERRCODE_FINISHED != ret)
	{
		FSDK_OutputLog("Failure: In function FSCRT_Progress_Continue with error code %d.\r\n", ret);
	} else {
		ret = FSCRT_ERRCODE_SUCCESS;
		FSDK_OutputLog("Success: Parse the PDF page.\r\n");
	}
	FSCRT_Progress_Release(progress);
	return ret;
}


FS_RESULT CAutoPDFPage::LoadTextPage()
{
	FS_RESULT ret = FSPDF_TextPage_Load(m_pdfPage,&m_textPage);
	if (FSCRT_ERRCODE_SUCCESS != ret){
		FSDK_OutputLog("Failure: In function FSPDF_TextPage_Load with error code %d.\r\n", ret);
	} else {
		FSDK_OutputLog("Success: Load the text page.\r\n", ret);
	}
	return ret;
}

FS_RESULT CAutoPDFPage::LoadAnnots()
{
	m_bIsLoadAnnots = TRUE;
	FS_RESULT ret = FSPDF_Page_LoadAnnots(m_pdfPage);
	if (FSCRT_ERRCODE_SUCCESS != ret){
		FSDK_OutputLog("Failure: In function FSPDF_Page_LoadAnnots with error code %d.\r\n", ret);
	} else {
		FSDK_OutputLog("Success: Load the PDF page's annotations.\r\n", ret);
	}
	return ret;
}

/*******************************************************************************/
/*				auto reflow page class											   */
/*******************************************************************************/
CAutoReflowPage::CAutoReflowPage()
	:m_reflowPage(NULL)
	,m_fScreenHeight(0)
	,m_fScreenWidth(0)
	,m_fLineSpace(0)
	,m_fTopSpace(0)
	,m_fLastYPixel(0)
	,m_fContentHeight(0)
{
	m_dwReflowFlag= FSPDF_REFLOWFLAG_NORMAL|FSPDF_REFLOWFLAG_IMAGE|FSPDF_REFLOWFLAG_NOTRUNCATE;
}

CAutoReflowPage::~CAutoReflowPage()
{
	if (m_reflowPage)
	{
		FSPDF_ReflowPage_Release(m_reflowPage);
		m_reflowPage = NULL;
	}
}

FSPDF_REFLOWPAGE CAutoReflowPage::GetReflowPage()
{
	return m_reflowPage;
}

FS_FLOAT CAutoReflowPage::GetScreenWidth()
{
	return m_fScreenWidth;
}

FS_FLOAT CAutoReflowPage::GetScreenHeight()
{
	return m_fScreenHeight;
}

FS_RESULT CAutoReflowPage::ReleaseReflowPage()
{
	if (NULL != m_reflowPage)
	{
		FSPDF_ReflowPage_Release(m_reflowPage);
		m_reflowPage = NULL;
	}
	return FSCRT_ERRCODE_SUCCESS;
}

void CAutoReflowPage::SetScreenParam(FS_FLOAT fScreenWidth,FS_FLOAT fScreenHeight)
{
	m_fScreenHeight = fScreenHeight;
	m_fScreenWidth = fScreenWidth;
}

void CAutoReflowPage::SetLineSpace(FS_FLOAT fLineSpace)
{
	m_fLineSpace = fLineSpace;	
}

void CAutoReflowPage::SetTopSpace(FS_FLOAT fTopSpace)
{
	m_fTopSpace = fTopSpace;
}

void CAutoReflowPage::SetLastYPixel(FS_FLOAT fLastYPixel)
{
	m_fLastYPixel = fLastYPixel;
}

FS_FLOAT CAutoReflowPage::GetLastYPixel()
{
	return m_fLastYPixel;
}

FS_FLOAT CAutoReflowPage::GetContentHeight()
{
	return m_fContentHeight;
}

FS_RESULT CAutoReflowPage::CreateReflowPage(const FSCRT_PAGE pdfPage)
{
	//Create reflow page.
	FS_RESULT ret = FSPDF_ReflowPage_Create(pdfPage, &m_reflowPage);
	if (FSCRT_ERRCODE_SUCCESS != ret)
	{
		FSDK_OutputLog("Failure: In function FSPDF_ReflowPage_Create with error code %d.\r\n", ret);
		return ret;
	}

	if (m_fLineSpace > 0)
	{
	   FSPDF_ReflowPage_SetLineSpace(m_reflowPage,m_fLineSpace);
	}
	if (m_fTopSpace > 0)
	{
		FSPDF_ReflowPage_SetTopSpace(m_reflowPage,m_fTopSpace);
	}

	//Set the screen size for reflow. This must be set before parse reflow page.
	FSPDF_ReflowPage_SetSize(m_reflowPage, m_fScreenWidth, m_fScreenHeight);

	
	//Parse reflow page before render it.
	//Here, choose to parse reflow page in normal mode--only reflow text.
	FSCRT_PROGRESS reflowProgress = NULL;
	/*m_dwReflowFlag = FSPDF_REFLOWFLAG_IMAGE | FSPDF_REFLOWFLAG_NOTRUNCATE*/
	ret = FSPDF_ReflowPage_StartParse(m_reflowPage, m_dwReflowFlag, &reflowProgress);
	if (FSCRT_ERRCODE_SUCCESS == ret)
	{		
		do 
		{
			ret = FSCRT_Progress_Continue(reflowProgress, NULL);
			if (FSCRT_ERRCODE_FINISHED == ret)
				ret = FSCRT_ERRCODE_SUCCESS;
		} while (FSCRT_ERRCODE_TOBECONTINUED == ret);

		FSDK_OutputLog("Success: The page has reflowed over.\r\n");
	}
	else if (FSCRT_ERRCODE_FINISHED == ret)
	{
		ret = FSCRT_ERRCODE_SUCCESS;
		FSDK_OutputLog("Success: The page has reflowed over.\r\n");	   
	}
	else
	{
		FSDK_OutputLog("Failure: In function FSPDF_ReflowPage_StartParse with error code %d.\r\n", ret);
	}

	if (FSCRT_ERRCODE_SUCCESS == ret)
	{
		FS_FLOAT fReflowPageWidth;
		FS_FLOAT fReflowPageHeight;

		ret = FSPDF_ReflowPage_GetContentSize(m_reflowPage, &fReflowPageWidth, &fReflowPageHeight);
		if (FSCRT_ERRCODE_SUCCESS != ret)
		{
			FSDK_OutputLog("Failure: In function FSPDF_ReflowPage_GetContentSize with error code %d.\r\n", ret);
			//return ret;
		}
		else
		{
			//m_fLastYPixel = 0;
			m_fContentHeight = fReflowPageHeight ;
			m_fContentWidth = fReflowPageWidth;
		}
	}

	//Release progress resources.
	FSCRT_Progress_Release(reflowProgress); 
	reflowProgress = NULL;
	
	return ret;
}

FS_FLOAT CAutoReflowPage::GetContentWidth()
{
	return m_fContentWidth;
}

void CAutoReflowPage::SetReflowFlag(FS_DWORD dwReflowFlag)
{
	m_dwReflowFlag = dwReflowFlag;
}

FS_FLOAT CAutoReflowPage::GetTopSpace()
{
	return m_fTopSpace;
}


/*******************************************************************************/
/*				auto image class											   */
/*******************************************************************************/
CAutoImage::CAutoImage()
	:m_image(NULL)
	,m_bitmap(NULL)
	,m_imageFile(NULL)
	,m_file(NULL)
	,m_renderer(NULL)
	,m_pdfRenderContext(NULL)
	,m_fImageWidth(0)
	,m_fImageHeight(0)
{
}

CAutoImage::CAutoImage(const char* fileName, const char* fileMode, BOOL useDef)
	:m_image(NULL)
	,m_bitmap(NULL)
	,m_imageFile(NULL)
	,m_file(NULL)
	,m_renderer(NULL)
	,m_pdfRenderContext(NULL)
	,m_fImageWidth(0)
	,m_fImageHeight(0)
{
	m_file = FSDK_OpenFile(fileName, fileMode, useDef);
}

CAutoImage::~CAutoImage()
{
	if (m_image)
	{
		FSCRT_Image_Release(m_image);
		m_image = NULL;
	}

	if (m_bitmap)
	{
		FSCRT_Bitmap_Release(m_bitmap);
		m_bitmap = NULL;
	}

	if (m_imageFile)
	{
		FSCRT_ImageFile_Release(m_imageFile);
		m_imageFile = NULL;
	}
	
	if (m_file)
	{
		FSCRT_File_Release(m_file);
		m_file = NULL;
	}

	ReleaseRenderResource();	
}

FSCRT_IMAGE CAutoImage::GetImage()
{
	return m_image;
}

FSCRT_BITMAP CAutoImage::GetBitmap()
{
	return m_bitmap;
}

FSCRT_IMAGEFILE CAutoImage::GetImageFile()
{
	return m_imageFile;
}

FSCRT_FILE CAutoImage::GetFile()
{
	return m_file;
}

FS_RESULT CAutoImage::LoadImage()
{
	FS_RESULT ret = FSCRT_Image_LoadFromFile(m_file, &m_image);
	if (FSCRT_ERRCODE_SUCCESS != ret){
		FSDK_OutputLog("Failure: In function FSCRT_Image_LoadFromFile with error code %d.\r\n", ret);
	} else {
		FSDK_OutputLog("Success: Load the Image.\r\n", ret);
	}
	return ret;
}

FS_RESULT CAutoImage::GenerateBitmapbyBarcode(const FSCRT_BSTR* info, FS_INT32 codeFormat, FS_INT32 unitWidth, FS_INT32 unitHeight, FS_INT32 qrLevel)
{
	FS_RESULT ret = FSCRT_Barcode_GenerateBitmap(info, codeFormat, unitWidth, unitHeight, qrLevel, &m_bitmap);
	if (FSCRT_ERRCODE_SUCCESS != ret){
		FSDK_OutputLog("Failure: In function GenerateBitmapbyBarcode with error code %d.\r\n", ret);
	} else {
		FSDK_OutputLog("Success: Generate the bitmap of barcode.\r\n", ret);
	}
	return ret;
}

FS_RESULT CAutoImage::CreateImageFile(FSCRT_FILE file, FS_INT32 imageType, FS_INT32 frameCount)
{
	FS_RESULT ret = FSCRT_ImageFile_Create(file, imageType, frameCount, &m_imageFile);
	if (FSCRT_ERRCODE_SUCCESS != ret){
		FSDK_OutputLog("Failure: In function FSCRT_ImageFile_Create with error code %d.\r\n", ret);
	} else {
		FSDK_OutputLog("Success: Load the ImageFile successfully.\r\n");
	}
	return ret;
}

FS_RESULT CAutoImage::PageToBitmap(FSCRT_PAGE page)
{
	//Get the given page's size.
	FS_FLOAT width = 0, height = 0;
	FS_RESULT ret = FSPDF_Page_GetSize(page, &width, &height);
	if (ret != FSCRT_ERRCODE_SUCCESS) 
	{
		FSDK_OutputLog("Failure: In function FSPDF_Page_GetSize with error code %d.\r\n", ret);
		return ret;
	}
	//release the previous bitmap
	if (m_bitmap)
	{
		FSCRT_Bitmap_Release(m_bitmap);
		m_bitmap = NULL;
	}
	//Get a bmp handler to hold bmp data from rendering progress.
	ret = FSCRT_Bitmap_Create((FS_INT32)width, (FS_INT32)height, FSCRT_BITMAPFORMAT_24BPP_BGR, NULL, 0, &m_bitmap);
	if (ret != FSCRT_ERRCODE_SUCCESS) 
	{
		FSDK_OutputLog("Failure: In function FSCRT_Bitmap_Create with error code %d.\r\n", ret);
		return ret;
	}
		
	//Set rect area and color space of bitmap.
	FSCRT_RECT rect = {0, 0, (FS_INT32)width, (FS_INT32)height};
	ret = FSCRT_Bitmap_FillRect(m_bitmap, FSCRT_ARGB_Encode(0xff, 0xff, 0xff, 0xff), &rect);
	if (ret != FSCRT_ERRCODE_SUCCESS) 
	{
		FSDK_OutputLog("Failure: In function FSCRT_Bitmap_FillRect with error code %d.\r\n", ret);
		return ret;
	}

	//Get the page's matrix.
	FSCRT_MATRIX mt;
	ret = FSPDF_Page_GetMatrix(page, 0, 0, (FS_INT32)width, (FS_INT32)height, 0, &mt);
	if (ret != FSCRT_ERRCODE_SUCCESS) 
	{
		FSDK_OutputLog("Failure: In function FSPDF_Page_GetMatrix with error code %d.\r\n", ret);
		return ret;
	}

	//Create a renderer on the given bitmap.
	FSCRT_RENDERER renderer;
	ret = FSCRT_Renderer_CreateOnBitmap(m_bitmap, &renderer);
	if (ret != FSCRT_ERRCODE_SUCCESS) 
	{
		FSDK_OutputLog("Failure: In function FSCRT_Renderer_CreateOnBitmap with error code %d.\r\n", ret);
		return ret;
	}

	//Create a render context and render a page to get a bmp later.
	FSPDF_RENDERCONTEXT rendercontext;
	ret = FSPDF_RenderContext_Create(&rendercontext);
	if (ret != FSCRT_ERRCODE_SUCCESS) 
	{
		//Release render
		FSCRT_Renderer_Release(renderer);
		FSDK_OutputLog("Failure: In function FSPDF_RenderContext_Create with error code %d.\r\n", ret);
		return ret;
	}

	//Set the matrix of the given render context.
	ret = FSPDF_RenderContext_SetMatrix(rendercontext, &mt);
	if (ret != FSCRT_ERRCODE_SUCCESS) 
	{
		//Release render
		FSCRT_Renderer_Release(renderer);
		//Release render context
		FSPDF_RenderContext_Release(rendercontext);
		FSDK_OutputLog("Failure: In function FSPDF_RenderContext_SetMatrix with error code %d.\r\n", ret);
		return ret;
	}

	//Start to render with the given render context, renderer, page to get the render progress.
	FSCRT_PROGRESS renderProgress = NULL;
	ret = FSPDF_RenderContext_StartPage(rendercontext, renderer, page,FSPDF_PAGERENDERFLAG_NORMAL, &renderProgress);
	if (ret != FSCRT_ERRCODE_SUCCESS) 
	{
		//Release render
		FSCRT_Renderer_Release(renderer);
		//Release render context
		FSPDF_RenderContext_Release(rendercontext);
		FSDK_OutputLog("Failure: In function FSPDF_RenderContext_StartPage with error code %d.\r\n", ret);
		return ret;
	}

	//Continue to render if it's not completed.
	ret = FSCRT_Progress_Continue(renderProgress, NULL);
	if (ret != FSCRT_ERRCODE_FINISHED) 
	{
		FSDK_OutputLog("Failure: In function FSCRT_Progress_Continue with error code %d.\r\n", ret);
	}

	if (ret == FSCRT_ERRCODE_FINISHED) 
	{
		ret = FSCRT_ERRCODE_SUCCESS;
		FSDK_OutputLog("Success: Render the page to bitmap.\r\n");
	}

	//Release render progress 
	FSCRT_Progress_Release(renderProgress);
	//Release render
	FSCRT_Renderer_Release(renderer);
	//Release render context
	FSPDF_RenderContext_Release(rendercontext);
	return ret;
}

FS_RESULT	CAutoImage::GetCurrentFramBitmap()
{
	FS_RESULT ret = FSCRT_Image_GetCurrentFrameBitmap(m_image, &m_bitmap);
	if (FSCRT_ERRCODE_SUCCESS != ret){
		FSDK_OutputLog("Failure: In function GetCurrentFramBitmap with error code %d.\r\n", ret);
	} else {
		FSDK_OutputLog("Success: Get Current Frame Bitmap.\r\n", ret);
	}
	return ret;
}

FS_RESULT CAutoImage::CreateRenderResource(FS_FLOAT fImageWidth,FS_FLOAT fImageHeight,FSCRT_RECT rectMargin)
{
	FS_RESULT ret = FSCRT_ERRCODE_ERROR;

	m_fImageWidth = fImageWidth;
	m_fImageHeight = fImageHeight;

	if (m_fImageWidth * m_fImageHeight == 0)
	{
		FSDK_OutputLog("Failure: In function CAutoImage::CreateRenderResource with error code %d, Image Size is incorrect.\r\n", ret);
		return ret;
	}

	ReleaseRenderResource();

	//Create bitmap handle.
	FS_FLOAT fWidth  = (FS_FLOAT)(m_fImageWidth  + 0.5) ; // 
	FS_FLOAT fHeight = (FS_FLOAT)(m_fImageHeight + 0.5) ;  // 
	ret = FSCRT_Bitmap_Create((FS_INT32)fWidth, (FS_INT32)fHeight,FSCRT_BITMAPFORMAT_32BPP_BGRA, NULL, 0, &m_bitmap);
	if (FSCRT_ERRCODE_SUCCESS != ret)
	{
		FSDK_OutputLog("Failure: In function FSCRT_Bitmap_Create with error code %d.\r\n", ret);
		return ret;
	}

	ret = FSCRT_Renderer_CreateOnBitmap(m_bitmap, &m_renderer);
	if (FSCRT_ERRCODE_SUCCESS != ret)
	{
		FSDK_OutputLog("Failure: In function FSCRT_Renderer_CreateOnBitmap with error code %d.\r\n", ret);
		return ret ;
	}

	// set Margin top and bottom
	//if (m_fMarginBottom > 0 || m_fMarginTop > 0)
	{
		FSCRT_RECT rect = {rectMargin.left,(FS_INT32)rectMargin.top,(FS_INT32)(fWidth-rectMargin.right), (FS_INT32)(fHeight - rectMargin.bottom)};
		ret = FSCRT_Renderer_SetClipRect(m_renderer, &rect);
	}

	ret = FSPDF_RenderContext_Create(&m_pdfRenderContext);
	if (FSCRT_ERRCODE_SUCCESS != ret) 
	{
		FSDK_OutputLog("Failure: In function FSPDF_RenderContext_Create with error code %d.\r\n", ret);
	}

	return ret ;
}

FS_RESULT CAutoImage::ReleaseRenderResource()
{
	if (NULL != m_bitmap)
	{
		FSCRT_Bitmap_Release(m_bitmap);
	}	
	if (NULL != m_renderer)
	{
		FSCRT_Renderer_Release(m_renderer);
	}	
	if (NULL != m_pdfRenderContext)
	{
		FSPDF_RenderContext_Release(m_pdfRenderContext); 
	}	

	m_bitmap = NULL;
	m_renderer = NULL;
	m_pdfRenderContext = NULL;

	return FSCRT_ERRCODE_SUCCESS;
}

FS_RESULT CAutoImage::FillBitMapWithWhite()
{
	FS_RESULT ret = FSCRT_ERRCODE_ERROR;
	if (m_fImageWidth * m_fImageHeight == 0)
	{
		FSDK_OutputLog("Failure: In function CAutoImage::FillBitMapWithWhite with error code %d, Image Size is incorrect.\r\n", ret);
		return ret;
	}
	//Fill the bitmap white.
	FSCRT_RECT rect = {0, 0, (FS_INT32)(m_fImageWidth+0.5), (FS_INT32)(m_fImageHeight+0.5)};
	ret = FSCRT_Bitmap_FillRect(m_bitmap, FSCRT_ARGB_Encode(0xff, 0xff, 0xff, 0xff), &rect);
	if (FSCRT_ERRCODE_SUCCESS != ret)
	{
		FSDK_OutputLog("Failure: In function FSCRT_Bitmap_FillRect with error code %d.\r\n", ret);		
	}
	return ret;
}

FS_RESULT CAutoImage::ReflowPageToBitmap(CAutoReflowPage* autoReflowPage,FSCRT_RECT rectRegion)
{
	FS_RESULT ret = FSCRT_ERRCODE_ERROR;

	if (NULL == autoReflowPage)
	{
		FSDK_OutputLog("Failure: In function CAutoImage::ReflowPageToBitmap, with null input param.\r\n");  
		return ret;
	}

	FSCRT_PROGRESS renderProgress = NULL;

	FSCRT_MATRIX matrix;

	FS_INT32 nX = rectRegion.left;   
	FS_INT32 nY = rectRegion.top;
	FS_INT32 nWidth = rectRegion.right - rectRegion.left;
	FS_INT32 nHeight = rectRegion.bottom - rectRegion.top;
	
	FSPDF_ReflowPage_GetMatrix(autoReflowPage->GetReflowPage(), nX, nY, nWidth,nHeight, 0, &matrix);

	//Set render context matrix with reflow page matrix.
	FSPDF_RenderContext_SetMatrix(m_pdfRenderContext, &matrix);
	//Start render reflow page.
	ret = FSPDF_RenderContext_StartReflowPage(m_pdfRenderContext, m_renderer, autoReflowPage->GetReflowPage(), &renderProgress);
	if (FSCRT_ERRCODE_SUCCESS != ret) 
	{
		//Release renderer handle.
		//FSCRT_Renderer_Release(renderer); renderer = NULL;
		FSDK_OutputLog("Failure: In function FSPDF_RenderContext_StartReflowPage with error code %d.\r\n", ret);
		return ret;
	}
	//Continue the progress of rendering reflow page.
	do 
	{
		ret = FSCRT_Progress_Continue(renderProgress, NULL);
	} while (FSCRT_ERRCODE_TOBECONTINUED == ret);

	if (FSCRT_ERRCODE_FINISHED == ret) 
	{
		ret = FSCRT_ERRCODE_SUCCESS;
		FSDK_OutputLog("Success: Render the reflowpage to bitmap.\r\n");
	}
	else
	{
		FSDK_OutputLog("Failure: In function FSCRT_Progress_Continue with error code %d.\r\n", ret);
	}
	//Release progress resources.
	FSCRT_Progress_Release(renderProgress); renderProgress = NULL;
	//Release renderer handle.
	//FSCRT_Renderer_Release(renderer); renderer = NULL;
	////Release render context handle.
	//FSPDF_RenderContext_Release(pdfRenderContext); pdfRenderContext = NULL;
	return ret;
}

/*******************************************************************************/
/*				auto pageobject class											   */
/*******************************************************************************/
CAutoPageObject::CAutoPageObject()
	:m_obj(NULL)
	,m_bInsert(FALSE)
	,m_page(NULL)
{

}

CAutoPageObject::~CAutoPageObject()
{
	Clear();
}

void CAutoPageObject::Clear()
{
	//if the page object don`t insert to the page. m_bInsert = FALSE.
	if (!m_bInsert && m_obj && m_page)
	{
		FSPDF_PageObject_Release(m_page, m_obj);
		m_obj = NULL;
		m_page = NULL;
	}
}

FS_RESULT CAutoPageObject::CreateTextObject(FSCRT_PAGE page)
{
	Clear();
	FS_RESULT ret = FSPDF_TextObject_Create(page, &m_obj);
	if (ret != FSCRT_ERRCODE_SUCCESS)
	{
		FSDK_OutputLog("Failure: In function FSPDF_TextObject_Create with error code %d.\r\n", ret);
		return ret;
	}
	m_page = page;
	FSDK_OutputLog("Success: Create Text Object.\r\n");
	return ret;
}

FS_RESULT CAutoPageObject::CreateImageObject(FSCRT_PAGE page)
{
	Clear();
	FS_RESULT ret = FSPDF_ImageObject_Create(page, &m_obj);
	if (ret != FSCRT_ERRCODE_SUCCESS)
	{
		FSDK_OutputLog("Failure: In function FSPDF_ImageObject_Create with error code %d.\r\n", ret);
		return ret;
	}
	m_page = page;
	FSDK_OutputLog("Success: Create Image Object.\r\n");
	return ret;
}

FS_RESULT CAutoPageObject::CreateFormXObject(FSCRT_PAGE page)
{
	Clear();
	FS_RESULT ret = FSPDF_FormXObject_Create(page, &m_obj);
	if (ret != FSCRT_ERRCODE_SUCCESS)
	{
		FSDK_OutputLog("Failure: In function FSPDF_FormXObject_Create with error code %d.\r\n", ret);
		return ret;
	}
	m_page = page;
	FSDK_OutputLog("Success: Create Form XObject.\r\n");
	return ret;
}

FS_RESULT CAutoPageObject::CreatePathObject(FSCRT_PAGE page)
{
	Clear();
	FS_RESULT ret = FSPDF_PathObject_Create(page, &m_obj);
	if (ret != FSCRT_ERRCODE_SUCCESS)
	{
		FSDK_OutputLog("Failure: In function FSPDF_PathObject_Create with error code %d.\r\n", ret);
		return ret;
	}
	m_page = page;
	FSDK_OutputLog("Success: Create Path Object.\r\n");
	return ret;
}

FSPDF_PAGEOBJECT CAutoPageObject::GetObject()
{
	return m_obj;
}

FS_RESULT CAutoPageObject::InsertGenerateContents(FSCRT_PAGE page, FSPDF_PAGEOBJECTS pageObjs, FS_INT32 typeFilter, FS_INT32 index)
{
	//Insert a page object
	FS_RESULT ret = FSPDF_PageObjects_InsertObject(page, pageObjs, typeFilter, index, m_obj);
	if (ret != FSCRT_ERRCODE_SUCCESS)
	{
		FSDK_OutputLog("Failure: In function FSPDF_PageObjects_InsertObject with error code %d.\r\n", ret);
		return ret;
	}
	m_bInsert = TRUE;
	//Generate PDF page contents.
	ret = FSPDF_PageObjects_GenerateContents(page, pageObjs);
	if (ret != FSCRT_ERRCODE_SUCCESS)
	{
		FSDK_OutputLog("Failure: In function FSPDF_PageObjects_GenerateContents with error code %d.\r\n", ret);
		return ret;
	}

	FSDK_OutputLog("Success: Generate PDF page contents.\r\n");
	return ret;
}

/*******************************************************************************/
/* Library management														  */
/*******************************************************************************/
//Extension to allocate memory buffer. Implementation to FSCRT_MEMMGRHANDLER::Alloc
static FS_LPVOID	FSDK_Alloc(FS_LPVOID clientData, FS_DWORD size)
{
	return malloc((size_t)size);
}

//Extension to reallocate memory buffer. Implementation to FSCRT_MEMMGRHANDLER::Realloc
static FS_LPVOID	FSDK_Realloc(FS_LPVOID clientData, FS_LPVOID ptr, FS_DWORD newSize)
{
	return realloc(ptr, (size_t)newSize);
}

//Extension to free memory buffer. Implementation to FSCRT_MEMMGRHANDLER::Free
static void			FSDK_Free(FS_LPVOID clientData, FS_LPVOID ptr)
{
	free(ptr);
}

//Global variables for extension manager
static FSCRT_MEMMGRHANDLER	g_MemMgrHandler = {NULL, FSDK_Alloc, FSDK_Realloc, FSDK_Free};
static FS_LPVOID			g_pGlobalBuffer = NULL;

//Default size of global buffer, 16MB
#define FSDK_GLOBALBUFFER_SIZE	(1024 * 1024 * 16)

BOOL FSDK_InitializeLibray(BOOL useDefMgr)
{
	FS_RESULT ret;
	if (useDefMgr){ //if use default manager
		ret = FSCRT_Library_CreateDefaultMgr();
		if (ret == FSCRT_ERRCODE_SUCCESS)
			FSDK_OutputLog("Success: Create the default Foxit PDF SDK manager.\r\n");
		else
			FSDK_OutputLog("Failure: In function FSCRT_Library_CreateDefaultMgr with error code %d.\r\n", ret);
	}
	else //if use extension manager
	{
		g_pGlobalBuffer = malloc(FSDK_GLOBALBUFFER_SIZE);
		if (!g_pGlobalBuffer) 
		{
			FSDK_OutputLog("Failure: Fail to malloc the buffer.\r\n");
			return FALSE;
		}
		ret = FSCRT_Library_CreateMgr(g_pGlobalBuffer, FSDK_GLOBALBUFFER_SIZE, &g_MemMgrHandler);
		if (ret == FSCRT_ERRCODE_SUCCESS)
			FSDK_OutputLog("Success: Create the Foxit PDF SDK manager.\r\n");
		else
			FSDK_OutputLog("Failure: In function FSCRT_Library_CreateMgr with error code %d.\r\n", ret);
	}
	if (ret == FSCRT_ERRCODE_SUCCESS)
	{
		FSCRT_BSTR license_id;
		FSCRT_BStr_Init(&license_id);
		license_id.str = (FS_LPSTR)"sl+HE+8iIA09c5Shh6L+8IeqjJqaUDKs66Om6n9juT/vZH3c5vrSQw==";
		license_id.len = strlen(license_id.str);
		FSCRT_BSTR unlockCode;
		unlockCode.str = (FS_LPSTR)"8f3w18GNtRkNBDdqtFZScbag2/9vMu8LvjFeiGZUbeEHv3jQN+GLyXf1Nb/cKTAT1gGnzcN2H2qTASFEto9F6nA0UU6TMi6r81W4lgJCH0ZNlXtHAF1M2It0zW87NSS7Fde63UY6xPG34/NVlpEzcdSBuDwtM56pl+nSuWL9HUU1bDY7uvA/V/heDdXzFlC9CXrGMwEq4kLa99GC2MWNj6bpXbkuteaqkrzioGSKjzG4xRQ9Xw06yM/7hHkdnSSFg/PThUIg0eYz9o4X9qsftVuSaum/se1e+FEH5oucEjZI4CEZujN2wO1iEAeS2PbBg5cVPyQ+OXdt4HXtfpfK4HSKft9EqURlpM0m2H8q3fYzf2SdACtzRgwkX2bHQR4DLET8BRs5RYMa8OEIAxk0UgZvQu0vmcEMkD40pAbH6lBNk3g2crYLTAHQKZ0qE/rQgKd7O6Oqt1hhxnKMI2kvB/loRE3IPAdX63rfQFfttL2r3SNfeT2A8fhFZTVdxTpwKSPRxCNCHiyVade05VJapOFqmFXZwawekEz8/WE9dKiq84m9UOjHKG01EWgLXHewUSyUIIdOBbRt71JIicNhCZR+GoD5SwNkDQh1V6ja8cKbz/iu+wgyKEUdnYRtZHBWvJvwTEoUDezRalx7mwU5wwa2acqkB25ZAyMeE/3cpkp5xhwGLPHdDqi9Kcp0EtcsG54omySsxBo4IR08MpTDHbR5xuIMoEoTKNHX6HNPmdYQR+iUkf9dq4c7jhM0OT9h49vMXL3ihX5/lZkOo30sfp/11jQLnAnVF52RIaQA/W/c9QNM1ysbAd80xuii73ZBgC1YwBkELTVzq19lXGNdzaRPz652uZrnCoP3EXf6CNHdyky6uPYOlsndsctHyJ3Gpx5NhgVrrYyKsEbBW35H1lzirGnoT3YhkCfYW2IIB1QORx6n1HoxCV1IkFufDCSG4pkuMwrLBd73/+4ocab34lBN+tpBs/1IbMUOU/2PFCWCjythhHIt5N6z9486qNvpqph/0BfAMAHGrOlR1NaoBXKxjfwH+X/nWckdE4F/M0ZVPxOdvx2FRYNtrml0W0ovlLpBnyg65TEqcQ65pH6l/KV+ToQQ2p6k+roS44cjmcVN/5DaR+wyxaqmjy6gtsjYAV/ph/Ts5IFVwyK+MggVeVi98ttelWnOWuVPXKcdu6HHsI3B8Q9HbpSJlnJvIr3iaBDQZfS8zGlDTFrbneWFgUK9JaT+AR9QWilD7BqoT7DujOrBKQVaKgwaTtoYwR/AVRdC6WOTbMopE0kK/M1ejWmvvN1ewwVVjTGoKW91ph7QOFFEhHI0BxogZcQ2rivDfLP59/pz8rR54sw9kBoMC74b1U+jI2SHBPxa2xAIYSoFK28DRjJkXyBGIc33+u/5KxbkaGw5e9O/z3h1UzlfD9bVoHhngxVXjXA7fk+45i3dfutfWPdfaQHQok8eRMu0srRIZ99RPwP27WJ5quKTqSjPaF35cg/5WVzbxcnstlqQT55sYqrs7/ws+zufkf0RpHd/m25LmDS4urP3c+l1JLUH5lZEIg5ouxDcKRb1edXZ5ac0Z59BqSlmx+f/N3CKiiIygczfVqqZASjacSyYJ34qc3GrwoYeQDvW7CtojPH8Z/uR82I7xLfA3Cad2jrlI6bMYSkwiKGVwFsWDxy50bs33HzajLhI7Qp4hZfvc8vlf6HMsI72p3q366vU0oUKz1JXsifHLlfcjNo=";
		unlockCode.len = strlen(unlockCode.str);
		ret = FSCRT_License_UnlockLibrary(&license_id, &unlockCode);
		if (FSCRT_ERRCODE_SUCCESS == ret)
		{
			FSDK_OutputLog("Success: Unlock the library.\r\n");
			return TRUE;
		}
		else
		{
			FSDK_FinalizeLibrary();
			FSDK_OutputLog("Failure: Fail to unlock the library, and the error code %d.\r\n", ret);	
			return FALSE;
		}
	}
	FSDK_OutputLog("Failure: Fail to initialize library and the error code %d.\r\n", ret);
	return FALSE;
}

void FSDK_FinalizeLibrary()
{
	//Destroy manager
	FSCRT_Library_DestroyMgr();
	FSDK_OutputLog("\r\nFinalize Foxit PDF SDK library.\r\n");

	//Free global buffer for extension manager
	if (!g_pGlobalBuffer) return;
	free(g_pGlobalBuffer);
	g_pGlobalBuffer = NULL;
}

FS_BOOL FSDK_PDFModule_Initialize()
{
	FS_RESULT nRet = FSCRT_PDFModule_Initialize();
	if (FSCRT_ERRCODE_SUCCESS == nRet)	{
		FSDK_OutputLog("Success: Initialize the PDF module.\r\n");
		return TRUE;
	} else {
		FSDK_OutputLog("Failure: Initialize PDF module with error code %d.\r\n", nRet);
		return FALSE;
	}
}

void FSDK_PDFModule_Finalize()
{
	FSCRT_PDFModule_Finalize();
	FSDK_OutputLog("\r\nFinalize PDF module.");
}

/*******************************************************************************/
/* Directories																 */
/*******************************************************************************/
//Static string object to save the current executing file path
static string g_strExeFilePath = "";
#ifdef FSDK_IOS_DEMO
static string g_strDocuments = "";
#endif

void FSDK_SetExecuteFilePath(int argc, char* argv[])
{
	//Save path
	g_strExeFilePath = argv[0];
#ifdef FSDK_IOS_DEMO
	g_strDocuments = argv[1];
#endif
}

string FSDK_GetExecuteFolder()
{
	//Determine whether the current executing file path contains directory delimiter
	string exePath = g_strExeFilePath;
	int nPos1 = (int)exePath.rfind('/');
	int nPos2 = (int)exePath.rfind('\\');
	int nPos = nPos1 >= nPos2 ? nPos1 : nPos2;
	if (nPos > 0) //if contains directory delimiter
		exePath.erase(nPos);
	else //if doesn't have delimiter, use the current process path
	{
#if defined(_MSC_VER)
		//For MS VC
		char path[1024];
		_getcwd(path, 1024);
		exePath = path;
#elif defined(__GNUC__)
		//For GCC
		char path[1024];
		getcwd(path, 1024);
		exePath = path;
#else
		//For unknown platforms
		exePath = "./";
#endif
	}
	return exePath;
}

string FSDK_GetInputFilesFolder(const char* moduleName)
{
	//Combine the current executing path, inputting files path (fixed) and module name together
	string inputPath = FSDK_GetExecuteFolder();
#if defined(FSDK_IOS_DEMO)
	//In IOS platform, input path is the app's resource path.
	inputPath += "/";
#else
	inputPath += "/../../input_files/";
#endif
	inputPath += moduleName;
	return inputPath;
}

string FSDK_GetOutputFilesFolder(const char* moduleName)
{
	//Combine the current executing path, outputting files path (fixed) and example module name together
	string outputPath = FSDK_GetExecuteFolder();
#if defined(FSDK_IOS_DEMO)
	//In IOS platform, output path is "Documents" in the app's home directory.
	outputPath = g_strDocuments;
#else
	outputPath += "/../../output_files/";
#endif
	outputPath += moduleName;
	FSDK_CreatFolder(outputPath.c_str());
	return outputPath;
}

string FSDK_GetLogFile(const char* moduleName)
{
	//Combine outputting files path of example module and log file name (fixed) together
	string logFile = FSDK_GetOutputFilesFolder(moduleName);
	FSDK_CreatFolder(logFile.c_str());
	logFile += "/log.txt";
	return logFile;
}

string	FSDK_GetFixFolder()
{
	string exePath = FSDK_GetExecuteFolder();
#ifndef FSDK_IOS_DEMO
	exePath += "/../../input_files";
#endif
	return exePath;
}

/*******************************************************************************/
/* Log																		 */
/*******************************************************************************/
//Global variables for log file and log lock.
static FILE*		g_pLogFile = NULL;
static FSDK_LOCK	g_LogLock;

BOOL FSDK_OpenLog(const char* logFileName)
{
	//Open log file
	//g_pLogFile = fopen(logFileName, "w+b");
	errno_t error = fopen_s(&g_pLogFile, logFileName, "w+b");
	if (error ==0) //if succeeds
	{
		//Initialize lock object for log
		FSDK_InitializeLock(&g_LogLock);
		return TRUE;
	}
	return FALSE;
}

BOOL FSDK_OutputLog(const char* format, ...)
{
	//Determine the current log file object, returns error value if it's invalid.
	if (!g_pLogFile) return FALSE;

	//Lock at first
	FSDK_Lock(&g_LogLock);
	//Get variable list
	va_list vars;
	va_start(vars, format);
	//Call vfprintf to format log data and output to log file
	int ret = vfprintf(g_pLogFile, format, vars);
	//End variable list
	va_end(vars);
	//Output log data to console again	
	va_start(vars, format);
	vfprintf(stdout, format, vars);
	va_end(vars);
	//Unlock
	FSDK_Unlock(&g_LogLock);
	return ret > -1;
}

BOOL FSDK_OutputStringLog(const char* str, int length)
{
	//Determine the current log file object, returns error value if it's invalid.
	if (!g_pLogFile) return FALSE;

	//Lock at first
	FSDK_Lock(&g_LogLock);
	//Write string data into log file directly
	int ret = (int)fwrite(str, 1, length, g_pLogFile);
	//Output string data to console again
	fwrite(str, 1, length, stdout);
	//Unlock
	FSDK_Unlock(&g_LogLock);
	return ret == length;
}

void FSDK_CloseLog()
{
	//Close log. If it's invalid, returns directly.
	if (!g_pLogFile) return;

	//Flush log file
	fflush(g_pLogFile);
	//Close log file
	fclose(g_pLogFile);
	g_pLogFile = NULL;

	//Finalize lock object
	FSDK_FinalizeLock(&g_LogLock);
}

/*******************************************************************************/
/* Time																		*/
/*******************************************************************************/
unsigned int FSDK_GetClock()
{
	//Calls clock function to get the current processor time, and returns milliseconds time value.
	return (unsigned int)clock() / (CLOCKS_PER_SEC / 1000);
}

/*******************************************************************************/
/* File system																 */
/*******************************************************************************/
#if defined(_MSC_VER)
	//Define a structure to save file finding data
	typedef struct _FSDK_FILEFINDDATA
	{
		WIN32_FIND_DATAA	findData;
		HANDLE				findHandle;
		BOOL				findEnd;
	}FSDK_FILEFINDDATA;
#endif

	void* FSDK_OpenFolder(const char* pathName, BOOL bOutputLog)
{
	//Invalid parameter, return NULL
	if (!pathName && strlen(pathName) < 1) 
	{
		if(bOutputLog)
			FSDK_OutputLog("Failure: The path name <%s> is invalid. Please check it.\r\n", pathName);
		return NULL;
	}
#if defined(_MSC_VER)
	//Allocate memory buffer for file finding data
	FSDK_FILEFINDDATA* ffd = (FSDK_FILEFINDDATA*)malloc(sizeof(FSDK_FILEFINDDATA));
	if (!ffd)
	{
		if(bOutputLog)
			FSDK_OutputLog("Failure: Fail to allocate memory buffer for file finding data.\r\n");
		return NULL;
	}
	//Initialize memory buffer to zero
	memset(ffd, 0, sizeof(FSDK_FILEFINDDATA));
	//Start finding file
	string strPath(pathName);
	strPath += "/*.*";
	ffd->findHandle = FindFirstFileA(strPath.c_str(), &ffd->findData);
	if (ffd->findHandle == INVALID_HANDLE_VALUE)
	{
		//Error occurs, free file finding data and return NULL value
		free(ffd);
		if(bOutputLog)
			FSDK_OutputLog("Failure: Fail to find file.\r\n");
		return NULL;
	}
	//Succeeds, return file finding data as folder object
	FSDK_OutputLog("Success: Find file.\r\n");
	return ffd;
#elif defined(__GNUC__)
	//Under GCC, call opendir to open folder
	return opendir(pathName);
#else
	//For unknown platform, return NULL directly
	return NULL;
#endif
}

BOOL FSDK_FolderExist(const char* pathName)   // Check the folder exists or not.
{ 	
		BOOL rValue = FALSE;
#if defined(_MSC_VER)
	FSDK_FILEFINDDATA ffd;
	//Initialize memory buffer to zero
	memset(&ffd, 0, sizeof(FSDK_FILEFINDDATA));

	HANDLE hFind = FindFirstFileA(pathName, &ffd.findData);
	if ((hFind != INVALID_HANDLE_VALUE) && (ffd.findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		rValue = TRUE;   // The folder exist.
	}
	FindClose(hFind);
	return rValue;	// The folder don't exist.
#elif defined(__GNUC__)
	DIR* dir = opendir(pathName);
	if(dir){
		rValue = TRUE;
		closedir(dir);
	}
	return rValue;
#endif
}

BOOL FSDK_FileExist(const char* pathName)   // Check the file exists or not.
{ 	
	BOOL rValue = FALSE;
#if defined(_MSC_VER)
	if (0 == _access(pathName, 0))
		rValue = TRUE;
#elif defined(__GNUC__)
	if (0 == access(pathName, F_OK))
		rValue = TRUE;
#endif
	return rValue;
}

BOOL FSDK_CreatFolder(const char* pathName)
{
	if(FSDK_FolderExist(pathName)) return TRUE;
#if defined(_MSC_VER)
	if(CreateDirectoryA(pathName, NULL))
		return TRUE;
	else
		return FALSE;

#elif defined(__GNUC__)
	mkdir(pathName, 0777); // create folder in linux
#endif
	return TRUE;
}



void FSDK_CloseFolder(void* folder)
{
	//Check folder
	if (!folder) return;

#if defined(_MSC_VER)
	//Close file finding handle
	FindClose(((FSDK_FILEFINDDATA*)folder)->findHandle);
	//Free file finding data
	free((FSDK_FILEFINDDATA*)folder);
#elif defined(__GNUC__)
	//Call closedir to close folder under GCC
	closedir((DIR*)folder);
#else
	//do nothing
#endif
}

string FSDK_GetNextFile(void* folder, FS_BOOL* isDir)
{
	//Check parameters at first
	string strFile;
	if (!folder || !isDir) return strFile;

#if defined(_MSC_VER)
	//Check if end of finding position
	if (((FSDK_FILEFINDDATA*)folder)->findEnd) return strFile;
	//Get file name
	strFile = ((FSDK_FILEFINDDATA*)folder)->findData.cFileName;
	//Check if it's a directory object
	*isDir = ((FSDK_FILEFINDDATA*)folder)->findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	//Find next file object
	if (!FindNextFileA(((FSDK_FILEFINDDATA*)folder)->findHandle, &((FSDK_FILEFINDDATA*)folder)->findData))
		((FSDK_FILEFINDDATA*)folder)->findEnd = TRUE; //Set to TRUE if reach end of finding position
	return strFile;
#elif defined(__GNUC__)
	//Call readdir to get a file object under GCC
	struct dirent *de = readdir((DIR*)folder);
	if (!de) return strFile; //Meet error
	strFile = de->d_name; //Get one file object
	*isDir = de->d_type == DT_DIR; //Determine if it's a directory
	return strFile;
#else
	//Return empty string value
	return strFile;
#endif
}

/*******************************************************************************/
/* Multiple threads															*/
/*******************************************************************************/
//Structure for thread parameters
typedef struct _FSDK_THREADPARAM
{
	FSDK_CALLBACK_ThreadProc	threadProc;	//thread procedure
	void*						threadParam;//user-defined thread parameter
} FSDK_THREADPARAM;

//Standard thread procedure
static void FSDK_ThreadProc(void* lpParameter)
{
	//If parameter is error, return immediately
	if (!lpParameter) return;

	//Get thread procedure
	FSDK_CALLBACK_ThreadProc lpfThreadProc = ((FSDK_THREADPARAM*)lpParameter)->threadProc;
	//Get user-defined thread parameters
	void* pThreadParam = ((FSDK_THREADPARAM*)lpParameter)->threadParam;
	//Free data object allocated in FSDK_CreateThread
	free((FSDK_THREADPARAM*)lpParameter);

	//Run real thread procedure
	lpfThreadProc(pThreadParam);
}

#if defined(_MSC_VER)
	//Thread procedure for MS VC platform
	static DWORD WINAPI FSDK_Windows_ThreadProc(void* lpParameter)
	{
		FSDK_ThreadProc(lpParameter);

		return 0;
	}
#elif defined(__GNUC__)
	//Thread procedure for GCC platform
	static void* FSDK_PThread_ThreadProc(void* lpParameter)
	{
		FSDK_ThreadProc(lpParameter);

		return NULL;
	}
#else
	//nothing
#endif

void* FSDK_CreateThread(FSDK_CALLBACK_ThreadProc threadProc, void* param)
{
	//If thread procedure is invalid, return NULL
	if (!threadProc) return NULL;

	//Allocate thread parameter data
	FSDK_THREADPARAM* threadParam = (FSDK_THREADPARAM*)malloc(sizeof(FSDK_THREADPARAM));
	if (!threadParam) return NULL;
	//Set thread procedure and user-defined parameter
	threadParam->threadProc = threadProc;
	threadParam->threadParam = param;

#if defined(_MSC_VER)
	//Create thread under MS VC platform
	DWORD dwThreadID = -1;
	HANDLE hThread = ::CreateThread(NULL, 0, &FSDK_Windows_ThreadProc, (void*)threadParam, 0, &dwThreadID);
	if (hThread) return (void*)hThread; //If succeeds, return thread handle.
#elif defined(__GNUC__)
	//Create thread under GCC
	pthread_t thread = 0;
	int nErr = pthread_create(&thread, NULL, &FSDK_PThread_ThreadProc, (void*)threadParam);
	if (!nErr) return (void*)thread; //If succeeds, return thread handle
#else
	//nothing
#endif

	//Error in creating thread, free thread parameter data
	free(threadParam);
	return NULL;
}

int FSDK_WaitForSingleThread(void* thread)
{
#if defined(_MSC_VER)
	//Call WaitForSingleObject under MS VC
	return ::WaitForSingleObject((HANDLE)thread, INFINITE) == WAIT_FAILED ? -1 : 0;
#elif defined(__GNUC__)
	//Call pthread_join under GCC
	void* status = NULL;
	return pthread_join((pthread_t)thread, &status);
#else
	//Return error codes for other platforms
	return -1;
#endif
}

int FSDK_WaitForMultipleThreads(void** threads, int count)
{
#if defined(_MSC_VER)
	//Call WaitForMultipleObjects under MS VC
	return ::WaitForMultipleObjects(count, (const HANDLE*)threads, TRUE, INFINITE) == WAIT_FAILED ? -1 : 0;
#elif defined(__GNUC__)
	//Call pthread_join under GCC
	for (int n = 0; n < count; n ++)
	{
		void* status = NULL;
		int nRet = pthread_join((pthread_t)threads[n], &status);
		if (nRet) return nRet;
	}
	return 0;
#else
	//Return error codes for other platforms
	return -1;
#endif
}

/*******************************************************************************/
/* Mutex																	   */
/*******************************************************************************/
BOOL FSDK_InitializeLock(FSDK_LOCK* lock)
{
	//Check parameter, if it's invalid, return error code.
	if (!lock) return FALSE;

#if defined(_WIN32)||defined(_WIN64)||defined(_WIN32_WCE)
#if defined(WINAPI_FAMILY) && ((WINAPI_FAMILY&WINAPI_PARTITION_APP) == WINAPI_PARTITION_APP)
#define _FS_WINAPI_PARTITION_APP_
#endif
#if !defined(WINAPI_FAMILY) || defined(WINAPI_FAMILY) && ((WINAPI_FAMILY&WINAPI_PARTITION_DESKTOP) == WINAPI_PARTITION_DESKTOP)
#define _FS_WINAPI_PARTITION_DESKTOP_	
#endif
#ifdef  _FS_WINAPI_PARTITION_DESKTOP_
	InitializeCriticalSection(lock);
	return TRUE;
#else
	BOOL bRet = InitializeCriticalSectionEx(lock, 0, 0);
	return bRet;
#endif
#elif defined(__GNUC__)
	//For GCC, use posix thread mutex
	pthread_mutexattr_t tma;
	pthread_mutexattr_init(&tma);
	pthread_mutexattr_settype(&tma, PTHREAD_MUTEX_RECURSIVE);
	int nErr = pthread_mutex_init(lock, &tma);
	pthread_mutexattr_destroy(&tma);
	if(0 == nErr)
		return TRUE;
	else
		return FALSE;
#else
	return FALSE;
#endif
}

void FSDK_FinalizeLock(FSDK_LOCK* lock)
{
	//Check parameter, if it's invalid, return error code.
	if (!lock) return;

#if defined(_MSC_VER)
	//For MS VC, call DeleteCriticalSection to destroy
	DeleteCriticalSection(lock);
#elif defined(__GNUC__)
	//For GCC, call pthread_mutex_destroy to destroy
	pthread_mutex_destroy(lock);
#else
	//do nothing
#endif
}

void FSDK_Lock(FSDK_LOCK* lock)
{
	//Check parameter, if it's invalid, return error code.
	if (!lock) return;

#if defined(_MSC_VER)
	//For MS VC, call EnterCriticalSection to lock
	EnterCriticalSection(lock);
#elif defined(__GNUC__)
	//For GCC, call pthread_mutex_lock to lock
	pthread_mutex_lock(lock);
#else
	//do nothing
#endif
}

void FSDK_Unlock(FSDK_LOCK* lock)
{
	//Check parameter, if it's invalid, return error code.
	if (!lock) return;

#if defined(_MSC_VER)
	//For MS VC, call LeaveCriticalSection to unlock
	LeaveCriticalSection(lock);
#elif defined(__GNUC__)
	//For GCC, call pthread_mutex_unlock to unlock
	pthread_mutex_unlock(lock);
#else
	//do nothing
#endif
}

/*******************************************************************************/
/* File access																 */
/*******************************************************************************/
BOOL FSDK_CompareExtName(const char* fileName, const char* extName)
{
	//Check lengths of two strings
	int fileLen = (int)strlen(fileName);
	int extLen = (int)strlen(extName);
	if (fileLen <= extLen) return FALSE;

#if defined(_MSC_VER)
	//For MS VC
	return _strnicmp(fileName + fileLen - extLen, extName, extLen) == 0;
#elif defined(__GNUC__)
	//For GCC
	return strncasecmp(fileName + fileLen - extLen, extName, extLen) == 0;
#else
	return FALSE;
#endif
}

//Structure for file data
typedef struct _FSDK_FILEDATA
{
	FILE*				fileStream;	//file stream object
	FSCRT_FILEHANDLER	fileHandler;//file handler of Foxit SDK
} FSDK_FILEDATA;

//Implementation to FSCRT_FILEHANDLER::Release
static void FSDK_FileHandler_Release(FS_LPVOID clientData)
{
	//Check parameter
	if (!clientData) return;
	//Close file stream
	fclose(((FSDK_FILEDATA*)clientData)->fileStream);
	//Free file data
	free((FSDK_FILEDATA*)clientData);
}

//Implementation to FSCRT_FILEHANDLER::GetSize
static FS_DWORD	FSDK_FileHandler_GetSize(FS_LPVOID clientData)
{
	//Check parameter
	if (!clientData) return 0;

	//Move file pointer to the end, and get file size
	long curPos = ftell(((FSDK_FILEDATA*)clientData)->fileStream);
	fseek(((FSDK_FILEDATA*)clientData)->fileStream, 0, SEEK_END);
	long size = ftell(((FSDK_FILEDATA*)clientData)->fileStream);
	fseek(((FSDK_FILEDATA*)clientData)->fileStream, curPos, SEEK_SET);

	return (FS_DWORD)size;
}

//Implementation to FSCRT_FILEHANDLER::ReadBlock
static FS_RESULT FSDK_FileHandler_ReadBlock(FS_LPVOID clientData, FS_DWORD offset, FS_LPVOID buffer, FS_DWORD size)
{
	//Check parameter
	if (!clientData) return FSCRT_ERRCODE_FILE;

	//Change file pointer to offset
	fseek(((FSDK_FILEDATA*)clientData)->fileStream, (long)offset, SEEK_SET);
	//Read data
	size_t readSize = fread(buffer, 1, size, ((FSDK_FILEDATA*)clientData)->fileStream);
	return readSize == size ? FSCRT_ERRCODE_SUCCESS : FSCRT_ERRCODE_FILE;
}

//Implementation to FSCRT_FILEHANDLER::WriteBlock
static FS_RESULT FSDK_FileHandler_WriteBlock(FS_LPVOID clientData, FS_DWORD offset, FS_LPCVOID buffer, FS_DWORD size)
{
	//Check parameter
	if (!clientData) return FSCRT_ERRCODE_FILE;

	//Change file pointer to offset
	fseek(((FSDK_FILEDATA*)clientData)->fileStream, (long)offset, SEEK_SET);
	//Write data
	size_t writeSize = fwrite(buffer, 1, size, ((FSDK_FILEDATA*)clientData)->fileStream);
	return writeSize == size ? FSCRT_ERRCODE_SUCCESS : FSCRT_ERRCODE_FILE;
}

//Implementation to FSCRT_FILEHANDLER::Flush
static FS_RESULT FSDK_FileHandler_Flush(FS_LPVOID clientData)
{
	//Check parameter
	if (!clientData) return FSCRT_ERRCODE_FILE;

	//Flush file
	fflush(((FSDK_FILEDATA*)clientData)->fileStream);

	return FSCRT_ERRCODE_SUCCESS;
}

//Implementation to FSCRT_FILEHANDLER::Truncate
static FS_RESULT FSDK_FileHandler_Truncate(FS_LPVOID clientData, FS_DWORD size)
{
	//Unsupported
	return FSCRT_ERRCODE_UNSUPPORTED;
}

FSCRT_FILE FSDK_OpenFile(const char* fileName, const char* fileMode, BOOL useDef)
{
	//Check parameters, return NULL if they are invalid
	if (!fileName || strlen(fileName) < 1 || !fileMode) 
	{
		if (!fileMode)
		{
			FSDK_OutputLog("Failure: The file mode how to access is invalid. Please check it.\r\n");
		}
		else
		{
			FSDK_OutputLog("Failure: The file name which to be open is invalid. Please check it.\r\n");
		}
		return NULL;
	}

	if (useDef) //if use default implementation
	{
		FSDK_OutputLog("Using default implementation to open the file.\r\n");

		//Check file modes
		FS_DWORD fileModes = 0;
		string fm(fileMode);
		if (fm.find('+') > -1)
			fileModes = FSCRT_FILEMODE_WRITE;
		else if (fm.find('w') > -1)
			fileModes = FSCRT_FILEMODE_TRUNCATE;
		else
			fileModes = FSCRT_FILEMODE_READONLY;


		char* old_locale =  setlocale(LC_CTYPE,NULL);
		setlocale( LC_ALL, "" );

		size_t length = strlen(fileName);
		wchar_t* buf = new wchar_t[length];
		memset(buf, 0 ,sizeof(wchar_t)*length);
		size_t size=0;
		mbstowcs_s(&size, buf, sizeof(wchar_t)*length, fileName, length);
		
		setlocale( LC_CTYPE, old_locale);

		FSCRT_BSTR bstrFile;
		FSCRT_BStr_Init(&bstrFile);
		if(sizeof(wchar_t)==2)
			FSCRT_UTF8_FromUTF16LE(&bstrFile, (FS_WORD*)buf, size);  //windwos use
		else
			FSCRT_UTF8_FromUTF32LE(&bstrFile, (FS_DWORD*)buf, size);  //linux use
		
		delete[] buf;

		FSCRT_FILE file = NULL;
		//Call FSCRT_File_CreateFromFileName to open file
		FS_RESULT ret = FSCRT_File_CreateFromFileName(&bstrFile, fileModes, &file);
		if (FSCRT_ERRCODE_SUCCESS != ret)
		{
			FSDK_OutputLog("Failure: Using the <%s> mode to access the file whose name is %s failed.\r\n", fileMode, fileName);
			FSDK_OutputLog("\tAnd the error code %d.\r\n", ret);
			return NULL;
		}
		//Clear string object
		FSCRT_BStr_Clear(&bstrFile);

		FSDK_OutputLog("Success: Use <%s> mode to access the file %s.\r\n", fileMode, fileName);
		return file;
	}

	//For extension implementation
	FSDK_OutputLog("Using extension implementation to open the file.\r\n");
	//Open file as stream
	FILE* fileStream = NULL;
	errno_t error = fopen_s(&fileStream, fileName, fileMode);
	string szFileName = FSDK_GetFileName(fileName);
	if (error!=0) 
	{
		FSDK_OutputLog("Failure: Fail to use <%s> mode to open the file %s and associate it with a stream.\r\n", fileMode, szFileName.c_str());
		return NULL;
	}
	FSDK_OutputLog("Success: Use <%s> mode to open the file %s and associate it with a stream.\r\n", fileMode, szFileName.c_str());
	//Allocate file data
	FSDK_FILEDATA* fileData = (FSDK_FILEDATA*)malloc(sizeof(FSDK_FILEDATA));
	if (!fileData)
	{
		//If error, close file stream and exit
		fclose(fileStream);
		FSDK_OutputLog("Failure: Fail to allocate the file data.\r\n");
		return NULL;
	}
	//Initialize file handler
	fileData->fileHandler.clientData = fileData;
	fileData->fileHandler.Release = FSDK_FileHandler_Release;
	fileData->fileHandler.GetSize = FSDK_FileHandler_GetSize;
	fileData->fileHandler.ReadBlock = FSDK_FileHandler_ReadBlock;
	fileData->fileHandler.WriteBlock = FSDK_FileHandler_WriteBlock;
	fileData->fileHandler.Flush = FSDK_FileHandler_Flush;
	fileData->fileHandler.Truncate = FSDK_FileHandler_Truncate;
	fileData->fileStream = fileStream;
	//Call FSCRT_File_Create to create Foxit GSDK file object
	FSCRT_FILE file = NULL;
	FS_RESULT ret = FSCRT_File_Create(&fileData->fileHandler, &file);
	if (ret == FSCRT_ERRCODE_SUCCESS)
	{
		//Success, return file object
		FSDK_OutputLog("Success: Create the file object.\r\n");
		return file;
	}

	//Free file data, close file stream and exit
	free(fileData);
	fclose(fileStream);
	FSDK_OutputLog("Failure: Fail to create the file object with error code %d.\r\n", ret);
	return NULL;
}

FS_RESULT FSDK_SaveImageFile(FSCRT_BITMAP bitmap, FS_INT32 type, const char* fileName)
{
	//Check parameters, return error if they are invalid
	if (!bitmap || !fileName || strlen(fileName) < 1) 
	{
		if (!bitmap)
			FSDK_OutputLog("Failure: The parameter <bitmap> is invalid. Please check it.\r\n");
		else
			FSDK_OutputLog("Failure: The parameter <filename> is invalid. Please check it.\r\n");
		return FSCRT_ERRCODE_ERROR;
	}

	//Open image file at first
	FSCRT_FILE fsFile = FSDK_OpenFile(fileName, "wb");
	if (!fsFile) return FSCRT_ERRCODE_FILE;

	//Create image file object
	FSCRT_IMAGEFILE imageFile = NULL;
	FS_RESULT ret = FSCRT_ImageFile_Create(fsFile, type, 1, &imageFile);
	if (ret == FSCRT_ERRCODE_SUCCESS)
	{
		//Add frame
		ret = FSCRT_ImageFile_AddFrame(imageFile, bitmap);
		if (ret == FSCRT_ERRCODE_SUCCESS)
			FSDK_OutputLog("Success: The image save to %s.\r\n", FSDK_GetFileName(fileName).c_str());
		else
			FSDK_OutputLog("Failure: In function FSCRT_ImageFile_Create with error code %d.\r\n", FSDK_GetFileName(fileName).c_str(), ret);		
		//Release image file object
		ret = FSCRT_ImageFile_Release(imageFile);
	}else{
		FSDK_OutputLog("Failure: In function FSCRT_ImageFile_Create with error code %d.\r\n", FSDK_GetFileName(fileName).c_str(), ret);		
	}

	//Close image file
	FSCRT_File_Release(fsFile);
	return ret;
}

FS_BOOL	FSDK_SavePDFFile(FSCRT_DOCUMENT document, const char* fileName, FS_INT32 flag)
{
	//Check parameters, return error if they are invalid
	if (!document || !fileName || strlen(fileName) < 1) 
	{
		if (!document)
			FSDK_OutputLog("Failure: The parameter <document> is invalid. Please check it.\r\n");
		else
			FSDK_OutputLog("Failure: The parameter <filename> is invalid. Please check it.\r\n");
		return FALSE;
	}

	//Open destination file at first
	FSCRT_FILE saveFile = FSDK_OpenFile(fileName, "wb");
	if (!saveFile) return FSCRT_ERRCODE_FILE;

	//Start saving PDF file
	FSCRT_PROGRESS progress = NULL;
	FS_RESULT ret = FSPDF_Doc_StartSaveToFile(document, saveFile, flag, &progress);
	if (ret == FSCRT_ERRCODE_SUCCESS)
	{
		//Continue to finish saving
		ret = FSCRT_Progress_Continue(progress, NULL);
		if (FSCRT_ERRCODE_FINISHED != ret)
		{
			FSDK_OutputLog("Failure: In function FSCRT_Progress_Continue with error code %d.\r\n", ret);
			FSCRT_Progress_Release(progress);
			return FALSE;
		}
		FSDK_OutputLog("Success: The PDF document save to %s.\r\n", FSDK_GetFileName(fileName).c_str());
		//Release saving progress object
		ret = FSCRT_Progress_Release(progress);
	}else{
		FSDK_OutputLog("Failure: In function FSPDF_Doc_StartSaveToFile with error code %d.\r\n", ret);
	}

	//Close destination file
	FSCRT_File_Release(saveFile);
	return ret == FSCRT_ERRCODE_SUCCESS;
}

// Check whether images are supported or not
FS_BOOL FSDK_IsSupportedImageFile(const char* fileName, FS_BOOL read)
{
	if (!fileName)	
	{
		FSDK_OutputLog("Failure: The parameter <filename> is invalid. Please check it.\r\n");
		return FALSE;
	}
	// Return supported if the ext name are shown as below
	// The extension name are not case sensitive 
	if (FSDK_CompareExtName(fileName, ".bmp"))	return TRUE;
	if (FSDK_CompareExtName(fileName, ".jpg"))	return TRUE;
	if (FSDK_CompareExtName(fileName, ".tif"))	return TRUE;
	if (FSDK_CompareExtName(fileName, ".png"))	return TRUE;
	if (FSDK_CompareExtName(fileName, ".jpx"))	return TRUE;

	// GIF format is only supported for reading in Foxit SDK
	if (read)
		if (FSDK_CompareExtName(fileName, ".gif"))	return TRUE;

	// Return not supported for all other formats apart from above
	return FALSE;
}

//Extract page range from string
BOOL FSDK_GetPageRange(const char* pageNum, FS_INT32*& pageArray, FS_INT32* count)
{
	if (pageNum == NULL||strlen(pageNum)<1)
	{
		FSDK_OutputLog("Failure: The parameter <pageNum> is invalid. Please check it.\r\n");
		return FALSE;
	}
	char pagebuff[126];
	memset(pagebuff, 0 ,sizeof(char)*126);
	strcpy_s(pagebuff,  pageNum);
	vector<int> v_num;
	vector<char*> v_char;
	char* str = NULL,*pNext=NULL;
	str = strtok_s(pagebuff, ",",&pNext);
	if (str)
	{
		v_char.push_back(str);
	}
	while(str)
	{
		str = strtok_s(NULL, ",", &pNext);
		if (!str)
		{
			break;
		}
		v_char.push_back(str);
	}
	vector<char*>:: iterator it;
	for (it = v_char.begin();it!=v_char.end();it++)
	{
		char t[126];
		memset(t,0,sizeof(char)*126);
		strcpy_s(t,*it);
		strtok_s(t, "-", &pNext);
		if (strcmp(t,*it))
		{
			if (0 != strcmp("0",t) && 0 == atoi(t))
				continue;
			int a1 = atoi(t);
			char* p1 = strtok_s(NULL, "-", &pNext);
			if (0 != strcmp("0",p1) && 0 == atoi(p1))
				continue;
			int a2 = atoi(p1);
			if (a1 < a2)
			{
				for (int i=a1;i<a2+1;i++)
				{
					v_num.push_back(i);
				}
			}
			else
			{
				for (int j = a2;j<a1+1;j++)
				{
					v_num.push_back(j);
				}
			}
		}
		else
		{
			if (0 != strcmp("0",t) && 0 == atoi(t))
				continue;
			v_num.push_back(atoi(t));
		}
	}
	vector<int>::iterator i;
	*count = v_num.size();
	if (*count < 1) return TRUE;
	FS_INT32* temArry = new FS_INT32[v_num.size()];
	int arryIndex = 0;
	for (i = v_num.begin();i!=v_num.end();i++,arryIndex++)
	{
		temArry[arryIndex] = *i;
	}

	pageArray = temArry;
	FSDK_OutputLog("Success: Extract page range from string.\r\n");
	return TRUE;
}

string FSDK_GetFileName(const char* filePath)
{
	string exePath = filePath;
	int nPos1 = (int)exePath.rfind('/');
	int nPos2 = (int)exePath.rfind('\\');
	int nPos = nPos1 >= nPos2 ? nPos1 : nPos2;
	exePath.assign(filePath, nPos+1, strlen(filePath)+1-nPos);
	return exePath;
}

string	FSDK_GetCustomInputPath(const char* inputPath)
{
	string inputStr = inputPath;
	int nPos1 = (int)inputStr.rfind('/');
	int nPos2 = (int)inputStr.rfind('\\');
	int nPos = nPos1 >= nPos2 ? nPos1 : nPos2;
	if (nPos > 0)
		inputStr.erase(nPos);
	return inputStr;
}


FS_FLOAT FSDK_GetFloat(const char * strValue)
{
	return (FS_FLOAT)atof(strValue);
}

FS_INT32 FSDK_GetInt(const char * strValue)
{
	return atoi(strValue);
}