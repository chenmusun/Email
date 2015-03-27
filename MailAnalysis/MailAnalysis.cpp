// MailAnalysis.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "MailAnalysis.h"
#include "../GGDataAPI/utf8.hpp"
#include "../ReceiveEmail/public.h"
#include "CodeConvert.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


TCHAR  g_MailHeadItem[][50] = {
	_T("date:"),			//日期和时间 
	_T("subject:"),			//主题 
	_T("from:"),			//发件人地址 
	_T("content-type:"),	//内容的类型 
	_T("content-transfer-encoding:"), //内容的传输编码方式
	_T("x-uidl:"),
	_T("message-id:"),
	_T("content-disposition:"),
	_T("content-id:"),
	_T("to:"),
	_T("reply-to:"),
};

TCHAR g_MailBoundaryHeadItem[][50] =
{
	_T("content-type:"),
	_T("content-transfer-encoding:"),
	_T("content-description:"),
	_T("content-disposition:"),
};

TCHAR g_MailContentTypeItem[][50] =
{
	_T("charset="),
	_T("name="),
	_T("boundary="),
	_T("type"),
};

TCHAR g_MailDispostionItem[][50] =
{
	_T("filename="),
	_T("size="),
};



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


CMailAnalysis::CMailAnalysis() :m_lHeadRowCount(0), m_lCurrRow(0), m_lAttachmentCount(0)
{
	memset(m_szMainPath, 0, MAX_PATH);
	m_stAttachMent.clear();
	m_stSubAttachMent.clear();
	m_stEmail.Init();
}

CMailAnalysis::~CMailAnalysis()
{
	m_stAttachMent.clear();
	m_stSubAttachMent.clear();
}

void CMailAnalysis::Test()
{
	OutputDebugStringA("Test\r\n");
}



long CMailAnalysis::LoadFile(LPCTSTR lpPath, LPCTSTR lpUIDL, long lType)
{
	CString csLog;
	BOOL bFirst = TRUE;
	CFile stream;
	SetMainPath(lpPath);
	m_csUIDL.Format(_T("%s"), lpUIDL);

	m_csFilePath.Format(_T("%s\\%s.eml"), m_szMainPath, m_csUIDL);
	m_csSavePath.Format(_T("%s\\%s\\"), m_szMainPath, m_csUIDL);
	BOOL bRet = stream.Open(m_csFilePath, CFile::modeRead);
	if (bRet == FALSE)
	{
		csLog.Format(_T("打开文件[%s]失败！"),m_csFilePath);
		m_log.Log(csLog.GetBuffer(),csLog.GetLength());
		return -1;
	}
	ULONGLONG dwSize = stream.GetLength();//读到的文件大小
#ifdef _DEBUG
	CString csDebug;
	csDebug.Format(_T("FilePath = %s\r\nSize = %I64u byte long."), m_csFilePath, dwSize);
	OutputDebugString(csDebug);
	OutputDebugString(_T("\r\n"));
#endif
	char *pFile = new char[dwSize + 1];
	memset(pFile, 0, dwSize + 1);
	stream.Read(pFile, (UINT)dwSize);
	stream.Close();

	CString csTemp, csTemp2, csItem;
	char cTmp[MAX_BUFFER];
	int j = 0;
#ifdef _DEBUG
	DWORD dwTime = GetTickCount();
#endif
	//TODO：需要优化，目前处理方式加载时间过长
	for (int i = 0; i < dwSize; i++)//去除\r每行保存到一个CstringArray中
	{
		if (pFile[i] == '\r')
		{
			memset(cTmp, 0x00, sizeof(cTmp));
			memcpy(cTmp, pFile + j, i - j);
			csTemp = cTmp;
			m_csText.Add(csTemp);
			if (csTemp.IsEmpty() && bFirst)
			{
				m_lHeadRowCount = m_csText.GetSize() - 1;
				bFirst = FALSE;
			}
			j = i + 1;
		}
		else if (pFile[i] == '\n')
		{
			j++;
		}
	}
	MSAFE_DELETE(pFile);
#ifdef _DEBUG
	dwTime = GetTickCount() - dwTime;
	csDebug.Format(_T("LoadFile Time = %d sec\r\n"), dwTime / 1000);
	OutputDebugString(csDebug);
#endif
	m_stEmail.csTo = m_csAbbreviation;
	m_stEmail.csUIDL = lpUIDL;
	return 0;
}


