// DialogPDF.cpp : 实现文件
//

#include "stdafx.h"
#include "ReceiveEmail.h"
#include "DialogPDF.h"
#include "afxdialogex.h"
#include "../MongoDB/MongoDB.h"
#include "../PDF/PDF.h"

extern TCHAR __Main_Path__[MAX_PATH];

// CDialogPDF 对话框

IMPLEMENT_DYNAMIC(CDialogPDF, CDialogEx)

CDialogPDF::CDialogPDF(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogPDF::IDD, pParent)
	, m_csGUID(_T(""))
	, m_csSavePath(_T(""))
	, m_csFilePath(_T(""))
{
	memset(&m_modbinfo, 0, sizeof(MongoDBInfo));
	m_vecType.clear();
}

CDialogPDF::~CDialogPDF()
{
	m_vecType.clear();
}

void CDialogPDF::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_RICHEDIT2_GUID, m_csGUID);
	DDX_Text(pDX, IDC_MFCEDITBROWSE1, m_csSavePath);
	DDX_Control(pDX, IDC_MFCBUTTON_GET, m_btnGet);
	DDX_Control(pDX, IDC_MFCEDITBROWSE1, m_Path);
	DDX_Text(pDX, IDC_MFCEDITBROWSE2, m_csFilePath);
	DDX_Control(pDX, IDC_BUTTON_TEST, m_btnTest);
}


BEGIN_MESSAGE_MAP(CDialogPDF, CDialogEx)
	ON_BN_CLICKED(IDC_MFCBUTTON_GET, &CDialogPDF::OnBnClickedMfcbuttonGet)
	ON_BN_CLICKED(IDC_MFCBUTTON_PDF2TXT, &CDialogPDF::OnBnClickedMfcbuttonPdf2txt)
	ON_BN_CLICKED(IDC_MFCBUTTON_OPENFO, &CDialogPDF::OnBnClickedMfcbuttonOpenfo)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CDialogPDF::OnBnClickedButtonTest)
	ON_BN_CLICKED(IDC_BUTTON_PER, &CDialogPDF::OnBnClickedButtonPer)
	ON_BN_CLICKED(IDC_BUTTON_WM, &CDialogPDF::OnBnClickedButtonWm)
	ON_BN_CLICKED(IDC_BUTTON_CONVERT, &CDialogPDF::OnBnClickedButtonConvert)
END_MESSAGE_MAP()


// CDialogPDF 消息处理程序



void CDialogPDF::OnBnClickedMfcbuttonGet()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (m_csGUID.IsEmpty())
	{
		AfxMessageBox(_T("文件名为空！"));
		return;
	}
	m_csGUID.Append(_T(";"));
	if (m_csSavePath.IsEmpty())
	{
		AfxMessageBox(_T("保存路径为空！"));
		return;
	}
	char chTemp[512] = { 0 };
	CString csText;
	string strFileName,strErr,strPath;
	WideCharToMultiByte(CP_ACP, 0, m_csGUID, m_csGUID.GetLength()*sizeof(TCHAR), chTemp, 512, NULL, NULL);
	strFileName = chTemp;
	memset(chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_csSavePath, m_csSavePath.GetLength()*sizeof(TCHAR), chTemp, 512, NULL, NULL);
	strPath = chTemp;
	m_modbinfo.nUseDB = 1;
	CMongoDB db;
	db.SetDBInfo(m_modbinfo);
	if (db.ConnectDataBase(strErr))
	{
		if (!db.GetFileFromMongoDB(strFileName, strPath, strErr))
			csText = strErr.c_str();
		else
			csText.Format(_T("Success!"));
	}
	else csText = strErr.c_str();
	AfxMessageBox(csText);
}

void CDialogPDF::SetMongoDBInfo(const MongoDBInfo& info)
{
	memcpy_s(&m_modbinfo, sizeof(MongoDBInfo), &info, sizeof(MongoDBInfo));
}


BOOL CDialogPDF::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_csSavePath.Format(_T("%s\\PDF\\"), __Main_Path__);
	if ((GetFileAttributes(m_csSavePath) == 0xFFFFFFFF))
		CreateDirectory(m_csSavePath, NULL);
	m_Path.SetWindowText(m_csSavePath);
