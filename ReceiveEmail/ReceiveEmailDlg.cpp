
// ReceiveEmailDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ReceiveEmail.h"
#include "ReceiveEmailDlg.h"
#include "afxdialogex.h"
#include <thread>
#include <threadpoolapiset.h>
#include <threadpoollegacyapiset.h>
#include "GGJsonAdapter.h"
#include "SettingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MIN_LENGTH 600
#define MIN_HEIGHT 400
#define MAILLIST_ITE map<CString, MailBoxInfo>::iterator
#define BTN_LENGTH 90
#define BTN_HEIGHT 25
#define GAP 20
#define HORIZON_GAP 15

TCHAR __Main_Path__[MAX_PATH];
HANDLE __HEVENT_EXIT__ = NULL;
HANDLE __HEVENT_TEST_EXIT__ = NULL;
HANDLE __HEVENT_MAIN_EXIT__ = NULL;


DWORD WINAPI  CReceiveEmailDlg::_AfxMain(LPVOID lpParam)
{
	CReceiveEmailDlg*pDlg = (CReceiveEmailDlg*)lpParam;
	try
	{
		TCHAR szInfo[128] = { 0 };
//#ifdef _DEBUG
		CString csDebug;
//#endif
		map<DWORD, ShowInfo>::iterator ite = pDlg->m_showinfo.begin();
		long lCurrPos = 0;
		double dValue(0);
		while (true)
		{
			memset(&szInfo, 0, 128);
			if (WaitForSingleObject(__HEVENT_EXIT__, 100L) == WAIT_OBJECT_0)
			{
				OutputDebugStringA("WAIT2 __HEVENT_EXIT__\r\n");
				break;
			}
			lCurrPos = 0;
			ite = pDlg->m_showinfo.begin();
			while (ite!=pDlg->m_showinfo.end())
			{
				if (WaitForSingleObject(__HEVENT_EXIT__, 0L) == WAIT_OBJECT_0)
				{
					OutputDebugStringA("WAIT1 __HEVENT_EXIT__\r\n");
					break;
				}
				dValue = (double)ite->second.lCurr / ite->second.lTotal;
				switch (ite->second.lStatus)
				{
				case 0:
					swprintf_s(szInfo, 128, _T("%s\t[%d/%d]"), ite->second.szName, ite->second.lCurr, ite->second.lTotal);
					break;
				case 1:
					swprintf_s(szInfo, 128, _T("清理----%s\t[%d/%d]"), ite->second.szName, ite->second.lCurr, ite->second.lTotal);
					break;
				default:
					break;
				}
				pDlg->SetShowInfo(lCurrPos, szInfo, (long)(dValue * 100));
				ite++;
				lCurrPos++;
			}
			pDlg->PostMessage(__umymessage__fres_hprogress__, NULL, NULL);
		}//End of while
	}
	catch (...)
	{
	}
	return 0;
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CReceiveEmailDlg 对话框



CReceiveEmailDlg::CReceiveEmailDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CReceiveEmailDlg::IDD, pParent), m_lLastPos(0)
, m_nHighJobPriority(0), m_nJobParamIntValue(0), m_nCurJobIndex(0)
, m_hMain(NULL), m_hMainTest(NULL), m_hMainTest2(NULL)
, m_csRunTime(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_mailList.clear();
	memset(m_TextWnd, 0, sizeof(m_TextWnd));
	::InitializeCriticalSection(&_cs_);
	memset(&m_dbinfo, 0, sizeof(MongoDBInfo));
	memset(&m_fsinfo, 0, sizeof(ForwardSet));
	memset(&m_sqldbinfo, 0, sizeof(SQLDBInfo));
	for (int i = 0; i < sizeof(m_hProcess)/sizeof(m_hProcess[0]);i++)
	{
		m_hProcess[i] = NULL;
	}
	m_showinfo.clear();
}

CReceiveEmailDlg::~CReceiveEmailDlg()
{
	DeleteCriticalSection(&_cs_);
	if (__HEVENT_EXIT__)
	{
		CloseHandle(__HEVENT_EXIT__);
		__HEVENT_EXIT__ = NULL;
	}
	if (__HEVENT_TEST_EXIT__)
	{
		CloseHandle(__HEVENT_TEST_EXIT__);
		__HEVENT_TEST_EXIT__ = NULL;
	}
	if (__HEVENT_MAIN_EXIT__)
	{
		CloseHandle(__HEVENT_MAIN_EXIT__);
		__HEVENT_MAIN_EXIT__ = NULL;
	}
	for (int i = 0; i < sizeof(m_hProcess) / sizeof(m_hProcess[0]); i++)
	{
		m_hProcess[i] = NULL;
	}
	m_showinfo.clear();
}

void CReceiveEmailDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCBUTTON_SET, m_btnSet);
	DDX_Control(pDX, IDC_LIST_MAILBOX, m_listMailBox);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_btnStop);
	DDX_Control(pDX, IDC_STATIC_GROUP, m_Group);
	DDX_Control(pDX, IDC_STATIC_NAME1, m_Name1);
	DDX_Control(pDX, IDC_STATIC_NAME2, m_Name2);
	DDX_Control(pDX, IDC_STATIC_NAME3, m_Name3);
	DDX_Control(pDX, IDC_STATIC_NAME4, m_Name4);
	DDX_Control(pDX, IDC_STATIC_NAME5, m_Name5);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress1);
	DDX_Control(pDX, IDC_PROGRESS2, m_progress2);
	DDX_Control(pDX, IDC_PROGRESS3, m_progress3);
	DDX_Control(pDX, IDC_PROGRESS4, m_progress4);
	DDX_Control(pDX, IDC_PROGRESS5, m_progress5);
	DDX_Control(pDX, IDC_STATIC_DBNAME, m_dbname);
	DDX_Control(pDX, IDC_STATIC_DBNAM, m_dbnam);
	DDX_Control(pDX, IDC_STATIC_DBADD, m_dbadd);
	DDX_Control(pDX, IDC_STATIC_TAB, m_tab);
	DDX_Control(pDX, IDC_STATIC_TABLENAM, m_tablename);
	DDX_Control(pDX, IDC_CHECK_DB, m_checkdb);
	DDX_Control(pDX, IDC_BUTTON1, m_btnTest);
	DDX_Control(pDX, IDC_BUTTON_SET, m_btnSetting);
	DDX_Control(pDX, IDC_BUTTON2, m_btnTest2);
	DDX_Control(pDX, IDC_MFCEDITBROWSE1, m_editpath);
	DDX_Control(pDX, IDC_STATIC_TIME, m_time);
}

