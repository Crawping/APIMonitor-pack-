
// APIMonitorDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "APIMonitor.h"
#include "APIMonitorDlg.h"
#include "afxdialogex.h"



//�����߳�,����DLL���Ͷ��������ݣ�д��list
DWORD WINAPI ThreadProcOfWork(LPVOID lpPara);
//ģ�����߳�
DWORD  WINAPI ThreadProcMain(LPVOID lpPara);
//UI��ʾ�̣߳���list�е�������ʾ��UI�ؼ���
DWORD  WINAPI ThreadProcUI(LPVOID lpPara);



#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnRecvmod(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnTrapinfo(WPARAM wParam, LPARAM lParam);
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)


END_MESSAGE_MAP()


// CAPIMonitorDlg �Ի���



CAPIMonitorDlg::CAPIMonitorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_APIMONITOR_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAPIMonitorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_LIST, m_TreeShow);
	DDX_Control(pDX, IDC_LIST_TRAP, m_ListTrap);
	DDX_Control(pDX, IDC_TREE_API, m_TreeAPI);
	DDX_Control(pDX, IDC_EDIT_SHOW, m_LogEdit);
	DDX_Control(pDX, IDC_EDIT_FINDAPI, m_EditApi);
}

BEGIN_MESSAGE_MAP(CAPIMonitorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_PROC, &CAPIMonitorDlg::OnOpenProc)
	ON_MESSAGE(WM_RECVMODINFO, &CAPIMonitorDlg::OnRecvmodinfo)
	ON_MESSAGE(WM_REVCAPIINFO, &CAPIMonitorDlg::OnRevcapiinfo)
	ON_WM_COPYDATA()

	ON_NOTIFY(NM_CLICK, IDC_TREE_LIST, &CAPIMonitorDlg::OnNMClickTreeList)
	ON_NOTIFY(NM_CLICK, IDC_TREE_API, &CAPIMonitorDlg::OnNMClickTreeApi)
	ON_EN_CHANGE(IDC_EDIT_FINDAPI, &CAPIMonitorDlg::OnEnChangeEditFindapi)
	ON_MESSAGE(WM_TRAPINFO, &CAPIMonitorDlg::OnTrapinfo)
	ON_COMMAND(ID_SaveLog, &CAPIMonitorDlg::OnSavelog)
END_MESSAGE_MAP()


// CAPIMonitorDlg ��Ϣ�������

