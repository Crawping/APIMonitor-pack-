
// SaerPackUIDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SaerPackUI.h"
#include "SaerPackUIDlg.h"
#include "afxdialogex.h"

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


// CSaerPackUIDlg �Ի���



CSaerPackUIDlg::CSaerPackUIDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SAERPACKUI_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSaerPackUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSaerPackUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, &CSaerPackUIDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_PACK, &CSaerPackUIDlg::OnBnClickedButtonPack)
	ON_BN_CLICKED(IDC_BUTTON_CHECK, &CSaerPackUIDlg::OnBnClickedButtonCheck)
	ON_BN_CLICKED(IDC_CHECK_COMPRESS, &CSaerPackUIDlg::OnBnClickedCheckCompress)
	ON_BN_CLICKED(IDC_BUTTON_VCODE, &CSaerPackUIDlg::OnBnClickedButtonVcode)
END_MESSAGE_MAP()


// CSaerPackUIDlg ��Ϣ�������

BOOL CSaerPackUIDlg::OnInitDialog()
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
	bEnabled = FALSE;
	((CComboBox*)GetDlgItem(IDC_COMBO_CPMPRESS))->AddString(L"aplib");
	((CComboBox*)GetDlgItem(IDC_COMBO_CPMPRESS))->AddString(L"JCALG1_FAST");
	((CComboBox*)GetDlgItem(IDC_COMBO_CPMPRESS))->AddString(L"JCALG1_SMALL");
	((CComboBox*)GetDlgItem(IDC_COMBO_CPMPRESS))->SetCurSel(0);


	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CSaerPackUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSaerPackUIDlg::OnPaint()
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
HCURSOR CSaerPackUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSaerPackUIDlg::OnBnClickedButtonOpen()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	OutputDebugString(L"��");
	CString	str;
	CFileDialog  fileDlg(TRUE);
	if (fileDlg.DoModal() == IDOK)
	{
		str = fileDlg.GetPathName();

		//�ж��ļ��Ƿ����
		this->SetDlgItemText(IDC_EDIT_PATH, str);
	}
	
}


void CSaerPackUIDlg::OnBnClickedButtonPack()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	SelectionInfo	stcConfig;
	CString			str;
	CString			strSample;
	HMODULE			hMod;
	fnPackBase		g_pfnPackBase;

	ZeroMemory(&stcConfig, sizeof(SelectionInfo));

	hMod			= 	 LoadLibraryEx(L"SaerPackBase.dll", NULL , NULL);
	g_pfnPackBase	=	(fnPackBase)GetProcAddress(hMod, "PackBase");

	OutputDebugString(L"��������ļ�·��");

	if (((CButton*)GetDlgItem(IDC_CHECK_COMPRESS))->GetCheck())
	{
		stcConfig.bIsCompression	= TRUE;
		stcConfig.dwCompressionType	= ((CComboBox*)GetDlgItem(IDC_COMBO_CPMPRESS))->GetCurSel();
	}

	if (((CButton*)GetDlgItem(IDC_CHECK_IAT))->GetCheck())
	{
		stcConfig.bIsTransferIAT = TRUE;
	}
	
	if (((CButton*)GetDlgItem(IDC_CHECK_RELOC))->GetCheck())
	{
		stcConfig.bIsTransferReloc = TRUE;
	}

	if (((CButton*)GetDlgItem(IDC_CHECK_HOOK))->GetCheck())
	{
		stcConfig.bIsApiRedirect = TRUE;
	}

	if (((CButton*)GetDlgItem(IDC_CHECK_DUMP))->GetCheck())
	{
		stcConfig.bIsAntiDump = TRUE;
	}

	if (((CButton*)GetDlgItem(IDC_CHECK_DEBUG))->GetCheck())
	{
		stcConfig.bIsAntiDebugging = TRUE;
	}


	if (((CButton*)GetDlgItem(IDC_CHECK_MEM))->GetCheck())
	{
		stcConfig.bIsMemVerification = TRUE;
	}

	if (((CButton*)GetDlgItem(IDC_CHECK_FILE))->GetCheck())
	{
		stcConfig.bIsFileVerification = TRUE;
	}

	this->GetDlgItemTextW(IDC_EDIT_PATH, str);
	

	if (((CButton*)GetDlgItem(IDC_CHECK_VCODE))->GetCheck())
	{
		stcConfig.bIsVirtualizeCode = TRUE;

		this->GetDlgItemTextW(IDC_EDIT_VCODE, strSample);

		if (strSample.IsEmpty())
		{
			ZeroMemory(stcConfig.szSample, MAX_PATH * sizeof(WCHAR));
		}
		else
		{
			wcsncpy_s(stcConfig.szSample, strSample, MAX_PATH);
		}

	}//if






	if (g_pfnPackBase(str.GetBuffer(), &stcConfig))
		MessageBox(L"�ӿǳɹ�", L"��ʾ", 0);
	else
		MessageBox(L"�ӿ�ʧ��", L"��ʾ", 0);

	FreeLibrary(hMod);
}


void CSaerPackUIDlg::OnBnClickedButtonCheck()
{
	DWORD DWSel = ((CComboBox*)GetDlgItem(IDC_COMBO_CPMPRESS))->GetCurSel();
}


void CSaerPackUIDlg::OnBnClickedCheckCompress()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	
	if (!bEnabled)
	{
		GetDlgItem(IDC_COMBO_CPMPRESS)->EnableWindow(TRUE);
		bEnabled = TRUE;
	}
	else
	{
		GetDlgItem(IDC_COMBO_CPMPRESS)->EnableWindow(FALSE);
		bEnabled = FALSE;
	}

}


void CSaerPackUIDlg::OnBnClickedButtonVcode()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString	str;
	CFileDialog  fileDlg(TRUE);
	if (fileDlg.DoModal() == IDOK)
	{
		str = fileDlg.GetPathName();
		//�ж��ļ��Ƿ����
		this->SetDlgItemText(IDC_EDIT_VCODE, str);
	}


}
