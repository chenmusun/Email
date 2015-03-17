#pragma once
#pragma pack(push,1)
#include <afx.h>
#include <list>
#include <vector>
using namespace std;

struct BOUNDARY_HEAD
{
	long lContentType;
	long lCharset;
	long lEncode;
	CString csContentType;
	CString csName;
	CString csCharset;
	CString csBoundary;
	CString csContentDisposition;
	CString csFilename;
	CString csContentDescription;
	CString csEncoding;

	void Init()//声明后使用Init函数初始化
	{
		lContentType = -1;
		lCharset = -1;
		lEncode = -1;
		csContentType.Empty();
		csName.Empty();
		csCharset.Empty();
		csBoundary.Empty();
		csContentDisposition.Empty();
		csFilename.Empty();
		csContentDescription.Empty();
		csEncoding.Empty();
	}
};

struct ATTACH
{
	list<CString> lsHead;
	CString csText;
};

struct ATTACH_FILE
{
	CString csFileName;	//附件文件名
	CString csFilePath;	//附件文件路径
	long lSize;			//附件大小
	CString csMD5;		//附件MD5
	CString csGUID;		//附件GUID
	long lType;			//0：正文；1：附件
	CString csAffixType;
	void Init()
	{
		csFileName.Empty();
		csFilePath.Empty();
		lSize = 0;
		csMD5.Empty();
		csGUID.Empty();
		lType = 1;
		csAffixType.Empty();
	}
};

struct EMAIL_ITEM
{
	long lSn;							//邮件序号
	CString csGUDI;
	CString csUIDL;						//UIDL
	CString csSubject;					//邮件主题
	CString csFrom;						//解析得到邮件来源
	CString csTo;						//配置中邮箱简称
	CString csDate;						//邮件中日期
	CString csTime;						//邮件中时间
	CString csContentType;				//正文类型
	CString csEmailContent;				//邮件正文
	CString csEmailContentHTML;			//邮件HTML格式正文
	long lHasHtml;						//是否有HTML正文
	long lHasAffix;						//是否有附件
	CString csRecDate;					//邮件接收日期
	long lStatus;						//状态
	vector<ATTACH_FILE> vecAttachFiles;	//附件列表
	
	void Init()							//初始化函数
	{
		lSn = 0;
		csGUDI.Empty();
		csUIDL.Empty();
		csSubject.Empty();
		csFrom.Empty();
		csTo.Empty();
		csDate.Empty();
		csTime.Empty();
		csContentType.Empty();
		csEmailContentHTML.Empty();
		lHasHtml = 0;
		lHasAffix = 0;
		csRecDate.Empty();
		lStatus = 0;
		vector<ATTACH_FILE>::iterator ite= vecAttachFiles.begin();
		while (ite != vecAttachFiles.end())
		{
			(*ite).csFileName.Empty();
			(*ite).csFilePath.Empty();
			(*ite).csGUID.Empty();
			(*ite).csMD5.Empty();
			(*ite).lSize = 0;
			(*ite).lType = 1;
			ite++;
		}
		vecAttachFiles.clear();
	}
};