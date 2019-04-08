#include "stdafx.h"
#include "ApiParam.h"


CONST WCHAR*	szFilesToFind = L"ApiDef\\*.ss";
CONST WCHAR*	szApiDefDir = L"ApiDef\\";
DWORD			g_dwDefNUm = 0;

DefFileInfo		g_stcDef[MAX_DEF_FILENAME_LENGTH];




//////////////////////////////////////////////////////////////////////////
//	��SS�ļ�ȫ�������ڴ�֮��
//////////////////////////////////////////////////////////////////////////


BOOL	CreateSSMapFile()
{

	HMODULE				hMod;
	WCHAR				szDllName[MAX_PATH];
	WIN32_FIND_DATA		FindData;
	HANDLE				hFind;
	WCHAR*				szDirPath = new WCHAR[MAX_PATH];
	WCHAR*				szSSPath = new WCHAR[MAX_PATH];


	hMod = GetModuleHandle(L"HookAPILibrary");

	if (hMod == NULL)
	{
		MessageBox(NULL, L"CreateSSMapFile Failed!", NULL, NULL);
		return FALSE;
	}

	GetModuleFileName(hMod, szDllName, sizeof(WCHAR)*MAX_PATH);
	
	PathRemoveFileSpec(szDllName);

	BOOL bRootDir = PathIsRoot(szDllName);
	
	if (bRootDir)
	{//���Ǹ�Ŀ¼�����üӷ�б��

		wsprintf(szSSPath, L"%s%s", szDllName, szFilesToFind);
		wcscat_s(szDllName, MAX_PATH, szApiDefDir);
	}
	else
	{
		wsprintf(szSSPath, L"%s\\%s", szDllName, szFilesToFind);
		wcscat_s(szDllName, MAX_PATH, L"\\");
		wcscat_s(szDllName, MAX_PATH, szApiDefDir);

	}

	

	hFind = FindFirstFile(szSSPath, &FindData);
	if (hFind == INVALID_HANDLE_VALUE) return  FALSE;

	
	if (DoMapFile(FindData.cFileName, szDllName) == FALSE)	return FALSE;



	while (FindNextFile(hFind, &FindData))
	{
		if (DoMapFile(FindData.cFileName, szDllName) == FALSE) return FALSE;

	}
	

	FindClose(hFind);

	return TRUE;

}




//////////////////////////////////////////////////////////////////////////
//	��ĳ��SS�ļ���ȡ���ڴ�֮��
//	������
//	WCHAR*	szFileName		- SS���ļ�����ֻ���ļ�����
//	WCHAR*	szDir			- װ��SS�ļ���Ŀ¼
//////////////////////////////////////////////////////////////////////////


