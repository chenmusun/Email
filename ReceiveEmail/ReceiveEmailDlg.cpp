
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
#include "DialogInfo.h"
#include "DialogPDF.h"
#include "../PDF/PDF.h"

#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/xml_parser.hpp>  
#include <boost/typeof/typeof.hpp>
using namespace boost::property_tree;

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

static BOOL bRun = FALSE;

TCHAR __Main_Path__[MAX_PATH];
HANDLE __HEVENT_TEST_EXIT__ = NULL;
HANDLE __HEVENT_MAIN_EXIT__ = NULL;

vector<CString> g_OldDataBase;


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
		long lCurrPos = 0,lPercent(0);
		double dValue(0);
		while (true)
		{
			memset(&szInfo, 0, 128);
			if (WaitForSingleObject(__HEVENT_MAIN_EXIT__, 100L) == WAIT_OBJECT_0)
			{
				OutputDebugStringA("WAIT2 __HEVENT_MAIN_EXIT__\r\n");
				break;
			}
			lCurrPos = 0;
			ite = pDlg->m_showinfo.begin();
			while (ite!=pDlg->m_showinfo.end())
			{
				if (WaitForSingleObject(__HEVENT_MAIN_EXIT__, 10L) == WAIT_OBJECT_0)
				{
					OutputDebugStringA("WAIT1 __HEVENT_MAIN_EXIT__\r\n");
					break;
				}
				dValue = ite->second.lTotal<=0?0:(double)ite->second.lCurr / ite->second.lTotal;
				lPercent = (long)(dValue * 100);
				switch (ite->second.lStatus)
				{
				case 0:
					swprintf_s(szInfo, 128, _T("%s\t[%d/%d]\t[%d%%]"), ite->second.szName, ite->second.lCurr, ite->second.lTotal, lPercent);
					break;
				case 1:
					swprintf_s(szInfo, 128, _T("清理----%s\t[%d/%d]\t[%d%%]"), ite->second.szName, ite->second.lCurr, ite->second.lTotal, lPercent);
					break;
				default:
					break;
				}
				pDlg->SetShowInfo(lCurrPos, szInfo, lPercent);
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
, m_csRunTime(_T("")), m_dwStartTime(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_mailList.clear();
	::InitializeCriticalSection(&_cs_);
	::InitializeCriticalSection(&_check_failed_);
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
	StopEx();
	DeleteCriticalSection(&_cs_);
	DeleteCriticalSection(&_check_failed_);
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
	DDX_Control(pDX, IDC_BUTTON1, m_btnTest);
	DDX_Control(pDX, IDC_BUTTON_SET, m_btnSetting);
	DDX_Control(pDX, IDC_BUTTON2, m_btnTest2);
	DDX_Control(pDX, IDC_MFCEDITBROWSE1, m_editpath);
	DDX_Control(pDX, IDC_STATIC_TIME, m_time);
	DDX_Control(pDX, IDC_BUTTON_PDF, m_btnPDF);
}

BEGIN_MESSAGE_MAP(CReceiveEmailDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_MFCBUTTON_SET, &CReceiveEmailDlg::OnBnClickedMfcbuttonSet)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CReceiveEmailDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON1, &CReceiveEmailDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON_SET, &CReceiveEmailDlg::OnBnClickedButtonSet)
	ON_BN_CLICKED(IDC_BUTTON2, &CReceiveEmailDlg::OnBnClickedButton2)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_MAILBOX, &CReceiveEmailDlg::OnLvnItemchangedListMailbox)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MAILBOX, &CReceiveEmailDlg::OnNMDblclkListMailbox)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_MAILBOX, &CReceiveEmailDlg::OnNMRClickListMailbox)
	ON_COMMAND(ID_ADDITEM, &CReceiveEmailDlg::OnAdditem)
	ON_COMMAND(ID_DELITEM, &CReceiveEmailDlg::OnDelitem)
	ON_BN_CLICKED(IDC_BUTTON_PDF, &CReceiveEmailDlg::OnBnClickedButtonPdf)
	ON_WM_ERASEBKGND()
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

	g_OldDataBase.push_back(_T("192.168.1.150"));
	g_OldDataBase.push_back(_T("192.168.0.6"));
	if (LoadFromConfig())
	{
		InitMailList();
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
	wsprintf(szPath, _T("%s\\Log\\main.txt"),__Main_Path__);
	m_csLogPath.Format(_T("%s"),szPath);
	m_log.SetPath(m_csLogPath,m_csLogPath.GetLength());
#ifdef _DEBUG
	if (m_csTestText.IsEmpty())
		m_csTestText.Format(_T("MD50000018634MSG723760304367911831483762"));
#endif
	m_startdate = COleDateTime::GetCurrentTime();
	m_csRunTime.Format(_T("%d年%d月%d日 %d:%d:%d")
		, m_startdate.GetYear(), m_startdate.GetMonth()
		, m_startdate.GetDay(), m_startdate.GetHour(), m_startdate.GetMinute(), m_startdate.GetSecond());
	m_time.SetWindowText(m_csRunTime);
	if (!InitializeLibrary())
		return FALSE;
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
	StopEx();
	DWORD dwId[5] = { 0 },id;
	ShowInfo stShow;
	memset(&stShow, 0, sizeof(ShowInfo));
	if (m_showinfo.size()>0)
		m_showinfo.clear();
	InitTextWnd();
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
				csDebug.Format(_T("ThreadID = [0x%x]\r\n"),dwId[i]);
				OutputDebugString(csDebug);
//#endif
			}
		}
		m_dwStartTime = m_dwStartTime == 0 ? GetTickCount64() : m_dwStartTime;
		m_hMain = CreateThread(NULL, 0, _AfxMain, (LPVOID)this, 0, &id);
		m_btnStop.EnableWindow(TRUE);
		m_btnSet.EnableWindow(FALSE);
		bRun = TRUE;
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
			m_listMailBox.DeleteAllItems();
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
		csTemp.Format(_T("mail_%d_saveday"), i);
		info.lSaveDay = GetPrivateProfileInt(_T("E-mail"), csTemp, 14, szConfigPath);
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
	GetPrivateProfileString(_T("DataBase"), _T("sqlusedb"), _T("yes"), szName, 64, szConfigPath);
	csTemp.Format(_T("%s"), szName);
	if (csTemp.CompareNoCase(_T("yes")) >= 0)
		m_sqldbinfo.lUseDB = 1;
	else m_sqldbinfo.lUseDB = 0;
	GetPrivateProfileString(_T("DataBase"), _T("sql_dbadd"), _T("127.0.0.1"), m_sqldbinfo.szDBAdd, 32, szConfigPath);
	vector<CString>::iterator ite= g_OldDataBase.begin();
	ite = find(g_OldDataBase.begin(), g_OldDataBase.end(), m_sqldbinfo.szDBAdd);
	if (ite!=g_OldDataBase.end())
	{
		m_sqldbinfo.lDBType = 1;
	}
	GetPrivateProfileString(_T("DataBase"), _T("sql_db"), _T("ReportEmailDB"), m_sqldbinfo.szDBName, 32, szConfigPath);
	GetPrivateProfileString(_T("DataBase"), _T("sql_username"), _T("sa"), m_sqldbinfo.szUserName, 32, szConfigPath);
	GetPrivateProfileString(_T("DataBase"), _T("sql_passwd"), _T("test.123"), m_sqldbinfo.szPasswd, 32, szConfigPath);

	memset(&m_fsinfo, 0, sizeof(ForwardSet));
	GetPrivateProfileStringA("ForwardSet", "srvadd", "", m_fsinfo.srvadd, 64, chConfigPath);
	GetPrivateProfileStringA("ForwardSet", "username", "", m_fsinfo.username, 64, chConfigPath);
	GetPrivateProfileStringA("ForwardSet", "pass", "", m_fsinfo.pass, 128, chConfigPath);
	GetPrivateProfileStringA("ForwardSet", "to", "", m_fsinfo.to, 512, chConfigPath);
	GetPrivateProfileStringA("ForwardSet", "from", "", m_fsinfo.from, 128, chConfigPath);
	////boost xml test
	//ptree pt;
	//string strDebug;
	//read_xml("conf.xml", pt);
	//memset(chTemp, 0, 512);
	//sprintf_s(chTemp, 512, "ID is %d\r\n", pt.get<int>("con.id"));
	//strDebug = chTemp;
	//OutputDebugStringA(strDebug.c_str());
	//memset(chTemp, 0, 512);
	//sprintf_s(chTemp, 512, "Try Default %d\r\n", pt.get<int>("con.no_prop", 100));
	//strDebug = chTemp;
	//OutputDebugStringA(strDebug.c_str());
	//ptree child = pt.get_child("con");    //取一个子节点  
	//memset(chTemp, 0, 512);
	//sprintf_s(chTemp, 512,"name is :%s\r\n", child.get<string>("name").c_str());
	//strDebug = chTemp;
	//OutputDebugStringA(strDebug.c_str());
	//strDebug.clear();

	//child = pt.get_child("con.urls");
	//for (BOOST_AUTO(pos, child.begin()); pos != child.end(); ++pos)  //boost中的auto  
	//{
	//	strDebug += "\n"; 
	//	strDebug+=pos->second.data();
	//}
	//strDebug += "\r\n";
	//OutputDebugStringA(strDebug.c_str());
	//pt.put("con.name", "Sword");    //更改某个键值  
	//pt.add("con.urls.url", "http://www.baidu.com"); //增加某个键值  
	//write_xml("conf.xml", pt);    //写入XML  
	return TRUE;
}