BEGIN_MESSAGE_MAP(CReceiveEmailDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_MFCBUTTON_SET, &CReceiveEmailDlg::OnBnClickedMfcbuttonSet)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CReceiveEmailDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_CHECK_DB, &CReceiveEmailDlg::OnBnClickedCheckDb)
	ON_BN_CLICKED(IDC_BUTTON1, &CReceiveEmailDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON_SET, &CReceiveEmailDlg::OnBnClickedButtonSet)
	ON_BN_CLICKED(IDC_BUTTON2, &CReceiveEmailDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CReceiveEmailDlg 消息处理程序

BOOL CReceiveEmailDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	GetCurrentDirectory(MAX_PATH, __Main_Path__);
	CRect rt;
	m_listMailBox.GetWindowRect(&rt);
	m_listMailBox.SetExtendedStyle(m_listMailBox.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);
	m_listMailBox.InsertColumn(0, _T("序号"), LVCFMT_CENTER,50);
	m_listMailBox.InsertColumn(1, _T("邮箱名称"), LVCFMT_LEFT, rt.right - 50);


	if (LoadFromConfig())
	{
		InitMailList();
		TCHAR szTemp[64] = { 0 };
		MultiByteToWideChar(CP_ACP, 0, m_dbinfo.chDBName, 32, szTemp, 64);
		m_dbname.SetWindowText(szTemp);
		MultiByteToWideChar(CP_ACP, 0, m_dbinfo.chDBAdd, 32, szTemp, 64);
		m_dbadd.SetWindowText(szTemp);
		MultiByteToWideChar(CP_ACP, 0, m_dbinfo.chTable, 32, szTemp, 64);
		m_tablename.SetWindowText(szTemp);
	}
	else
	{
		if (m_listMailBox.m_hWnd)
		{
			m_listMailBox.InsertItem(0, NULL);
			m_listMailBox.SetItemText(0, 1, _T("Load Error!"));
		}
	}
	m_btnStop.EnableWindow(FALSE);

	::GetClientRect(m_hWnd, &rt);
	LayoutDialog(rt.right, rt.bottom);
	TCHAR szPath[MAX_PATH] = { 0 };
	wsprintf(szPath, _T("%s\\Mail\\"), __Main_Path__);
	if ((GetFileAttributes(szPath) == 0xFFFFFFFF))
		CreateDirectory(szPath, NULL);
	wsprintf(szPath, _T("%s\\Log\\"), __Main_Path__);
	if ((GetFileAttributes(szPath) == 0xFFFFFFFF))
		CreateDirectory(szPath, NULL);
	wsprintf(szPath, _T("%s\\Email\\"), __Main_Path__);
	if ((GetFileAttributes(szPath) == 0xFFFFFFFF))
		CreateDirectory(szPath, NULL);
	InitTextWnd();
	for (int n = 0; n < 5; n++)
		m_TextWnd[n] = -1;
	m_checkdb.SetCheck(m_dbinfo.nUseDB);
	if (m_dbinfo.nUseDB != 1)
	{
		m_dbadd.EnableWindow(FALSE);
		m_dbname.EnableWindow(FALSE);
		m_tablename.EnableWindow(FALSE);
	}
	wsprintf(szPath, _T("%s\\Log\\main.txt"),__Main_Path__);
	m_csLogPath.Format(_T("%s"),szPath);
	m_log.SetPath(m_csLogPath,m_csLogPath.GetLength());
#ifdef _DEBUG
	if (m_csTestText.IsEmpty())
		m_csTestText.Format(_T("MD50000007766MSG1347790304363772117458219"));
#endif
	m_startdate = COleDateTime::GetCurrentTime();
	m_csRunTime.Format(_T("%d年%d月%d日 %d:%d:%d")
		, m_startdate.GetYear(), m_startdate.GetMonth()
		, m_startdate.GetDay(), m_startdate.GetHour(), m_startdate.GetMinute(), m_startdate.GetSecond());
	m_time.SetWindowText(m_csRunTime);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CReceiveEmailDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CReceiveEmailDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CReceiveEmailDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CReceiveEmailDlg::OnBnClickedMfcbuttonSet()
{
	// TODO:  在此添加控件通知处理程序代码
	StopMain();
	Stop();
	DWORD dwId[5] = { 0 },id;
	ShowInfo stShow;
	memset(&stShow, 0, sizeof(ShowInfo));
	if (m_listMailBox.GetItemCount() > 0)
	{
		if (__HEVENT_MAIN_EXIT__ == NULL)
			__HEVENT_MAIN_EXIT__ = CreateEvent(NULL, TRUE, FALSE, NULL);
		for (int i = 0; i < sizeof(m_hProcess) / sizeof(m_hProcess[0]); i++)
		{
			if (m_hProcess[i] == NULL)
			{
				m_hProcess[i] = CreateThread(NULL, 0, _AfxMainProcess, (LPVOID)this, 0, &dwId[i]);
				m_showinfo.insert(make_pair(dwId[i], stShow));
//#ifdef _DEBUG
				CString csDebug;
				csDebug.Format(_T("ThreadID = [%d]\r\n"),dwId[i]);
				OutputDebugString(csDebug);
//#endif
			}
		}
		
		if (__HEVENT_EXIT__ == NULL)
			__HEVENT_EXIT__ = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_hMain = CreateThread(NULL, 0, _AfxMain, (LPVOID)this, 0, &id);
		m_btnStop.EnableWindow(TRUE);
		m_btnSet.EnableWindow(FALSE);
	}
	else
		AfxMessageBox(_T("请检查配置文件！"));
}


void CReceiveEmailDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO:  在此处添加消息处理程序代码
	LayoutDialog(cx, cy);
}

BOOL CReceiveEmailDlg::InitMailList()
{
	unsigned long i(0);
	CString csNum;
	COLORREF crTextColor = ::GetSysColor(COLOR_WINDOWTEXT);
	if (m_mailList.size() > 0)
	{
		if (m_listMailBox.m_hWnd)
		{
			MAILLIST_ITE ite = m_mailList.begin();
			for (i = 0;ite!=m_mailList.end() && i < m_mailList.size();ite++, i++)
			{
				csNum.Format(_T("%d"), i+1);
				m_listMailBox.InsertItem(i, _T(""));
				m_listMailBox.SetItemText(i, 0, csNum);
				m_listMailBox.SetItemText(i, 1, ite->first);
				m_listMailBox.SetTextColor(crTextColor);
				ASSERT(m_listMailBox.GetTextColor() == crTextColor);
			}
		}
	}
	else return FALSE;
	return TRUE;
}

