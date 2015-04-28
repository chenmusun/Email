#pragma once
#include "afxbutton.h"
#include "POP3.h"
#include "afxeditbrowsectrl.h"


// CDialogPDF �Ի���

class CDialogPDF : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogPDF)

public:
	CDialogPDF(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDialogPDF();

// �Ի�������
	enum { IDD = IDD_DIALOG_PDF };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP() 
private:
	MongoDBInfo m_modbinfo;
public:
	CString m_csGUID;
	CString m_csSavePath;
	CMFCButton m_btnGet;
public:
	afx_msg void OnBnClickedMfcbuttonGet();
	void SetMongoDBInfo(const MongoDBInfo& info);
	virtual BOOL OnInitDialog();
	CMFCEditBrowseCtrl m_Path;
};