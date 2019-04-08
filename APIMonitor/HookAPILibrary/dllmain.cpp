// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"

extern"C" __declspec(dllexport) void _stdcall Test();

extern HANDLE			g_hPipe1;
extern HANDLE			g_hPipe2;
extern lpModInfo		g_lpModList;

extern DWORD			dwObjectModBaseStart;
extern DWORD			dwObjectModBaseEnd;
extern DWORD			dwSelfModBaseStart;
extern DWORD			dwSelfModBaseEnd;

extern HWND				hDlg;

//////////////////////////////////////////////////////////////////////////
//	Detourע����ҪDll��һ��������������
//////////////////////////////////////////////////////////////////////////
void _stdcall Test(){}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{

	DWORD	dwProcessID;
	WCHAR	szNamePipe_1[MAX_PATH] = { 0 };
	WCHAR	szNamePipe_2[MAX_PATH] = { 0 };
	BOOL	bStartedHook;
	HANDLE	hThread[2];
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

		if (CreateSSMapFile() == FALSE)
		{
			
			return TRUE;
		}

		bStartedHook = GetHookModel();


		DetourRestoreAfterWith();
		//hDlg =  FindWindow(NULL, L"APIMonitor");
		//if (hDlg == NULL)
		//{
		//	MessageBox(NULL, L"FindWindows Failed!", NULL, NULL);
		//	return FALSE;
		//}
		OutputDebugString(L"[DLL]�����ʼ��");
		//���������ܵ�������
		dwProcessID = GetCurrentProcessId();
		wsprintf(szNamePipe_1, L"\\\\.\\pipe\\NamePipe_APIMonitor_1");
		wsprintf(szNamePipe_2, L"\\\\.\\pipe\\NamePipe_APIMonitor_2");

		OutputDebugString(L"[DLL]�ܵ����ƴ���ɹ�");
		//���������ܵ�����һЩ����ʧ�ܵ�Ԥ����
		g_hPipe1 = ConnectToNamePipe(szNamePipe_1);	//����ָ���߳�
		g_hPipe2 = ConnectToNamePipe(szNamePipe_2);	//���߳�

		OutputDebugString(L"[DLL]�ܵ����ӳɹ�");
		if (!g_hPipe1 || !g_hPipe2)	return FALSE;

		//�����ܵ����������Ӻ��ڴ��߳̽���һ���ܵ���ͨ�Ų���

		hThread[0] = CreateThread(NULL, NULL, ThreadProc, (LPVOID)g_hPipe1, NULL, NULL);
	//	hThread[1] = CreateThread(NULL, NULL, ThreadProc2, (LPVOID)g_hPipe2, NULL, NULL);

		if (!hThread[0] || !hThread[1])	return FALSE;

		OutputDebugString(L"[DLL]�����߳����ӳɹ�");

		//Ϊģ���б�����ռ�
		g_lpModList = (lpModInfo)VirtualAlloc(NULL, LIST_INC * sizeof(ModInfo), MEM_COMMIT, PAGE_READWRITE);
		if (!g_lpModList) return 0;
		ZeroMemory(g_lpModList, LIST_INC * sizeof(ModInfo));
		

		//��ȡ����ģ���Ŀ������ģ��ķ�Χ
		dwObjectModBaseStart = (DWORD) GetModuleHandle(NULL);
		dwObjectModBaseEnd = ((PIMAGE_NT_HEADERS)(((PIMAGE_DOS_HEADER)dwObjectModBaseStart)->e_lfanew + dwObjectModBaseStart))->OptionalHeader.SizeOfImage + dwObjectModBaseStart;

		dwSelfModBaseStart = (DWORD)hModule;
		dwSelfModBaseEnd = ((PIMAGE_NT_HEADERS)(((PIMAGE_DOS_HEADER)dwSelfModBaseStart)->e_lfanew + dwSelfModBaseStart))->OptionalHeader.SizeOfImage + dwSelfModBaseStart;

		if (bStartedHook)
		{
			ScanModule();
		}
		else
		{
			ScanModule2();
		}

	/*	if (ScanModule() == TRUE)
			OutputDebugString(L"ScanModule�ɹ�����");*/

		break;

	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:

		//ж�ع���
		UnHookAllApi();
		//�ͷ�list�ռ�
		FreeListMemory();
		//�ж�ͨ��
		CloseHandle(g_hPipe1);
		CloseHandle(g_hPipe2);

		break;
	}
	return TRUE;
}