BOOL CReceiveEmailDlg::LoadFromConfig()
{
	char chTemp[512] = { 0 }, chConfigPath[MAX_PATH] = {0};
	TCHAR szConfigPath[MAX_PATH] = { 0 }, szName[64] = {0};
	wsprintf(szConfigPath, _T("%s\\config.ini"), __Main_Path__);
	MailBoxInfo info;
	memset(&info, 0, sizeof(info));
	CString csTemp, csName;
	long lTemp(0);

	long lCount = GetPrivateProfileInt(_T("E-mail"), _T("count"), 0, szConfigPath);
	for (long i = 0; i < lCount; i++)
	{
		memset(&info, 0, sizeof(info));
		csTemp.Format(_T("mail_%d_username"),i);
		GetPrivateProfileString(_T("E-mail"), csTemp, _T(""), szName, 64, szConfigPath);
		csName.Format(_T("%s"), szName);
		if (csName.IsEmpty())
			continue;
		csTemp.Format(_T("mail_%d_name"), i);
		GetPrivateProfileString(_T("E-mail"), csTemp, _T(""), info.szName, 64, szConfigPath);
		csTemp.Format(_T("mail_%d_add"), i);
		GetPrivateProfileString(_T("E-mail"), csTemp, _T(""), info.szServerAdd, 64, szConfigPath);
		csTemp.Format(_T("mail_%d_port"), i);
		info.lPort = GetPrivateProfileInt(_T("E-mail"), csTemp, 110, szConfigPath);
		csTemp.Format(_T("mail_%d_passwd"), i);
		GetPrivateProfileString(_T("E-mail"), csTemp, _T(""), info.szPasswd, 128, szConfigPath);
		csTemp.Format(_T("mail_%d_abbreviation"), i);
		GetPrivateProfileString(_T("E-mail"), csTemp, _T(""), info.szAbbreviation, 64, szConfigPath);
		csTemp.Format(_T("mail_%d_bsend"), i);
		lTemp = GetPrivateProfileInt(_T("E-mail"), csTemp,0,szConfigPath);
		if (lTemp == 1)
		{
			info.bSendMail = TRUE;
			csTemp.Format(_T("mail_%d_mailadd"), i);
			GetPrivateProfileString(_T("E-mail"), csTemp, _T(""), info.szMailAdd, 128, szConfigPath);
		}
		else info.bSendMail = FALSE;
		m_mailList.insert(make_pair(csName, info));
	}
	

	GetPrivateProfileString(_T("DataBase"), _T("usedb"), _T("yes"), szName, 64, szConfigPath);
	csTemp.Format(_T("%s"),szName);
	if (csTemp.CompareNoCase(_T("yes")) >= 0)
		m_dbinfo.nUseDB = 1;
	else m_dbinfo.nUseDB = 0;
	WideCharToMultiByte(CP_ACP, 0, szConfigPath, MAX_PATH, chConfigPath, MAX_PATH, NULL, NULL);
	GetPrivateProfileStringA("DataBase", "dbadd", "locahost:27017", chTemp, 512, chConfigPath);
	sprintf_s(m_dbinfo.chDBAdd, 32, "%s", chTemp);
	memset(chTemp, 0, 512);
	GetPrivateProfileStringA("DataBase", "data_base", "EmailUIDL", chTemp, 512, chConfigPath);
	sprintf_s(m_dbinfo.chDBName, 32, "%s", chTemp);
	memset(chTemp, 0, 512);
	GetPrivateProfileStringA("DataBase", "table", "uidl", chTemp, 512, chConfigPath);
	sprintf_s(m_dbinfo.chTable, 32, "%s", chTemp);
	memset(chTemp, 0, 512);
	GetPrivateProfileStringA("DataBase", "username", "zhangyu", chTemp, 512, chConfigPath);
	sprintf_s(m_dbinfo.chUserName, 32, "%s", chTemp);
	memset(chTemp, 0, 512);
	GetPrivateProfileStringA("DataBase", "passwd", "123456", chTemp, 512, chConfigPath);
	sprintf_s(m_dbinfo.chPasswd, 32, "%s", chTemp);
	memset(&m_sqldbinfo, 0, sizeof(SQLDBInfo));
	GetPrivateProfileString(_T("DataBase"), _T("sql_dbadd"), _T("OFFICE-PC\\SQLSERVER"), m_sqldbinfo.szDBAdd, 32, szConfigPath);
	GetPrivateProfileString(_T("DataBase"), _T("sql_db"), _T("ReportEmailDB"), m_sqldbinfo.szDBName, 32, szConfigPath);
	GetPrivateProfileString(_T("DataBase"), _T("sql_username"), _T("sa"), m_sqldbinfo.szUserName, 32, szConfigPath);
	GetPrivateProfileString(_T("DataBase"), _T("sql_passwd"), _T("test.123"), m_sqldbinfo.szPasswd, 32, szConfigPath);

	memset(&m_fsinfo, 0, sizeof(ForwardSet));
	GetPrivateProfileStringA("ForwardSet", "srvadd", "", m_fsinfo.srvadd, 64, chConfigPath);
	GetPrivateProfileStringA("ForwardSet", "username", "", m_fsinfo.username, 64, chConfigPath);
	GetPrivateProfileStringA("ForwardSet", "pass", "", m_fsinfo.pass, 128, chConfigPath);
	GetPrivateProfileStringA("ForwardSet", "to", "", m_fsinfo.to, 512, chConfigPath);
	GetPrivateProfileStringA("ForwardSet", "from", "", m_fsinfo.from, 128, chConfigPath);
	return TRUE;
}


void CReceiveEmailDlg::OnCancel()
{
	// TODO:  在此添加专用代码和/或调用基类
	Stop();
	StopTest();
	StopTest2();
	StopMain();
	WriteToConfig();
	CDialogEx::OnCancel();
}


void CReceiveEmailDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO:  在此处添加消息处理程序代码
	/*if (m_pTestTheradPool)
	{
		m_pTestTheradPool->StopAndWait(FTL_MAX_THREAD_DEADLINE_CHECK);
		delete m_pTestTheradPool;
		m_pTestTheradPool = NULL;
	}*/
}

BOOL CReceiveEmailDlg::GetMailBoxInfo(CString&csUserName, MailBoxInfo& info, long lStatus)
{
	::EnterCriticalSection(&_cs_);
//#ifdef _DEBUG
	CString csDebug;
//#endif
	BOOL bFound = FALSE;
	if (m_lLastPos > m_mailList.size()-1)
		m_lLastPos = 0;
	long lCurrPos(0),lCount(0);
	memset(&info, 0, sizeof(MailBoxInfo));
	map<CString, MailBoxInfo>::iterator ite = m_mailList.begin();
	switch (lStatus)
	{
	case 1:
	{
		while (TRUE)
		{
			if (lCount > 1)
				break;
			if (ite->second.lStatus == 0 && lCurrPos >= m_lLastPos)
			{
				csUserName = ite->first;
				wsprintf(info.szName, ite->second.szName);
				wsprintf(info.szPasswd, ite->second.szPasswd);
				wsprintf(info.szServerAdd, ite->second.szServerAdd);
				info.lPort = ite->second.lPort;
				wsprintf(info.szAbbreviation, ite->second.szAbbreviation);
				info.bSendMail = ite->second.bSendMail;
				wsprintf(info.szMailAdd, ite->second.szMailAdd);
				m_lLastPos++;
				ite->second.lStatus = 1;
				break;
			}
			ite++;
			if (ite == m_mailList.end())
			{
				ite = m_mailList.begin();
				lCount++;
			}
			lCurrPos++;
		}
	}
		break;
	case 0:
	{
		ite = m_mailList.find(csUserName);
		if (ite != m_mailList.end())
		{
			ite->second.lStatus = 0;
		}
		else
		{
			int a = 0;
		}
	}
		break;
	case 3:
	{
		while (ite!=m_mailList.end())
		{
			ite->second.lStatus = 0;
			ite++;
		}
	}
		break;
	default:
		break;
	}
	::LeaveCriticalSection(&_cs_);
	return TRUE;
}

void CReceiveEmailDlg::GetDataBaseInfo(MongoDBInfo& dbinfo, SQLDBInfo& sqlinfo)
{
	memset(&dbinfo, 0, sizeof(MongoDBInfo));
	memcpy_s(&dbinfo, sizeof(MongoDBInfo), &m_dbinfo, sizeof(MongoDBInfo));
	memcpy_s(&sqlinfo, sizeof(SQLDBInfo), &m_sqldbinfo, sizeof(SQLDBInfo));
}


