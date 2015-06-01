// SettingDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ReceiveEmail.h"
#include "SettingDlg.h"
#include "afxdialogex.h"
#include "../MongoDB/MongoDB.h"
#include "../SQLServer/SQLServer.h"


// CSettingDlg �Ի���

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
	, m_pMoInfo(NULL)
	, m_pSqlInfo(NULL)
	,m_pFoInfo(NULL)
	, m_bUseDB2(FALSE)
{
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
	DDX_Check(pDX, IDC_CHECK_USEDB2, m_bUseDB2);
	DDX_Control(pDX, IDC_EDIT_SRVADD2, m_editSQLAdd);
	DDX_Control(pDX, IDC_EDIT_DBNAME2, m_editSQLDBName);
	DDX_Control(pDX, IDC_EDIT_USRNAM2, m_editSQLUsername);
	DDX_Control(pDX, IDC_EDIT_PASSWD2, m_editSQLPass);
}


BEGIN_MESSAGE_MAP(CSettingDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHECK_USEDB, &CSettingDlg::OnBnClickedCheckUsedb)
	ON_BN_CLICKED(IDC_MFCBUTTON_MOTEST, &CSettingDlg::OnBnClickedMfcbuttonMotest)
	ON_BN_CLICKED(IDC_MFCBUTTON_SQLTEST, &CSettingDlg::OnBnClickedMfcbuttonSqltest)
	ON_BN_CLICKED(IDC_CHECK_USEDB2, &CSettingDlg::OnBnClickedCheckUsedb2)
END_MESSAGE_MAP()


// CSettingDlg ��Ϣ�������

void CSettingDlg::SetInfo(MongoDBInfo* moinf, SQLDBInfo* sqlinf, ForwardSet* fsinf)
{
	m_pFoInfo = fsinf;
	m_pMoInfo = moinf;
	m_pSqlInfo = sqlinf;
	if (moinf->nUseDB == 1)
		m_bUseDB = TRUE;
	m_csmosrvadd = moinf->chDBAdd;
	m_csmodbname = moinf->chDBName;
	m_csmotabnam = moinf->chTable;
	m_csmousrnam = moinf->chUserName;
	m_csmopasswd = moinf->chPasswd;
	
	if (sqlinf->lUseDB == 1)
		m_bUseDB2 = TRUE;
	m_cssqlsrvadd = sqlinf->szDBAdd;
	m_cssqldbnam = sqlinf->szDBName;
	m_cssqlusrnam = sqlinf->szUserName;
	m_cssqlpasswd = sqlinf->szPasswd;

	m_cssmtpadd = fsinf->srvadd;
	m_cssmtpusrnam = fsinf->username;
	m_cssmtppasswd = fsinf->pass;
	m_csto = fsinf->to;
}