#ifndef _DEBUG
	m_btnTest.ShowWindow(SW_HIDE);
#endif
	m_vecType.push_back(_T(".doc"));
	m_vecType.push_back(_T(".docx"));
	m_vecType.push_back(_T(".xls"));
	m_vecType.push_back(_T(".xlsx"));
	m_vecType.push_back(_T(".ppt"));
	m_vecType.push_back(_T(".pptx"));
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CDialogPDF::OnBnClickedMfcbuttonPdf2txt()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (m_csFilePath.IsEmpty())
	{
		AfxMessageBox(_T("请输入文件路径!"));
		return;
	}
	CString csTemp(m_csSavePath);
	char chPath[512] = { 0 };
	string strInputPath;
	int nPageNum(0),nTime(0);
	if (m_csFilePath.Find(_T(".pdf")) > 0)
	{
		WideCharToMultiByte(CP_ACP, 0, m_csFilePath, m_csFilePath.GetLength(), chPath, 512, NULL, NULL);		strInputPath = chPath;		int nValue = PDF2TEXT(strInputPath);		if (nValue == 0)		{
			csTemp.Format(_T("Success!"));
			AfxMessageBox(csTemp);
		}
	}	
}


void CDialogPDF::OnBnClickedMfcbuttonOpenfo()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (!m_csSavePath.IsEmpty())
		ShellExecute(NULL, _T("open"), m_csSavePath, NULL, NULL, SW_SHOWNORMAL);
}


void CDialogPDF::OnBnClickedButtonTest()
{
	// TODO:  在此添加控件通知处理程序代码
}


void CDialogPDF::OnBnClickedButtonPer()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (m_csFilePath.IsEmpty())
	{
		AfxMessageBox(_T("请输入文件路径!"));
		return;
	}
	CString csTemp(m_csSavePath), csFileName;
	char chPath[512] = { 0 };
	string strInputPath, strOutputPath, strOutPutName;
	if (m_csFilePath.Find(_T(".pdf")) > 0)
	{
		WideCharToMultiByte(CP_ACP, 0, m_csFilePath, m_csFilePath.GetLength(), chPath, 512, NULL, NULL);		strInputPath = chPath;		int nValue = RemovePasswd(strInputPath);		if (nValue == 2)		{
			csTemp.Format(_T("Success!"));
			AfxMessageBox(csTemp);
		}
	}
}


void CDialogPDF::OnBnClickedButtonWm()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (m_csFilePath.IsEmpty())
	{
		AfxMessageBox(_T("请输入文件路径!"));
		return;
	}
	CString csTemp;
	char chPath[512] = { 0 };
	string strInputPath;
	int nPageNum(0), nTime(0);
	if (m_csFilePath.Find(_T(".pdf")) > 0)
	{
		WideCharToMultiByte(CP_ACP, 0, m_csFilePath, m_csFilePath.GetLength(), chPath, 512, NULL, NULL);		strInputPath = chPath;		int nValue = RemoveWaterMark(strInputPath);		if (nValue == 0)		{
			csTemp.Format(_T("Success!"));
			AfxMessageBox(csTemp);
		}
	}
}


void CDialogPDF::OnBnClickedButtonConvert()
{
	// TODO:  在此添加控件通知处理程序代码
	CString csTemp;
	vector<CString>::iterator ite = m_vecType.begin();
	UpdateData(TRUE);
	if (m_csFilePath.IsEmpty())
	{
		AfxMessageBox(_T("请输入文件路径!"));
		return;
	}
	auto pos = m_csFilePath.ReverseFind(_T('.'));
	if (pos > 0)
	{
		csTemp = m_csFilePath.Mid(pos);
		ite = find(m_vecType.begin(), m_vecType.end(), csTemp);
		if (ite != m_vecType.end())
		{
			char chPath[512] = { 0 };
			string strInputPath;
			WideCharToMultiByte(CP_ACP, 0, m_csFilePath, m_csFilePath.GetLength(), chPath, 512, NULL, NULL);			strInputPath = chPath;
			int nValue = OFFICE2PDF(strInputPath);
			if (nValue == 0)
			{
				csTemp.Format(_T("Success!"));
				AfxMessageBox(csTemp);
			}
		}
	}
}
