// DialogPDF.cpp : 实现文件
//

#include "stdafx.h"
#include "ReceiveEmail.h"
#include "DialogPDF.h"
#include "afxdialogex.h"
#include "../DataBase/DataBase.h"

extern TCHAR __Main_Path__[MAX_PATH];

// CDialogPDF 对话框

IMPLEMENT_DYNAMIC(CDialogPDF, CDialogEx)

CDialogPDF::CDialogPDF(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogPDF::IDD, pParent)
	, m_csGUID(_T(""))
	, m_csSavePath(_T(""))
{
	memset(&m_modbinfo, 0, sizeof(MongoDBInfo));
}

CDialogPDF::~CDialogPDF()
{
}

void CDialogPDF::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_RICHEDIT2_GUID, m_csGUID);
	DDX_Text(pDX, IDC_MFCEDITBROWSE1, m_csSavePath);
	DDX_Control(pDX, IDC_MFCBUTTON_GET, m_btnGet);
	DDX_Control(pDX, IDC_MFCEDITBROWSE1, m_Path);
}


BEGIN_MESSAGE_MAP(CDialogPDF, CDialogEx)
	ON_BN_CLICKED(IDC_MFCBUTTON_GET, &CDialogPDF::OnBnClickedMfcbuttonGet)
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
	CDataBase db;
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
	OnOK();
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


	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}