void CSettingDlg::OnOK()
{
	// TODO:  �ڴ����ר�ô����/����û���
	UpdateData(TRUE);
	m_pMoInfo->nUseDB = m_bUseDB ? 1 : 0;
	char chTemp[512] = { 0 };
	WideCharToMultiByte(CP_ACP, 0, m_csmosrvadd, m_csmosrvadd.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_pMoInfo->chDBAdd, 32, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_csmodbname, m_csmodbname.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_pMoInfo->chDBName, 32, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_csmotabnam, m_csmotabnam.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_pMoInfo->chTable, 32, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_csmousrnam, m_csmousrnam.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_pMoInfo->chUserName, 32, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_csmopasswd, m_csmopasswd.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_pMoInfo->chPasswd, 32, "%s", chTemp);

	m_pSqlInfo->lUseDB = m_bUseDB2 ? 1 : 0;
	wsprintf(m_pSqlInfo->szDBAdd, m_cssqlsrvadd);
	wsprintf(m_pSqlInfo->szDBName, m_cssqldbnam);
	wsprintf(m_pSqlInfo->szPasswd, m_cssqlpasswd);
	wsprintf(m_pSqlInfo->szUserName, m_cssqlusrnam);

	
	WideCharToMultiByte(CP_ACP, 0, m_cssmtpadd, m_cssmtpadd.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_pFoInfo->srvadd, 64, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_cssmtpusrnam, m_cssmtpusrnam.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_pFoInfo->username, 64, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_cssmtppasswd, m_cssmtppasswd.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_pFoInfo->pass, 128, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_csto, m_csto.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(m_pFoInfo->to, 512, "%s", chTemp);
	CDialogEx::OnOK();
}


void CSettingDlg::OnBnClickedCheckUsedb()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
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

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	if (!m_bUseDB)
	{
		m_editDBname.EnableWindow(FALSE);
		m_editPass.EnableWindow(FALSE);
		m_editSrvadd.EnableWindow(FALSE);
		m_editTab.EnableWindow(FALSE);
		m_editUsrnam.EnableWindow(FALSE);
	}

	if (!m_bUseDB2)
	{
		m_editSQLAdd.EnableWindow(FALSE);
		m_editSQLDBName.EnableWindow(FALSE);
		m_editSQLUsername.EnableWindow(FALSE);
		m_editSQLPass.EnableWindow(FALSE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣:  OCX ����ҳӦ���� FALSE
}


void CSettingDlg::OnBnClickedMfcbuttonMotest()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	MongoDBInfo moinfo;
	memset(&moinfo, 0, sizeof(MongoDBInfo));
	moinfo.nUseDB = m_bUseDB ? 1 : 0;
	char chTemp[512] = { 0 };
	WideCharToMultiByte(CP_ACP, 0, m_csmosrvadd, m_csmosrvadd.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(moinfo.chDBAdd, 32, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_csmodbname, m_csmodbname.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(moinfo.chDBName, 32, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_csmotabnam, m_csmotabnam.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(moinfo.chTable, 32, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_csmousrnam, m_csmousrnam.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(moinfo.chUserName, 32, "%s", chTemp);
	memset(&chTemp, 0, 512);
	WideCharToMultiByte(CP_ACP, 0, m_csmopasswd, m_csmopasswd.GetLength(), chTemp, 512, NULL, NULL);
	sprintf_s(moinfo.chPasswd, 32, "%s", chTemp);
	if (moinfo.nUseDB == 1)
	{
		CMongoDB db;
		CString csDebug;
		string strErr;
		db.SetDBInfo(moinfo);
		if (db.ConnectDataBase(strErr))
		{
			db.DisConnectDataBase();
			char chDebug[512] = { 0 };
			sprintf_s(chDebug, 512, "Connect [%s]-[%s] Success!", moinfo.chDBAdd, moinfo.chDBName);
			csDebug = chDebug;
		}
		else
		{
			csDebug.Format(_T("Conncet failed! "));
			csDebug += strErr.c_str();
		}
		AfxMessageBox(csDebug);
	}
}


void CSettingDlg::OnBnClickedMfcbuttonSqltest()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	SQLDBInfo sqlinfo;
	memset(&sqlinfo, 0, sizeof(SQLDBInfo));
	sqlinfo.lUseDB = m_bUseDB2 ? 1 : 0;
	wsprintf(sqlinfo.szDBAdd, m_cssqlsrvadd);
	wsprintf(sqlinfo.szDBName, m_cssqldbnam);
	wsprintf(sqlinfo.szPasswd, m_cssqlpasswd);
	wsprintf(sqlinfo.szUserName, m_cssqlusrnam);
	if (sqlinfo.lUseDB ==1)
	{
		CSQLServer sql;
		CString csDebug;
		if (sql.Connect(sqlinfo))
		{
			sql.CloseDB();
			csDebug.Format(_T("Connect [%s]-[%s] Success!"), sqlinfo.szDBAdd, sqlinfo.szDBName);
		}
		else
		{
			csDebug.Format(_T("Conncet failed! "));
		}
		AfxMessageBox(csDebug);
	}
}


void CSettingDlg::OnBnClickedCheckUsedb2()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	UpdateData(TRUE);
	if (!m_bUseDB2)
	{
		m_editSQLAdd.EnableWindow(FALSE);
		m_editSQLDBName.EnableWindow(FALSE);
		m_editSQLUsername.EnableWindow(FALSE);
		m_editSQLPass.EnableWindow(FALSE);
	}
	else
	{
		m_editSQLAdd.EnableWindow(TRUE);
		m_editSQLDBName.EnableWindow(TRUE);
		m_editSQLUsername.EnableWindow(TRUE);
		m_editSQLPass.EnableWindow(TRUE);
	}
}