long CMailAnalysis::AnalysisHead()
{
	long long llSize = m_csText.GetSize();
	if (llSize <= 0)
		return -1;
	CString csItem, csTemp, csKey, csDate, csFrom, csContentType,
		csEncoding, csUIDL, csMessageID, csBoundry, csSrc, csExtra,
		csLog;
	CStringArray csSubject, csTo;
	int nSave = -1;
	//////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
	CString csDebug;
#endif
	for (long long i = 0; i < m_lHeadRowCount; i++)
	{
		csTemp = csItem = m_csText.GetAt(i);
		csTemp.MakeLower();
		for (int n = 0; n < sizeof(g_MailHeadItem) / sizeof(g_MailHeadItem[0]); n++)
		{
			if (csItem.GetAt(0) == 0x20 || csItem.GetAt(0) == 0x09)
			{
				switch (nSave)
				{
				case _DATE_:
					csDate += csItem;
					break;
				case _SUBJECT_:
					csSubject.Add(csItem);
					break;
				case _FROM_:
					csFrom += csItem;
					break;
				case _CONTENT_TYPE_:
					csContentType += csItem;
					break;
				case _CONTENT_TRANSFER_ENCODING_:
					csEncoding += csItem;
					break;
				case _X_UIDL_:
					csUIDL += csItem;
					break;
				case _MESSAGE_ID_:
					csMessageID += csItem;
					break;
				case _TO_:
				case _REPLY_TO_:
					csTo.Add(csItem);
					break;
				default:
					break;
				}
				break;
			}
			csKey = g_MailHeadItem[n];
			if (csTemp.Find(csKey) == 0)
			{
				nSave = n;
				switch (nSave)
				{
				case _DATE_:
					csDate += csItem;
					break;
				case _SUBJECT_:
					csSubject.Add(csItem);
					break;
				case _FROM_:
					csFrom += csItem;
					break;
				case _CONTENT_TYPE_:
					csContentType += csItem;
					break;
				case _CONTENT_TRANSFER_ENCODING_:
					csEncoding += csItem;
					break;
				case _X_UIDL_:
					csUIDL += csItem;
					break;
				case _MESSAGE_ID_:
					csMessageID += csItem;
					break;
				case _TO_:
				case _REPLY_TO_:
					csTo.Add(csItem);
					break;
				default:
					break;
				}
				break;
			}
			nSave = -1;
		}
	}
	if (!csContentType.IsEmpty())
	{
		long lType(0), lValue(0);
		//Content-type: multipart/mixed; 	Boundary="0__=C7BBF72EDF8239E08f9e8a93df938690918cC7BBF72EDF8239E0"
		csTemp.Format(_T("%s"), csContentType);
		lValue = GetContentInfo(csTemp, m_cscsContentType, csExtra, lType);
		switch (lValue)
		{
		case _CHARSET_:
			m_csCharset = csExtra;
			break;
		case _BOUNDARY_:
			m_csBoundary = csExtra;
			break;
		default:
			break;
		}
		m_cscsContentType.TrimLeft();
		m_cscsContentType.TrimRight();
	}
	if (!csFrom.IsEmpty())
	{
		StringProcess(csFrom, csFrom, _FROM_);
		StringProcess(csFrom, csTemp, csExtra, 0);
		StringProcess(csTemp, csFrom);
		csTemp = StringEncode(csFrom);
		if (csTemp.Find(_T("@")) > 0)
			m_stEmail.csFrom = csTemp;
		else if (csExtra.Find(_T("@")) > 0)
			m_stEmail.csFrom = csExtra;
		else m_stEmail.csFrom = csTemp;
#ifdef _DEBUG
		OutputDebugString(_T("From:"));
		OutputDebugString(csTemp);
		OutputDebugString(_T("\t"));
		OutputDebugString(csExtra);
		OutputDebugString(_T("\r\n"));
#endif

	}
	if (!csMessageID.IsEmpty())
	{
		StringProcess(csMessageID, csTemp);
		StringProcess(csTemp, csMessageID, _MESSAGE_ID_);
		m_csMessageID = csMessageID;
#ifdef _DEBUG
		OutputDebugString(_T("MessageID:"));
		OutputDebugString(csMessageID);
		OutputDebugString(_T("\r\n"));
#endif
	}
	if (!csEncoding.IsEmpty())
	{
		StringProcess(csEncoding, csTemp);
		StringProcess(csTemp, csEncoding, _CONTENT_TRANSFER_ENCODING_);
		if (!csEncoding.IsEmpty())
		{
			m_csEncoding = csEncoding;
#ifdef _DEBUG
			OutputDebugString(_T("Content-Transfer-Encoding:"));
			OutputDebugString(m_csEncoding);
			OutputDebugString(_T("\r\n------------------------------------------------\r\n"));
#endif
		}
	}
	COleDateTime datetime;
	int nOffset = csDate.Find(_T(","), 0), nHour(0);
	csDate = csDate.Mid(nOffset + 1);
	nOffset = csDate.Find(_T("+"), 0);
	if (nOffset > 0)
	{
		csTemp = csDate.Mid(nOffset + 1);
		nHour = _ttoi(csTemp);
		nHour = nHour / 100;
	}
	csDate = csDate.Left(nOffset);
	datetime.ParseDateTime(csDate);
	if (datetime.GetStatus() != COleDateTime::valid) datetime = COleDateTime::GetCurrentTime();
	datetime += COleDateTimeSpan(0, nHour, 0, 0);
	csDate.Format(_T("%d-%d-%d\t%d:%d:%d"),
		datetime.GetYear(), datetime.GetMonth(), datetime.GetDay()
		, datetime.GetHour(), datetime.GetMinute(), datetime.GetSecond());
	m_stEmail.csDate = csDate;
#ifdef _DEBUG
	csDebug.Format(_T("Date:%s\r\n"),csDate);
	OutputDebugString(csDebug);
#endif
	csTemp.Empty();
	CString csOutPut;
	for (long long i = 0; i < csSubject.GetSize(); i++)
	{
		StringProcess(csSubject.GetAt(i), csTemp, _SUBJECT_);
		StringProcess(csTemp, csTemp);
		csOutPut.AppendFormat(_T("%s"), StringEncode(csTemp));
	}
	if (!csOutPut.IsEmpty())
		m_stEmail.csSubject = csOutPut;
#ifdef _DEBUG
	OutputDebugString(_T("Subject:"));
	OutputDebugString(csOutPut);
	OutputDebugString(_T("\r\n"));
	OutputDebugString(_T("To:"));
#endif

	
	for (long long i = 0; i < csTo.GetSize(); i++)
	{
		csSrc = csTo.GetAt(i);
		StringProcess(csSrc, csSrc, _TO_);
		csSrc.Replace(_T(","), _T(""));
		csSrc.TrimLeft();
		StringProcess(csSrc, csTemp, csExtra, 0);
		StringProcess(csTemp, csSrc);
		csTemp = StringEncode(csSrc);
#ifdef _DEBUG
		csDebug.Format(_T("%s\t%s\r\n"), csTemp, csExtra);
		OutputDebugString(csDebug);
#endif
	}
	return 0;
}

long CMailAnalysis::AnalysisBody(const CString& csBoundry, long long lCurrentRow)
{
	long lStatus = 0;
	BOOL bFirst = TRUE;
	CString csItem, csTemp, csText;
	CString csHead;
	CStringArray csaHead;
	long long llSize = m_csText.GetSize();
	if (llSize <= 0)
		return -1;
	if (m_csBoundary.IsEmpty())
	{
		m_csBoundary.Format(_T("----BOUNDARY----"));
		for (long long i = lCurrentRow; i < llSize; i++)
		{
			csText.AppendFormat(_T("%s\n"), m_csText.GetAt(i));
		}
		ATTACH Attach;
		Attach.lsHead.clear();
		Attach.lsHead.push_back(m_csBoundary);
		csTemp.Format(_T("Content-Type:%s; charset=%s"), m_cscsContentType, m_csCharset);
		Attach.lsHead.push_back(csTemp);
		csTemp.Format(_T("Content-Transfer-Encoding:%s"), m_csEncoding);
		Attach.lsHead.push_back(csTemp);
		Attach.csText = csText;
		m_stAttachMent.push_back(Attach);
#ifdef _DEBUG
		CString csDebug;
		csDebug.Format(_T("HeadSize = %d\tSize = %d\r\n"), csHead.GetLength(), csText.GetLength());
		OutputDebugString(csDebug);
#endif
		return 0;
	}
	//MIME
	for (long long i = lCurrentRow; i < llSize; i++)
	{
		csTemp = csItem = m_csText.GetAt(i);
		m_lCurrRow = i;
		if (csItem.Find(csBoundry) >= 0)
		{
			if (bFirst)
			{
				csHead.AppendFormat(_T("%s\n"), csItem);
				csaHead.Add(csItem);
				bFirst = FALSE;
			}
			else
			{
				ATTACH Attach;
				Attach.lsHead.clear();
				for (int n = 0; n < csaHead.GetSize(); n++)
				{
					Attach.lsHead.push_back(csaHead.GetAt(n));
				}
				Attach.csText = csText;
				m_stAttachMent.push_back(Attach);
#ifdef _DEBUG
				CString csDebug;
				csDebug.Format(_T("HeadSize = %d\tSize = %d\r\n"), csHead.GetLength(), csText.GetLength());
				OutputDebugString(csDebug);
#endif
				AnalysisBody(m_csBoundary, m_lCurrRow);
				break;
			}
		}
		else
		{
			if (csTemp.IsEmpty() && !bFirst)//空格之后为内容
			{
				lStatus = 1;
				continue;
			}
			switch (lStatus)
			{
			case 0:
			{
				if (!csItem.IsEmpty())
				{
					csHead.AppendFormat(_T("%s\n"), csItem);
					csaHead.Add(csItem);
				}
			}
				break;
			case 1:
				csText.AppendFormat(_T("%s\n"), csItem);
				break;
			default:
				break;
			}
		}
	}
	return 0;
}

