// DialogPDF.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ReceiveEmail.h"
#include "DialogPDF.h"
#include "afxdialogex.h"

extern TCHAR __Main_Path__[MAX_PATH];

// CDialogPDF �Ի���

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


// CDialogPDF ��Ϣ��������



void CDialogPDF::OnBnClickedMfcbuttonGet()
{
	// TODO:  �ڴ����ӿؼ�֪ͨ�����������
	UpdateData(TRUE);
	if (m_csGUID.IsEmpty())
	{
		AfxMessageBox(_T("�ļ���Ϊ�գ�"));
		return;
	}
	m_csGUID.Append(_T(";"));
	if (m_csSavePath.IsEmpty())
	{
		AfxMessageBox(_T("����·��Ϊ�գ�"));
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
	POP3 pop3;
	pop3.SetDBInfo(m_modbinfo);
	if (pop3.ConnectDataBase())
	{
		if (pop3.GetFileFromDB(strFileName, strPath, strErr))
		{
			csText.Format(_T("Success!"));
		}
		else csText = strErr.c_str();
		pop3.QuitDataBase();
	}
	else csText.Format(_T("Connect to DB Error!"));
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

	// TODO:  �ڴ����Ӷ���ĳ�ʼ��
	m_csSavePath.Format(_T("%s\\PDF\\"), __Main_Path__);
	if ((GetFileAttributes(m_csSavePath) == 0xFFFFFFFF))
		CreateDirectory(m_csSavePath, NULL);
	m_Path.SetWindowText(m_csSavePath);


	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣:  OCX ����ҳӦ���� FALSE
}