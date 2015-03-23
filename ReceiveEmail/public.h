#pragma once
#pragma pack(push,1)


enum ErrorCode
{
	SUCCESS = 0,
	SOCKETINIT_ERROR = -1,
	SEND_ERROR = -2,
	RECEIVE_ERROR = -3,
	LOGIN_ERROR = -4,
	NO_INTERNET = -5,
	LOGIN_ERROR_LOCK = -6,
	RETURN_FAIL = -7,
	HOST_UNREACHABLE = -8,
	GETADDRINFO_ERROR = -9
};

const UINT __umymessage__fres_hprogress__(WM_USER + 0x1001);//跟新进度条消息
const UINT __umymessage__kill_hprogress__(WM_USER + 0x1002);//从进度条队列中删除
const UINT __umymessage_api_netcommand__(WM_USER + 0x1003);
const UINT __umymessage__delfres_hprogress__(WM_USER + 0x1004);
const UINT __umymessage__sendcomplete__(WM_USER + 0x1005);
const UINT __umymessage__senduncomplete__(WM_USER + 0x1006);
const UINT __umymessage__anacomplete__(WM_USER + 0x1007);
const UINT __umymessage__anauncomplete__(WM_USER + 0x1008);


struct MongoDBInfo
{
	int  nUseDB;
	char chDBAdd[32];
	char chDBName[32];
	char chTable[32];
	char chUserName[32];
	char chPasswd[32];
};

struct SQLDBInfo
{
	TCHAR szDBAdd[32];
	TCHAR szDBName[32];
	TCHAR szUserName[32];
	TCHAR szPasswd[32];
};


struct MailBoxInfo
{
	TCHAR szName[64];			//中文名称
	TCHAR szServerAdd[64];		//服务器地址
	TCHAR szPasswd[128];		//密码
	TCHAR szAbbreviation[64];	//简称
	long  lPort;				//端口
	BOOL  bSendMail;			//是否转发
	TCHAR szMailAdd[128];		//接收邮箱
	long lStatus;
};

enum RETURN_VALUE
{
	MONGO_NOT_FOUND=-1,
	MONGO_FOUND,
	MONGO_DELETE,
};

#define MAX_BUFFER				20480


enum KeyWords
{
	_DATE_ = 0,
	_SUBJECT_,
	_FROM_,
	_CONTENT_TYPE_,
	_CONTENT_TRANSFER_ENCODING_,
	_X_UIDL_,
	_MESSAGE_ID_,
	_CONTENT_DISPOSITION_,
	_CONTENT_ID_,
	_TO_,
	_REPLY_TO_,
};

enum KeyWords2
{
	_CONTENT_TYPE2_ = 0,
	_CONTENT_TRANSFER_ENCODING2_,
	_CONTENT_DESCRIPTION_,
	_CONTENT_DISPOSITION2_,
};

enum KeyWords3
{
	_CHARSET_ = 0,
	_NAME_,
	_BOUNDARY_,
	_TYPE_
};

enum KeyWords4
{
	_FILE_NAME_ = 0,
	_SIZE_,
};

#define MSAFE_DELETE(ptr) if((ptr)){delete (ptr);(ptr)=NULL;}

enum CODE
{
	UTF8 = 0,
	GB2312,
	BASE64,
	QUOTED_PRINTABLE,
	BINARY,
	GBK,
};


/*
text / plain（纯文本）
text / html（HTML文档）
application / xhtml + xml（XHTML文档）
image / gif（GIF图像）
image / jpeg（JPEG图像）【PHP中为：image / pjpeg】
image / png（PNG图像）【PHP中为：image / x - png】
video / mpeg（MPEG动画）
application / octet - stream（任意的二进制数据）
application / pdf（PDF文档）
application / msword（Microsoft Word文件）
application / vnd.wap.xhtml + xml(wap1.0 + )
application / xhtml + xml(wap2.0 + )
message / rfc822（RFC 822形式）
multipart / alternative（HTML邮件的HTML形式和纯文本形式，相同内容使用不同形式表示）
application / x - www - form - urlencoded（使用HTTP的POST方法提交的表单）
multipart / form - data（同上，但主要用于表单提交时伴随文件上传的场合）
*/
enum CONTENT_TYPE
{
	NO_ATT = -1,
	TEXT_PLAIN = 0,
	TEXT_HTML,
	APP_XHTML_XML,
	IMG_GIF,
	IMG_JPG,
	IMG_PNG,
	VIDEO_MPEG,
	APP_OCTET,
	APP_PDF,
	APP_MSWD,
	APP_WPA1,
	APP_WPA2,
	APP_XHTML_XML2,
	MESSAGE_RFC822,
	MULTI_ALTERNATIVE,
	APP_X_WWW,
	MULTI_FROM_DATA,
	MULTI_MIXED,
	APP_ZIP,
	MULTI_RELATED,
	UNKNOWN_TYPE,
	APP_MSEX,
	APP_MSPT,
};


struct ForwardSet
{
	char srvadd[64];
	char username[64];
	char pass[128];
	char to[512];
	char from[128];
};

struct ProcessInfo
{
	double dPercent;
	long lCurrtentProcess;
	TCHAR szMailAdd[64];
};