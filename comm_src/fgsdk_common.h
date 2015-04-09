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
 * This file is a common header to provide some general functions which are used by examples.
 */
#pragma  once
#ifndef _FSDK_COMMON_H_
#define _FSDK_COMMON_H_

/* Common header files. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string>
#include <iostream>
#include <vector>
#include <locale.h>

#if defined(_MSC_VER)
	/* Include MS VC header files, for Windows platforms. */

	//#if _MSC_VER >= 1200
	//	#define _WIN32_WINNT 0x0501 //_WIN32_WINNT_WINXP
	//#endif
	//#include <windows.h>
	#include <direct.h>
	#include <io.h>

#if defined(_WIN64)  //windows 64bit platforms.
	#if defined(_DEBUG)
		#pragma comment(lib, "../lib/fsdk_win64.lib")
	#else
		#pragma comment(lib, "../lib/fsdk_win64.lib")
	#endif

#elif defined(_WIN32) //windows 32bit platforms.
	#if defined(_DEBUG)
		#pragma comment(lib, "../lib/fsdk_win32.lib")
	#else
		#pragma comment(lib, "../lib/fsdk_win32.lib")
	#endif
#endif

#elif defined(__GNUC__)
	/* Include GCC header files, for Linux/Unix platforms. */
	#include <sys/stat.h>
	#include <unistd.h>
	#include <pthread.h>
	#include "stdarg.h"
	#include <dirent.h>
	
	#ifndef FALSE
		#define FALSE 0
	#endif
	#ifndef TRUE
		#define TRUE 1
	#endif
	
	
#else
	/* Wrong platforms. */

	#error "Wrong compiler!"
#endif

#if defined(__APPLE__)
	#include "TargetConditionals.h"
	#if (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE == 1) || (defined(TARGET_IPHONE_SIMULATOR) && TARGET_IPHONE_SIMULATOR == 1)
		//It means a IOS demo
		#define FSDK_IOS_DEMO
	#endif
#endif

/* Include Foxit GSDK header files. */
#include "../include/foxit/fsdk.h"
//#include "../../../include/fsdk.h"

/* Ignore namespace check for STL. */
using namespace std;

 class CAutoReflowPage;
 class CAutoPDFPage;
 class CAutoImage;

/////////////////////////////////////////////////////////////
// common auto class
/////////////////////////////////////////////////////////////
class CAutoDocument
{
public:
	CAutoDocument();
	CAutoDocument(const char* fileName, const char* fileMode, BOOL useDef = FALSE);
	~CAutoDocument();

	FS_RESULT		LoadPDFDocument(FSCRT_BSTR* password, FS_BOOL bProgress = FALSE);
	FS_BOOL			LoadFDFDocument();
	FS_RESULT		CreatePDFDocument();
	FS_RESULT		CreateFDFDocument(FS_INT32  nformat);
	FSCRT_DOCUMENT	GetDocument();
	void			CloseDocument();
	FSCRT_FILE		GetFileHander();
	FS_RESULT		LoadAttachments();
	FSPDF_ATTACHMENTS GetAttachments();
	FSPDF_ATTACHMENT* GetAllAttachment();
	FSCRT_BSTR*		GetAllAttachmentFileName();
	void			SaveAttachmentByFileByName(string path);
	FSPDF_ATTACHMENT	GetAttachment();
	FS_RESULT		CreateAttachment();
	FS_INT32			GetPagesCount();

protected:
	FSCRT_DOCUMENT	m_Doc;
	FSCRT_FILE		m_file;
	FS_BOOL			m_bPDFDoc;
	FSPDF_ATTACHMENTS m_attachments;
	FSPDF_ATTACHMENT* m_pAttachment;
	FSCRT_BSTR*	m_pAttachmentFileName;
	FS_INT32 m_AttachmentFileCount;
	FSPDF_ATTACHMENT m_attachment;
};

