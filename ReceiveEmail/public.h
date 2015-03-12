#pragma once
#pragma pack(push,1)


enum ErrorCode
{
	SOCKETINIT_ERROR = -1,
	SEND_ERROR = -2,
	RECEIVE_ERROR = -3,
	LOGIN_ERROR = -4,
	NO_INTERNET = -5,
	LOGIN_ERROR_LOCK = -6,
	RETURN_FAIL = -7,
	HOST_UNREACHABLE = -8,
	SUCCESS = 0
};

const UINT __umymessage__fres_hprogress__(WM_USER + 0x1001);//���½�������Ϣ
const UINT __umymessage__kill_hprogress__(WM_USER + 0x1002);//�ӽ�����������ɾ��
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


struct MailBoxInfo
{
	TCHAR szName[64];			//��������
	TCHAR szServerAdd[64];		//��������ַ
	TCHAR szPasswd[128];		//����
	TCHAR szAbbreviation[64];	//���
	long  lPort;				//�˿�
	BOOL  bSendMail;			//�Ƿ�ת��
	TCHAR szMailAdd[128];		//��������
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
text / plain�����ı���
text / html��HTML�ĵ���
application / xhtml + xml��XHTML�ĵ���
image / gif��GIFͼ��
image / jpeg��JPEGͼ�񣩡�PHP��Ϊ��image / pjpeg��
image / png��PNGͼ�񣩡�PHP��Ϊ��image / x - png��
video / mpeg��MPEG������
application / octet - stream������Ķ��������ݣ�
application / pdf��PDF�ĵ���
application / msword��Microsoft Word�ļ���
application / vnd.wap.xhtml + xml(wap1.0 + )
application / xhtml + xml(wap2.0 + )
message / rfc822��RFC 822��ʽ��
multipart / alternative��HTML�ʼ���HTML��ʽ�ʹ��ı���ʽ����ͬ����ʹ�ò�ͬ��ʽ��ʾ��
application / x - www - form - urlencoded��ʹ��HTTP��POST�����ύ�ı���
multipart / form - data��ͬ�ϣ�����Ҫ���ڱ��ύʱ�����ļ��ϴ��ĳ��ϣ�
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
};


struct ForwardSet
{
	char srvadd[64];
	char username[64];
	char pass[128];
	char to[512];
	char from[128];
};