void CReceiveEmailDlg::OnBnClickedButtonStop()
{
	// TODO:  在此添加控件通知处理程序代码
	Stop();
	StopMain();
	m_btnStop.EnableWindow(FALSE);
	m_btnSet.EnableWindow(TRUE);
}

void CReceiveEmailDlg::LayoutDialog(long cx, long cy)
{
	BOOL bMove = TRUE;
	if (cx < MIN_LENGTH || cy < MIN_HEIGHT)
		return;
	long lGap = 0, lTop = 40;;
	CRect rtTmp;
	if (m_listMailBox.m_hWnd)
	{
		SetRect(&rtTmp, 0, 0, 228, cy);
		m_listMailBox.MoveWindow(&rtTmp);
	}
	if (m_Group.m_hWnd)
	{
		SetRect(&rtTmp, 243, 5, cx-HORIZON_GAP, cy/3*2);
		m_Group.MoveWindow(&rtTmp);
		lGap = (rtTmp.Height() - 5 * BTN_HEIGHT*2) / 6;
	}
	if (m_checkdb.m_hWnd)
	{
		SetRect(&rtTmp, 243, cy / 3 * 2 + HORIZON_GAP, 243 + 120, cy / 3 * 2 + HORIZON_GAP + BTN_HEIGHT);
		m_checkdb.MoveWindow(&rtTmp);
	}
	if (m_dbadd.m_hWnd)
	{
		SetRect(&rtTmp, 373, cy / 3 * 2 + HORIZON_GAP, 373 + 150, cy / 3 * 2 + HORIZON_GAP + BTN_HEIGHT);
		m_dbadd.MoveWindow(&rtTmp);
	}
	if (m_dbnam.m_hWnd)
	{
		SetRect(&rtTmp, 523+GAP, cy / 3 * 2 + HORIZON_GAP, 523+GAP + 70, cy / 3 * 2 + HORIZON_GAP + BTN_HEIGHT);
		m_dbnam.MoveWindow(&rtTmp);
	}
	if (m_dbname.m_hWnd)
	{
		SetRect(&rtTmp, 593 + GAP, cy / 3 * 2 + HORIZON_GAP, 593 + GAP + 100, cy / 3 * 2 + HORIZON_GAP + BTN_HEIGHT);
		m_dbname.MoveWindow(&rtTmp);
	}
	if (m_tab.m_hWnd)
	{
		SetRect(&rtTmp, 693 + GAP*2, cy / 3 * 2 + HORIZON_GAP, 693 + GAP*2 + 40, cy / 3 * 2 + HORIZON_GAP + BTN_HEIGHT);
		m_tab.MoveWindow(&rtTmp);
	}
	if (m_tablename.m_hWnd)
	{
		SetRect(&rtTmp, 733 + GAP*2, cy / 3 * 2 + HORIZON_GAP, 733 + GAP*2 + 100, cy / 3 * 2 + HORIZON_GAP + BTN_HEIGHT);
		m_tablename.MoveWindow(&rtTmp);
	}
	if (m_Name1.m_hWnd)
	{
		SetRect(&rtTmp, 253, 10 + lGap, cx - 25, lGap + BTN_HEIGHT + 10);
		m_Name1.MoveWindow(&rtTmp);
	}
	if (m_progress1.m_hWnd)
	{
		SetRect(&rtTmp, 253, lGap + 10 + BTN_HEIGHT, cx - 25, BTN_HEIGHT * 2 + lGap + 10);
		m_progress1.MoveWindow(&rtTmp);
	}
	if (m_Name2.m_hWnd)
	{
		SetRect(&rtTmp, 253, lTop + BTN_HEIGHT * 2 + lGap, cx - 25, lTop + BTN_HEIGHT * 3 + lGap);
		m_Name2.MoveWindow(&rtTmp);
	}
	if (m_progress2.m_hWnd)
	{
		SetRect(&rtTmp, 253, lTop + BTN_HEIGHT * 3 + lGap, cx - 25, lTop + BTN_HEIGHT * 4 + lGap);
		m_progress2.MoveWindow(&rtTmp);
	}
	if (m_Name3.m_hWnd)
	{
		SetRect(&rtTmp, 253, lTop + BTN_HEIGHT * 4 + lGap * 2, cx - 25, lTop + BTN_HEIGHT * 5 + lGap * 2);
		m_Name3.MoveWindow(&rtTmp);
	}
	if (m_progress3.m_hWnd)
	{
		SetRect(&rtTmp, 253, lTop + BTN_HEIGHT * 5 + lGap * 2, cx - 25, lTop + BTN_HEIGHT * 6 + lGap * 2);
		m_progress3.MoveWindow(&rtTmp);
	}
	if (m_Name4.m_hWnd)
	{
		SetRect(&rtTmp, 253, lTop + BTN_HEIGHT * 6 + lGap * 3, cx - 25, lTop + BTN_HEIGHT * 7 + lGap * 3);
		m_Name4.MoveWindow(&rtTmp);
	}
	if (m_progress4.m_hWnd)
	{
		SetRect(&rtTmp, 253, lTop + BTN_HEIGHT * 7 + lGap * 3, cx - 25, lTop + BTN_HEIGHT * 8 + lGap * 3);
		m_progress4.MoveWindow(&rtTmp);
	}
	if (m_Name5.m_hWnd)
	{
		SetRect(&rtTmp, 253, lTop + BTN_HEIGHT * 8 + lGap * 4, cx - 25, lTop + BTN_HEIGHT * 9 + lGap * 4);
		m_Name5.MoveWindow(&rtTmp);
	}
	if (m_progress5.m_hWnd)
	{
		SetRect(&rtTmp, 253, lTop + BTN_HEIGHT * 9 + lGap * 4, cx - 25, lTop + BTN_HEIGHT * 10 + lGap * 4);
		m_progress5.MoveWindow(&rtTmp);
	}
	if (m_btnSetting.m_hWnd)
	{
		if (cx - BTN_LENGTH * 3 - GAP * 3 > 243 + BTN_LENGTH * 2+HORIZON_GAP)
		{
			SetRect(&rtTmp, cx - BTN_LENGTH * 3 - GAP * 3, cy - BTN_HEIGHT - GAP, cx - GAP * 3 - BTN_LENGTH * 2, cy - GAP);
			m_btnSetting.MoveWindow(&rtTmp);
		}
		else bMove = FALSE;
	}
	if (m_btnStop.m_hWnd)
	{
		if (bMove)
		{
			SetRect(&rtTmp, cx - BTN_LENGTH * 2 - GAP * 2, cy - BTN_HEIGHT - GAP, cx - GAP - BTN_LENGTH - GAP, cy - GAP);
			m_btnStop.MoveWindow(&rtTmp);
		}
	}
	if (m_btnSet.m_hWnd)
	{
		if (bMove)
		{
			SetRect(&rtTmp, cx - BTN_LENGTH - GAP, cy - BTN_HEIGHT - GAP, cx - GAP, cy - GAP);
			m_btnSet.MoveWindow(&rtTmp);
		}
	}
	if (m_editpath.m_hWnd)
	{
		SetRect(&rtTmp, 243, cy - BTN_HEIGHT*2 - GAP*2, 243 + BTN_LENGTH * 3 + HORIZON_GAP, cy - BTN_HEIGHT-GAP*2);
		m_editpath.MoveWindow(&rtTmp);
	}
	if (m_time.m_hWnd)
	{
		SetRect(&rtTmp,cx - BTN_LENGTH * 3 - GAP * 3, cy - BTN_HEIGHT*2 - GAP*2, cx - GAP, cy - GAP*2-BTN_HEIGHT);
		m_time.MoveWindow(&rtTmp);
	}
	if (m_btnTest.m_hWnd)
	{
		SetRect(&rtTmp, 243, cy - BTN_HEIGHT - GAP, 243 + BTN_LENGTH, cy - GAP);
		m_btnTest.MoveWindow(&rtTmp);
	}
	if (m_btnTest2.m_hWnd)
	{
		SetRect(&rtTmp, 243 + HORIZON_GAP + BTN_LENGTH*2, cy - BTN_HEIGHT - GAP, 243 + BTN_LENGTH*3 + HORIZON_GAP, cy - GAP);
		m_btnTest2.MoveWindow(&rtTmp);
	}
}

