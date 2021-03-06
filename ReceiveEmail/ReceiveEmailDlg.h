
// ReceiveEmailDlg.h : 头文件
//

#pragma once
#include "afxbutton.h"
#include "afxcmn.h"
#include "resource.h"

#include "afxwin.h"
#include "afxdtctl.h"
#include "public.h"
#include "../GGDataAPI/ggdataapi.h"
#include "afxeditbrowsectrl.h"
#include "POP3.h"
#include "../Log/Log.h"
#include "SendEmail.h"
#include "../MailAnalysis/MailAnalysis.h"
#include "../SQLServer/SQLServer.h"
#include "../MongoDB/MongoDB.h"

struct ShowInfo
{
	TCHAR szAbbreviation[64];
	TCHAR szName[128];
	long lCurr;
	long lTotal;
	long lStatus;
};


// CReceiveEmailDlg 对话框
class CReceiveEmailDlg : public CDialogEx
{
// 构造
public:
	CReceiveEmailDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CReceiveEmailDlg();

// 对话框数据
	enum { IDD = IDD_RECEIVEEMAIL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	map<CString, MailBoxInfo> m_mailList;//邮箱列表
	long m_nHighJobPriority;
	long m_nJobParamIntValue;
	long m_nCurJobIndex;
	HANDLE m_hMain;
	CRITICAL_SECTION _cs_;
	MongoDBInfo m_dbinfo;
	HANDLE m_hMainTest;
	ForwardSet m_fsinfo;
	HANDLE m_hMainTest2;
	CString m_csLogPath;
	CString m_csTestText;
	SQLDBInfo m_sqldbinfo;
	HANDLE m_hProcess[5];
	long m_lLastPos;
	map<DWORD, ShowInfo> m_showinfo;
	DWORD m_dwStartTime; 
	CRITICAL_SECTION _check_failed_;
	map<string, long> m_mapFailedUIDLs;
public:
	CMFCButton m_btnSet;//设置按钮
	CListCtrl m_listMailBox;//表格控件
	CStatic m_Group;
	CButton m_btnStop;
	CStatic m_Name1;
	CStatic m_Name2;
	CStatic m_Name3;
	CStatic m_Name4;
	CStatic m_Name5;
	CProgressCtrl m_progress1;
	CProgressCtrl m_progress2;
	CProgressCtrl m_progress3;
	CProgressCtrl m_progress4;
	CProgressCtrl m_progress5;
	CButton m_btnTest;
	CButton m_btnSetting;
	CLog m_log;
	CButton m_btnTest2;
	CMFCEditBrowseCtrl m_editpath;
	CString m_csRunTime;
	CStatic m_time;
	COleDateTime m_startdate;
public:
	afx_msg void OnBnClickedMfcbuttonSet();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	BOOL InitMailList();//初始化邮箱表格
	BOOL LoadFromConfig();//加载配置文件
	virtual void OnCancel();
	afx_msg void OnDestroy();
	BOOL GetMailBoxInfo(CString&csUserName,MailBoxInfo& info,long lStatus);
	void GetDataBaseInfo(MongoDBInfo& dbinfo,SQLDBInfo& sqlinfo);
	
	afx_msg void OnBnClickedButtonStop();
	void LayoutDialog(long cx, long cy);
	static DWORD WINAPI _AfxMain(LPVOID lpParam);
	void Stop(long lType=0);
	void SetShowInfo(long lTextWnd,LPCTSTR =NULL,long lProgress=0);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void InitTextWnd();
	void WriteToConfig();
	afx_msg void OnBnClickedButton1();
	static DWORD WINAPI _AfxMainTestAna(LPVOID lpParam);
	static DWORD WINAPI _AfxMainTestSend(LPVOID lpParam);
	
	afx_msg void OnBnClickedButtonSet();
	void GetForwardInfo(ForwardSet& fdsinfo);
	afx_msg void OnBnClickedButton2();
	void StopTest();
	void StopTest2();
	void StopMain();
	static DWORD WINAPI _AfxMainProcess(LPVOID lpParam);
	long MailAnalysis(POP3& pop3, CSQLServer& sql, SMTP& smtp,const string& strUIDL, const MailBoxInfo& info,const char* pLogPath ,long lLen,long lType=0);
	afx_msg void OnLvnItemchangedListMailbox(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkListMailbox(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickListMailbox(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnAdditem();
	afx_msg void OnDelitem();
	long SendEmail(SMTP& smtp);
	CButton m_btnPDF;
	afx_msg void OnBnClickedButtonPdf();
	void StopEx();
	BOOL CheckFailedUIDL(const string& strUIDL);//检查UIDL是否需要跳过，返回FALSE不从MongoUIDL数据中删除
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
