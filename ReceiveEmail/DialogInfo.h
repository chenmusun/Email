#pragma once

#include "Resource.h"
#include "public.h"
// CDialogInfo �Ի���

class CDialogInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogInfo)

public:
	CDialogInfo(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDialogInfo();

// �Ի�������
	enum { IDD = IDD_DIALOG_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

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
private:
	MailBoxInfo* m_pInfo;
	CString* m_pName;
public:
	void SetMailBoxInfo(CString*pcsName,MailBoxInfo* pinfo);
	virtual void OnOK();
	long m_lDay;
};
