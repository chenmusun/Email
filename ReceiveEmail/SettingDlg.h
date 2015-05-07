#pragma once
#include "Resource.h"
#include "public.h"
#include "afxwin.h"

// CSettingDlg 对话框

class CSettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSettingDlg)

public:
	CSettingDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSettingDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_SET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bUseDB;
	CString m_csmosrvadd;
	CString m_csmodbname;
	CString m_csmotabnam;
	CString m_csmousrnam;
	CString m_csmopasswd;
	CString m_cssqlsrvadd;
	CString m_cssqldbnam;
	CString m_cssqlusrnam;
	CString m_cssqlpasswd;
	CString m_cssmtpadd;
	CString m_cssmtpusrnam;
	CString m_cssmtppasswd;
	CString m_csto;
private:
	MongoDBInfo* m_pMoInfo;
	SQLDBInfo* m_pSqlInfo;
	ForwardSet* m_pFoInfo;
public:
	void SetInfo(MongoDBInfo* moinf, SQLDBInfo* sqlinf, ForwardSet* fsinf);
	virtual void OnOK();
	afx_msg void OnBnClickedCheckUsedb();
	CEdit m_editSrvadd;
	CEdit m_editDBname;
	CEdit m_editTab;
	CEdit m_editUsrnam;
	CEdit m_editPass;
	virtual BOOL OnInitDialog();
};