void CReceiveEmailDlg::Stop(long lType)//用于停止工作分配线程
{
//#ifdef _DEBUG
	OutputDebugStringA("STOP FRESH\r\n");
//#endif
	if (__HEVENT_EXIT__)
		SetEvent(__HEVENT_EXIT__);
	if (m_hMain)
	{
		if (WaitForSingleObject(m_hMain, 5000L) != WAIT_OBJECT_0)
		{
//#ifdef DEBUG
			OutputDebugString(_T("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXSTOPXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n"));
//#endif
			TerminateThread(m_hMain, 0);
		}
	}
	
	if (__HEVENT_EXIT__)
	{
		CloseHandle(__HEVENT_EXIT__);
		__HEVENT_EXIT__ = NULL;
	}
	for (int n = 0; n < 5; n++)
		m_TextWnd[n] = -1;
}

void CReceiveEmailDlg::SetShowInfo(long lTextWnd, LPCTSTR lpName, long lProgress)
{
	if (lpName)
		lProgress += 1;
	switch (lTextWnd)
	{
	case 0:
	{
		m_Name1.SetWindowText(lpName);
		m_progress1.SetPos(lProgress);
	}
		break;
	case 1:
	{
		m_Name2.SetWindowText(lpName);
		m_progress2.SetPos(lProgress);
	}
		break;
	case 2:
	{
		m_Name3.SetWindowText(lpName);
		m_progress3.SetPos(lProgress);
	}
		break;
	case 3:
	{
		m_Name4.SetWindowText(lpName);
		m_progress4.SetPos(lProgress);
	}
		break;
	case 4:
	{
		m_Name5.SetWindowText(lpName);
		m_progress5.SetPos(lProgress);
	}
		break;
	default:
		break;
	}
}


BOOL CReceiveEmailDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO:  在此添加专用代码和/或调用基类
	if (pMsg->message == __umymessage__fres_hprogress__)
	{
		COleDateTime date;
		date = COleDateTime::GetCurrentTime(); 
		date = date - m_startdate;
		CString csDate;
		csDate.Format(_T("%s| RT:%d天%d时%d分%d秒")
			, m_csRunTime
			,date.GetDay()-30,date.GetHour(),date.GetMinute(),date.GetSecond());
		m_time.SetWindowText(csDate);
	}
	if (pMsg->message == __umymessage__kill_hprogress__)
	{
		long lTextWnd = 0;
		lTextWnd = SetTextWnd(1, (long)pMsg->wParam, (long)pMsg->lParam);
		if (lTextWnd >= 0)
			SetShowInfo(lTextWnd);
	}
	if (pMsg->message == __umymessage_api_netcommand__)
	{
//#ifdef _DEBUG
	CStringA csApiParam;
//#endif
		GGJsonAdapter jsonAdapter;
		char *pData = (char*)pMsg->lParam;
		std::string strContent(pData);
		if (pData && pData[0] != '\0')
		{
			delete[] pData;
			pData = NULL;
		}
//#ifdef _DEBUG
		OutputDebugStringA(strContent.c_str());
		OutputDebugStringA("\r\n");
//#endif
		if (!jsonAdapter.Parse(strContent))//解析
		{
//#ifdef _DEBUG
			
			csApiParam.Append("------------服务器无返回！");
			OutputDebugStringA(csApiParam);
			OutputDebugString(_T("\r\n"));
//#endif
		}
		else
		{
			if (jsonAdapter.ReadValueLongFromNum(_T("code")))//返回值不为0，则表示不成功
			{
//#ifdef DEBUG
				csApiParam.Append("------------服务器返回Code非0！");
				OutputDebugStringA(csApiParam);
				OutputDebugString(_T("\r\n"));
//#endif
			}
			else
			{
//#ifdef _DEBUG
				csApiParam.Format("Success!\r\n");
				OutputDebugStringA(csApiParam);
//#endif
			}

		}
	}
	if (pMsg->message == __umymessage__sendcomplete__)
	{
#ifndef _DEBUG
		AfxMessageBox(_T("发送成功！"));
#endif
	}
	if (pMsg->message == __umymessage__senduncomplete__)
	{
#ifndef _DEBUG
		AfxMessageBox(_T("发送失败！"));
#endif
	}
	if (pMsg->message == __umymessage__anacomplete__)
	{
#ifndef _DEBUG
		AfxMessageBox(_T("解析完成！"));
#endif
	}
	if (pMsg->message == __umymessage__anauncomplete__)
	{
//#ifdef _DEBUG
		OutputDebugString(_T("Analysis Error!\r\n"));
//#endif
#ifndef _DEBUG
		AfxMessageBox(_T("解析失败！"));
#endif
	}
	return __super::PreTranslateMessage(pMsg);
}

void CReceiveEmailDlg::OnJobBegin(LONG nJobIndex, CFJobBase<MyJobParam*>* pJob)
{
}
void CReceiveEmailDlg::OnJobEnd(LONG nJobIndex, CFJobBase<MyJobParam*>* pJob)
{
	PostMessage(__umymessage__kill_hprogress__, (WPARAM)pJob->m_JobParam->m_lPos, (LPARAM)-1);
}

long CReceiveEmailDlg::SetTextWnd(long lType, long lCurrPos,long lStatus)
{
	::EnterCriticalSection(&_cs_);
	long lTextWnd = -1;
	int n = 0;
	BOOL bFind = FALSE;
	switch (lType)
	{
	case 0://Get
	{
		for (n = 0; n < 5; n++)
		{
			if (m_TextWnd[n] == lCurrPos)
			{
				lTextWnd = n;
				break;
			}
		}
	}
		break;
	case 1://Set
	{
		for (n = 0; n < 5; n++)
		{
			if (m_TextWnd[n] == lCurrPos)
			{
				bFind = TRUE;
				lTextWnd = n;
				if (lStatus < 0)
				{
					m_TextWnd[n] = lStatus;
				}
				break;
			}
		}
		if (!bFind)
		{
			lTextWnd = -1;
			for (n = 0; n < 5; n++)
			{
				if (m_TextWnd[n] == -1)
				{
					m_TextWnd[n] = lCurrPos;
					break;
				}
			}
		}
	}
		break;
	default:
		break;
	}
	::LeaveCriticalSection(&_cs_);
	return lTextWnd;
}