long CMailAnalysis::AnalysisBoundary(const CString& csBoundary, vector<ATTACH>& vAttach)
{
	TCHAR chMainname[64] = { 0 };
	BOUNDARY_HEAD stBouHead;
	ATTACH_FILE attachfile;
	if (vAttach.size() > 0)
	{
		vector<ATTACH>::iterator ite = vAttach.begin();
		while (ite != vAttach.end())
		{
			stBouHead.Init();
			AnalysisBoundaryHead(ite->lsHead, csBoundary, stBouHead);
			switch (stBouHead.lContentType)
			{
			case NO_ATT:
			case TEXT_PLAIN:
			{
				wsprintf(chMainname, _T("main%d.txt"), m_lAttachmentCount);
				if (SaveToFile(ite->csText, chMainname, stBouHead.lCharset,stBouHead.lEncode) == 0)
				{
					attachfile.Init();
					attachfile.lType = 0;
					attachfile.csFilePath.Format(_T("%s%s"), m_csSavePath, chMainname);
					attachfile.csFileName.Format(_T("%s"), chMainname);
					m_stEmail.csContentType = _T("text/plain");
					m_stEmail.csEmailContent = attachfile.csFilePath;
					m_stEmail.vecAttachFiles.push_back(attachfile);
					m_lAttachmentCount++;
				}
			}
				break;
			case TEXT_HTML://
			{
				wsprintf(chMainname, _T("main%d.html"), m_lAttachmentCount);
				if (SaveToFile(ite->csText, chMainname, stBouHead.lCharset, stBouHead.lEncode) == 0)
				{
					attachfile.Init();
					attachfile.lType = 0;
					attachfile.csFilePath.Format(_T("%s%s"), m_csSavePath, chMainname);
					attachfile.csFileName.Format(_T("%s"),chMainname);
					if (m_stEmail.csContentType.IsEmpty())
						m_stEmail.csContentType = _T("text/html");
					m_stEmail.lHasHtml = 1;
					m_stEmail.csEmailContentHTML = attachfile.csFilePath;
					m_stEmail.vecAttachFiles.push_back(attachfile);
					m_lAttachmentCount++;
				}				
			}
				break;
			case MULTI_ALTERNATIVE:
			case MULTI_MIXED:
			case MULTI_RELATED:
			{
				vector<ATTACH> attach;
				attach = AnalysisBody(ite->csText, stBouHead.csBoundary);
				if (!ite->csText.IsEmpty())
					ite->csText.TrimLeft();
				AnalysisBoundary(stBouHead.csBoundary, attach);
			}
				break;
			case IMG_PNG:
			default:
			{
				if (SaveToFile(ite->csText, stBouHead.csFilename.IsEmpty() ? stBouHead.csName : stBouHead.csFilename, stBouHead.lEncode) == 0)
				{
					attachfile.Init();
					attachfile.lType=1;
					m_stEmail.lHasAffix = 1;
					m_lAttachmentCount++;
					FormatFileName(stBouHead.csFilename);
					FormatFileName(stBouHead.csName);
					attachfile.csFileName = stBouHead.csFilename.IsEmpty() ? stBouHead.csName : stBouHead.csFilename;
					attachfile.csFilePath.Format(_T("%s%s"), m_csSavePath, stBouHead.csFilename.IsEmpty() ? stBouHead.csName : stBouHead.csFilename);
					GetContentType(stBouHead.lContentType, attachfile.csAffixType);
					m_stEmail.vecAttachFiles.push_back(attachfile);
				}
			}
				break;
			}
			ite++;
		}
	}
	else
		return -1;
	return 0;
}

long CMailAnalysis::AnalysisBoundaryHead(list<CString>& lsHead, const CString& csBoundary, BOUNDARY_HEAD& info)
{
	BOOL bStart = FALSE;
	CString csTemp, csItem, cskey, csFileName,
		csTransferEncod, csDescription, csDisposition, csContentType;
	CStringArray csaDescription;
	int nOffset(0), nEnd(0), nSave(-1), nKeyPos(-1), nBoundary(-1);
	long lSize = lsHead.size();
	if (lSize <= 0)
		return -1;
	list<CString>::iterator ite = lsHead.begin();

	while (ite != lsHead.end())
	{
		csItem = csTemp = *ite;
		if (csTemp.IsEmpty())
		{
			ite++;
			continue;
		}
		if (!bStart)
		{
			nBoundary = csTemp.Find(csBoundary);
			if (nBoundary >= 0)
			{
				bStart = TRUE;
				ite++;
				continue;
			}
			else
			{
				ite++;
				continue;
			}
		}
		csTemp.MakeLower();
		for (int n = 0; n < sizeof(g_MailBoundaryHeadItem) / sizeof(g_MailBoundaryHeadItem[0]); n++)
		{
			if (csItem.GetAt(0) == 0x20 || csItem.GetAt(0) == 0x09)
			{
				switch (nSave)
				{
				case _CONTENT_TYPE2_:
				{
					csContentType.Append(csItem);
				}
					break;
				case _CONTENT_DESCRIPTION_:
				{
					csaDescription.Add(csItem);
				}
					break;
				case _CONTENT_DISPOSITION2_:
				{
					csDisposition.Append(csItem);
				}
					break;
				case _CONTENT_TRANSFER_ENCODING2_:
				{
					GetKeyWords(csTemp, csItem, cskey, NULL, info.csEncoding);
				}
					break;
				default:
					break;
				}
				break;
			}
			cskey = g_MailBoundaryHeadItem[n];
			nKeyPos = csTemp.Find(cskey);
			if (nKeyPos >= 0)
			{
				nSave = n;
				switch (nSave)
				{
				case _CONTENT_TYPE2_:
				{
					csContentType.Append(csItem);
				}
					break;
				case _CONTENT_DESCRIPTION_:
				{
					GetKeyWords(csTemp, csItem, cskey, NULL, csDescription);
					if (!csDescription.IsEmpty())
						csaDescription.Add(csDescription);
				}
					break;
				case _CONTENT_DISPOSITION2_:
				{
					csDisposition.Append(csItem);
				}
					break;
				case _CONTENT_TRANSFER_ENCODING2_:
				{
					GetKeyWords(csTemp, csItem, cskey, NULL, csTransferEncod);
				}
					break;
				default:
					break;
				}
				break;
			}
			nSave = -1;
		}
		ite++;
	}
	CString csContent, csExtra;
	int nRet = GetContentInfo(csContentType, info.csContentType, csExtra, info.lContentType);
	switch (nRet)
	{
	case _CHARSET_:
	{
		info.csCharset = csExtra;
		csExtra.MakeLower();
		if (csExtra.Find(_T("gb2312")) >= 0)
			info.lCharset = GB2312;
		else if (csExtra.Find(_T("utf-8")) >= 0)
			info.lCharset = UTF8;
		else if (csExtra.Find(_T("iso-8859-1")) >= 0)
			info.lCharset = UTF8;
		else if (csExtra.Find(_T("gbk")) >= 0)
			info.lCharset = GBK;
		else info.lCharset = UTF8;
	}
		break;
	case _NAME_:
		info.csName = csExtra;
		break;
	case _BOUNDARY_:
		info.csBoundary = csExtra;
		break;
	default:
		break;
	}
	GetDescription(csaDescription, info.csContentDescription);
	GetDispositionInfo(csDisposition, info.csContentDisposition, info.csFilename);
#ifdef _DEBUG
	CString csDebug;
	csDebug.Format(_T("Content-Transfer-Encoding:%s\r\n------------------------------------------------\r\n"), csTransferEncod);
	OutputDebugString(csDebug);
#endif
	info.csEncoding = csTransferEncod;
	csTransferEncod.MakeLower();
	if (csTransferEncod.Find(_T("base64")) >= 0)
		info.lEncode = BASE64;
	else if (csTransferEncod.Find(_T("quoted-printable")) >= 0)
		info.lEncode = QUOTED_PRINTABLE;
	else if (csTransferEncod.Find(_T("binary")) >= 0)
		info.lEncode = BINARY;
	else info.lEncode = BASE64;

	return 0;
}

