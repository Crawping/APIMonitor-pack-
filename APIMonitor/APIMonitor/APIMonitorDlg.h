
// APIMonitorDlg.h : ͷ�ļ�
//

#pragma once
#include "ProcessOperator.h"
#include "ProcessingTransmit.h"
#include "./src/detours.h"
#include <shlwapi.h>
#include <locale.h>
#include "afxcmn.h"
#include "afxwin.h"
#pragma comment(lib,"shlwapi.lib")

//ע�������Զ�����Ϣ
#define WM_RECVMODINFO				WM_USER + 1	
#define	WM_REVCAPIINFO				WM_USER + 2
#define	 WM_TRAPINFO                 WM_USER + 3



//������ʾ������Ϣ
typedef struct _TrapShow
{
	WCHAR*	szModName;
	WCHAR*	szApiName;
	WCHAR*	szInvokedModName;
	DWORD 	dwRetAddr;
	WCHAR*	szParam;
	DWORD	dwParamLen;
}TrapShow, *lpTrapShow;


// CAPIMonitorDlg �Ի���
class CAPIMonitorDlg : public CDialogEx
{
	// ����
public:
	CAPIMonitorDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_APIMONITOR_DIALOG };
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
	afx_msg void		OnOpenProc();
	HANDLE				GeneratePipe(CString szNamePipe);		//�ܵ����ɺ���
	ProcessingList*		m_lpProessing;							//������Ϣ�����������
	HTREEITEM			FindItem(CTreeCtrl& ctrlTree, HTREEITEM hItem, CString strText);
private:
	CMenu				m_Menu;
	CProcessOperator	m_Proc;
	CString				StrPipeName;
	HANDLE				hPipe_1;			// ��ض˷���ָ��Ĺܵ����
	HANDLE				hPipe_2;			// ��ض˽���DLL���ݵĹܵ����

public:
	CTreeCtrl			m_TreeShow;			// ��ʾģ���б�
	CTreeCtrl			m_TreeAPI;			// ��ʾAPI�б�
	HTREEITEM			m_hRoot;
	HTREEITEM			hTestTree;
	CListCtrl			m_ListTrap;		
	CEdit				m_LogEdit;
	CFile				m_FileLogText;
	CFile				m_FileLogBinary;



protected:
	afx_msg LRESULT		OnRecvmodinfo(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT		OnRevcapiinfo(WPARAM wParam, LPARAM lParam);
	DWORD				m_dwModOrder;			// ����m_TreeAPI �����ʾAPI�����οؼ���������ʾ��ǰѡ��ģ�����ţ���API��ʾ���ο�
public:
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	afx_msg void OnNMClickTreeList(NMHDR *pNMHDR, LRESULT *pResult);
	VOID		 InsertTrap(WCHAR* szInvokedMod, WCHAR* szApiName, DWORD dwRetAddr);
	afx_msg void OnNMClickTreeApi(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEditFindapi();
	CEdit			m_EditApi;
protected:
	afx_msg LRESULT OnTrapinfo(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnSavelog();
};