void CReceiveEmailDlg::InitTextWnd()
{
	m_Name1.SetWindowText(_T("Empty"));
	m_progress1.SetRange(0, 100);
	m_progress1.SetPos(0);
	m_Name2.SetWindowText(_T("Empty"));
	m_progress2.SetRange(0, 100);
	m_progress2.SetPos(0);
	m_Name3.SetWindowText(_T("Empty"));
	m_progress3.SetRange(0, 100);
	m_progress3.SetPos(0);
	m_Name4.SetWindowText(_T("Empty"));
	m_progress4.SetRange(0, 100);
	m_progress4.SetPos(0);
	m_Name5.SetWindowText(_T("Empty"));
	m_progress5.SetRange(0, 100);
	m_progress5.SetPos(0);
}


void CReceiveEmailDlg::OnBnClickedCheckDb()
{
	// TODO:  在此添加控件通知处理程序代码
	int nChenck = m_checkdb.GetCheck();
	m_dbinfo.nUseDB = nChenck;
	if (m_dbinfo.nUseDB != 1)
	{
		m_dbadd.EnableWindow(FALSE);
		m_dbname.EnableWindow(FALSE);
		m_tablename.EnableWindow(FALSE);
	}
	else
	{
		m_dbadd.EnableWindow(TRUE);
		m_dbname.EnableWindow(TRUE);
		m_tablename.EnableWindow(TRUE);
	}
}


void CReceiveEmailDlg::WriteToConfig()
{
	TCHAR szConfigPath[MAX_PATH] = { 0 }, szName[64] = { 0 };
	wsprintf(szConfigPath, _T("%s\\config.ini"), __Main_Path__);
	CString csTemp;
	if (m_dbinfo.nUseDB == 1)
		csTemp.Format(_T("yes"));
	else
		csTemp.Format(_T("no"));
	WritePrivateProfileString(_T("DataBase"), _T("usedb"), csTemp, szConfigPath);
}

DWORD WINAPI  CReceiveEmailDlg::_AfxMainTestAna(LPVOID lpParam)
{
	char chPath[MAX_PATH] = { 0 };
	BOOL bRet = TRUE;
	CString csUIDL,csPath,csLog;
	CReceiveEmailDlg*pDlg = (CReceiveEmailDlg*)lpParam;
	unsigned long lCurrPos = 0;
	size_t lCount = 0;
	CMailAnalysis ana;
	CSQLServer sql;
	sql.Connect(pDlg->m_sqldbinfo);
	csUIDL.Format(_T("%s"), pDlg->m_csTestText);
	csPath.Format(_T("Email\\%s"),csUIDL);
	WideCharToMultiByte(CP_ACP, 0, pDlg->m_csLogPath, MAX_PATH, chPath, MAX_PATH, NULL, NULL);
	ana.SetAbbreviation(_T("test"));
	ana.SetLogPath(chPath);
	if (ana.LoadFile(__Main_Path__, csPath))
	{
		bRet = FALSE;
		csLog.Format(_T("加载文件失败！"));
		pDlg->m_log.Log(csLog, csLog.GetLength());
		pDlg->PostMessage(__umymessage__anauncomplete__);
		return -1;
	}
	if (WaitForSingleObject(__HEVENT_TEST_EXIT__, 0L) == WAIT_OBJECT_0)
	{
		return 0;
	}
//#ifdef _DEBUG
	DWORD dwTime(0);
	dwTime = GetTickCount();
//#endif
	do 
	{
		if (ana.AnalysisHead() < 0)
		{
			ana.Clear(1);
			bRet = FALSE;
			csLog.Format(_T("AnalysisHead错误！"));
			pDlg->m_log.Log(csLog, csLog.GetLength());
			break;
		}
		if (ana.AnalysisBody(ana.GetBoundry(), ana.GetHeadRowCount()) < 0)
		{
			ana.Clear(1);
			bRet = FALSE;
			csLog.Format(_T("AnalysisBody错误！"));
			pDlg->m_log.Log(csLog, csLog.GetLength());
			break;
		}
		if (ana.AnalysisBoundary(ana.GetBoundry(), ana.GetAttach()) < 0)
		{
			ana.Clear(1);
			bRet = FALSE;
			csLog.Format(_T("AnalysisBoundary错误！"));
			pDlg->m_log.Log(csLog, csLog.GetLength());
			break;
		}
		sql.SaveToDB(ana.GetEmailItem());
		ana.Clear(0);
		sql.CloseDB();
//#ifdef _DEBUG
		dwTime = GetTickCount() - dwTime;
		CString csDebug;
		csDebug.Format(_T("Analysis Time = %d\tAnalysis [%s] Complete!\r\n"), dwTime / 1000, csUIDL);
		OutputDebugString(csDebug);
		pDlg->m_log.Log(csDebug, csDebug.GetLength());
//#endif
	} while (0);
	if (bRet)
		pDlg->PostMessage(__umymessage__anacomplete__);
	else pDlg->PostMessage(__umymessage__anauncomplete__);
	return 0;
}


void CReceiveEmailDlg::OnBnClickedButtonSet()
{
	// TODO:  在此添加控件通知处理程序代码
	/*CSettingDlg setting;
	if (setting.DoModal() == IDOK)
	{
		AfxMessageBox(_T("Test"));
	}*/
}

void CReceiveEmailDlg::GetForwardInfo(ForwardSet& fdsinfo)
{
	sprintf_s(fdsinfo.srvadd, 64, "%s", m_fsinfo.srvadd);
	sprintf_s(fdsinfo.username, 64, "%s", m_fsinfo.username);
	sprintf_s(fdsinfo.pass, 128, "%s", m_fsinfo.pass);
	sprintf_s(fdsinfo.to, 512, "%s", m_fsinfo.to);
	sprintf_s(fdsinfo.from, 128, "%s", m_fsinfo.from);
}