long CMailAnalysis::GetContentInfo(const CString& csSrc, CString& csContent, CString& csExtra, long& lConttype)
{
	if (csSrc.IsEmpty())
		return -1;
	CString csTemp(csSrc), csContentType, csSrcTemp(csSrc)
		, csName, csCharset, csBoundary, csText, csNameTemp, csCnName;
	int nOffset(0), nStart(0), nPos(0), nLast(0), nRet(0);
	csTemp.MakeLower();
	nOffset = csTemp.Find(_T("content-type:"));
	if (nOffset >= 0)
	{
		nStart = nOffset + 14;
		nOffset = csTemp.Find(_T(";"), nStart);
		if (nOffset >= 0)
		{
			GetKeyWords(csTemp, csSrcTemp, _T("content-type:"), _T(";"), csContentType);
			csTemp = csTemp.Mid(nOffset + 1);
			csSrcTemp = csSrcTemp.Mid(nOffset + 1);
		}
		else
			GetKeyWords(csTemp, csSrcTemp, _T("content-type:"), NULL, csContentType);
		StringProcess(csContentType, csContent);
		csContentType.MakeLower();
		if (csContentType.Find(_T("text/plain")) >= 0)
			lConttype = TEXT_PLAIN;
		else if (csContentType.Find(_T("application/octet-stream")) >= 0)
			lConttype = APP_OCTET;
		else if (csContentType.Find(_T("application/pdf")) >= 0)
			lConttype = APP_PDF;
		else if (csContentType.Find(_T("multipart/mixed")) >= 0)
			lConttype = MULTI_MIXED;
		else if (csContentType.Find(_T("multipart/alternative")) >= 0)
			lConttype = MULTI_ALTERNATIVE;
		else if (csContentType.Find(_T("application/zip")) >= 0)
			lConttype = APP_ZIP;
		else if (csContentType.Find(_T("text/html")) >= 0)
			lConttype = TEXT_HTML;
		else if (csContentType.Find(_T("multipart/related")) >= 0)
			lConttype = MULTI_RELATED;
		else if (csContentType.Find(_T("image/png")) >= 0)
			lConttype = IMG_PNG;
		else if (csContentType.Find(_T("application/msexcel")) >= 0)
			lConttype = APP_MSEX;
		else if (csContentType.Find(_T("image/jpeg")) >= 0)
			lConttype = IMG_JPG;
		else if (csContentType.Find(_T("application/msword")) >= 0)
			lConttype = APP_MSWD;
		else if (csContentType.Find(_T("application/vnd.openxmlformats-officedocument.wordprocessingml.document")) >= 0)
			lConttype = APP_MSWD;
		else if (csContentType.Find(_T("application/mspowerpoint")) >= 0)
			lConttype = APP_MSPT;
		else lConttype = UNKNOWN_TYPE;

		nOffset = nOffset < 0 ? 0 : nOffset;

		if (!csTemp.IsEmpty())
		{
			for (size_t i = 0; i < sizeof(g_MailContentTypeItem) / sizeof(g_MailContentTypeItem[0]); i++)
			{
				nOffset = csTemp.Find(_T(";"), (nStart < 0 ? 0 : nStart));
				if (nOffset >= 0)
					nStart = GetKeyWords(csTemp, csSrcTemp, g_MailContentTypeItem[i], _T(";"), csText);
				else GetKeyWords(csTemp, csSrcTemp, g_MailContentTypeItem[i], NULL, csText);
				if (!csText.IsEmpty())
				{
					switch (i)
					{
					case _CHARSET_:
						csCharset = csText;
						StringProcess(csCharset, csCharset);
						csExtra = csCharset;
						nRet = i;
						break;
					case _NAME_:
					{
						csName = csText;
						StringProcess(csName, csName);
						nLast = 0;
						do
						{
							nPos = csName.Find(_T("?=\t=?"), nLast);
							if (nPos < 0)
							{
								nPos = csName.Find(_T("?= =?"), nLast);
								if (nPos < 0)
								{
									csNameTemp = csName.Mid(nLast);
									StringProcess(csNameTemp, csNameTemp);
									csCnName.Append(StringEncode(csNameTemp));
								}
								else
								{
									//nPos += 2;
									csNameTemp = csName.Mid(nLast, nPos - nLast);
									nLast = nPos + 1;
									csNameTemp.Replace(_T("\t"), _T(""));
									StringProcess(csNameTemp, csNameTemp);
									csCnName.Append(StringEncode(csNameTemp));
								}
							}
							else
							{
								nPos += 2;
								csNameTemp = csName.Mid(nLast, nPos - nLast);
								nLast = nPos + 1;
								csNameTemp.Replace(_T("\t"), _T(""));
								StringProcess(csNameTemp, csNameTemp);
								csCnName.Append(StringEncode(csNameTemp));
							}
						} while (nPos >= 0);
						csName = csCnName;
						csExtra = csName;
						nRet = i;
					}
						break;
					case _BOUNDARY_:
						csBoundary = csText;
						StringProcess(csBoundary, csBoundary);
						csExtra = csBoundary;
						nRet = i;
						break;
					case _TYPE_:
						break;
					default:
						break;
					}
				}
			}
		}
	}
#ifdef _DEBUG
	CString csDebug;
	csDebug.Format(_T("------------------------------------------------\r\nContent-type:%s\tcharset=%s\tname=%s\tboundary=%s\r\n"),
		csContentType, csCharset, csName, csBoundary);
	OutputDebugString(csDebug);
#endif
	return nRet;
}