BOOL CAPIMonitorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	m_Menu.LoadMenuW(IDR_MENU1);
	SetMenu(&m_Menu);

	//�����ܵ���ǰ׺
	StrPipeName = L"\\\\.\\pipe\\NamePipe_APIMonitor";
	//Ϊ�˴��ݵ�UI�߳��е��ò����࣬����ָ��
	m_lpProessing = new ProcessingList;

	CRect rect;

	// ��ȡ��������б���ͼ�ؼ���λ�úʹ�С   
	m_ListTrap.GetClientRect(&rect);

	// Ϊ�б���ͼ�ؼ����ȫ��ѡ�к�դ����   
	m_ListTrap.SetExtendedStyle(m_ListTrap.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// Ϊ�б���ͼ�ؼ����4��   
	m_ListTrap.InsertColumn(0, _T("���õ�ַ"), LVCFMT_CENTER,	rect.Width() / 4, 0);
	m_ListTrap.InsertColumn(1, _T("����ģ��"), LVCFMT_CENTER,	rect.Width() / 4, 1);
	m_ListTrap.InsertColumn(2, _T("API����"), LVCFMT_CENTER,	rect.Width() / 4, 2);
	m_ListTrap.InsertColumn(3, _T("���ô���"), LVCFMT_CENTER,	rect.Width() / 4, 3);




	//CString szTest1 = L"FYCK1";
	//CString szTest2 = L"FUCK2";
	//szTest1 += "\r\n";
	//szTest2 += "\r\n";

	//for (DWORD i = 0; i < 100; i++)
	//{
	//	m_LogEdit.LineScroll(m_LogEdit.GetLineCount());
	//	m_LogEdit.SetSel(-1);
	//	m_LogEdit.ReplaceSel(szTest1);
	//	m_LogEdit.LineScroll(m_LogEdit.GetLineCount());
	//	m_LogEdit.SetSel(-1);
	//	m_LogEdit.ReplaceSel(szTest2);
	//}



	//DWORD dwRow;
	//m_ListTrap.SetItemText(m_ListTrap.InsertItem(0, _T("����1")),
	//	2, _T("fuck1"));
	//dwRow = m_ListTrap.InsertItem(1, _T("����2"));
	//dwRow =  m_ListTrap.InsertItem(2, _T("����3"));
	//dwRow = m_ListTrap.InsertItem(3, _T("����4"));

	//dwRow = m_ListTrap.GetItemCount();
	//CString szTest =  m_ListTrap.GetItemText(0, 2);



	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CAPIMonitorDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CAPIMonitorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CAPIMonitorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CAPIMonitorDlg::OnOpenProc()
{
	// TODO: �ڴ���������������
	if (m_Proc.DoModal() == IDOK)
	{
		//////////////////////////////////////////////////////////////////////////
		//Ԥ��������������̣߳����ӹܵ�
		//�ڴ˴����������ܵ������ƣ����Ŵ������ܵ�
		CString	szNamePipe_1;
		CString	szNamePipe_2;

		szNamePipe_1.Format(L"%s_1", StrPipeName);
		szNamePipe_2.Format(L"%s_2", StrPipeName);

		hPipe_1 = GeneratePipe(szNamePipe_1);	//���߳�
		hPipe_2 = GeneratePipe(szNamePipe_2);	//UI�߳�

		if (!hPipe_1 || !hPipe_2)	return ;

		//���뱣���߳�����,һ���ǹܵ������һ�������������ָ��  
		//Pipe | Ptr
		PDWORD	pThreadData = new DWORD[3];
		pThreadData[0] = (DWORD)hPipe_2;
		pThreadData[1] = (DWORD)m_lpProessing;
		pThreadData[2] = (DWORD)this;
		//���������ܵ����ͻ������ӳɹ��󣬿����߳�ȥ����ĳ���ܵ���ͨ��
		CreateThread(NULL, NULL, ThreadProcOfWork, (LPVOID)pThreadData, NULL, NULL);
	//	CreateThread(NULL, NULL, ThreadProcMain, (LPVOID)hPipe_1, NULL, NULL);


		
		WCHAR szDirPath[MAX_PATH] = { 0 };
		WCHAR szLogText[MAX_PATH] = { 0 };
		WCHAR szLogBinary[MAX_PATH] = { 0 };
		CString szTimeOrder;
		CTime	tm;

		tm = CTime::GetCurrentTime();
		szTimeOrder = tm.Format("%y-%m-%d");
		szTimeOrder.Format(L"%s-%d", 
			tm.Format("%y-%m-%d"),
			tm.GetSecond() + tm.GetMinute() * 60);

		GetModuleFileName(NULL, szDirPath, MAX_PATH);
		PathRemoveFileSpec(szDirPath);


		BOOL bRootDir = PathIsRoot(szDirPath);
		if (bRootDir)
		{//���Ǹ�Ŀ¼�����üӷ�б��
		 //	wcscat_s(szDirPath, MAX_PATH, L"HookAPILibrary.dll");

			wsprintf(szLogText, L"%s%s\\%s(%s).txt", szDirPath,
				FILE_DIR,
				PathFindFileName(m_Proc.szFile),
				szTimeOrder);

			wsprintf(szLogBinary, L"%s%s\\%s(%s).log", szDirPath,
				FILE_DIR,
				PathFindFileName(m_Proc.szFile),
				szTimeOrder);


			wcscat_s(szDirPath, FILE_DIR);

		}
		else
		{
			//	wcscat_s(szDirPath, MAX_PATH, L"\\HookAPILibrary.dll");
			wsprintf(szLogText, L"%s\\%s\\%s(%s).txt", szDirPath,
				FILE_DIR,
				PathFindFileName(m_Proc.szFile),
				szTimeOrder);


			wsprintf(szLogBinary, L"%s\\%s\\%s(%s).log", szDirPath,
				FILE_DIR,
				PathFindFileName(m_Proc.szFile),
				szTimeOrder);


			wcscat_s(szDirPath, L"\\");
			wcscat_s(szDirPath, FILE_DIR);

		}


		CFileFind m_sFileFind;
		//�ж�Ŀ¼�Ƿ񴴽���
		if (!m_sFileFind.FindFile(szDirPath))
		{
			CreateDirectory(szDirPath, NULL);
		}


		if (m_FileLogText.Open(szLogText, CFile::modeCreate | CFile::modeReadWrite) == FALSE) exit(0x0);

		char szVarName[MAX_BUF_SIZE] = { 0 };

		WideCharToMultiByte(CP_ACP, NULL, L"����ģ�����˳��:\r\n", -1, szVarName, _countof(szVarName), NULL, FALSE);

		m_FileLogText.Write(szVarName, strlen(szVarName));
		
		if (m_FileLogBinary.Open(szLogBinary, CFile::modeCreate | CFile::modeReadWrite) == FALSE) exit(0x1);


	}

}

//////////////////////////////////////////////////////////////////////////
//	������szNamePipeΪ���ֵ������ܵ�
//	������
//	char*	szNamePipe				�����ܵ��ַ�����ָ��
//	����ֵ�������ɹ������ش����ɹ��ľ��,����NULL
//////////////////////////////////////////////////////////////////////////

HANDLE	CAPIMonitorDlg::GeneratePipe(CString szNamePipe)
{
	HANDLE hPipe = CreateNamedPipe(szNamePipe, PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES, MAX_BUF_SIZE, MAX_BUF_SIZE, NMPWAIT_WAIT_FOREVER, 0);

	if (!hPipe)	return NULL;

	if (ConnectNamedPipe(hPipe, NULL) == NULL)
	{
		CloseHandle(hPipe);//�رչܵ�
		return	NULL;
	}

	return hPipe;
}




//////////////////////////////////////////////////////////////////////////
//	���̸߳������dll���������ݣ����չ���д��list��
//	ע����list�Ĳ���һ��Ҫ���û���������ֹUI��ʾ�̵߳Ķ�ȡ��������
//	������LPVOID lpPara				�����ؼ���Ϣ�Ľṹ�壨�����list�������ָ�룩
//////////////////////////////////////////////////////////////////////////

DWORD WINAPI ThreadProcOfWork(LPVOID lpPara)
{
	LPBYTE					pBuff = new BYTE[MAX_BUF_SIZE];
	DWORD					dwBytesOfRead;
	HANDLE					hPipe_2 = (HANDLE)(*(PDWORD)lpPara);
	DWORD					dwRet;
	ProcessingList*			lpProcessing = (ProcessingList*)(*(PDWORD)((DWORD)lpPara + 4));
	CAPIMonitorDlg*			pMainWindows = (CAPIMonitorDlg*)(*(PDWORD)((DWORD)lpPara + 8));
	lpModList				lpModule = lpProcessing->m_pModList;
	lpTrapInfo				lpCurTrap;
	WCHAR*					szInvokedModName;
	WCHAR*					szApiName;
	DWORD					dwType;
	CString					szTrap;
	lpTrapShow				lpTrap;

	ZeroMemory(pBuff, MAX_BUF_SIZE);

	do
	{
		if (dwRet = ReadFile(hPipe_2, pBuff, MAX_BUF_SIZE, &dwBytesOfRead, NULL))
		{
			// BUG����List����ǰ���л�����������
			dwType = lpProcessing->GetMsgInfo((lpPacketInfo)pBuff);

			switch (dwType)
			{
			case MOD_MSG:
				break;
			case API_MSG:			
				break;
			case FINISH_MODMSG:
	
				::SendMessage(pMainWindows->m_hWnd, WM_RECVMODINFO, 0, 0);
				break;
			
			case  FINISH_APIMSG:
				::SendMessage(pMainWindows->m_hWnd, WM_REVCAPIINFO, 0, 0);
				break;

			case TRAP_MSG:
				//1. ���ݰ���Ϣ����
				lpCurTrap = (lpTrapInfo)((lpPacketInfo)pBuff)->Data;
				szInvokedModName = lpProcessing->GetInvokedModName(lpCurTrap->dwRetAddr);
				szApiName = lpProcessing->GetApiName(lpCurTrap->dwModIndex, lpCurTrap->dwApiIndex);
				
				//1.1 ��װ��Ϣ��������Ϣ����
				lpTrap = new TrapShow;
				lpTrap->szModName			= lpModule[lpCurTrap->dwModIndex].szModName;
				lpTrap->szInvokedModName	= szInvokedModName;
				lpTrap->szApiName			= szApiName;
				lpTrap->dwRetAddr			= lpCurTrap->dwRetAddr;
				lpTrap->dwParamLen			= lpCurTrap->dwLength;
				if (lpCurTrap->dwLength != 0)
				{
					lpTrap->szParam = (WCHAR*)VirtualAlloc(NULL, lpTrap->dwParamLen, MEM_COMMIT, PAGE_READWRITE);

					wcscpy_s(lpTrap->szParam, lpTrap->dwParamLen, (WCHAR*)lpCurTrap->byPara);
				}
				

				//2. ������ʾ����Ϣ
				::SendMessage(pMainWindows->m_hWnd, WM_TRAPINFO, 0, (LPARAM)lpTrap);

				break;


			default:
				break;
			}

		}

	} while (dwRet);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
//	ģ�����̹߳���������ָ���DLL
//	����20��ָ��
//	������
//	LPVOID lpPara	- �ܵ����
//////////////////////////////////////////////////////////////////////////

DWORD  WINAPI ThreadProcMain(LPVOID lpPara)
{
	HANDLE	hPipe_1 = (HANDLE)lpPara;
	DWORD	dwNumOfWritten;
	DWORD	dwPorcessID;
	CString	szTest;


	dwPorcessID = GetCurrentProcessId();
	OutputDebugString(L"[��ض�]����ܵ�1�����߳��Ѿ�����");

	for (DWORD dwIndex = 0; dwIndex < 20; dwIndex++)
	{
		szTest.Format(L"%s_ID:%d_Index:%d", L"[���߳�]����ָ��", dwPorcessID, dwIndex);
		WriteFile(hPipe_1, szTest, sizeof(WCHAR)*(szTest.GetLength() + 1), &dwNumOfWritten, NULL);
	}

	DisconnectNamedPipe(hPipe_1);
	CloseHandle(hPipe_1);
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//	UI��ʾ�߳�,��Modlist��ApiList������ʾ
//	����:
//	LPVOID lpPara - ����Ÿ����ڵ�ָ��
//////////////////////////////////////////////////////////////////////////

DWORD  WINAPI ThreadProcUI(LPVOID lpPara)
{
	CAPIMonitorDlg* pMainWindows = (CAPIMonitorDlg*)lpPara;
	

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//	��MOD_MSG���͵�ͨ�Ž���UI�Ĵ���
//////////////////////////////////////////////////////////////////////////

afx_msg LRESULT CAPIMonitorDlg::OnRecvmodinfo(WPARAM wParam, LPARAM lParam)
{
	lpModList lpCurModlist = this->m_lpProessing->m_pModList;
	HTREEITEM hitem;

	m_TreeShow.DeleteAllItems();

	for (DWORD dwIndex = 0; dwIndex < this->m_lpProessing->m_dwModListLen; dwIndex++)
	{
		if (lpCurModlist[dwIndex].bActive)
		{
			hitem = m_TreeShow.InsertItem(lpCurModlist[dwIndex].szModName, 0, 0);
			m_TreeShow.SetItemData(hitem, lpCurModlist[dwIndex].dwModIndex);
		}
	}

	UpdateData(TRUE);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
//	��API_MSG��Ϣ���ͽ��д�����API��ӵ���ӦMOD�ĸ��ڵ���
//////////////////////////////////////////////////////////////////////////

afx_msg LRESULT CAPIMonitorDlg::OnRevcapiinfo(WPARAM wParam, LPARAM lParam)
{
	//�������ڵ㣬�ҵ���Ӧ��mod����
	lpModList		lpCurModlist = this->m_lpProessing->m_pModList;
	lpPacketApiInfo lpCurApiList = NULL;
	HTREEITEM		hInsert;

	m_TreeAPI.DeleteAllItems();

	m_hRoot =		m_TreeAPI.GetRootItem();

		if (lpCurModlist[m_dwModOrder].bActive && lpCurModlist[m_dwModOrder].lpApiList)
		{
			lpCurApiList = lpCurModlist[m_dwModOrder].lpApiList;

			for (DWORD dwApiIndex = 0;dwApiIndex < lpCurModlist[m_dwModOrder].dwApiListLen; dwApiIndex++)
			{
				if (lpCurApiList[dwApiIndex].bActive)
				{
				//	hInsert = FindItem(m_TreeAPI, m_hRoot, lpCurModlist[m_dwModOrder].szModName);
					hInsert = m_TreeAPI.InsertItem(lpCurApiList[dwApiIndex].szApiName, 0, 0);
					m_TreeAPI.SetItemData(hInsert, lpCurApiList[dwApiIndex].dwApiIndex);
					m_TreeAPI.SetCheck(hInsert, TRUE);
				}
			}
		}
	

	UpdateData(TRUE);

	return 0;
}



//////////////////////////////////////////////////////////////////////////
//	��Trap����Ϣ����������List��
//	������
//	WCHAR*			szInvokedMod	- ����ģ�������
//	WCHAR*			szApiName		- �����õ�API����
//	DWORD			dwRetAddr		- ���õ�ַ
//////////////////////////////////////////////////////////////////////////

VOID	CAPIMonitorDlg::InsertTrap(WCHAR* szInvokedMod, WCHAR* szApiName, DWORD dwRetAddr)
{

	//1. ���¼��,��������
	DWORD	dwItemCount = m_ListTrap.GetItemCount();
	CString	szInvokedAddr;
	CString	szInvokedModName;
	CString	szInvokedCount;
	DWORD	dwTransferdAddr;
	DWORD	dwInvokedCount;
	DWORD	dwCurRow;

	if (szInvokedMod == NULL)
		for (DWORD dwRow = 0; dwRow < dwItemCount; dwRow++)
	{
		szInvokedAddr		= m_ListTrap.GetItemText(dwRow, 0);
		szInvokedModName	= m_ListTrap.GetItemText(dwRow, 1);
		szInvokedCount		= m_ListTrap.GetItemText(dwRow, 3);
		dwTransferdAddr		= wcstol(szInvokedAddr, NULL, 16);

		if (dwTransferdAddr == dwRetAddr)
		{
			dwInvokedCount = wcstol(szInvokedCount, NULL, 10);
			dwInvokedCount++;
			szInvokedCount.Format(L"%d", dwInvokedCount);
			m_ListTrap.SetItemText(dwRow, 3, szInvokedCount);
			return;
		}	
	}
	else
	{
		for (DWORD dwRow = 0; dwRow < dwItemCount; dwRow++)
		{
			szInvokedAddr = m_ListTrap.GetItemText(dwRow, 0);
			szInvokedModName = m_ListTrap.GetItemText(dwRow, 1);
			szInvokedCount = m_ListTrap.GetItemText(dwRow, 3);

			dwTransferdAddr = wcstol(szInvokedAddr, NULL, 16);


			if (wcscmp(szInvokedModName, szInvokedMod) == 0 &&
				dwTransferdAddr == dwRetAddr)
			{//�������������и���
				dwInvokedCount = wcstol(szInvokedCount, NULL, 10);
				dwInvokedCount++;
				szInvokedCount.Format(L"%d", dwInvokedCount);
				m_ListTrap.SetItemText(dwRow, 3, szInvokedCount);
				return;
			}
		}//for
	}

	 //2. ��������
	szInvokedAddr.Format(L"0x%x", dwRetAddr);

	dwCurRow = m_ListTrap.InsertItem(dwItemCount, szInvokedAddr);
	if(szInvokedMod != NULL)
	m_ListTrap.SetItemText(dwCurRow, 1, szInvokedMod);
	else
	m_ListTrap.SetItemText(dwCurRow, 1,L"δ֪ģ��");

	m_ListTrap.SetItemText(dwCurRow, 2, szApiName);
	m_ListTrap.SetItemText(dwCurRow, 3, L"1");
		
	return ;

}




//////////////////////////////////////////////////////////////////////////
//	����TreeControl�ؼ����ҵ���Ӧ�ڵ�ľ��
//	����:
//	CTreeCtrl& ctrlTree - ���οؼ�����������
//	HTREEITEM hItem		- ��ǰ��ʼ�����ڵ�ľ��
//	CString strtext		- ƥ��ֵ
//	����ֵ��	�ɹ�����ƥ��ڵ���������NULL
//////////////////////////////////////////////////////////////////////////


HTREEITEM	CAPIMonitorDlg::FindItem(CTreeCtrl& ctrlTree, HTREEITEM hItem, CString strText)
{
	HTREEITEM  hFind;

	//������ֱ�ӷ���NULL 
	if (hItem == NULL)
		return  NULL;

	//�������� 
	while (hItem != NULL)
	{
		//��ǰ�ڵ㼴������ҽڵ� 
	/*	if (ctrlTree.GetItemText(hItem) == strText)
			return  hItem;*/

		if (_wcsnicmp(strText, ctrlTree.GetItemText(hItem), strText.GetLength()) == 0)
			return  hItem;


		//���ҵ�ǰ�ڵ���ӽڵ� 
		if (ctrlTree.ItemHasChildren(hItem))
		{
			hItem = ctrlTree.GetChildItem(hItem);
			//�ݹ���ò����ӽڵ��½ڵ�

			hFind = FindItem(ctrlTree, hItem, strText);
			if (hFind)
			{
				return  hFind;
			}
			else
			{
				//�ӽڵ���δ��������ڵ㣬���������ֵܽڵ�
				hItem = ctrlTree.GetNextSiblingItem(ctrlTree.GetParentItem(hItem));
			}
		}
		else
		{
			//�����ӽڵ㣬���������ֵܽڵ�
			hItem = ctrlTree.GetNextSiblingItem(hItem);
		}
	}

	return hItem;
}




BOOL CAPIMonitorDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	WCHAR				szTrap[MAX_BUF_SIZE];
	lpTrapInfo			lpCurTrap =  (lpTrapInfo)pCopyDataStruct->lpData;

	wsprintf(szTrap, L"Mod:%s ApiIndex:%s RetAddr:%x", m_lpProessing->m_pModList[lpCurTrap->dwModIndex].szModName,
		m_lpProessing->m_pModList[lpCurTrap->dwModIndex].lpApiList[lpCurTrap->dwApiIndex].szApiName,
		lpCurTrap->dwRetAddr);

	return CDialogEx::OnCopyData(pWnd, pCopyDataStruct);
}



//////////////////////////////////////////////////////////////////////////
//	ʵ�ֹ��̣�ͨ��HitTest��ȡ���ָ��λ�õĽڵ������Ӷ���ȡ�ýڵ���ַ���
//////////////////////////////////////////////////////////////////////////

void CAPIMonitorDlg::OnNMClickTreeList(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CPoint pt;
	UINT nFlags;

	 pt = GetCurrentMessage()->pt; //��ȡ��ǰ�������Ϣ�������  
	m_TreeShow.ScreenToClient(&pt);      //��������Ļ���꣬ת�������οؼ��Ŀͻ������� 

	HTREEITEM	hItem = m_TreeShow.HitTest(pt, &nFlags);


	// û�й�ѡ������ʾ��ģ���µ�API,û����ѡ��
	if ((hItem != NULL) && (TVHT_ONITEM & nFlags))
	{
		HTREEITEM hSelected;
		// ��ʾ��ģ���API�б�
		// ��Ҫ��API�б�ĸ�ѡ��״̬���й�ѡ
		m_TreeShow.Select(hItem, TVGN_CARET);
		hSelected = m_TreeShow.GetSelectedItem();
		m_dwModOrder = m_TreeShow.GetItemData(hSelected);

		// ���Ϊ0���������������ģ�飬������������������
		if (m_dwModOrder == 0)	return;

		SendMessage(WM_REVCAPIINFO, NULL, NULL);
	}


	if ((hItem != NULL) && (TVHT_ONITEMSTATEICON  & nFlags))	
	{
		BOOL			bChecked = m_TreeShow.GetCheck(hItem);
		HTREEITEM		hSelected;

		m_TreeShow.Select(hItem, TVGN_CARET);
		hSelected = m_TreeShow.GetSelectedItem();
		m_dwModOrder = m_TreeShow.GetItemData(hSelected);

		// ���Ϊ0���������������ģ�飬������������������
		if (m_dwModOrder == 0)	return;

		if (bChecked)
		{//����UnHook
			m_lpProessing->SendHookMod(hPipe_1, m_TreeShow.GetItemText(hItem).GetBuffer(), FALSE);
		}
		else
		{//����Hook

			m_lpProessing->SendHookMod(hPipe_1, m_TreeShow.GetItemText(hItem).GetBuffer(), TRUE);	
		}

	}

	*pResult = 0;
}






void CAPIMonitorDlg::OnNMClickTreeApi(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CPoint pt;
	UINT nFlags;

	pt = GetCurrentMessage()->pt; //��ȡ��ǰ�������Ϣ�������  
	m_TreeAPI.ScreenToClient(&pt);      //��������Ļ���꣬ת�������οؼ��Ŀͻ������� 

	HTREEITEM	hItem = m_TreeAPI.HitTest(pt, &nFlags);
	DWORD		dwApiIndex;


	//������Ҳ����˸�ѡ��
	if ((hItem != NULL) && (TVHT_ONITEMSTATEICON  & nFlags))
	{
		BOOL			bChecked = m_TreeAPI.GetCheck(hItem);
		HTREEITEM		hSelected;

		m_TreeAPI.Select(hItem, TVGN_CARET);
		hSelected = m_TreeAPI.GetSelectedItem();
		dwApiIndex = m_TreeAPI.GetItemData(hSelected);

		if (!bChecked)
		{//���в�����
			m_lpProessing->SendFilteredApi(hPipe_1, m_dwModOrder, dwApiIndex, FALSE);
		}
		else
		{//���й���
			m_lpProessing->SendFilteredApi(hPipe_1, m_dwModOrder, dwApiIndex, TRUE);
		}

	}




	*pResult = 0;
}


void CAPIMonitorDlg::OnEnChangeEditFindapi()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString			szEdit;
	CString			szSelected;
	HTREEITEM		hSelected;


	((CEdit*)GetDlgItem(IDC_EDIT_FINDAPI))->GetWindowTextW(szEdit);

	if (szEdit.IsEmpty()) return;

	hSelected = FindItem(m_TreeAPI, m_TreeAPI.GetRootItem(), szEdit);

	m_TreeAPI.SelectItem(NULL);

	m_TreeAPI.SelectItem(hSelected);


	szSelected = m_TreeAPI.GetItemText(hSelected);

	if (wcscmp(szSelected, szEdit) == 0)
	{
		m_TreeAPI.SetFocus();
	}


//	m_TreeShow.Select(m_hSelected, TVIS_SELECTED);
//	m_TreeShow.SetItemState(m_hSelected, TVIS_SELECTED, TVIS_SELECTED);
	UpdateData(TRUE);


}




//////////////////////////////////////////////////////////////////////////
//	�����������Ϣ������
//	������
//	LPARAM lParam		 - �����ؼ����ݵĽṹ��ָ��
//////////////////////////////////////////////////////////////////////////

afx_msg LRESULT CAPIMonitorDlg::OnTrapinfo(WPARAM wParam, LPARAM lParam)
{
	lpTrapShow	lpTrap = (lpTrapShow)lParam;
	CString		szTrap;

	if (lpTrap->szInvokedModName == NULL)
	{
		lpTrap->szInvokedModName = L"δ֪ģ��";
	}

	InsertTrap(lpTrap->szInvokedModName, lpTrap->szApiName, lpTrap->dwRetAddr);

	//3. ��ʾ����־��
	
	if (m_LogEdit.GetLineCount() > 300)
	{
		m_LogEdit.SetWindowTextW(L"");
	}

	if(lpTrap->dwParamLen != 0x0)
	szTrap.Format(L"Trapped api : API - %s<%s> . Called from 0x%x<%s> ,Param: %s\r\n",
		lpTrap->szApiName, lpTrap->szModName,
		lpTrap->dwRetAddr, lpTrap->szInvokedModName,
		lpTrap->szParam);
	else
	szTrap.Format(L"Trapped api : API - %s<%s> . Called from 0x%x<%s> \r\n",
		lpTrap->szApiName, lpTrap->szModName,
		lpTrap->dwRetAddr, lpTrap->szInvokedModName
		);

	m_LogEdit.LineScroll(m_LogEdit.GetLineCount());
	m_LogEdit.SetSel(-1);
	m_LogEdit.ReplaceSel(szTrap);
	


	if (wcscmp(lpTrap->szInvokedModName, m_lpProessing->m_pModList[0].szModName) == 0)
	{
		char szVarName[MAX_BUF_SIZE] = { 0 };
		WideCharToMultiByte(CP_ACP, NULL, szTrap, -1, szVarName, _countof(szVarName), NULL, FALSE);
		m_FileLogText.Write(szVarName, strlen(szVarName));
		
		WideCharToMultiByte(CP_ACP, NULL, lpTrap->szApiName, -1, szVarName, _countof(szVarName), NULL, FALSE);

		//������뵽��������������
		m_lpProessing->InsertOfBinary(szVarName);

	}

	delete lpTrap;

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//	������־����ǰ�ļ�����
//////////////////////////////////////////////////////////////////////////


void CAPIMonitorDlg::OnSavelog()
{

	//�жϹܵ�����
	DisconnectNamedPipe(hPipe_1);
	DisconnectNamedPipe(hPipe_2);
	CloseHandle(hPipe_1);
	CloseHandle(hPipe_2);
	

	//��m_FileLogTextд��ģ��������
	char szVarName[MAX_BUF_SIZE] = { 0 };
	WideCharToMultiByte(CP_ACP, NULL, L"\r\nģ����ô������:\r\n", -1, szVarName, _countof(szVarName), NULL, FALSE);

	m_FileLogText.Write(szVarName, strlen(szVarName));

	DWORD		dwItemCount = m_ListTrap.GetItemCount();
	CString		szShow;


	for (DWORD dwRow = 0; dwRow < dwItemCount; dwRow++)
	{

		szShow.Format(L"[%d] %s �� %s����, ���õ�ַ = 0x%x, ���ô��� = %d \r\n",
			dwRow,
			m_ListTrap.GetItemText(dwRow, 2),
			m_ListTrap.GetItemText(dwRow, 1),
			wcstol(m_ListTrap.GetItemText(dwRow, 0), NULL, 16),
			wcstol(m_ListTrap.GetItemText(dwRow, 3), NULL, 10));

		WideCharToMultiByte(CP_ACP, NULL, szShow, -1, szVarName, _countof(szVarName), NULL, FALSE);

		m_FileLogText.Write(szVarName, strlen(szVarName));

	}

	//��m_FileLogBinaryд����������,�����
	DWORD	dwSizeOfBytes;
	DWORD	dwAddr;
	dwAddr = m_lpProessing->GetLogInfo(&dwSizeOfBytes);
	if(dwAddr != NULL)
	m_FileLogBinary.Write((PVOID)dwAddr, dwSizeOfBytes);



	m_FileLogText.Close();
	m_FileLogBinary.Close();

	MessageBox(L"��־����ɹ������򼴽��˳�.......", L"Tip");

	exit(0x0);

}