DWORD WINAPI  CReceiveEmailDlg::_AfxMainTestSend(LPVOID lpParam)
{
	TCHAR szUIDL[256] = { 0 };
	BOOL bRet = TRUE;
	CReceiveEmailDlg*pDlg = (CReceiveEmailDlg*)lpParam;
	char chPath[MAX_PATH] = { 0 }, chLog[128] = { 0 }, chUIDL[256] = {0};
	ForwardSet fsinfo;
	memset(&fsinfo, 0, sizeof(ForwardSet));
	pDlg->GetForwardInfo(fsinfo);
	SMTP smtp;
	smtp.SetForwardInfo(fsinfo);
	WideCharToMultiByte(CP_ACP, 0, __Main_Path__, MAX_PATH, chPath, MAX_PATH, NULL, NULL);
	strcat_s(chPath, MAX_PATH, "\\Email");
	smtp.SetCurrPath(chPath);
	wsprintf(szUIDL, _T("%s"), pDlg->m_csTestText.GetBuffer(0));
	WideCharToMultiByte(CP_ACP, 0, szUIDL, 256, chUIDL, 256, NULL, NULL);
	smtp.AddAttachFileName(chUIDL);
	WideCharToMultiByte(CP_ACP, 0, pDlg->m_csLogPath, MAX_PATH, chPath, MAX_PATH, NULL, NULL);
	smtp.SetLogPath(chPath);
	smtp.SetReceiver(fsinfo.to);

	do 
	{
		if (smtp.Logon() != 0)
		{
			bRet = FALSE;
			break;
		}
		if (smtp.SendHead() != 0)
		{
			bRet = FALSE;
			break;
		}
		if (smtp.SendTextBody() != 0)
		{
			bRet = FALSE;
			break;
		}
		if (smtp.SendFileBody() != 0)
		{
			bRet = FALSE;
			break;
		}
		if (!smtp.Quit())
			bRet = FALSE;
		//else
		//	smtp.DeleteEMLFile();
		break;
	} while (0);

	if (bRet)
		pDlg->PostMessage(__umymessage__sendcomplete__);
	else pDlg->PostMessage(__umymessage__senduncomplete__);
	return 0;
}


void CReceiveEmailDlg::OnBnClickedButton1()
{
	// TODO:  在此添加控件通知处理程序代码
	StopTest();
#ifndef _DEBUG
	m_editpath.GetWindowText(m_csTestText);
#endif
	if (!m_csTestText.IsEmpty())
	{
		if (__HEVENT_TEST_EXIT__ == NULL)
			__HEVENT_TEST_EXIT__ = CreateEvent(NULL, TRUE, FALSE, NULL);
		DWORD id(0);
		if (m_hMainTest == NULL)
		{
			m_hMainTest = CreateThread(NULL, 0, _AfxMainTestAna, (LPVOID)this, 0, &id);
		}
	}
	else
	{
		AfxMessageBox(_T("请输入UIDL！"));
	}
}


void CReceiveEmailDlg::OnBnClickedButton2()
{
	// TODO:  在此添加控件通知处理程序代码
	string strAPIname = "v1/system/guid", strParam;
	GGDataAPI::PostAsyncRequest(strAPIname, strParam, m_hWnd);
	StopTest2();
#ifndef _DEBUG
	m_editpath.GetWindowText(m_csTestText);
#endif
	
	if (!m_csTestText.IsEmpty())
	{
		DWORD id(0);
		if (m_hMainTest2 == NULL)
		{
			m_hMainTest2 = CreateThread(NULL, 0, _AfxMainTestSend, (LPVOID)this, 0, &id);
		}
	}
	else
	{
		AfxMessageBox(_T("请输入UIDL！"));
	}
}

void CReceiveEmailDlg::StopTest()
{
	if (__HEVENT_TEST_EXIT__)
		SetEvent(__HEVENT_TEST_EXIT__);
	if (m_hMainTest)
	{
		if (WaitForSingleObject(m_hMainTest, 10000L) != WAIT_OBJECT_0)
		{
			TerminateThread(m_hMainTest, 0);
		}
		CloseHandle(m_hMainTest);
		m_hMainTest = NULL;
	}
	if (__HEVENT_TEST_EXIT__)
	{
		CloseHandle(__HEVENT_TEST_EXIT__);
		__HEVENT_TEST_EXIT__ = NULL;
	}
}

void CReceiveEmailDlg::StopTest2()
{
	if (m_hMainTest2)
	{
		if (WaitForSingleObject(m_hMainTest2, INFINITE) != WAIT_OBJECT_0)
		{
			TerminateThread(m_hMainTest2, 0);
		}
		CloseHandle(m_hMainTest2);
		m_hMainTest2 = NULL;
	}
}

void CReceiveEmailDlg::StopMain()
{
	if (__HEVENT_MAIN_EXIT__)
		SetEvent(__HEVENT_MAIN_EXIT__);
	for (int i = 0; i < sizeof(m_hProcess) / sizeof(m_hProcess[0]);i++)
	{
		if (m_hProcess[i])
		{
			if (WaitForSingleObject(m_hProcess[i], 10000L) != WAIT_OBJECT_0)
			{
				TerminateThread(m_hProcess[i], 0);
			}
			CloseHandle(m_hProcess[i]);
			m_hProcess[i] = NULL;
		}
	}
	if (__HEVENT_MAIN_EXIT__)
	{
		CloseHandle(__HEVENT_MAIN_EXIT__);
		__HEVENT_MAIN_EXIT__ = NULL;
	}
	map<CString, MailBoxInfo>::iterator ite = m_mailList.begin();
	while (ite != m_mailList.end())
	{
		ite->second.lStatus = 0;
		ite++;
	}
	//m_showinfo.clear();
//#ifdef _DEBUG
	OutputDebugStringA("STOP　MAIN\r\n");
//#endif
}


