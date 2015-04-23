#pragma once

#include "Resource.h"
#include "public.h"
// CDialogInfo 对话框

class CDialogInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogInfo)

public:
	CDialogInfo(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogInfo();

// 对话框数据
	enum { IDD = IDD_DIALOG_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_csMailAdd;
	CString m_csName;
	CString m_csSrvAdd;
	CString m_csPasswd;
	long m_lPort;
	CString m_csAbb;
	BOOL m_bSend;
	CString m_csRecAdd;
	MailBoxInfo m_info;
private:
	
public:
	void SetMailBoxInfo(const CString&csName,const MailBoxInfo& info);
	virtual void OnOK();
	long m_lDay;
};