void CReceiveEmailDlg::OnCancel()
{
	// TODO:  在此添加专用代码和/或调用基类
	StopEx();
	StopTest();
	StopTest2();
	WriteToConfig();
	FinalizeLibrary();
	CDialogEx::OnCancel();
}


void CReceiveEmailDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO:  在此处添加消息处理程序代码
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
	MAILLIST_ITE ite = m_mailList.begin();
	switch (lStatus)
	{
	case 1:
	{
		memset(&info, 0, sizeof(MailBoxInfo));
		while (TRUE)
		{
			if (lCount > 1)
				break;
			if (ite->second.lStatus == 0 && lCurrPos > m_lLastPos)
			{
				csUserName = ite->first;
				wsprintf(info.szName, ite->second.szName);
				wsprintf(info.szPasswd, ite->second.szPasswd);
				wsprintf(info.szServerAdd, ite->second.szServerAdd);
				info.lPort = ite->second.lPort;
				wsprintf(info.szAbbreviation, ite->second.szAbbreviation);
				info.bSendMail = ite->second.bSendMail;
				wsprintf(info.szMailAdd, ite->second.szMailAdd);
				info.lSaveDay = ite->second.lSaveDay;
				memset(info.chUIDL, 0, 64);
				sprintf_s(info.chUIDL,64,"%s",ite->second.chUIDL);
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
			memset(ite->second.chUIDL, 0, 64);
			sprintf_s(ite->second.chUIDL, 64, "%s", info.chUIDL);
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
	CString csUsername;
	MailBoxInfo info;
	memset(&info, 0, sizeof(MailBoxInfo));
	StopEx();
	m_btnStop.EnableWindow(FALSE);
	m_btnSet.EnableWindow(TRUE);
	m_showinfo.clear();
	GetMailBoxInfo(csUsername,info,3);
	bRun = FALSE;
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
		SetRect(&rtTmp, 243, 5, cx - HORIZON_GAP, cy - BTN_HEIGHT*2 - GAP * 3);
		m_Group.MoveWindow(&rtTmp);
		lGap = (rtTmp.Height() - 5 * BTN_HEIGHT*2) / 6;
		lTop = rtTmp.top;
		if (m_Name1.m_hWnd)
		{
			SetRect(&rtTmp, 253, lTop + lGap, cx - 25, lTop+lGap + BTN_HEIGHT);
			m_Name1.MoveWindow(&rtTmp);
		}
		if (m_progress1.m_hWnd)
		{
			SetRect(&rtTmp, 253, lGap + lTop + BTN_HEIGHT, cx - 25, lTop+BTN_HEIGHT * 2 + lGap);
			m_progress1.MoveWindow(&rtTmp);
		}
		if (m_Name2.m_hWnd)
		{
			SetRect(&rtTmp, 253, lTop + BTN_HEIGHT * 2 + lGap * 2, cx - 25, lTop + BTN_HEIGHT * 3 + lGap * 2);
			m_Name2.MoveWindow(&rtTmp);
		}
		if (m_progress2.m_hWnd)
		{
			SetRect(&rtTmp, 253, lTop + BTN_HEIGHT * 3 + lGap * 2, cx - 25, lTop + BTN_HEIGHT * 4 + lGap * 2);
			m_progress2.MoveWindow(&rtTmp);
		}
		if (m_Name3.m_hWnd)
		{
			SetRect(&rtTmp, 253, lTop + BTN_HEIGHT * 4 + lGap * 3, cx - 25, lTop + BTN_HEIGHT * 5 + lGap * 3);
			m_Name3.MoveWindow(&rtTmp);
		}
		if (m_progress3.m_hWnd)
		{
			SetRect(&rtTmp, 253, lTop + BTN_HEIGHT * 5 + lGap * 3, cx - 25, lTop + BTN_HEIGHT * 6 + lGap * 3);
			m_progress3.MoveWindow(&rtTmp);
		}
		if (m_Name4.m_hWnd)
		{
			SetRect(&rtTmp, 253, lTop + BTN_HEIGHT * 6 + lGap * 4, cx - 25, lTop + BTN_HEIGHT * 7 + lGap * 4);
			m_Name4.MoveWindow(&rtTmp);
		}
		if (m_progress4.m_hWnd)
		{
			SetRect(&rtTmp, 253, lTop + BTN_HEIGHT * 7 + lGap * 4, cx - 25, lTop + BTN_HEIGHT * 8 + lGap * 4);
			m_progress4.MoveWindow(&rtTmp);
		}
		if (m_Name5.m_hWnd)
		{
			SetRect(&rtTmp, 253, lTop + BTN_HEIGHT * 8 + lGap * 5, cx - 25, lTop + BTN_HEIGHT * 9 + lGap * 5);
			m_Name5.MoveWindow(&rtTmp);
		}
		if (m_progress5.m_hWnd)
		{
			SetRect(&rtTmp, 253, lTop + BTN_HEIGHT * 9 + lGap * 5, cx - 25, lTop + BTN_HEIGHT * 10 + lGap * 5);
			m_progress5.MoveWindow(&rtTmp);
		}
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
		SetRect(&rtTmp, 243, cy - BTN_HEIGHT*2 - GAP*2, 243 + BTN_LENGTH * 3 + HORIZON_GAP*2, cy - BTN_HEIGHT-GAP*2);
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
		SetRect(&rtTmp, 243 + HORIZON_GAP + BTN_LENGTH, cy - BTN_HEIGHT - GAP, 243 + BTN_LENGTH*2 + HORIZON_GAP, cy - GAP);
		m_btnTest2.MoveWindow(&rtTmp);
	}
	if (m_btnPDF.m_hWnd)
	{
		SetRect(&rtTmp, 243 + HORIZON_GAP*2 + BTN_LENGTH*2, cy - BTN_HEIGHT - GAP, 243 + BTN_LENGTH * 3 + HORIZON_GAP*2, cy - GAP);
		m_btnPDF.MoveWindow(&rtTmp);
	}
}

void CReceiveEmailDlg::Stop(long lType)//用于停止工作分配线程
{
	OutputDebugStringA("STOP FRESH\r\n");
	if (__HEVENT_MAIN_EXIT__)
		SetEvent(__HEVENT_MAIN_EXIT__);
	if (m_hMain)
	{
		if (WaitForSingleObject(m_hMain, INFINITE) != WAIT_OBJECT_0)
		{
			TerminateThread(m_hMain, 0);
		}
	}
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
		DWORD dwTime(0),dwDay(0),dwHour(0),dwMin(0),dwSec(0);
		dwTime = GetTickCount64() - m_dwStartTime;
		dwDay = dwTime / 86400000;
		dwTime -=dwDay * 86400000;
		dwHour = dwTime/3600000;
		dwTime -=dwHour * 3600000;
		dwMin = dwTime/60000;
		dwTime -=dwMin * 60000;
		dwSec = dwTime / 1000;
		CString csDate;
		csDate.Format(_T("%s| RT:%d天%d时%d分%d秒")
			, m_csRunTime
			,dwDay,dwHour,dwMin,dwSec);
		m_time.SetWindowText(csDate);
	}
	if (pMsg->message == __umymessage__kill_hprogress__)
	{
		
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



void CReceiveEmailDlg::WriteToConfig()
{
	TCHAR szConfigPath[MAX_PATH] = { 0 }, szTemp[64] = { 0 };
	wsprintf(szConfigPath, _T("%s\\config.ini"), __Main_Path__);
	CString csTemp;
	if (m_dbinfo.nUseDB == 1)
		csTemp.Format(_T("yes"));
	else
		csTemp.Format(_T("no"));
	WritePrivateProfileString(_T("DataBase"), _T("usedb"), csTemp, szConfigPath);
	csTemp = m_dbinfo.chDBAdd;
	WritePrivateProfileString(_T("DataBase"), _T("dbadd"), csTemp, szConfigPath);
	csTemp = m_dbinfo.chDBName;
	WritePrivateProfileString(_T("DataBase"), _T("data_base"), csTemp, szConfigPath);
	csTemp = m_dbinfo.chTable;
	WritePrivateProfileString(_T("DataBase"), _T("table"), csTemp, szConfigPath);
	csTemp = m_dbinfo.chUserName;
	WritePrivateProfileString(_T("DataBase"), _T("username"), csTemp, szConfigPath);
	csTemp = m_dbinfo.chPasswd;
	WritePrivateProfileString(_T("DataBase"), _T("passwd"), csTemp, szConfigPath);

	if (m_sqldbinfo.lUseDB == 1)
		csTemp.Format(_T("yes"));
	else
		csTemp.Format(_T("no"));
	WritePrivateProfileString(_T("DataBase"), _T("sqlusedb"), csTemp, szConfigPath);
	csTemp = m_sqldbinfo.szDBAdd;
	WritePrivateProfileString(_T("DataBase"), _T("sql_dbadd"), csTemp, szConfigPath);
	csTemp = m_sqldbinfo.szDBName;
	WritePrivateProfileString(_T("DataBase"), _T("sql_db"), csTemp, szConfigPath);
	csTemp = m_sqldbinfo.szUserName;
	WritePrivateProfileString(_T("DataBase"), _T("sql_username"), csTemp, szConfigPath);
	csTemp = m_sqldbinfo.szPasswd;
	WritePrivateProfileString(_T("DataBase"), _T("sql_passwd"), csTemp, szConfigPath);
	long lCount = m_mailList.size(),i(0);
	csTemp.Format(_T("%d"),lCount);
	WritePrivateProfileString(_T("E-mail"), _T("count"), csTemp, szConfigPath);
	MAILLIST_ITE ite = m_mailList.begin();
	while (ite != m_mailList.end())
	{
		csTemp.Format(_T("mail_%d_username"), i);
		WritePrivateProfileString(_T("E-mail"), csTemp,(*ite).first, szConfigPath);
		csTemp.Format(_T("mail_%d_name"), i);
		WritePrivateProfileString(_T("E-mail"), csTemp, (*ite).second.szName, szConfigPath);
		csTemp.Format(_T("mail_%d_add"), i);
		WritePrivateProfileString(_T("E-mail"), csTemp, (*ite).second.szServerAdd, szConfigPath);
		csTemp.Format(_T("mail_%d_port"), i);
		wsprintf(szTemp, _T("%d"), (*ite).second.lPort);
		WritePrivateProfileString(_T("E-mail"), csTemp, szTemp, szConfigPath);
		csTemp.Format(_T("mail_%d_passwd"), i);
		WritePrivateProfileString(_T("E-mail"), csTemp, (*ite).second.szPasswd, szConfigPath);
		csTemp.Format(_T("mail_%d_abbreviation"), i);
		WritePrivateProfileString(_T("E-mail"), csTemp, (*ite).second.szAbbreviation, szConfigPath);
		if ((*ite).second.bSendMail)
		{
			csTemp.Format(_T("mail_%d_bsend"), i);
			WritePrivateProfileString(_T("E-mail"), csTemp, _T("1"), szConfigPath);
			csTemp.Format(_T("mail_%d_mailadd"), i);
			WritePrivateProfileString(_T("E-mail"), csTemp, (*ite).second.szMailAdd, szConfigPath);
		}
		else
		{
			csTemp.Format(_T("mail_%d_bsend"), i);
			WritePrivateProfileString(_T("E-mail"), csTemp, _T("0"), szConfigPath);
		}
		csTemp.Format(_T("mail_%d_saveday"), i);
		wsprintf(szTemp, _T("%d"), (*ite).second.lSaveDay);
		WritePrivateProfileString(_T("E-mail"), csTemp, szTemp, szConfigPath);
		i++;
		ite++;
	}

	csTemp = m_fsinfo.srvadd;
	WritePrivateProfileString(_T("ForwardSet"), _T("srvadd"), csTemp, szConfigPath);
	csTemp = m_fsinfo.username;
	WritePrivateProfileString(_T("ForwardSet"), _T("username"), csTemp, szConfigPath);
	csTemp = m_fsinfo.pass;
	WritePrivateProfileString(_T("ForwardSet"), _T("pass"), csTemp, szConfigPath);
	csTemp = m_fsinfo.to;
	WritePrivateProfileString(_T("ForwardSet"), _T("to"), csTemp, szConfigPath);
	csTemp = m_fsinfo.from;
	WritePrivateProfileString(_T("ForwardSet"), _T("from"), csTemp, szConfigPath);
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
	ana.SetLogPath(chPath,strlen(chPath));
	if (ana.LoadFile(__Main_Path__, csPath))
	{
		bRet = FALSE;
		csLog.Format(_T("加载文件失败！"));
		pDlg->m_log.Log(csLog, csLog.GetLength());
		csLog.Append(_T("\r\n"));
		OutputDebugString(csLog);
		pDlg->PostMessage(__umymessage__anauncomplete__);
		return -1;
	}
	ana.SetClearType(0);
	if (WaitForSingleObject(__HEVENT_TEST_EXIT__, 0L) == WAIT_OBJECT_0)
	{
		return 0;
	}
//#ifdef _DEBUG
	DWORD dwTime(0);
	dwTime = GetTickCount64();
//#endif
	do 
	{
		if (ana.AnalysisHead() < 0)
		{
			ana.SetClearType(0);
			bRet = FALSE;
			csLog.Format(_T("AnalysisHead错误！"));
			pDlg->m_log.Log(csLog, csLog.GetLength());
			csLog.Append(_T("\r\n"));
			OutputDebugString(csLog);
			break;
		}
		if (ana.AnalysisBody(ana.GetBoundry(), ana.GetHeadRowCount()) < 0)
		{
			ana.SetClearType(0);
			bRet = FALSE;
			csLog.Format(_T("AnalysisBody错误！"));
			pDlg->m_log.Log(csLog, csLog.GetLength());
			csLog.Append(_T("\r\n"));
			OutputDebugString(csLog);
			break;
		}
		if (ana.AnalysisBoundary(ana.GetBoundry(), ana.GetAttach()) < 0)
		{
			ana.SetClearType(0);
			bRet = FALSE;
			csLog.Format(_T("AnalysisBoundary错误！"));
			pDlg->m_log.Log(csLog, csLog.GetLength());
			csLog.Append(_T("\r\n"));
			OutputDebugString(csLog);
			break;
		}
		if (sql.GetSQLDBType() == 0)
			sql.SaveToDB(ana.GetEmailItem(),FALSE);
		else sql.SaveToDBOld(ana.GetEmailItem(), FALSE);
		ana.Clear();
		sql.CloseDB();
//#ifdef _DEBUG
		dwTime = GetTickCount64() - dwTime;
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
	CSettingDlg setting;
	MongoDBInfo moinfo;
	memset(&moinfo, 0, sizeof(MongoDBInfo));
	SQLDBInfo sqlinfo;
	memset(&sqlinfo, 0, sizeof(SQLDBInfo));
	ForwardSet fsinfo;
	memset(&fsinfo, 0, sizeof(ForwardSet));
	memcpy_s(&moinfo, sizeof(MongoDBInfo), &m_dbinfo, sizeof(MongoDBInfo));
	memcpy_s(&sqlinfo, sizeof(SQLDBInfo), &m_sqldbinfo, sizeof(SQLDBInfo));
	memcpy_s(&fsinfo, sizeof(ForwardSet), &m_fsinfo, sizeof(ForwardSet));
	setting.SetInfo(&moinfo, &sqlinfo, &fsinfo);
	if (setting.DoModal() == IDOK)
	{
		if (!bRun)
		{
			memcpy_s(&m_dbinfo, sizeof(MongoDBInfo), &moinfo, sizeof(MongoDBInfo));
			memcpy_s(&m_sqldbinfo, sizeof(SQLDBInfo), &sqlinfo, sizeof(SQLDBInfo));
			memcpy_s(&m_fsinfo, sizeof(ForwardSet), &fsinfo, sizeof(ForwardSet));
		}
	}
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
	smtp.SetLogPath(chPath,strlen(chPath));
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
		if (smtp.SendTextBody("xxxxxxxxxxxxxxxxxxxxxxxxxxx") != 0)
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
#else
	CString csPath;
	m_editpath.GetWindowText(csPath);
	if (!csPath.IsEmpty())
		m_csTestText = csPath;
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
	long lSize = sizeof(m_hProcess) / sizeof(m_hProcess[0]);
	for (int i = 0; i < sizeof(m_hProcess) / sizeof(m_hProcess[0]); i++)
	{
		if (m_hProcess[i])
		{
			if (WaitForSingleObject(m_hProcess[i], 10000L) != WAIT_OBJECT_0)
			{
				OutputDebugString(_T("*************************************TerminateThread*****************************************"));
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
	MAILLIST_ITE ite = m_mailList.begin();
	while (ite != m_mailList.end())
	{
		ite->second.lStatus = 0;
		ite++;
	}
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
	ForwardSet fdsinfo;
	TCHAR szLogPath[MAX_PATH] = { 0 },szError[256] = { 0 };
	char chTemp[MAX_PATH] = { 0 }, chLogPath[MAX_PATH] = { 0 }, chDebug[512] = {0};
	long lResult(0), lReturnvalue(0), lCount(0),lFailedCount(0),lType(0),lLen(0),i(1),j(1);
	string strUDIL, strName;
	CString csUserName,csDebug;
	map<long, string> mapUIDLs,mapDelUIDLs;
	map<long, string>::iterator iteuidl = mapUIDLs.begin();
	map<long, string>::iterator itedel = mapDelUIDLs.begin();
	try
	{
		POP3 pop3;
		CSQLServer sql;
		SMTP smtp;
		DWORD dwID = GetCurrentThreadId();
		DWORD dwTime(0);
		while (true)
		{
			memset(&szLogPath, 0, MAX_PATH);
			memset(&chLogPath, 0, MAX_PATH);
			memset(&info, 0, sizeof(MailBoxInfo));
			memset(&dbinfo, 0, sizeof(MongoDBInfo));
			memset(&sqlinfo, 0, sizeof(SQLDBInfo));
			memset(&chTemp, 0, MAX_PATH);
			csUserName.Empty();
			mapUIDLs.clear();
			mapDelUIDLs.clear();
			lFailedCount = 0;
			if (WaitForSingleObject(__HEVENT_MAIN_EXIT__, 10L) == WAIT_OBJECT_0)
			{
				OutputDebugStringA("WAIT3 __HEVENT_MAIN_EXIT__\r\n");
				pDlg->GetMailBoxInfo(csUserName, info, 0);
				break;
			}
			pDlg->GetMailBoxInfo(csUserName, info, 1);
			if (!csUserName.IsEmpty())
			{
				pDlg->GetDataBaseInfo(dbinfo,sqlinfo);
				wsprintf(szLogPath, _T("%s\\Log\\%s.txt"), __Main_Path__, info.szName);
				WideCharToMultiByte(CP_ACP, 0, szLogPath, MAX_PATH, chLogPath, MAX_PATH, NULL, NULL);
				lLen = strlen(chLogPath);
				pop3.SetLogPath(chLogPath, lLen);
				pop3.SetInfo(info.szName, info, __Main_Path__, lstrlen(__Main_Path__));
				pop3.SetDBinfo(dbinfo);
				sql.SetLogPath(chLogPath, lLen);
				WideCharToMultiByte(CP_ACP, 0, info.szAbbreviation, 64, chTemp, MAX_PATH, NULL, NULL);
				strName = chTemp;
				if (info.bSendMail)
				{
					memset(&fdsinfo, 0, sizeof(ForwardSet));
					pDlg->GetForwardInfo(fdsinfo);
					smtp.SetForwardInfo(fdsinfo);
					smtp.InitSMTPPro();
					smtp.SetLogPath(chLogPath, lLen);
				}
				lResult = pop3.Login(info.szServerAdd, info.lPort, csUserName, info.szPasswd);
				if (lResult >= 0)
				{
					if (pop3.ConnectDataBase() && sql.Connect(sqlinfo))
					{
						swprintf_s(pDlg->m_showinfo[dwID].szAbbreviation, 64, _T("%s"), info.szAbbreviation);
						swprintf_s(pDlg->m_showinfo[dwID].szName, 128, _T("%s-获取UDIL..."), info.szName);
						lResult = pop3.GetMailCount();
						if (pop3.GetUDILs(mapUIDLs, lResult))
						{
							lResult = mapUIDLs.size();
							if (lResult > 0)
							{
								swprintf_s(pDlg->m_showinfo[dwID].szName, 128, _T("%s-接收中..."), info.szName);
								pDlg->m_showinfo[dwID].lTotal = lResult;
								pDlg->m_showinfo[dwID].lStatus = 0;
								strUDIL.clear();
								strUDIL = info.chUIDL;
								if (strUDIL.length() > 0)
								{
									iteuidl = mapUIDLs.begin();
									while (iteuidl != mapUIDLs.end())
									{
										if (iteuidl->second == strUDIL)
										{
											pDlg->m_showinfo[dwID].lCurr = iteuidl->first;
											break;
										}
										iteuidl++;
									}
								}
								else iteuidl = mapUIDLs.begin();
								while (iteuidl != mapUIDLs.end())
								{
									if (WaitForSingleObject(__HEVENT_MAIN_EXIT__, 10L) == WAIT_OBJECT_0)
									{
										OutputDebugStringA("WAIT4 __HEVENT_MAIN_EXIT__\r\n");
										sprintf_s(info.chUIDL, 64, "%s", iteuidl->second.c_str());
										pDlg->GetMailBoxInfo(csUserName, info, 0);
										pop3.QuitDataBase();
										sql.CloseDB();
										break;
									}
									csDebug.Format(_T("%s Count = %d\tTotal = %d\r\n"), info.szName, iteuidl->first, lResult);
									OutputDebugString(csDebug);
									dwTime = GetTickCount64();
									strUDIL.clear();
									strUDIL = iteuidl->second;
									sprintf_s(chDebug, 512, "UIDL: [%s]\r\n", strUDIL.c_str());
									OutputDebugStringA(chDebug);
									if (strUDIL.length() > 0)
									{
										lReturnvalue = pop3.CheckUIDL(strUDIL, strName, info.lSaveDay);
										if (lReturnvalue == MONGO_NOT_FOUND)
										{
											if (pop3.GetEMLFile(iteuidl->first, strUDIL) == 0)
											{
#ifdef _DEBUG
												lType = 0;
#endif
												if (pDlg->MailAnalysis(pop3, sql, smtp, strUDIL, info, chLogPath, lLen, lType) < 0)//邮件解析
												{
													swprintf_s(pDlg->m_showinfo[dwID].szName, 128, _T("%s-解析[%s]失败！"), info.szName, CString(iteuidl->second.c_str()));
													sprintf_s(info.chUIDL, 64, "%s", iteuidl->second.c_str());
													sprintf_s(chDebug, 512, "Analysis [%s] Error!", strUDIL.c_str());
													OutputDebugStringA(chDebug);
													OutputDebugStringA("\r\n");
												}
											}
											else
											{
												swprintf_s(pDlg->m_showinfo[dwID].szName, 128, _T("%s-接收[%s]失败！"), info.szName, CString(iteuidl->second.c_str()));
												sprintf_s(info.chUIDL, 64, "%s", iteuidl->second.c_str());
												if (pDlg->CheckFailedUIDL(strUDIL))
												{
													//TODO: Delete mongo uidl data
													if (pop3.DeleteFromDB(strUDIL))
													{
														sprintf_s(chDebug, 512, "Delete [%s] From MongoDB-UIDL!", strUDIL.c_str());
														OutputDebugStringA(chDebug);
														OutputDebugStringA("\r\n");
													}
												}
												OutputDebugStringA("*************Failed Exit!!!!!!!*************\r\n");
												pop3.QuitDataBase();
												sql.CloseDB();
												break;
											}
										}
										else if (lReturnvalue == MONGO_DELETE)
											mapDelUIDLs.insert(make_pair(iteuidl->first, iteuidl->second));
									}
									else
									{
										OutputDebugString(_T("Can't get UIDL!\r\n"));
										pDlg->GetMailBoxInfo(csUserName, info, 0);
										pop3.QuitDataBase();
										sql.CloseDB();
										break;
									}
									dwTime = GetTickCount64() - dwTime;
									csDebug.Format(_T("Process Time = %d\r\n"), dwTime / 1000);
									OutputDebugString(csDebug);
									pDlg->m_showinfo[dwID].lCurr = iteuidl->first;
									iteuidl++;
									if (iteuidl == mapUIDLs.end())
										memset(info.chUIDL, 0, 64);
								}//end of while
								sql.CloseDB();
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
								lCount = mapDelUIDLs.size();
								if (lCount > 0)
								{
									swprintf_s(pDlg->m_showinfo[dwID].szName, 128, _T("%s-清理中..."), info.szName);
									itedel = mapDelUIDLs.begin();
									pDlg->m_showinfo[dwID].lStatus = j =i = 1;
									pDlg->m_showinfo[dwID].lTotal = lCount;

									while (itedel != mapDelUIDLs.end())
									{
										if (WaitForSingleObject(__HEVENT_MAIN_EXIT__, 10L) == WAIT_OBJECT_0)
										{
											OutputDebugStringA("WAIT5 __HEVENT_MAIN_EXIT__\r\n");
											pDlg->GetMailBoxInfo(csUserName, info, 0);
											pop3.QuitDataBase();
											sql.CloseDB();
											break;
										}
										if (pop3.DelEmail(itedel->first, itedel->second) == SUCCESS)
										{
											csDebug.Format(_T("%s Del-Count = %d\tTotal = %d\r\n"), info.szName, i++, lCount);
											OutputDebugString(csDebug);
										}
										pDlg->m_showinfo[dwID].lCurr = j++;
										itedel++;
									}
								}
								pop3.QuitDataBase();
								swprintf_s(pDlg->m_showinfo[dwID].szName, 128, _T("%s-处理完成！"), info.szName);
								pDlg->GetMailBoxInfo(csUserName, info, 0);
								if (WaitForSingleObject(__HEVENT_MAIN_EXIT__, 500L) == WAIT_OBJECT_0)
								{
									OutputDebugStringA("WAIT6 __HEVENT_MAIN_EXIT__\r\n");
									break;
								}
							}
						}
						else
						{
							swprintf_s(pDlg->m_showinfo[dwID].szName, 128, _T("%s-获取UDIL失败"), info.szName);
							if (WaitForSingleObject(__HEVENT_MAIN_EXIT__, 500L) == WAIT_OBJECT_0)
							{
								OutputDebugStringA("WAIT7 __HEVENT_MAIN_EXIT__\r\n");
								break;
							}
							pDlg->GetMailBoxInfo(csUserName, info, 0);
							break;
						}
					}// end of if
				}
				else
				{
					GetErrorMessage(lResult, szError);
					csDebug.Format(_T("登陆 %s 出现问题[%s]\r\nThread [0x%x] will wait 5sec!\r\n"), 
						info.szName, szError, GetCurrentThreadId());
					pop3.QuitDataBase();
					OutputDebugString(csDebug);
					if (WaitForSingleObject(__HEVENT_MAIN_EXIT__, 5000L) == WAIT_OBJECT_0)
					{
						OutputDebugStringA("WAIT8 __HEVENT_MAIN_EXIT__\r\n");
						break;
					}
					pDlg->GetMailBoxInfo(csUserName, info, 0);
				}
				memset(&pDlg->m_showinfo[dwID], 0, sizeof(ShowInfo));
				pop3.Close();
			}
			else
			{
				csDebug.Format(_T("Thread [0x%x] will wait 5sec!\r\n"), GetCurrentThreadId());
#ifdef _DEBUG
				OutputDebugString(csDebug);
#endif
				
				if (WaitForSingleObject(__HEVENT_MAIN_EXIT__, 5000L) == WAIT_OBJECT_0)
				{
					OutputDebugStringA("WAIT9 __HEVENT_MAIN_EXIT__\r\n");
					break;
				}
			}// end of else
		}// end of while
		memset(&pDlg->m_showinfo[dwID], 0, sizeof(ShowInfo));
	}// end of tyr
	catch (...)
	{
		csDebug.Format(_T("Thread [0x%x] will exit!\r\n"), GetCurrentThreadId());
		OutputDebugString(csDebug);
	}
	return 0;
}

long CReceiveEmailDlg::MailAnalysis(POP3& pop3, CSQLServer& sql, SMTP& smtp, const string& strUIDL, const MailBoxInfo& info, const char* pLogPath,long lLen, long lType)
{
	char chTemp[MAX_PATH] = { 0 };
	long lValue(0);
	BOOL bRet = TRUE;
	CString csUIDL(strUIDL.c_str()), csPath(pop3.GetCurrPath());
	CMailAnalysis ana;
	DWORD dwTime(0);
	ana.SetLogPath(pLogPath, lLen);
	ana.SetAbbreviation(info.szAbbreviation);
	if(ana.LoadFile(csPath, csUIDL)==0)
	{
		ana.SetLogPath(pLogPath,lLen);
		ana.SetClearType(lType);
		dwTime = GetTickCount64();
		do
		{
			if (ana.AnalysisHead() < 0)
			{
				ana.SetClearType(0);
				bRet = FALSE;
				break;
			}
			if (ana.AnalysisBody(ana.GetBoundry(), ana.GetHeadRowCount()) < 0)
			{
				ana.SetClearType(0);
				bRet = FALSE;
				break;
			}
			if (ana.AnalysisBoundary(ana.GetBoundry(), ana.GetAttach()) < 0)
			{
				ana.SetClearType(0);
				bRet = FALSE;
				break;
			}
			if (sql.GetSQLDBType() == 0)
			{
				lValue = sql.SaveToDB(ana.GetEmailItem());
				if (lValue == 0)
				{
					pop3.SaveFileToDB(ana.GetEmailItem());
				}
				else
				{
					pop3.DeleteFromDB(ana.GetEmailItem());
					sql.DeleteFromSQL(ana.GetEmailItem());
					ana.SetClearType(0);
				}
			}
			else
			{
				lValue = sql.SaveToDBOld(ana.GetEmailItem());
				if (lValue != 0)
				{
					sql.DeleteFromSQLOld(ana.GetEmailItem());
					ana.SetClearType(0);
				}
			}
		} while (0);
		if (info.bSendMail)
		{
			char chTemp[MAX_PATH] = { 0 };
			WideCharToMultiByte(CP_ACP, 0, info.szMailAdd, 128, chTemp, MAX_PATH, NULL, NULL);
			smtp.InitSMTPPro();
			smtp.SetReceiver(chTemp);
			smtp.SetCurrPath(pop3.GetCurrPath());
			smtp.AddAttachFileName(strUIDL);
			SendEmail(smtp);
		}
		ana.Clear();
	}
	else bRet = FALSE;
//#ifdef _DEBUG
	dwTime = GetTickCount64() - dwTime;
	CString csDebug;
	csDebug.Format(_T("Analysis Time = %d\tAnalysis [%s] Complete!"), dwTime / 1000, csUIDL);
	OutputDebugString(csDebug);
	OutputDebugString(_T("\r\n"));
//#endif
	if (bRet)
		return 0;
	return -1;
}

void CReceiveEmailDlg::OnLvnItemchangedListMailbox(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	
	*pResult = 0;
}


void CReceiveEmailDlg::OnNMDblclkListMailbox(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	CDialogInfo dlg;
	MailBoxInfo info;
	memset(&info, 0, sizeof(MailBoxInfo));
	CString csName,csTemp;
	NM_LISTVIEW *pNMListView = (NM_LISTVIEW *)pNMHDR;
	int nItem = pNMListView->iItem;
	if (nItem >= 0 && nItem < m_listMailBox.GetItemCount())//判断双击位置是否在有数据的列表项上面
	{
		csName = m_listMailBox.GetItemText(nItem, 1);
		MAILLIST_ITE ite = m_mailList.begin();
		ite = m_mailList.find(csName);
		if (ite != m_mailList.end())
		{
			csTemp.Format(_T("%s"), csName);
			MailBoxInfo info;
			memset(&info, 0, sizeof(MailBoxInfo));
			memcpy_s(&info, sizeof(MailBoxInfo), &(*ite).second, sizeof(MailBoxInfo));
			dlg.SetMailBoxInfo(&csName, &info);
			if (dlg.DoModal() == IDOK)
			{
				if (!bRun)
				{
					if (csTemp != csName)
					{
						MAILLIST_ITE itecheck = m_mailList.begin();
						itecheck = m_mailList.find(csName);
						if (itecheck != m_mailList.end())
						{
							csTemp.Format(_T("已存在[%s]!"), csName);
							AfxMessageBox(csTemp);
						}
						else
						{
							m_mailList.erase(ite);
							m_mailList.insert(make_pair(csName, info));
							InitMailList();
						}
					}
					else
					{
						memset(&(*ite).second, 0, sizeof(MailBoxInfo));
						memcpy_s(&(*ite).second, sizeof(MailBoxInfo), &info, sizeof(MailBoxInfo));
					}
				}
			}
		}
	}
	*pResult = 0;
}


void CReceiveEmailDlg::OnNMRClickListMailbox(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
	if (!bRun)
	{
		NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
		DWORD dwPos = GetMessagePos();
		CPoint point(LOWORD(dwPos), HIWORD(dwPos));

		CMenu menu;
		VERIFY(menu.LoadMenu(IDR_MENU1));
		CMenu* popup = menu.GetSubMenu(0);
		ASSERT(popup != NULL);
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
	*pResult = 0;
}


void CReceiveEmailDlg::OnAdditem()
{
	// TODO:  在此添加命令处理程序代码
	CDialogInfo dlg;
	MailBoxInfo info;
	memset(&info, 0, sizeof(MailBoxInfo));
	CString csName;
	dlg.SetMailBoxInfo(&csName, &info);
	if (dlg.DoModal() == IDOK)
	{
		MAILLIST_ITE ite = m_mailList.begin();
		ite = m_mailList.find(dlg.m_csMailAdd);
		if (ite!=m_mailList.end())
		{
			CString csText;
			csText.Format(_T("已存在[%s]!"), dlg.m_csMailAdd);
			AfxMessageBox(csText);
		}
		else
		{
			m_mailList.insert(make_pair(dlg.m_csMailAdd, info));
			InitMailList();
		}
	}
}


void CReceiveEmailDlg::OnDelitem()
{
	// TODO:  在此添加命令处理程序代码
	POSITION pos = m_listMailBox.GetFirstSelectedItemPosition();
	if (pos)
	{
		int nItem = m_listMailBox.GetNextSelectedItem(pos);
		CString csName = m_listMailBox.GetItemText(nItem, 1);
		MAILLIST_ITE ite = m_mailList.begin();
		ite = m_mailList.find(csName);
		if (ite != m_mailList.end())
		{
			m_mailList.erase(ite);
			InitMailList();
		}
	}
	
}

long CReceiveEmailDlg::SendEmail(SMTP& smtp)
{
	if (smtp.Logon() != 0)
		return -1;
	if (smtp.SendHead() != 0)
		return -1;
	/*if (smtp.SendTextBody() != 0)
	return -1;*/
	if (smtp.SendFileBody() != 0)
		return -1;
	if (smtp.Quit())
		return 0;
	return 0;
}


void CReceiveEmailDlg::OnBnClickedButtonPdf()
{
	// TODO:  在此添加控件通知处理程序代码
	CDialogPDF pdf;
	pdf.SetMongoDBInfo(m_dbinfo);
	pdf.DoModal();
}

void CReceiveEmailDlg::StopEx()
{
	Stop();
	StopMain();
}

BOOL CReceiveEmailDlg::CheckFailedUIDL(const string& strUIDL)
{
	BOOL bRet = TRUE;
	::EnterCriticalSection(&_check_failed_);
	map<string, long>::iterator ite = m_mapFailedUIDLs.begin();
	ite = m_mapFailedUIDLs.find(strUIDL);
	if (ite != m_mapFailedUIDLs.end())
	{
		if (ite->second > 3)
		{
			bRet = FALSE;
			m_mapFailedUIDLs.erase(ite);
		}
		else ite->second++;
	}
	else
	{
		m_mapFailedUIDLs.insert(make_pair(strUIDL, 1));
	}
	::LeaveCriticalSection(&_check_failed_);
	return bRet;
}


BOOL CReceiveEmailDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	//return FALSE;
	return CDialogEx::OnEraseBkgnd(pDC);
}