DWORD CReceiveEmailDlg::_AfxMainProcess(LPVOID lpParam)
{
	CReceiveEmailDlg*pDlg = (CReceiveEmailDlg*)lpParam;
	MailBoxInfo info;
	MongoDBInfo dbinfo;
	SQLDBInfo sqlinfo;
	TCHAR szLogPath[MAX_PATH] = { 0 };
	char chTemp[MAX_PATH] = { 0 }, chLogPath[MAX_PATH] = { 0 };
	long lResult(0), lReturnvalue(0), lCount(0),i(1),lFailedCount(0);
	string strUDIL, strName;
	vector<string> UidlData;
	std::vector<string>::iterator ite = UidlData.begin();
	CString csUserName;
	char chDebug[512] = { 0 };
	CString csDebug;
	try
	{
		POP3 pop3;
		CSQLServer sql;
		DWORD dwID = GetCurrentThreadId();
//#ifdef _DEBUG
		DWORD dwTime(0);
//#endif
		while (true)
		{
			pop3.Close();
			sql.CloseDB();
			memset(&szLogPath, 0, MAX_PATH);
			memset(&chLogPath, 0, MAX_PATH);
			memset(&info, 0, sizeof(MailBoxInfo));
			memset(&dbinfo, 0, sizeof(MongoDBInfo));
			memset(&sqlinfo, 0, sizeof(SQLDBInfo));
			memset(&chTemp, 0, MAX_PATH);
			csUserName.Empty();
			UidlData.clear();
			lFailedCount = 0;
			if (WaitForSingleObject(__HEVENT_MAIN_EXIT__, 0L) == WAIT_OBJECT_0)
			{
				pDlg->GetMailBoxInfo(csUserName, info, 0);
				break;
			}
			pDlg->GetMailBoxInfo(csUserName, info, 1);
			if (!csUserName.IsEmpty())
			{
				pDlg->GetDataBaseInfo(dbinfo,sqlinfo);
				wsprintf(szLogPath, _T("%s\\Log\\%s.txt"), __Main_Path__, info.szName);
				WideCharToMultiByte(CP_ACP, 0, szLogPath, MAX_PATH, chLogPath, MAX_PATH, NULL, NULL);
				pop3.SetLogPath(chLogPath);
				pop3.SetInfo(info.szName, info, dbinfo, __Main_Path__, lstrlen(__Main_Path__));
				sql.SetLogPath(chLogPath);
				WideCharToMultiByte(CP_ACP, 0, info.szAbbreviation, 64, chTemp, MAX_PATH, NULL, NULL);
				strName = chTemp;
				lResult = pop3.Login(info.szServerAdd, info.lPort, csUserName, info.szPasswd);
				if (lResult >= 0)
				{
					if (pop3.ConnectDataBase() && sql.Connect(sqlinfo))
					{
						lResult = pop3.GetMailCount();
						pDlg->m_showinfo[dwID].lTotal = lResult;
						swprintf_s(pDlg->m_showinfo[dwID].szAbbreviation, 64, _T("%s"), info.szAbbreviation);
						swprintf_s(pDlg->m_showinfo[dwID].szName, 128, _T("%s"), info.szName);
						pDlg->m_showinfo[dwID].lStatus = 0;
						if (lResult > 0)
						{
							for (i = 1; i < lResult + 1; i++)
							{
								if (WaitForSingleObject(__HEVENT_MAIN_EXIT__, 10L) == WAIT_OBJECT_0)
								{
									pDlg->GetMailBoxInfo(csUserName, info, 0);
									pop3.QuitDataBase();
									sql.CloseDB();
									pop3.Close();
									break;
								}
//#ifdef _DEBUG
								csDebug.Format(_T("%s Count = %d\tTotal = %d\r\n"), info.szName, i,lResult);
								OutputDebugString(csDebug);
								dwTime = GetTickCount();
//#endif
								if (pop3.GetStatus())
									break;
								strUDIL.clear();
								strUDIL = pop3.GetUIDL(i);
								if (strUDIL.length()>0)
								{
									lReturnvalue = pop3.CheckUIDL(strUDIL, strName);
									if (lReturnvalue == MONGO_NOT_FOUND)
									{
										if (pop3.GetEMLFile(i, strUDIL) == 0)
										{
											if (pDlg->MailAnalysis(pop3, sql, strUDIL, info.szAbbreviation, 0)<0)
											{
												sprintf_s(chDebug, 512, "Analysis [%s] Error!", strUDIL.c_str());
//#ifdef _DEBUG
												OutputDebugStringA(chDebug);
												OutputDebugStringA("\r\n");
//#endif
											}
										}
									}
									else if (lReturnvalue == MONGO_DELETE)
										UidlData.push_back(strUDIL);
								}
								else
								{
									lFailedCount++;
									if (lFailedCount > 10)
									{
//#ifdef _DEBUG
										OutputDebugString(_T("Can't get UIDL!\r\n"));
//#endif
									}
								}
//#ifdef _DEBUG
								dwTime = GetTickCount() - dwTime;
								csDebug.Format(_T("Process Time = %d\r\n"), dwTime / 1000);
								OutputDebugString(csDebug);
//#endif
								pDlg->m_showinfo[dwID].lCurr = i;
								
							}
							sql.CloseDB();
							lCount = UidlData.size();
							if (lCount>0)
							{
								ite = UidlData.begin();
								pDlg->m_showinfo[dwID].lStatus = 1;
								pDlg->m_showinfo[dwID].lTotal = lCount;
								for (long i = 1; i < lResult + 1; i++)
								{
//#ifdef _DEBUG
									csDebug.Format(_T("%s Del-Count = %d\r\n"), info.szName, i);
									OutputDebugString(csDebug);
//#endif
									strUDIL.clear();
									strUDIL = pop3.GetUIDL(i);
									if (strUDIL.length() > 0)
									{
										ite = std::find(UidlData.begin(), UidlData.end(), strUDIL);
										if (ite != UidlData.end())
										{
											if (pop3.DelEmail(i, strUDIL) == SUCCESS)
												UidlData.erase(ite);
										}
									}
									pDlg->m_showinfo[dwID].lCurr = i;
								}
							}
							pop3.QuitDataBase();
							pDlg->GetMailBoxInfo(csUserName, info,0);
						}
					}// end of if
				}
				else
				{
					TCHAR szError[256] = { 0 };
					GetErrorMessage(lResult, szError);
					csDebug.Format(_T("登陆 %s 出现问题[%s]\r\nThread [0x%x] will wait 5 sec!\r\n"), 
						info.szName, szError, GetCurrentThreadId());
					pop3.QuitDataBase();
					OutputDebugString(csDebug);
					OutputDebugString(_T("\r\n"));
					if (WaitForSingleObject(__HEVENT_MAIN_EXIT__, 5000L) == WAIT_OBJECT_0)
						break;
					pDlg->GetMailBoxInfo(csUserName, info, 0);
				}
				pop3.Close();
			}
			else
			{
				//pDlg->GetMailBoxInfo(csUserName, info, 0);
				csDebug.Format(_T("Thread [0x%x] will wait 5sec!\r\n"), GetCurrentThreadId());
				OutputDebugString(csDebug);
				WaitForSingleObject(__HEVENT_MAIN_EXIT__, 5000L);
			}// end of else
		}// end of while
	}// end of tyr
	catch (...)
	{
		csDebug.Format(_T("Thread [0x%x] will exit!\r\n"), GetCurrentThreadId());
		OutputDebugString(csDebug);
	}
	return 0;
}

long CReceiveEmailDlg::MailAnalysis(POP3& pop3, CSQLServer& sql, const string& strUIDL, LPCTSTR lpAbb, long lType)
{
	BOOL bRet = TRUE;
	CString csUIDL(strUIDL.c_str()), csPath(pop3.GetCurrPath());
	CMailAnalysis ana;
	ana.SetAbbreviation(lpAbb);
	ana.LoadFile(csPath, csUIDL);
//#ifdef _DEBUG
	DWORD dwTime(0);
	dwTime = GetTickCount();
//#endif
	do
	{
		if (ana.AnalysisHead() < 0)
		{
			ana.Clear(1);
			bRet = FALSE;
			break;
		}
		if (ana.AnalysisBody(ana.GetBoundry(), ana.GetHeadRowCount()) < 0)
		{
			ana.Clear(1);
			bRet = FALSE;
			break;
		}
		if (ana.AnalysisBoundary(ana.GetBoundry(), ana.GetAttach()) < 0)
		{
			ana.Clear(1);
			bRet = FALSE;
			break;
		}
		if (sql.SaveToDB(ana.GetEmailItem()) == 0)
		{
			pop3.SaveFileToDB(ana.GetEmailItem());
			ana.Clear(lType);
		}
		else
		{
			pop3.DeleteFromDB(ana.GetEmailItem());
			sql.DeleteFromSQL(ana.GetEmailItem());
			ana.Clear(0);
		}
	} while (0);
//#ifdef _DEBUG
	dwTime = GetTickCount() - dwTime;
	CString csDebug;
	csDebug.Format(_T("Analysis Time = %d\tAnalysis [%s] Complete!"), dwTime / 1000, csUIDL);
	OutputDebugString(csDebug);
	OutputDebugString(_T("\r\n"));
//#endif
	if (bRet)
		return 0;
	else
		return -1;
}