void CMailAnalysis::GetDispositionInfo(const CString& csSrc, CString& csDis, CString& csExtra)
{
	if (csSrc.IsEmpty())
		return;
	CString csTemp(csSrc), csSrcBack(csSrc),
		csDisposition, csText, csFNTemp, csCnName;
	int nOffset(0), nStart(0), nPos(0), nLast(0);
	csTemp.MakeLower();
	nOffset = csTemp.Find(_T("content-disposition:"));
	if (nOffset >= 0)
	{
		nStart = nOffset + 21;
		nOffset = csTemp.Find(_T(";"), nStart);
		if (nOffset >= 0)
		{
			GetKeyWords(csTemp, csSrcBack, _T("content-disposition:"), _T(";"), csDisposition);
			csTemp = csTemp.Mid(nOffset + 1);
			csSrcBack = csSrcBack.Mid(nOffset + 1);
		}
		else
			GetKeyWords(csTemp, csSrcBack, _T("content-disposition:"), NULL, csDisposition);
	}
	StringProcess(csDisposition, csDis);
	nOffset = nOffset < 0 ? 0 : nOffset;
	if (!csTemp.IsEmpty())
	{
		for (size_t i = 0; i < sizeof(g_MailDispostionItem) / sizeof(g_MailDispostionItem[0]); i++)
		{
			nOffset = csTemp.Find(_T(";"), nStart);
			if (nOffset >= 0)
				nStart = GetKeyWords(csTemp, csSrcBack, g_MailDispostionItem[i], _T(";"), csText);
			else GetKeyWords(csTemp, csSrcBack, g_MailDispostionItem[i], NULL, csText);
			if (!csText.IsEmpty())
			{
				switch (i)
				{
				case _FILE_NAME_:
				{
					StringProcess(csText, csText);
					nLast = 0;
					do
					{
						nPos = csText.Find(_T("?=\t=?"), nLast);
						if (nPos < 0)
						{
							nPos = csText.Find(_T("?= =?"), nLast);
							if (nPos < 0)
							{
								csFNTemp = csText.Mid(nLast);
								StringProcess(csFNTemp, csFNTemp);
								csCnName.Append(StringEncode(csFNTemp));
							}
							else
							{
								//nPos += 2;
								csFNTemp = csText.Mid(nLast, nPos - nLast);
								nLast = nPos + 1;
								csFNTemp.Replace(_T("\t"), _T(""));
								StringProcess(csFNTemp, csFNTemp);
								csCnName.Append(StringEncode(csFNTemp));
							}
						}
						else
						{
							nPos += 2;
							csFNTemp = csText.Mid(nLast, nPos - nLast);
							nLast = nPos + 1;
							csFNTemp.Replace(_T("\t"), _T(""));
							StringProcess(csFNTemp, csFNTemp);
							csCnName.Append(StringEncode(csFNTemp));
						}
					} while (nPos >= 0);
					csExtra = csCnName;
				}
					break;
				case _SIZE_:
					break;
				default:
					break;
				}
			}
		}
	}
#ifdef _DEBUG
	CString csDebug;
	csDebug.Format(_T("Content-Disposition:%s\tfilename=%s\r\n"),
		csDis, csExtra);
	OutputDebugString(csDebug);
#endif
}

void CMailAnalysis::GetDescription(const CStringArray& csaSrc, CString& csDescription)
{
	int nSize = (int)csaSrc.GetSize();
	if (nSize <= 0)
		return;
	csDescription.Empty();
	CString csItem;
	for (int i = 0; i < nSize; i++)
	{
		csItem = csaSrc.GetAt(i);
		csItem.Replace(_T("\n"), _T(""));
		csItem.Replace(_T("\t"), _T(""));
		StringProcess(csItem, csItem);
		csDescription.Append(StringEncode(csItem));
	}
#ifdef _DEBUG
	OutputDebugString(_T("Content-Description:"));
	OutputDebugString(csDescription);
	OutputDebugString(_T("\r\n"));
#endif
}

vector<ATTACH> CMailAnalysis::AnalysisBody(const CString& csBody, const CString& csBoundry)
{
	BOOL bText = FALSE, bFirst = TRUE;
	vector<ATTACH> stAttach;
	CStringArray csaText, csaContent;
	CString csTemp(csBody), csData, cskey, csEncodie, csHead, csText;
	csTemp.TrimLeft();
	int nOffset(0), nStart(0), nSize(0), nSave(-1), nKeyPos;
	nSize = csTemp.GetLength();
	do
	{
		nOffset = csTemp.Find(_T("\n"), nStart);
		if (nOffset >= 0)
		{
			csData = csTemp.Mid(nStart, nOffset - nStart);
			csaText.Add(csData);
			nStart = nOffset + 1;
		}
		else break;
		if (nOffset > nSize)
			break;
	} while (1);
	for (int i = 0; i < csaText.GetSize(); i++)
	{
		csTemp = csData = csaText.GetAt(i);
		if (csData.IsEmpty())
		{
			bText = TRUE;
		}
		csTemp.MakeLower();
		if (csData.Find(csBoundry) >= 0)
		{
			if (bFirst)
				bFirst = FALSE;
			else
			{
				ATTACH Attach;
				Attach.lsHead.clear();
				for (int n = 0; n < csaContent.GetSize(); n++)
				{
					Attach.lsHead.push_back(csaContent.GetAt(n));
				}
				Attach.csText = csText;
				stAttach.push_back(Attach);
				csaContent.RemoveAll();
				csText.Empty();
			}
			csaContent.Add(csData);
			bText = FALSE;
			continue;
		}
		if (!bText)
		{
			for (int n = 0; n < sizeof(g_MailBoundaryHeadItem) / sizeof(g_MailBoundaryHeadItem[0]); n++)
			{
				if (csData.GetAt(0) == 0x20 || csData.GetAt(0) == 0x09)
				{
					switch (nSave)
					{
					case _CONTENT_TRANSFER_ENCODING2_:
					{
						csaContent.Add(csData);
					}
						break;
					default:
					{
						csaContent.Add(csData);
					}
						break;
					}
					break;
				}
				cskey = g_MailBoundaryHeadItem[n];
				nKeyPos = csTemp.Find(cskey);
				if (nKeyPos >= 0)
				{
					nSave = n;
					switch (nSave)
					{
					case _CONTENT_TRANSFER_ENCODING2_:
					{
						csaContent.Add(csData);
					}
						break;
					default:
					{
						csaContent.Add(csData);
					}
						break;
					}
					break;
				}
				nSave = -1;
				if (n == sizeof(g_MailBoundaryHeadItem) / sizeof(g_MailBoundaryHeadItem[0])-1)
				{
					bText = TRUE;
					csText.AppendFormat(_T("%s\n"), csData);
				}
			}
		}
		else
		{
			csText.AppendFormat(_T("%s\n"), csData);
		}
	}
	return stAttach;
}