BOOL DoMapFile(WCHAR* szFileName, WCHAR* szDir)
{
	HANDLE  hFile;
	DWORD   dwSize, dwBytesRead;
	VOID*   pMem;
	WCHAR*	pCh;
	WCHAR	szFileRoot[MAX_PATH];

	if (g_dwDefNUm == MAX_DEF_FILENAME_LENGTH) return FALSE;

	wsprintf(szFileRoot, L"%s%s", szDir, szFileName);

	hFile = CreateFile(szFileRoot, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, 0);

	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	dwSize = GetFileSize(hFile, NULL);

	pMem = VirtualAlloc(NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);

	if (pMem == NULL)
	{
		CloseHandle(hFile);
		return FALSE;
	}

	ReadFile(hFile, pMem, dwSize, &dwBytesRead, NULL);

	g_stcDef[g_dwDefNUm].dwMapSize = dwSize;
	g_stcDef[g_dwDefNUm].lpMapAddress = (PVOID)((DWORD)pMem + 2);


	pCh = wcsstr(szFileName, L".");

	ZeroMemory(g_stcDef[g_dwDefNUm].szDefFileObject, sizeof(WCHAR)*MAX_PATH);

	lstrcpyn(g_stcDef[g_dwDefNUm].szDefFileObject, szFileName, pCh - szFileName + 1);

	g_dwDefNUm++;

	CloseHandle(hFile);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//	�ͷŵ�ǰ�ռ䱣��SS�ļ��Ļ�����
//////////////////////////////////////////////////////////////////////////

VOID UnmapSSFiles()
{

	for (DWORD dwIndex = 0; dwIndex < g_dwDefNUm; dwIndex++)
		VirtualFree(g_stcDef[dwIndex].lpMapAddress, 0, MEM_RELEASE);

}


//////////////////////////////////////////////////////////////////////////
//	ʹ��ǰ������ָ��������һ��
//	������
//	WCHAR*			&pCh	- [in & out]������ָ��
//////////////////////////////////////////////////////////////////////////

BOOL ToNextLine(WCHAR* &pCh)
{
	while (*pCh != 0xA)
	{
		if (!*pCh)
			return FALSE;
		pCh++;
	}
	while (*pCh < 0x20)
	{
		if (!*pCh)
			return FALSE;
		pCh++;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//	��ȡapi�������������ַ�������ʽд�뻺������
//	����:
//	WCHAR*						szDll	- ģ������
//	WCHAR*						szApi	- api����
//	VOID*						pStack	- ��ջָ��
//	����ֵ��	�����������ַ���ָ��
//////////////////////////////////////////////////////////////////////////

WCHAR* GetApiParam(WCHAR* szDll, WCHAR* szApi, VOID* pStack)
{
	WCHAR	*pCh;
	DWORD	dwDllLength;
	DWORD	dwIndex;
	WCHAR*	lpParam;
	PDWORD	pdStack;
	DWORD	dwStackVar;
	DWORD	dwWideCharSize;
	WCHAR	szTemp[MAX_PATH];




	dwDllLength = lstrlen(szDll) - 4;

	for (dwIndex = 0; dwIndex < g_dwDefNUm; dwIndex++)
	{
		if (_wcsnicmp(szDll, g_stcDef[dwIndex].szDefFileObject, dwDllLength) == 0)
			break;
	}


	if (dwIndex == g_dwDefNUm)	return NULL;

	pCh = wcsstr((WCHAR*)g_stcDef[dwIndex].lpMapAddress, szApi);

	if (pCh == NULL)	return NULL;

	lpParam = (WCHAR*)VirtualAlloc(NULL, MAX_PATH, MEM_COMMIT, PAGE_READWRITE);
	if (lpParam == NULL)	return NULL;

	ZeroMemory(lpParam, MAX_PATH);

//	wsprintf(lpParam, L"Param :");

	pdStack = (PDWORD)((DWORD)pStack + 4);


	OutputDebugString(szDll);
	OutputDebugString(szApi);

	ToNextLine(pCh);

	while (*pCh != L'-' && *pCh > 0x20)
	{
		dwStackVar = *pdStack;

		switch (*pCh)
		{
		case '0': // BOOL
			if (!dwStackVar)
				lstrcat(lpParam, L"FALSE");

			else
				lstrcat(lpParam, L"TRUE");
			break;

		case '1': // DWORD
			wsprintf(szTemp, L"%08lXh", dwStackVar);
			lstrcat(lpParam, szTemp);
			break;

		case '2': // WORD
			wsprintf(szTemp, L"%04lXh", (WORD)dwStackVar);
			lstrcat(lpParam, szTemp);
			break;

		case '3': // BYTE
			wsprintf(szTemp, L"%02lXh", (BYTE)dwStackVar);
			lstrcat(lpParam, szTemp);
			break;

		case '4': // PSTR
			wsprintf(szTemp, L"%08lXh", dwStackVar);
			lstrcat(lpParam, szTemp);
			lstrcat(lpParam, L"=");
			if (HIWORD(dwStackVar)) // is it a string ?
			{
				// grab the string 
				memset(szTemp, 0, MAX_PATH);
				if (IsBadReadPtr((VOID*)dwStackVar, MAX_PATH - 1))
					lstrcat(lpParam, L"?");
				else
				{


					dwWideCharSize = MultiByteToWideChar(CP_ACP, NULL, (CHAR*)dwStackVar,
						strlen((CHAR*)dwStackVar),
						NULL, NULL);

					MultiByteToWideChar(CP_ACP, NULL, (CHAR*)dwStackVar,
						strlen((CHAR*)dwStackVar),
						szTemp, dwWideCharSize);

					lstrcat(lpParam, L"\"");
					lstrcat(lpParam, szTemp);
					lstrcat(lpParam, L"\"");


				}
			}
			else
				lstrcat(lpParam, L"?");
			break;

		case '5': // LPDWORD
			wsprintf(szTemp, L"%08lX", dwStackVar);
			lstrcat(lpParam, szTemp);

			if (dwStackVar)
			{
				lstrcat(lpParam, L"=");
				if (IsBadReadPtr((VOID*)dwStackVar, 4))
					lstrcat(lpParam, L"?");
				else
				{
					wsprintf(szTemp, L"%08lXh", *(DWORD*)dwStackVar);
					lstrcat(lpParam, szTemp);
				}
			}
			break;

		case '6': // LPWORD
			wsprintf(szTemp, L"%08lX", dwStackVar);
			lstrcat(lpParam, szTemp);
			if (dwStackVar)
			{
				lstrcat(lpParam, L"=");
				if (IsBadReadPtr((VOID*)dwStackVar, 2))
					lstrcat(lpParam, L"?");
				else
				{
					wsprintf(szTemp, L"%08lXh", *(WORD*)dwStackVar);
					lstrcat(lpParam, szTemp);
				}
			}
			break;

		case '7': // LPBYTE
			wsprintf(szTemp, L"%08lX", dwStackVar);
			lstrcat(lpParam, szTemp);
			if (dwStackVar)
			{
				lstrcat(lpParam, L"=");
				if (IsBadReadPtr((VOID*)dwStackVar, 1))
					lstrcat(lpParam, L"?");
				else
				{
					wsprintf(szTemp, L"%08lXh", *(BYTE*)dwStackVar);
					lstrcat(lpParam, szTemp);
				}
			}
			break;

		case '8': // LPWSTR
			wsprintf(szTemp, L"%08lXh", dwStackVar);
			lstrcat(lpParam, szTemp);
			lstrcat(lpParam, L"=");

			if (HIWORD(dwStackVar)) // is it a string ?
			{

				// grab the string out
				if (IsBadReadPtr((VOID*)dwStackVar, MAX_PATH - 1))
					lstrcat(lpParam, L"?");
				else
				{
					lstrcat(lpParam, L"\"");
					// convert UNICODE to ASCII string per byte

					wcscpy_s(szTemp, (WCHAR*)dwStackVar);

					lstrcat(lpParam, szTemp);

					lstrcat(lpParam, L"\"");
				}
			}
			else
				lstrcat(lpParam, L"?");
			break;

		case '9': // filter
			VirtualFree(lpParam, 0, MEM_RELEASE);
			
			return NULL;
			break;

		default: // handle as DWORD
			wsprintf(szTemp, L"%08lXh", dwStackVar);
			lstrcat(lpParam, szTemp);
			break;

		}//switch

		if (!ToNextLine(pCh)) // the ss file mem is at the end
			break;
		pdStack++;

		lstrcat(lpParam, L", ");


	}//while



	return lpParam;
}


//////////////////////////////////////////////////////////////////////////
//	��ȡ��ǰĿ¼�е�config�ļ�����ȡ�Ƿ�IMM HOOK����Ϣ
//	����ֵ��
//	TRUE - IMM HOOK��or else 
//////////////////////////////////////////////////////////////////////////

BOOL	GetHookModel()
{
	HMODULE				hMod;
	WCHAR				szConfig[MAX_PATH];
	HANDLE				hFile;
	BOOL				bStartedHook;
	DWORD				dwNumOfBytesRead;


	hMod = GetModuleHandle(L"HookAPILibrary");

	if (hMod == NULL)
	{
		MessageBox(NULL, L"CreateSSMapFile Failed!", NULL, NULL);
		return FALSE;
	}

	GetModuleFileName(hMod, szConfig, sizeof(WCHAR)*MAX_PATH);

	PathRemoveFileSpec(szConfig);

	BOOL bRootDir = PathIsRoot(szConfig);

	if (bRootDir)
	{//���Ǹ�Ŀ¼�����üӷ�б��
		wcscat_s(szConfig, MAX_PATH, L"Config.ini");
	}
	else
	{
		wcscat_s(szConfig, MAX_PATH, L"\\Config.ini");
	}


	hFile = CreateFile(szConfig, GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);


	if (hFile == NULL)
	{//�ļ������ڣ�Ĭ�ϲ�IMM HOOK
		return FALSE;
	}


	ReadFile(hFile, &bStartedHook, sizeof(BOOL), &dwNumOfBytesRead, NULL);

	CloseHandle(hFile);

	if (bStartedHook)
	{
		OutputDebugString(L"bStartedHook = TRUE");

	}
	else
	{
		OutputDebugString(L"bStartedHook = FALSE");
	}

	return bStartedHook;

}