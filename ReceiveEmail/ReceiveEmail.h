
// ReceiveEmail.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CReceiveEmailApp: 
// �йش����ʵ�֣������ ReceiveEmail.cpp
//

class CReceiveEmailApp : public CWinApp
{
public:
	CReceiveEmailApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CReceiveEmailApp theApp;