long CMailAnalysis::SaveToFile(CString& csCode, LPCTSTR lpFileName, int nCharset, int nCodeType)
{
	char*pCode = NULL;
	CString csSavePath, csDeCode,csFileName(lpFileName);
	if (csCode.IsEmpty() || csFileName.IsEmpty())
		return -1;
	if ((GetFileAttributes(m_csSavePath) == 0xFFFFFFFF))
		CreateDirectory(m_csSavePath, NULL);
	if (csFileName.Find(_T(".")) < 0)
		csFileName.Append(_T(".dat"));
	FormatFileName(csFileName);
	csSavePath.Format(_T("%s%s"), m_csSavePath, csFileName);
	CodeConvert(csCode, csDeCode, nCharset, nCodeType);
	if (!csDeCode.IsEmpty())
	{
		int nSize = WideCharToMultiByte(CP_ACP, 0, csDeCode, -1, NULL, 0, NULL, NULL);
		pCode = new char[nSize + 1];
		memset(pCode, 0, nSize + 1);
		WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, csDeCode, -1, pCode, nSize, NULL, NULL);
		CFile stream;
		BOOL bRet = stream.Open(csSavePath, CFile::modeCreate | CFile::modeWrite);
		if (bRet == FALSE)
			return 0;
		stream.Write(pCode, nSize);
		stream.Close();
		MSAFE_DELETE(pCode);
	}
	return 0;
}

long CMailAnalysis::SaveToFile(CString& csCode, LPCTSTR lpFileName, int nCodeType)
{
	char*pCode = NULL;
	CString csSavePath, csDeCode, csFileName(lpFileName);
	string strDecode;
	if (csCode.IsEmpty() || csFileName.IsEmpty())
		return -1;
	if ((GetFileAttributes(m_csSavePath) == 0xFFFFFFFF))
		CreateDirectory(m_csSavePath, NULL);
	if (csFileName.Find(_T(".")) < 0)
		csFileName.Append(_T(".dat"));
	FormatFileName(csFileName);
	csSavePath.Format(_T("%s%s"), m_csSavePath, csFileName);
	int LEN(0), ibytes(0);
	LEN = WideCharToMultiByte(CP_ACP, 0, csCode, -1, NULL, 0, NULL, NULL);
	char*pText = new char[LEN + 1];
	memset(pText, 0, LEN + 1);
	WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, csCode, -1, pText, LEN, NULL, NULL);
	char*pPath = NULL;
	int nLen = WideCharToMultiByte(CP_ACP, 0, csSavePath, -1, NULL, 0, NULL, NULL);
	pPath = new char[nLen + 1];
	memset(pPath, 0, nLen + 1);
	WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, csSavePath, -1, pPath, nLen, NULL, NULL);
	switch (nCodeType)
	{
	case BASE64:
	{
		int j = 0;
		// 分配及初始化缓存
		BYTE *sbuf = new BYTE[(LEN * 3) / 4];
		memset(sbuf, 0, (LEN * 3) / 4);
		try
		{
			BYTE temp[3];
			int offset = 0;
			int i = 0;
			BYTE n[4];
			do
			{
				for (i = 0; i < 4; i++)
				{
					while (pText[offset] == 0x09 ||
						pText[offset] == 0x0A ||
						pText[offset] == 0x0D ||
						pText[offset] == 0x20)
					{
						//跳过回车换行空格字符
						offset++;
						if (offset >= LEN) break;
					}
					if (offset >= LEN) break;
					n[i] = GetB64Code(pText[offset++]);
				}
				if (offset >= LEN) break;
				if (n[0] == 64 || n[1] == 64) break;
				temp[0] = ((n[0] << 2) & 0xFC) + ((n[1] >> 4) & 0x03);
				if (n[2] == 64) //判断是否是'='
				{
					sbuf[j++] = temp[0];
					break;
				}
				temp[1] = ((n[1] << 4) & 0xF0) + ((n[2] >> 2) & 0x0F);
				if (n[3] == 64) //判断是否是'='
				{
					sbuf[j++] = temp[0];
					sbuf[j++] = temp[1];
					break;
				}
				if (j > 490)
				{
					int i = 0;
				}
				temp[2] = ((n[2] << 6) & 0xC0) + ((n[3]) & 0x03F);
				sbuf[j++] = temp[0];
				sbuf[j++] = temp[1];
				sbuf[j++] = temp[2];
			} while (offset < LEN);
			// 缓存空间已初始化为空，在解码结束后无需再置0
			// 另外，使用"++"符合会导致向文件内写入一个无法识别的字符
			// sbuf[j++]='\r';			
			// sbuf[j++]='\0xFD';
		}
		catch (...)
		{
			MSAFE_DELETE(pText);
			MSAFE_DELETE(sbuf);
			return -1;
		}
		
		try
		{
			CFile stream;
			BOOL bRet = stream.Open(csSavePath, CFile::modeCreate | CFile::modeWrite);
			if (bRet == FALSE)
			{
				MSAFE_DELETE(sbuf);
				MSAFE_DELETE(pText);
				return 0;
			}
			stream.Write(sbuf, j);
			stream.Close();
			MSAFE_DELETE(sbuf);
		}
		catch (...)
		{
			MSAFE_DELETE(pText);
			MSAFE_DELETE(sbuf);
			return -1;
		}
		MSAFE_DELETE(sbuf);
	}
		break;
	case QUOTED_PRINTABLE:
	{
		FILE *fp = NULL;
		try
		{
			if ((fopen_s(&fp, pPath, "w+b")) != 0) return -1;
			if (LEN == 0) { fclose(fp);	return 0; }
		}
		catch (...)
		{
			MSAFE_DELETE(pText);
			MSAFE_DELETE(pPath);
			fclose(fp);
			return -1;
		}
		try
		{
			char temp[3];
			long pbefore = 0;
			long offset = 0;
			do
			{
				if (pText[offset] != '=')
				{
					offset++;
					continue;
				}

				fwrite(pText + pbefore, sizeof(char), offset - pbefore, fp);

				offset++;
				//判断是否为 "回车换行"
				if ((pText[offset] == 0x0D) || (pText[offset] == 0x0A))
				{
					offset++;
					if ((pText[offset] == 0x0D) || (pText[offset] == 0x0A)) offset++;
					pbefore = offset;
					continue;
				}

				//解码
				temp[0] = pText[offset];
				temp[0] = temp[0] < 0x41 ? temp[0] - 0x30 : (temp[0] < 0x61 ? temp[0] - 0x37 : temp[0] - 0x57);

				offset++;
				temp[1] = pText[offset];
				temp[1] = temp[1] < 0x41 ? temp[1] - 0x30 : (temp[1] < 0x61 ? temp[1] - 0x37 : temp[1] - 0x57);

				temp[2] = (temp[0] << 4) | (temp[1] & 0x0F);
				fwrite(temp + 2, 1, 1, fp); //只写入temp[2]字节

				offset++;
				pbefore = offset;

			} while (offset < LEN);

			//将最后没有编码的字节保存
			if (pbefore < LEN) fwrite(pText + pbefore, sizeof(char), LEN - pbefore, fp);
			fclose(fp);
		}
		catch (...)
		{
			MSAFE_DELETE(pText);
			MSAFE_DELETE(pPath);
			fclose(fp);
			return -1;
		}
	}
		break;
	default:
	{
		FILE *fp = NULL;			   
		try
		{
			if (fopen_s(&fp, pPath, "w+b") == 0)
			{
				fwrite(pText, LEN, 1, fp);
				fclose(fp);
			}
		}
		catch (...)
		{
			MSAFE_DELETE(pText);
			MSAFE_DELETE(pPath);
			fclose(fp);
			return -1;
		}
	}
		break;
	}
	MSAFE_DELETE(pText);
	MSAFE_DELETE(pPath);
	return 0;
}