class CAutoPDFPage
{
public:
	CAutoPDFPage();
	~CAutoPDFPage();

	FS_RESULT		LoadPDFPage(FSCRT_DOCUMENT pdfDoc,FS_INT32 nIndex);
	FS_RESULT		CreatePDFPage(FSCRT_DOCUMENT pdfDoc,FS_INT32 nIndex);
	FSCRT_PAGE		GetPDFPage();
	FSPDF_TEXTPAGE	GetPDFTextPage();
	FS_RESULT		ParsePage();
	FS_RESULT		LoadTextPage();
	FS_RESULT		LoadAnnots();
	void			ClearPage();

protected:
	FS_BOOL			m_bIsLoadAnnots;
	FSCRT_PAGE		m_pdfPage;
	FSPDF_TEXTPAGE	m_textPage;
};

class CAutoReflowPage
{
public:
	CAutoReflowPage();
	~CAutoReflowPage();

	FSPDF_REFLOWPAGE	GetReflowPage();
	FS_RESULT		   ReleaseReflowPage();
	FS_RESULT		   CreateReflowPage(const FSCRT_PAGE pdfPage);

	void				SetReflowFlag(FS_DWORD dwReflowFlag);
	void				SetScreenParam(FS_FLOAT fScreenWidth,FS_FLOAT fScreenHeight);
	void				SetLineSpace(FS_FLOAT fLineSpace );
	void				SetTopSpace(FS_FLOAT fTopSpace);
	FS_FLOAT			GetTopSpace();
	void				SetLastYPixel(FS_FLOAT fLastYPixel);
	FS_FLOAT			GetScreenWidth();
	FS_FLOAT			GetScreenHeight();
	FS_FLOAT			GetLastYPixel();
	FS_FLOAT			GetContentHeight();	
	FS_FLOAT			GetContentWidth();  

private:
	FSPDF_REFLOWPAGE m_reflowPage;

	/** @brief	specify Y value ,which specify the location of top-left point of reflowed page in device coordinate. 
	*		   if the value is negative, the top-left point is Above the origin of the device coordinate;
	*		   if the value is positive, the top-left point is under the origin of the device coordinate
	*/
	FS_FLOAT m_fLastYPixel;
	FS_FLOAT m_fScreenWidth ;
	FS_FLOAT m_fScreenHeight ;
	FS_FLOAT m_fLineSpace ;
	FS_FLOAT m_fTopSpace ;
	FS_DWORD m_dwReflowFlag;
	FS_FLOAT m_fContentHeight;
	FS_FLOAT m_fContentWidth;  
};

class CAutoImage
{
public:
	CAutoImage();
	CAutoImage(const char* fileName, const char* fileMode, BOOL useDef = FALSE);
	~CAutoImage();
	
	FSCRT_IMAGE		GetImage();
	FSCRT_BITMAP	GetBitmap();
	FSCRT_IMAGEFILE	GetImageFile();
	FSCRT_FILE		GetFile();
	FS_RESULT		LoadImage();
	FS_RESULT		GenerateBitmapbyBarcode(const FSCRT_BSTR* info, FS_INT32 codeFormat, FS_INT32 unitWidth, FS_INT32 unitHeight, FS_INT32 qrLevel);
	FS_RESULT		CreateImageFile(FSCRT_FILE file, FS_INT32 imageType, FS_INT32 frameCount);
	FS_RESULT		PageToBitmap(FSCRT_PAGE page);
	FS_RESULT		GetCurrentFramBitmap();

