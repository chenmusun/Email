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
	, m_lDay(0)
	, m_pInfo(NULL)
	, m_pName(NULL)
{
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
	DDX_Text(pDX, IDC_EDIT_DAY, m_lDay);
}


BEGIN_MESSAGE_MAP(CDialogInfo, CDialogEx)
END_MESSAGE_MAP()


// CDialogInfo 消息处理程序

void CDialogInfo::SetMailBoxInfo(CString*pcsName, MailBoxInfo* pinfo)
{
	m_pInfo = pinfo;
	m_csMailAdd.Format(_T("%s"), *pcsName);
	m_csName.Format(_T("%s"), pinfo->szName);
	m_csSrvAdd.Format(_T("%s"), pinfo->szServerAdd);
	m_csPasswd.Format(_T("%s"), pinfo->szPasswd);
	m_csAbb.Format(_T("%s"), pinfo->szAbbreviation);
	m_csRecAdd.Format(_T("%s"), pinfo->szMailAdd);
	m_lPort = pinfo->lPort;
	m_bSend = pinfo->bSendMail;
	m_lDay = pinfo->lSaveDay;
	m_pName = pcsName;
}


void CDialogInfo::OnOK()
{
	// TODO:  在此添加专用代码和/或调用基类
	UpdateData(TRUE);
	//UpdateData();
	CString csText;
	long lErr(0);
	do 
	{
		if (m_csMailAdd.IsEmpty() || m_csMailAdd.GetLength()>64)
		{
			lErr = 1;
			break;
		}
		if (m_csMailAdd.Find(_T("@")) < 0)
		{
			lErr = 2;
			break;
		}
		if (m_csName.IsEmpty() || m_csName.GetLength()>64)
		{
			lErr = 3;
			break;
		}
		if (m_csAbb.IsEmpty() || m_csAbb.GetLength()>64)
		{
			lErr = 4;
			break;
		}
		if (m_csSrvAdd.IsEmpty() || m_csSrvAdd.GetLength()>64)
		{
			lErr = 5; 
			break;
		}
		if (m_csPasswd.IsEmpty()||m_csPasswd.GetLength()>128)
		{
			lErr = 6; 
			break;
		}
		if (m_lPort<=0)
		{
			lErr = 7; 
			break;
		}
		if (m_bSend)
		{
			if (m_csRecAdd.IsEmpty() || m_csRecAdd.GetLength()>128)
			{
				lErr = 8; 
				break;
			}
		}
	} while (0);
	switch (lErr)
	{
	case 1:
		csText.Format(_T("邮箱地址为空！"));
		break;
	case 2:
		csText.Format(_T("邮箱地址非法！"));
		break;
	case 3:
		csText.Format(_T("邮箱名称为空！"));
		break;
	case 4:
		csText.Format(_T("邮箱简称为空！"));
		break;
	case 5:
		csText.Format(_T("POP3服务器地址为空！"));
		break;
	case 6:
		csText.Format(_T("密码为空！"));
		break;
	case 7:
		csText.Format(_T("端口为空！"));
		break;
	case 8:
		csText.Format(_T("接收邮箱地址为空！"));
		break;
	case 0:
	default:
	{
		memset(m_pInfo, 0, sizeof(MailBoxInfo));
		m_pInfo->bSendMail = m_bSend;
		m_pInfo->lPort = m_lPort;
		m_pInfo->lSaveDay = m_lDay;
		wsprintf(m_pInfo->szMailAdd, _T("%s"), m_csRecAdd);
		wsprintf(m_pInfo->szName, _T("%s"), m_csName);
		wsprintf(m_pInfo->szAbbreviation, _T("%s"), m_csAbb);
		wsprintf(m_pInfo->szPasswd, _T("%s"), m_csPasswd);
		wsprintf(m_pInfo->szServerAdd, _T("%s"), m_csSrvAdd);
		*m_pName = m_csMailAdd;
		CDialogEx::OnOK();
	}
		break;
	}
	if (csText.GetLength() > 0)
		AfxMessageBox(csText);
}