void CMailAnalysis::Clear(long lType)
{
	//m_csText.RemoveAll();
	m_lHeadRowCount = 0;
	m_lCurrRow = 0;
	m_csBoundary.Empty();
	m_stAttachMent.clear();
	m_stSubAttachMent.clear();
	m_cscsContentType.Empty();
	m_csEncoding.Empty();
	m_csUIDL.Empty();
	m_csMessageID.Empty();
	m_csCharset.Empty();
	//删除本地解析结果
	if (m_stEmail.vecAttachFiles.size() > 0)
	{
		if (lType == 1)
		{
			vector<ATTACH_FILE>::iterator ite = m_stEmail.vecAttachFiles.begin();
			while (ite != m_stEmail.vecAttachFiles.end())
			{
				DeleteFile((*ite).csFilePath);
				ite++;
			}
		}
		m_stEmail.vecAttachFiles.clear();
	}
	if (!m_csSavePath.IsEmpty())
	{
		if (lType ==1)
			RemoveDirectory(m_csSavePath);
		m_csSavePath.Empty();
	}
	if (!m_csFilePath.IsEmpty())
	{
		if (lType ==1)
			DeleteFile(m_csFilePath);
	}
	m_csFilePath.Empty();
	m_lAttachmentCount = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL StringProcess(const CString& csSrc, CString& csDest)
{
	int nOffset(0), nEnd(0);
	if (csSrc.IsEmpty())
		return FALSE;
	CString csTemp(csSrc);
	csDest = csSrc;
	nOffset = csTemp.Find(_T("\""));
	if (nOffset == 0)
	{
		csTemp = csTemp.Right(csTemp.GetLength() - nOffset - 1);
		nOffset = csTemp.Find(_T("\""));
		nEnd = csTemp.ReverseFind(_T('\"'));
		if (nOffset >= 0 && nOffset == nEnd)
		{
			csDest = csTemp.Mid(0, nEnd);
		}
	}
	nOffset = csTemp.Find(_T("<"));
	if (nOffset == 0)
	{
		csTemp = csTemp.Right(csTemp.GetLength() - nOffset - 1);
		nOffset = csTemp.Find(_T(">"));
		nEnd = csTemp.ReverseFind(_T('>'));
		if (nOffset >= 0 && nOffset == nEnd)
		{
			csDest = csTemp.Mid(0, nEnd);
		}
	}
	//else csDest = csSrc;
	//=? ?=
	nOffset = csTemp.Find(_T("=?"));
	if (nOffset == 0)
	{
		csTemp = csTemp.Right(csTemp.GetLength() - nOffset - 2);
		//nOffset = csTemp.Find(_T("?="));
		nEnd = csTemp.ReverseFind(_T('?'));
		nOffset = csTemp.ReverseFind(_T('='));
		if (nEnd >= 0 && (nOffset - nEnd == 1))
		{
			csDest = csTemp.Mid(0, nEnd);
			return TRUE;
		}
	}
	//= =
	else
	{
		nOffset = csTemp.Find(_T("="));
		if (nOffset == 0)
		{
			csTemp = csTemp.Right(csTemp.GetLength() - nOffset - 1);
			nEnd = csTemp.ReverseFind(_T('='));
			if ((nEnd >= 0) && (nEnd == (csTemp.GetLength() - 1)))
			{
				csDest = csTemp.Mid(0, nEnd);
				return TRUE;
			}
		}
		//else csDest = csSrc;
	}
	return FALSE;
}

void StringProcess(const CString& csSrc, CString& csDest, CString& csExtra, int nFlag)
{
	if (csSrc.IsEmpty())
		return;
	CString csTemp(csSrc);
	csDest = csTemp;
	int nOffset(0), nEnd(0), nTemp(0);
	if (nFlag == 0)
	{
		nOffset = csTemp.Find(_T(" "));
		nEnd = csTemp.ReverseFind(_T(' '));
		if (nOffset > 0 && nEnd == nOffset)
		{
			csDest = csTemp.Left(nOffset);
		}
		else csExtra.Empty();
		nOffset = csTemp.Find(_T("<"), nOffset + 1);
		if (nOffset > 0 && nOffset > nEnd)
		{
			nTemp = csTemp.Find(_T(">"));
			nEnd = csTemp.ReverseFind(_T('>'));
			if (nTemp > nOffset && nTemp == nEnd)
			{
				csExtra = csTemp.Mid(nOffset + 1, nEnd - nOffset - 1);
			}
		}
	}
}

void StringProcess(const CString& csSrc, CString& csDest, int nType)
{
	CString csTemp, csItem;
	int nOffset(0);
	csTemp = csItem = csSrc;
	csTemp.MakeLower();
	switch (nType)
	{
	case _DATE_:
	{
		nOffset = csTemp.Find(_T("date:"));
		if (nOffset == 0)
			csTemp = csItem.Mid(nOffset + 6);
		else
			csTemp = csItem;
	}
		break;
	case _SUBJECT_:
	{
		nOffset = csTemp.Find(_T("subject:"));
		if (nOffset == 0)
			csTemp = csItem.Mid(nOffset + 9);
		else
			csTemp = csItem;
	}
		break;
	case _FROM_:
	{
		nOffset = csTemp.Find(_T("from:"));
		if (nOffset == 0)
			csTemp = csItem.Mid(nOffset + 6);
		else
			csTemp = csItem;
	}
		break;
	case _MESSAGE_ID_:
	{
		nOffset = csTemp.Find(_T("message-id:"));
		if (nOffset == 0)
			csTemp = csItem.Mid(nOffset + 12);
		else
			csTemp = csItem;
	}
		break;
	case _TO_:
	{
		nOffset = csTemp.Find(_T("to:"));
		if (nOffset == 0)
		{
			csTemp = csItem.Mid(nOffset + 4);
		}
		else
		{
			nOffset = csTemp.Find(_T("reply-to:"));
			if (nOffset == 0)
				csTemp = csItem.Mid(nOffset + 10);
			else
				csTemp = csItem;
		}
	}
		break;
	case _CONTENT_TRANSFER_ENCODING_:
	{
		nOffset = csTemp.Find(_T("content-transfer-encoding:"));
		if (nOffset == 0)
			csTemp = csItem.Mid(nOffset + 27);
		else
			csTemp = csItem;
	}
		break;
	default:
		break;
	}
	csTemp.TrimLeft();
	csTemp.TrimRight();
	csDest = csTemp;
	//StringProcess(csTemp, csDest);
}

void CodeConvert(const CString& csSrc, CString&csDest, int nCharset, int nCodetype)
{
	if (csSrc.IsEmpty())
		return;
	wstring wstrText;
	CString csTemp(csSrc);
	if (nCodetype == BASE64)
	{
		csTemp.Replace(_T("\r\n"), _T(""));
		csTemp.Replace(_T("\n"), _T(""));
	}
	char*pTemp = NULL,*pValue = NULL;
	int nSize = WideCharToMultiByte(CP_ACP, 0, csTemp, -1, NULL, 0, NULL, NULL);
	pTemp = new char[nSize + 1];
	memset(pTemp, 0, nSize + 1);
	long lSize = nSize + 1;
	pValue = new char[lSize];
	memset(pValue, 0, lSize);
	WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, csTemp, -1, pTemp, nSize, NULL, NULL);
	string strValue,strTemp = pTemp;
	switch (nCodetype)
	{
	case BASE64:
		strValue = base64_decode(strTemp);
		break;
	case QUOTED_PRINTABLE:
		quotedprintable_decode(pTemp, nSize, pValue, lSize);
		strValue = pValue;
		break;
	case BIT8:
		strValue = pTemp;
		break;
	default:
		break;
	}
	MSAFE_DELETE(pTemp);
	MSAFE_DELETE(pValue);
	switch (nCharset)
	{
	case UTF8://UTF-8
	{
		Utf8::Decode(strValue, wstrText);
		csDest = wstrText.c_str();
	}
		break;
	case GBK:
	case GB2312://GB2312
	{
		csDest = strValue.c_str();
	}
		break;
	default:
		csDest = strValue.c_str();
		break;
	}
}

