
// SaerPackUIDlg.h : ͷ�ļ�
//

#pragma once


// CSaerPackUIDlg �Ի���

#include "Config.h"




class CSaerPackUIDlg : public CDialogEx
{
// ����
public:
	CSaerPackUIDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SAERPACKUI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonPack();
	BOOL	bEnabled;





	afx_msg void OnBnClickedButtonCheck();
	afx_msg void OnBnClickedCheckCompress();
	afx_msg void OnBnClickedButtonVcode();
};
