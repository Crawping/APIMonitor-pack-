#pragma once
#include "ProcessingTransmit.h"
#include "./src/detours.h"
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")
// CProcessOperator �Ի���




class CProcessOperator : public CDialogEx
{
	DECLARE_DYNAMIC(CProcessOperator)

public:
	CProcessOperator(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CProcessOperator();

	// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PROC };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	afx_msg			void OnBnClickedButtonOK();
	afx_msg			void OnBnClickedButtonOpen();
	WCHAR			szFile[MAX_PATH];
	BOOL			bStartedHook;		//�Ƿ�һ���ؾ�HOOK SS�ļ��е�API

};