	// for reflow
	// create bitmap, renderer, RenderContext
	FS_RESULT		CreateRenderResource(FS_FLOAT fImageWidth,FS_FLOAT fImageHeight,FSCRT_RECT rectMargin);
	FS_RESULT		ReleaseRenderResource();
	FS_RESULT		FillBitMapWithWhite();
	FS_RESULT		ReflowPageToBitmap(CAutoReflowPage* autoReflowPage,FSCRT_RECT rectRegion);

protected:
	FSCRT_IMAGE		m_image; 
	FSCRT_BITMAP	m_bitmap;
	FSCRT_IMAGEFILE	m_imageFile;
	FSCRT_FILE		m_file;
	FSPDF_RENDERCONTEXT	m_pdfRenderContext ;
	FSCRT_RENDERER		m_renderer ;

	FS_FLOAT		m_fImageWidth;
	FS_FLOAT		m_fImageHeight;
};

class CAutoPageObject
{
public:
	CAutoPageObject();
	~CAutoPageObject();
	 
	FS_RESULT CreateImageObject(FSCRT_PAGE page);
	FS_RESULT CreateTextObject(FSCRT_PAGE page);
	FS_RESULT CreatePathObject(FSCRT_PAGE page);
	FS_RESULT CreateFormXObject(FSCRT_PAGE page);
	FS_RESULT InsertGenerateContents(FSCRT_PAGE page, FSPDF_PAGEOBJECTS pageObjs, FS_INT32 typeFilter, FS_INT32 index);
	FSPDF_PAGEOBJECT GetObject();

	void Clear();
protected:
	FSPDF_PAGEOBJECT m_obj;
	FS_BOOL m_bInsert;
	FSCRT_PAGE m_page;
};

/*******************************************************************************/
/* Library management														  */
/*******************************************************************************/
/**
 * @brief	Initialize Foxit SDK library.
 *
 * @param[in] useDefMgr		Specifies how to initialize Foxit SDK. TRUE if use default manager, or FALSE if use extending manager.
 *
 * @return	Indicate initialization status, TRUE if success, or FALSE for failure.
 *
 * @note	When use Foxit SDK library, the first step is to initialize library.
 *			This function calls FSCRT_Library_CreateDefaultMgr function to initialize for default manager,
 *			or calls FSCRT_Library_CreateMgr function for extending manager.
 */
BOOL			FSDK_InitializeLibray(BOOL useDefMgr);
/**
 * @brief	Finalize Foxit SDK library.
 *
 * @note	Before exit from application, user need call this function to destroy Foxit SDK library.
 *			This function calls FSCRT_Library_DestroyMgr function to finalize manager created by FSDK_InitializeLibray.
 */
void			FSDK_FinalizeLibrary();

FS_BOOL			FSDK_PDFModule_Initialize();
void			FSDK_PDFModule_Finalize();
/*******************************************************************************/
/* Directories																 */
/*******************************************************************************/
/**
 * @brief	Set the current executing file path.
 *
 * @param[in] argc          The count of arguments.
 * @param[in] argv          The value of arguments.
 *
 * @note	In order to track accessing path for each example in Foxit GSDK, this function set the current executing file path and store internally.
 *			In main function of each example, this is the first function call.
 */
void			FSDK_SetExecuteFilePath(int argc, char* argv[]);
/**
 * @brief	Get the current executing folder.
 *
 * @return	The current executing folder.
 *
 * @note	This function returns executing folder according to the file path which is set by calling of FSDK_SetExecuteFilePath.
 */
string			FSDK_GetExecuteFolder();
/**
 * @brief	Get folder of inputting files for an example module.
 *
 * @param[in] moduleName	Example module name.
 *
 * @return	Inputting files folder.
 *
 * @note	This function returns inputting files folder according to the file path which is set by calling of FSDK_SetExecuteFilePath.
 */
string			FSDK_GetInputFilesFolder(const char* moduleName);
/**
 * @brief	Get folder of outputting files for an example module.
 *
 * @param[in] moduleName	Example module name.
 *
 * @return	Outputting files folder.
 *
 * @note	This function returns outputting files folder according to the file path which is set by calling of FSDK_SetExecuteFilePath.
 *			if the folder don't exist, the function will create the folder.
 */
