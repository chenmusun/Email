#pragma once
#include "afxbutton.h"
#include "afxeditbrowsectrl.h"


// CDialogPDF 对话框

class CDialogPDF : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogPDF)

public:
	CDialogPDF(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogPDF();

// 对话框数据
	enum { IDD = IDD_DIALOG_PDF };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

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
	CString m_csFilePath;
	afx_msg void OnBnClickedMfcbuttonPdf2txt();
};
