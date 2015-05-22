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
	CString csAttachmentName;

	void Init()//������ʹ��Init������ʼ��
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
		csAttachmentName.Empty();
	}
};

struct ATTACH
{
	list<CString> lsHead;
	CString csText;
};

struct ATTACH_FILE
{
	CString csFileName;	//�����ļ���
	CString csFilePath;	//�����ļ�·��
	long lSize;			//������С
	CString csMD5;		//����MD5
	CString csGUID;		//����GUID
	long lType;			//0�����ģ�1��������3��ԭʼ����
	CString csAffixType;
	CString csRemoteName;
	CString csLocalFileName;
	CString csFileText;
	int nTime;
	int nPageNum;
	void Init()
	{
		csFileName.Empty();
		csFilePath.Empty();
		lSize = 0;
		csMD5.Empty();
		csGUID.Empty();
		lType = 1;
		csAffixType.Empty();
		csRemoteName.Empty();
		csLocalFileName.Empty();
		csFileText.Empty();
		nTime = -1;
		nPageNum = 0;
	}
};

struct EMAIL_ITEM
{
	long lSn;							//�ʼ����
	CString csUIDL;						//UIDL
	CString csSubject;					//�ʼ�����
	CString csFrom;						//�����õ��ʼ���Դ
	CString csTo;						//������������
	CString csDate;						//�ʼ�������
	CString csTime;						//�ʼ���ʱ��
	CString csContentType;				//��������
	CString csEmailContent;				//�ʼ�����
	CString csEmailContentHTML;			//�ʼ�HTML��ʽ����
	long lHasHtml;						//�Ƿ���HTML����
	long lHasAffix;						//�Ƿ��и���
	CString csRecDate;					//�ʼ���������
	long lStatus;						//״̬
	vector<ATTACH_FILE> vecAttachFiles;	//�����б�
	
	void Init()							//��ʼ������
	{
		lSn = 0;
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