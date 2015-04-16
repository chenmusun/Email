// DialogInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "ReceiveEmail.h"
#include "DialogInfo.h"
#include "afxdialogex.h"


// CDialogInfo 对话框

IMPLEMENT_DYNAMIC(CDialogInfo, CDialogEx)

CDialogInfo::CDialogInfo(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogInfo::IDD, pParent)
	, m_csMailAdd(_T(""))
	, m_csName(_T(""))
	, m_csSrvAdd(_T(""))
	, m_csPasswd(_T(""))
	, m_lPort(0)
	, m_csAbb(_T(""))
	, m_bSend(FALSE)
	, m_csRecAdd(_T(""))
{
	memset(&m_info, 0, sizeof(MailBoxInfo));
}

CDialogInfo::~CDialogInfo()
{
}

void CDialogInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ADD, m_csMailAdd);
	DDX_Text(pDX, IDC_EDIT_NAME, m_csName);
	DDX_Text(pDX, IDC_EDIT_SRVADD, m_csSrvAdd);
	DDX_Text(pDX, IDC_EDIT_PSD, m_csPasswd);
	DDX_Text(pDX, IDC_EDIT_PORT, m_lPort);
	DDX_Text(pDX, IDC_EDIT_ABB, m_csAbb);
	DDX_Check(pDX, IDC_CHECK_SEND, m_bSend);
	DDX_Text(pDX, IDC_EDIT_REC, m_csRecAdd);
}


BEGIN_MESSAGE_MAP(CDialogInfo, CDialogEx)
END_MESSAGE_MAP()


// CDialogInfo 消息处理程序

void CDialogInfo::SetMailBoxInfo(const CString&csName,const MailBoxInfo& info)
{
	memcpy_s(&m_info, sizeof(MailBoxInfo), &info, sizeof(MailBoxInfo));
	m_csMailAdd.Format(_T("%s"), csName);
	m_csName.Format(_T("%s"), m_info.szName);
	m_csSrvAdd.Format(_T("%s"), m_info.szServerAdd);
	m_csPasswd.Format(_T("%s"), m_info.szPasswd);
	m_csAbb.Format(_T("%s"), m_info.szAbbreviation);
	m_csRecAdd.Format(_T("%s"),m_info.szMailAdd);
	m_lPort = m_info.lPort;
	m_bSend = m_info.bSendMail;
}


void CDialogInfo::OnOK()
{
	// TODO:  在此添加专用代码和/或调用基类
	UpdateData(TRUE);
	do 
	{
		if (m_csMailAdd.IsEmpty())
			break;
		if (m_csName.IsEmpty())
			break;
		wsprintf(m_info.szName, _T("%s"),m_csName.Left(63));
		if (m_csAbb.IsEmpty())
			break;
		wsprintf(m_info.szAbbreviation, _T("%s"), m_csAbb.Left(63));
		if (m_csSrvAdd.IsEmpty())
			break;
		wsprintf(m_info.szServerAdd, _T("%s"), m_csSrvAdd.Left(63));
		if (m_csPasswd.IsEmpty())
			break;
		wsprintf(m_info.szPasswd, _T("%s"), m_csPasswd.Left(127));
		if (m_lPort<=0)
			break;
		m_info.lPort = m_lPort;
		m_info.bSendMail = m_bSend;
		if (m_bSend)
		{
			if (m_csRecAdd.IsEmpty())
				break;
			wsprintf(m_info.szMailAdd, _T("%s"), m_csRecAdd.Left(63));
		}
		CDialogEx::OnOK();
	} while (0);
}
