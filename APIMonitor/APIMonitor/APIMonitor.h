
// APIMonitor.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CAPIMonitorApp: 
// �йش����ʵ�֣������ APIMonitor.cpp
//

class CAPIMonitorApp : public CWinApp
{
public:
	CAPIMonitorApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CAPIMonitorApp theApp;