string			FSDK_GetOutputFilesFolder(const char* moduleName);
/**
 * @brief	Get path of log file for an example module.
 *
 * @param[in] moduleName	Example module name.
 *
 * @return	Outputting files folder.
 *
 * @note	This function returns outputting files folder according to the file path which is set by calling of FSDK_SetExecuteFilePath.
 *			if the folder don't exist, the function will create the folder.
 */
string			FSDK_GetLogFile(const char* moduleName);


string			FSDK_GetFixFolder();
/*******************************************************************************/
/* Log																		 */
/*******************************************************************************/
/**
 * @brief	Open log file.
 *
 * @param[in] logFileName	Full path name of log file.
 *
 * @return	TRUE if success, or FALSE for failure.
 *
 * @note	Call this function to open a log file. To get log file path name, call FSDK_GetLogFile for an example module.
 */
BOOL			FSDK_OpenLog(const char* logFileName);
/**
 * @brief	Output log data.
 *
 * @param[in] format	Format string of log data.
 *
 * @return	TRUE if success, or FALSE for failure.
 *
 * @note	This function supports variable argument list.
 */
BOOL			FSDK_OutputLog(const char* format, ...);
/**
 * @brief	Output log string.
 *
 * @param[in] str		String buffer.
 * @param[in] length	Length of string in bytes.
 *
 * @return	TRUE if success, or FALSE for failure.
 *
 * @note	This function outputs a string into log file.
 */
BOOL			FSDK_OutputStringLog(const char* str, int length);
/**
 * @brief	Close log file.
 *
 * @note	User should call this function to close log file when application exits.
 */
void			FSDK_CloseLog();

/*******************************************************************************/
/* Time																		*/
/*******************************************************************************/
/**
 * @brief	Get the current processor time.
 *
 * @return	The current processor time in milliseconds.
 *
 * @note	This function provides a simple way to get the current processor time used by the calling process.
 */
unsigned int	FSDK_GetClock();

/*******************************************************************************/
/* File system																 */
/*******************************************************************************/
/**
 * @brief	Open a folder to enumerate all files in it.
 *
 * @param[in] pathName		Folder name to be enumerated.
 * @param[in] bOutputLog	Flag to output the error information.
 *
 * @return	A pointer object to the given folder.
 *
 * @note	This function is used to open a folder when user need enumerate its contents.
 *			Call FSDK_GetNextFile to get a file object in the folder, or call FSDK_CloseFolder to close the folder.
 */
void*			FSDK_OpenFolder(const char* pathName, BOOL bOutputLog = TRUE);
/**
 * @brief	Close a folder.
 *
 * @param[in] folder	Folder pointer which is returned by previous call to FSDK_OpenFolder.
 *
 * @note	Call this function to close folder after enumerating files.
 */
void			FSDK_CloseFolder(void* folder);
/**
 * @brief	Get a file in folder.
 *
 * @param[in] folder	Folder pointer which is returned by previous call to FSDK_OpenFolder.
 * @param[out] isDir	Specifies the file is a directory object if its value is TRUE or a common file object if its value is FALSE.
 *
 * @return	File name enumerated in folder, without path name.
 *
 * @note	If the returned file name is empty, isDir makes no sense.
 */
string			FSDK_GetNextFile(void* folder, FS_BOOL* isDir);
/**
 * @brief	Create a folder, if the folder don't exist.
 *
 * @param[in] pathName	Folder name to be created.
 *
 * @return	true if the folder exist or create folder success.
 *
 */
BOOL			FSDK_CreatFolder(const char* pathName);
/**
 * @brief	Check the folder exists or not.
 *
 * @param[in] pathName	Folder name.
 *
 * @return	true if the folder exist otherwise false.
 *
 */
BOOL			FSDK_FolderExist(const char* pathName);
/**
 * @brief	Check the file exists or not.
 *
 * @param[in] pathName	File name.
 *
 * @return	true if the file exist otherwise false.
 *
 */
