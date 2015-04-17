// SettingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ReceiveEmail.h"
#include "SettingDlg.h"
#include "afxdialogex.h"


// CSettingDlg 对话框

IMPLEMENT_DYNAMIC(CSettingDlg, CDialogEx)

CSettingDlg::CSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSettingDlg::IDD, pParent)
	, m_bUseDB(FALSE)
	, m_csmosrvadd(_T(""))
	, m_csmodbname(_T(""))
	, m_csmotabnam(_T(""))
	, m_csmousrnam(_T(""))
	, m_csmopasswd(_T(""))
	, m_cssqlsrvadd(_T(""))
	, m_cssqldbnam(_T(""))
	, m_cssqlusrnam(_T(""))
	, m_cssqlpasswd(_T(""))
	, m_cssmtpadd(_T(""))
	, m_cssmtpusrnam(_T(""))
	, m_cssmtppasswd(_T(""))
	, m_csto(_T(""))
{
	memset(&m_moinfo, 0, sizeof(MongoDBInfo));
	memset(&m_sqlinfo, 0, sizeof(SQLDBInfo));
	memset(&m_fsinfo, 0, sizeof(ForwardSet));
}

CSettingDlg::~CSettingDlg()
{
}

void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_USEDB, m_bUseDB);
	DDX_Text(pDX, IDC_EDIT_SRVADD, m_csmosrvadd);
	DDX_Text(pDX, IDC_EDIT_DBNAME, m_csmodbname);
	DDX_Text(pDX, IDC_EDIT_TABNAME, m_csmotabnam);
	DDX_Text(pDX, IDC_EDIT_USRNAM, m_csmousrnam);
	DDX_Text(pDX, IDC_EDIT_PASSWD, m_csmopasswd);
	DDX_Text(pDX, IDC_EDIT_SRVADD2, m_cssqlsrvadd);
	DDX_Text(pDX, IDC_EDIT_DBNAME2, m_cssqldbnam);
	DDX_Text(pDX, IDC_EDIT_USRNAM2, m_cssqlusrnam);
	DDX_Text(pDX, IDC_EDIT_PASSWD2, m_cssqlpasswd);
	DDX_Text(pDX, IDC_EDIT_SRVADD3, m_cssmtpadd);
	DDX_Text(pDX, IDC_EDIT_USRNAM3, m_cssmtpusrnam);
	DDX_Text(pDX, IDC_EDIT_PASSWD3, m_cssmtppasswd);
	DDX_Text(pDX, IDC_EDIT_TO, m_csto);
	DDX_Control(pDX, IDC_EDIT_SRVADD, m_editSrvadd);
	DDX_Control(pDX, IDC_EDIT_DBNAME, m_editDBname);
	DDX_Control(pDX, IDC_EDIT_TABNAME, m_editTab);
	DDX_Control(pDX, IDC_EDIT_USRNAM, m_editUsrnam);
	DDX_Control(pDX, IDC_EDIT_PASSWD, m_editPass);
}


BEGIN_MESSAGE_MAP(CSettingDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_USEDB, &CSettingDlg::OnBnClickedCheckUsedb)
END_MESSAGE_MAP()


// CSettingDlg 消息处理程序

void CSettingDlg::SetInfo(MongoDBInfo& moinf, SQLDBInfo& sqlinf, ForwardSet& fsinf)
{
	memcpy_s(&m_moinfo, sizeof(MongoDBInfo), &moinf, sizeof(MongoDBInfo));
	memcpy_s(&m_sqlinfo, sizeof(SQLDBInfo), &sqlinf, sizeof(SQLDBInfo));
	memcpy_s(&m_fsinfo, sizeof(ForwardSet), &fsinf, sizeof(ForwardSet));
	if (moinf.nUseDB == 1)
		m_bUseDB = TRUE;
	m_csmosrvadd=moinf.chDBAdd;
	m_csmodbname=moinf.chDBName;
	m_csmotabnam=moinf.chTable;
	m_csmousrnam= moinf.chUserName;
	m_csmopasswd= moinf.chPasswd;
	
	m_cssqlsrvadd=sqlinf.szDBAdd;
	m_cssqldbnam=sqlinf.szDBName;
	m_cssqlusrnam=sqlinf.szUserName;
	m_cssqlpasswd=sqlinf.szPasswd;

	m_cssmtpadd=fsinf.srvadd;
	m_cssmtpusrnam=fsinf.username;
	m_cssmtppasswd= fsinf.pass;
	m_csto=fsinf.to;
}


void CSettingDlg::OnOK()
{
	// TODO:  在此添加专用代码和/或调用基类
	UpdateData(TRUE);
	m_moinfo.nUseDB = m_bUseDB?1:0;
	char chTemp[512] = { 0 };
	WideCharToMultiByte(CP_ACP, 0, m_csmosrvadd, m_csmosrvadd.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_moinfo.chDBAdd, 32, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_csmodbname, m_csmodbname.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_moinfo.chDBName, 32, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_csmotabnam, m_csmotabnam.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_moinfo.chTable, 32, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_csmousrnam, m_csmousrnam.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_moinfo.chUserName, 32, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_csmopasswd, m_csmopasswd.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_moinfo.chPasswd, 32, "%s", chTemp);

	wsprintf(m_sqlinfo.szDBAdd, m_cssqlsrvadd);
	wsprintf(m_sqlinfo.szDBName,m_cssqldbnam);
	wsprintf(m_sqlinfo.szPasswd, m_cssqlpasswd);
	wsprintf(m_sqlinfo.szUserName, m_cssqlusrnam);

	
	WideCharToMultiByte(CP_ACP, 0, m_cssmtpadd, m_cssmtpadd.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_fsinfo.srvadd, 64, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_cssmtpusrnam, m_cssmtpusrnam.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_fsinfo.username, 64, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_cssmtppasswd, m_cssmtppasswd.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_fsinfo.pass, 128, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_csto, m_csto.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_fsinfo.to, 512, "%s", chTemp);
	CDialogEx::OnOK();
}


void CSettingDlg::OnBnClickedCheckUsedb()
{
	// TODO:  在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if (!m_bUseDB)
	{
		m_editDBname.EnableWindow(FALSE);
		m_editPass.EnableWindow(FALSE);
		m_editSrvadd.EnableWindow(FALSE);
		m_editTab.EnableWindow(FALSE);
		m_editUsrnam.EnableWindow(FALSE);
	}
	else
	{
		m_editDBname.EnableWindow(TRUE);
		m_editPass.EnableWindow(TRUE);
		m_editSrvadd.EnableWindow(TRUE);
		m_editTab.EnableWindow(TRUE);
		m_editUsrnam.EnableWindow(TRUE);
	}
}


BOOL CSettingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	if (!m_bUseDB)
	{
		m_editDBname.EnableWindow(FALSE);
		m_editPass.EnableWindow(FALSE);
		m_editSrvadd.EnableWindow(FALSE);
		m_editTab.EnableWindow(FALSE);
		m_editUsrnam.EnableWindow(FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}
