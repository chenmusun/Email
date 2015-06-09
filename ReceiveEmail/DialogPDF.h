#pragma once
#include "afxbutton.h"
#include "afxeditbrowsectrl.h"
#include "afxwin.h"


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
	vector<CString> m_vecType;
public:
	afx_msg void OnBnClickedMfcbuttonGet();
	void SetMongoDBInfo(const MongoDBInfo& info);
	virtual BOOL OnInitDialog();
	CMFCEditBrowseCtrl m_Path;
	CString m_csFilePath;
	afx_msg void OnBnClickedMfcbuttonPdf2txt();
	afx_msg void OnBnClickedMfcbuttonOpenfo();
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnBnClickedButtonPer();
	afx_msg void OnBnClickedButtonWm();
	CButton m_btnTest;
	afx_msg void OnBnClickedButtonConvert();
};