BOOL			FSDK_FileExist(const char* pathName);

/*******************************************************************************/
/* Multiple threads															*/
/*******************************************************************************/
/**
 * @brief	Type definition for callback function of thread.
 *
 * @param[in] param		User-defined parameter passed to thread procedure.
 *
 * @note	Pass address of thread procedure when call FSDK_CreateThread.
 */
typedef void	(*FSDK_CALLBACK_ThreadProc)(void* param);

/**
 * @brief	Create thread.
 *
 * @param[in] threadProc	Address of thread procedure.
 * @param[in] param			User-defined parameter passed to thread procedure.
 *
 * @return	Thread handle, NULL if fails.
 *
 * @note	This function create a new thread. New thread runs immediately when it is created successfully.
 */
void*			FSDK_CreateThread(FSDK_CALLBACK_ThreadProc threadProc, void* param);
/**
 * @brief	Wait exit for single thread.
 *
 * @param[in] thread	Thread handle which is returned by previous call to FSDK_CreateThread.
 *
 * @return	0 if succeeds, other value if fails.
 *
 * @note	Call this function to wait for the exit status of single thread.
 */
int				FSDK_WaitForSingleThread(void* thread);
/**
 * @brief	Wait full exit for multiple threads.
 *
 * @param[in] threads	Array to multiple threads. Each element is a thread handle which is returned by previous call to FSDK_CreateThread.
 * @param[in] count		Count of multiple threads which are stored in threads array.
 *
 * @return	0 if succeeds, other value if fails.
 *
 * @note	Call this function to wait for the exit status of multiple threads.
 */
int				FSDK_WaitForMultipleThreads(void** threads, int count);

/*******************************************************************************/
/* Mutex																	   */
/*******************************************************************************/
#if defined(_MSC_VER)
	/* Lock definition for MS VC compiler, use critical section. */
	typedef CRITICAL_SECTION	FSDK_LOCK;
#elif defined(__GNUC__)
	/* Lock definition for GCC compiler, use posix mutex. */
	typedef pthread_mutex_t		FSDK_LOCK;
#else
	/* Lock definition for other compilers, use void pointer. */
	typedef void*				FSDK_LOCK;
#endif

/**
 * @brief	Initialize a lock.
 *
 * @param[in] lock	Pointer to a lock object.
 *
 * @return	TRUE if success, FALSE if failure.
 *
 * @note	Call this function to initialize a lock object before use it.
 */
BOOL			FSDK_InitializeLock(FSDK_LOCK* lock);
/**
 * @brief	Finalize a lock.
 *
 * @param[in] lock	Pointer to a lock object.
 *
 * @note	Call this function to finalize a lock object after use it.
 */
void			FSDK_FinalizeLock(FSDK_LOCK* lock);
/**
 * @brief	Lock a lock object.
 *
 * @param[in] lock	Pointer to a lock object.
 *
 * @note	Call this function to lock.
 */
void			FSDK_Lock(FSDK_LOCK* lock);
/**
 * @brief	Unlock a lock object.
 *
 * @param[in] lock	Pointer to a lock object.
 *
 * @note	Call this function to unlock.
 */
void			FSDK_Unlock(FSDK_LOCK* lock);

/*******************************************************************************/
/* File access																 */
/*******************************************************************************/
/**
 * @brief	Compare extension name.
 *
 * @param[in] fileName	Source file name.
 * @param[in] extName	Extension name.
 *
 * @return	TRUE if extension name are same, or FALSE if different.
 *
 * @note	Call this function to distinguish file types, just a simple way.
 */
BOOL			FSDK_CompareExtName(const char* fileName, const char* extName);