long GetKeyWords(const CString&csSrc1, const CString& csSrc2, LPCTSTR lpKey, LPCTSTR lpEnd, CString& csDest)
{
	CString csKey(lpKey), csEnd(lpEnd);
	if (csSrc1.IsEmpty() || csKey.IsEmpty())
		return -1;
	csKey.MakeLower();
	int nOffset(0), nEnd(0), nKeyLen = csKey.GetLength();
	nOffset = csSrc1.Find(csKey);
	csDest.Empty();
	if (nOffset >= 0)
	{
		if (csEnd.IsEmpty())
			nEnd = csSrc2.GetLength();
		else
		{
			nEnd = csSrc1.Find(csEnd, nOffset + nKeyLen);
			if (nEnd<0)
				nEnd = csSrc2.GetLength();
		}
		if (nEnd >= 0 && nEnd > nOffset)
		{
			csDest = csSrc2.Mid(nOffset + nKeyLen, nEnd - nOffset - nKeyLen);
		}
	}
	return nEnd;
}

CString StringEncode(const CString& csSrc)
{
	int nOffset(0), nEnd(0), nStart(0), nCharset(-1), nCodetype(-1);
	CString csRet, csTemp, csTemp2(csSrc);
	StringProcess(csSrc, csRet);
	csTemp = csRet;
	csTemp.MakeLower();

	do 
	{
		nOffset = csTemp.Find(_T("gb2312?"));
		if (nOffset >= 0)
		{
			nCharset = GB2312;
			nStart += 7;
			break;
		}
		nOffset = csTemp.Find(_T("utf-8?"));
		if (nOffset >= 0)
		{
			nCharset = UTF8;
			nStart += 6;
			break;
		}
		nOffset = csTemp.Find(_T("gbk?"));
		if (nOffset >= 0)
		{
			nCharset = GBK;
			nStart += 5;
			break;
		}
		nOffset = csTemp.Find(_T("8bit?"));
		if (nOffset >= 0)
		{
			nCharset = BIT8;
			nStart += 5;
			break;
		}
		return csTemp2;
	} while (0);
	
	do 
	{
		nEnd = csTemp.Find(_T("b?"), nStart);
		if (nEnd > 0)
		{
			nCodetype = BASE64;
			csTemp2 = csRet.Mid(nEnd + 2);
			break;
		}
		nEnd = csTemp.Find(_T("q?"), nOffset);
		if (nEnd > 0)
		{
			nCodetype = QUOTED_PRINTABLE;
			csTemp2 = csRet.Mid(nEnd + 2);
			break;
		}
		return csTemp2;
	} while (0);
	CodeConvert(csTemp2, csRet, nCharset, nCodetype);
	return csRet;
}

void CMailAnalysis::SetLogPath(const char*pPath)
{
	if (pPath && pPath[0] != '\0')
	{
		m_log.SetPath(pPath);
	}
}

void CMailAnalysis::GetContentType(long lContentType, CString& csContentType)
{
	switch (lContentType)
	{
	case TEXT_PLAIN:
		csContentType.Format(_T("text/plain"));
		break;
	case APP_OCTET:
		csContentType.Format(_T("application/octet-stream"));
		break;
	case APP_PDF:
		csContentType.Format(_T("application/pdf"));
		break;
	case APP_ZIP:
		csContentType.Format(_T("application/zip"));
		break;
	case TEXT_HTML:
		csContentType.Format(_T("text/html"));
		break;
	case IMG_PNG:
		csContentType.Format(_T("image/png"));
		break;
	case IMG_JPG:
		csContentType.Format(_T("image/jpeg"));
		break;
	case APP_MSEX:
		csContentType.Format(_T("application/msexcel"));
		break;
	case APP_MSWD:
		csContentType.Format(_T("application/msword"));
		break;
	case APP_MSPT:
		csContentType.Format(_T("application/mspowerpoint"));
		break;
	default:
		csContentType.Format(_T("unknow"));
		break;
	}
}

void FormatFileName(CString& csFileName)
{
	if (csFileName.IsEmpty())
		return;
	csFileName.Replace(_T("*"), _T(""));
	csFileName.Replace(_T("\\"), _T(""));
	csFileName.Replace(_T(":"), _T(""));
	csFileName.Replace(_T("?"), _T(""));
	csFileName.Replace(_T("="), _T(""));
	csFileName.Replace(_T("<"), _T(""));
	csFileName.Replace(_T(">"), _T(""));
	csFileName.Replace(_T("\t"), _T(""));
	csFileName.Replace(_T(" "), _T(""));
}