/**
 * @brief	Open a file with given access mode.
 *
 * @param[in] fileName	File name to be opened.
 * @param[in] fileMode	File mode string to specify how to access, it has the same value as fopen function.
 * @param[in] useDef	Specifies how to open file.
 *						If it's TRUE, this function calls FSCRT_File_CreateFromFileName to open directly.
 *						If it's FALSE, this function calls FSCRT_File_Create to open.
 *
 * @return	File handle. NULL if fails.
 *
 * @note	Call this function to distinguish file types, just a simple way.
 */
FSCRT_FILE		FSDK_OpenFile(const char* fileName, const char* fileMode, BOOL useDef = FALSE);

/**
 * @brief	Save bitmap into image file.
 *
 * @param[in] bitmap	Bitmap object to be saved.
 * @param[in] type		Image file type, please refer to macro definitions FSCRT_IMAGETYPE_XXX in fs_image_r.h.
 * @param[in] fileName	Image file name.
 *
 * @return	FSCRT_ERRCODE_SUCCESS if succeeds, other value if fails.
 *
 * @note	This function calls FSCRT_ImageFile_Create to create image file,
 *			and calls FSCRT_ImageFile_AddFrame to add bitmap,
 *			and then calls FSCRT_ImageFile_Release to release image file.
 */
FS_RESULT		FSDK_SaveImageFile(FSCRT_BITMAP bitmap, FS_INT32 type, const char* fileName);

/**
 * @brief	Save PDF file.
 *
 * @param[in] document	PDF document to be saved.
 * @param[in] fileName	PDF File name.
 * @param[in] flag		Flag to specify how to save PDF, default value is FSPDF_SAVEFLAG_INCREMENTAL which means saving PDF as incremental way.
 *
 * @return	TRUE if success, FALSE if fails.
 *
 * @note	This function calls FSPDF_Doc_StartSaveToFile to start saving PDF as progressive way,
 *			and calls FSCRT_Progress_Continue to go on,
 *			and then calls FSCRT_Progress_Release to finish saving.
 */
FS_BOOL			FSDK_SavePDFFile(FSCRT_DOCUMENT document, const char* fileName, FS_INT32 flag = FSPDF_SAVEFLAG_INCREMENTAL);

/**
 * @brief	Check whether image formats are supported by Foxit SDK.
 *
 * @param[in] fileName	Image File Name.
 * @param[in] read		Flag to indicate reading or writing an image file .
 *
 * @return	TRUE if supported, FALSE if not supported.
 */
FS_BOOL			FSDK_IsSupportedImageFile(const char* fileName, FS_BOOL read = TRUE);

/**
 * @brief	Get range of page indexes from input page range string.
 *
 * @param[in] pageNum			Sting for page range.
 * @param[out] pageArray		An integer array for storing page indexes.
 * @param[out] endIndex			Count of elements in parameter <i>pageArray</i>.
 *
 * @return	TRUE if success, FALSE if fails.
 */
BOOL			FSDK_GetPageRange(const char* pageNum, FS_INT32*& pageArray, FS_INT32* count);

/**
 * @brief	Get a file name from input path string.
 *
 * @param[in] filePath			A file path.
 *
 * @return	File name.
 */
string			FSDK_GetFileName(const char* filePath);

/**
 * @brief	Get custom input path of input folder from input path string.
 *
 * @param[in] inputPath			Custom input file path.
 *
 * @return	Path of custom input folder.
 */
string			FSDK_GetCustomInputPath(const char* inputPath);

/**
 * @brief	Get a float value from input string.
 *
 * @param[in]  strValue		a input string.
 * @param[out] fParam		An float variable for storing output value.
 *
 * @return	File name.
 */
FS_FLOAT FSDK_GetFloat(const char * strValue);

/**
 * @brief	Get a integer value from input string.
 *
 * @param[in]  strValue		a input string.
 * @param[out] fParam		An integer variable for storing output value.
 *
 * @return	File name.
 */
FS_INT32 FSDK_GetInt(const char * strValue);

#endif /* _FSDK_COMMON_H_ */
