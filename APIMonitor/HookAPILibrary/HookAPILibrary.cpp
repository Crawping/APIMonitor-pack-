// HookAPILibrary.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"

HANDLE					g_hPipe1		= NULL;
HANDLE					g_hPipe2		= NULL;
lpModInfo				g_lpModList		= NULL;
DWORD					g_dwModListLen	= LIST_INC;

DWORD					dwObjectModBaseStart;				//���������ַ
DWORD					dwObjectModBaseEnd;
DWORD					dwSelfModBaseStart;					//ע��DLL��ַ
DWORD					dwSelfModBaseEnd;

DWORD					g_dwPrivateIndex = 0;		//AddModuleToList��˽��ȫ�ֱ���,��Ҫ��ʼ��Ϊ0
BOOL					IsHookCallAgain = FALSE;

extern DWORD			g_dwDefNUm;

extern DefFileInfo		g_stcDef[MAX_DEF_FILENAME_LENGTH];


StringFilteredDllList	g_szFilteredDllList[] = { { L"MFC",3 },{L"GDI32",5} , \
{L"gdi32full", 9 }, { L"ntdll",5 }, { L"win32u",6 }, { L"uxtheme",7 }, {L"VCRUNTIME" ,9  }, \
{L"ole32", 5}, {L"windows",7} };





StringFilteredDllList	g_szProcessingDlllist[] = { { L"KERNEL32", 8 }   /*{L"WS2_32",6},{L"KERNELBASE",10} , \
						{L"KERNEL32",8 }, {L"USER32",6} ,{L"IMM32",5} */ };


//////////////////////////////////////////////////////////////////////////
//	ģ��ָ����߳̽���ָ������
//	������
//	LPVOID lpPara	�ܵ����
//////////////////////////////////////////////////////////////////////////

DWORD  WINAPI ThreadProc(LPVOID lpPara)
{
	HANDLE			hPipe = (HANDLE)lpPara;
	LPBYTE			pBuff = new BYTE[1024];
	DWORD			dwNumOfRead;
	DWORD			dwRet;
	lpPacketInfo	lpPacket;
	lpHookMod		lpHook;
	lpFilteredInfo	lpFilterd;


	ZeroMemory(pBuff, 1024);

	do
	{
		if (dwRet = ReadFile(hPipe, pBuff, 1024, &dwNumOfRead, NULL))
			//�˴�����һ����Ϣ�����������Է�����ض˷��Ͷ�������Ϣ
		{
			
			lpPacket = (lpPacketInfo)pBuff;
			//������

			switch (lpPacket->dwType)
			{
			case HOOK_MSG:

				lpHook = (lpHookMod)lpPacket->Data;
				if (lpHook->bHook)
				{//��ָ��ģ�����Hook
					OutputDebugString(L"ָ��ģ�鼴����Hook");
					if (g_lpModList[lpHook->dwModIndex].bActive == FALSE)
					{
						HookModule(lpHook->dwModIndex);
					}
					else
					{
						ReHookModule(lpHook->dwModIndex);
					}
					OutputDebugString(L"ָ��ģ�鱻Hook");
				}
				else
				{//��ָ��ģ�����UnHook
					OutputDebugString(L"ָ��ģ�鼴����UnHook");
					UnHookModule(lpHook->dwModIndex);
					OutputDebugString(L"ָ��ģ�鱻UnHook");
				}


				break;

			case FILTER_MSG:
				lpFilterd = (lpFilteredInfo)lpPacket->Data;

				if (lpFilterd->bFilterd)
				{//����
					g_lpModList[lpFilterd->dwModIndex].lpApiList[lpFilterd->dwApiIndex].bIsFiltered = TRUE;
				}
				else
				{//������
					g_lpModList[lpFilterd->dwModIndex].lpApiList[lpFilterd->dwApiIndex].bIsFiltered = FALSE;
				}

				break;


			default:
				break;
			}

			
		

		}

	}while (dwRet);

	CloseHandle(hPipe);
	delete[] pBuff;
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//	ģ�����̷߳���API�ɼ������������
//	������
//	LPVOID lpPara	�ܵ����
//////////////////////////////////////////////////////////////////////////

DWORD  WINAPI ThreadProc2(LPVOID lpPara)
{
	HANDLE	hPipe = (HANDLE)lpPara;
	DWORD	dwPorcessID;
	DWORD	dwNumOfWritten;
	WCHAR	szNamePipe[1024] = { 0 };

	OutputDebugString(L"[DLL��]��ȡ����ID");
	dwPorcessID = GetCurrentProcessId();

	OutputDebugString(L"[DLL��]����ܵ�1�����߳��Ѿ�����");

	for (DWORD dwIndex = 0; dwIndex < 20; dwIndex++)
	{
		wsprintf(szNamePipe, L"%s_ID:%d_Index:%d", L"[DLL]ָ����Ͳɼ��ĵ�API����", dwPorcessID, dwIndex);
		WriteFile(hPipe, szNamePipe, sizeof(WCHAR)*(wcslen(szNamePipe) + 1), &dwNumOfWritten, NULL);
	}

	CloseHandle(hPipe);

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//	���ӹܵ������ؾ��
//	������
//	WCHAR* szNamePipe	�ܵ�������
//	����ֵ��			�ܵ����,����ΪNULL
//////////////////////////////////////////////////////////////////////////

HANDLE ConnectToNamePipe(WCHAR* szNamePipe)
{
	HANDLE  hPipe;
	DWORD	dwConnectCounter = 50;//50�ε����ӳ���

	while(WaitNamedPipe(szNamePipe, NMPWAIT_WAIT_FOREVER) == FALSE)
	{
		Sleep(200);
		dwConnectCounter--;

		if (dwConnectCounter == 0)
			return NULL;
	}

	hPipe = CreateFile(szNamePipe, GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	return hPipe;
}


//////////////////////////////////////////////////////////////////////////
//	ɨ���ڴ��е�ģ�飬������ӵ�Modlist���У�Ȼ���ڽ�һ��ɨ����Api
//	���Api�󣬽���Hook�������ڸù����лὫ������ݷ�������ض�
//////////////////////////////////////////////////////////////////////////

BOOL	ScanModule()
{
	//ɨ���ڴ��е�ģ��
	//ʹ��GetSystemInfo��VirtualQuery
	DWORD						dwPageAddr;				
	DWORD						dwMinPageAddr;			//�û��ռ���С��ַ
	DWORD						dwMaxPageAddr;			//�û��ռ���ߵ�ַ
	MEMORY_BASIC_INFORMATION	stcMbi;
	MEMORY_BASIC_INFORMATION	stcMibOfApi;
	SYSTEM_INFO					stcSi;
	PIMAGE_DOS_HEADER			pDosHeader;
	PIMAGE_NT_HEADERS			pNtHeader;
	PIMAGE_EXPORT_DIRECTORY		pExp;
	WCHAR						szDllName[MAX_NAME_LEN];
	DWORD						dwWideCharSize;
	DWORD						dwCurModIndex;			//��ǰModIndex
	DWORD						dwCurApiIndex;			//��ǰApiIndex
	DWORD						dwCurModBaseAddr;		//��ǰģ�����ʼ��ַ
	DWORD						dwCurModBaseEnd;		//��ǰģ���ĩ��ַ
	PDWORD						dpExpAddrOfFunc;
	PDWORD						dpExpAddrOfName;
	PWORD						wpExpAddrOfNameOri;

	lpApiInfo					lpCurApiList;
	ShellCode					stcShellCode;

	HANDLE						hSnapShot;
	THREADENTRY32				stcThreadInfo;
	BOOL						bIgnoringHook;
	WCHAR						szTest[MAX_NAME_LEN] = { 0 };
	DWORD						dwNewAddr;
	DWORD						dwObjectBaseAddr;
	WCHAR						szProgramName[MAX_PATH] = { 0 };
	BOOL						bIgnored = TRUE;
	//////////////////////////////////////////////////////////////////////////
	// 1.1 ���detour�Ե�ǰ�����̵߳�ˢ�£�������ЩС�ֽڵĺ���
	//////////////////////////////////////////////////////////////////////////
	OutputDebugString(L"1.1 ���detour�Ե�ǰ�����̵߳�ˢ�£�������ЩС�ֽڵĺ���");
	DetourTransactionBegin();
	DetourSetIgnoreTooSmall(TRUE);
	
	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());

	if (hSnapShot != INVALID_HANDLE_VALUE)
	{
		if (Thread32First(hSnapShot, &stcThreadInfo))
		{
			DetourUpdateThread(OpenThread(THREAD_QUERY_INFORMATION, FALSE, stcThreadInfo.th32ThreadID));
			while (Thread32Next(hSnapShot, &stcThreadInfo))
			{
				DetourUpdateThread(OpenThread(THREAD_QUERY_INFORMATION, FALSE, stcThreadInfo.th32ThreadID));
			}
		}
	}
	
	//1.2 ��ȡϵͳ��Ϣ���õ��û��ռ�ķ�Χ�����ڱ���ģ��
	OutputDebugString(L"1.2 ��ȡϵͳ��Ϣ���õ��û��ռ�ķ�Χ�����ڱ���ģ��");

	GetSystemInfo(&stcSi);

	dwMinPageAddr	= (DWORD)stcSi.lpMinimumApplicationAddress;
	dwMaxPageAddr	= (DWORD)stcSi.lpMaximumApplicationAddress;
	dwPageAddr		= dwMinPageAddr;
	

	//1.3 ��ȡ���������ģ����Ϣ����ӵ��б����ǲ����ڼ�ض���ʾ
	dwObjectBaseAddr = (DWORD)GetModuleHandle(NULL);

	GetModuleFileName((HMODULE)dwObjectBaseAddr, szProgramName, MAX_PATH);

	pDosHeader = (PIMAGE_DOS_HEADER)dwObjectBaseAddr;

	pNtHeader = (PIMAGE_NT_HEADERS)(pDosHeader->e_lfanew + dwObjectBaseAddr);


	dwCurModIndex = AddModuleToList(dwObjectBaseAddr, pNtHeader->OptionalHeader.SizeOfImage, PathFindFileName(szProgramName));

	SendModInfo(&g_lpModList[dwCurModIndex]);



	//2. ����ģ��
	for (; dwPageAddr < dwMaxPageAddr; dwPageAddr += stcMbi.RegionSize)
	{
		bIgnored = TRUE;

		VirtualQuery( (LPVOID)dwPageAddr, &stcMbi, sizeof(stcMbi));

		if (stcMbi.State != MEM_COMMIT || stcMbi.Protect & PAGE_GUARD)	continue;
	
		pDosHeader = (PIMAGE_DOS_HEADER)dwPageAddr;

		if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)	continue;
	
		pNtHeader = (PIMAGE_NT_HEADERS)(pDosHeader->e_lfanew + dwPageAddr);

		if (pNtHeader->Signature != IMAGE_NT_SIGNATURE)	continue;
	
		if (pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress == 0)	continue;
	
		pExp = (PIMAGE_EXPORT_DIRECTORY)(dwPageAddr + pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
		
		//���������ע��DLL��������
		if(dwPageAddr == dwSelfModBaseStart)	continue;

		ZeroMemory(szDllName, sizeof(WCHAR)*MAX_NAME_LEN);
		if (pExp->Name)
		{//����ģ���������⴦��������GetModuleFileName����ȡģ������
			
			dwWideCharSize = MultiByteToWideChar(CP_ACP, NULL, (CHAR*)(dwPageAddr + pExp->Name), strlen((CHAR*)(dwPageAddr + pExp->Name)),
				NULL, NULL);

			MultiByteToWideChar(CP_ACP, NULL, (CHAR*)(dwPageAddr + pExp->Name),
				strlen((CHAR*)(dwPageAddr + pExp->Name)),
				szDllName, dwWideCharSize);

			OutputDebugString(L"����ģ���������⴦��������GetModuleFileName����ȡģ������");
		}
		else
		{
			//�˴��Ĵ�����Ҫ�����Wow64ϵ�е�DLL�����ǵ����Ʋ����ø�API��ȡ,ֱ��������һ��ģ��
			if(GetModuleFileName((HMODULE)dwPageAddr, szDllName, MAX_NAME_LEN) == 0)
				continue;	
		}


		OutputDebugString(szDllName);

		OutputDebugString(L"2.1 ���ģ����Ϣ");
		//2.1 ���ģ����Ϣ
		dwCurModIndex	= AddModuleToList(dwPageAddr, pNtHeader->OptionalHeader.SizeOfImage, szDllName);
		dpExpAddrOfFunc = (PDWORD)(pExp->AddressOfFunctions + dwPageAddr);
		dpExpAddrOfName = (PDWORD)(pExp->AddressOfNames + dwPageAddr);
		wpExpAddrOfNameOri = (PWORD)(pExp->AddressOfNameOrdinals + dwPageAddr);
		g_lpModList[dwCurModIndex].dwApiListLen = pExp->NumberOfNames;

		// ��ǰģ��ķ�Χ
		dwCurModBaseAddr	= dwPageAddr;
		dwCurModBaseEnd		= dwPageAddr + pNtHeader->OptionalHeader.SizeOfImage;


		//2.3 ��ʼ��Shellcode
		OutputDebugString(L"2.3 ��ʼ��Shellcode");
		InitApiShellStruct(&stcShellCode);

		//2.4 ����Mod�����Ϣ����ض�
		OutputDebugString(L"2.4 ����Mod�����Ϣ����ض�");

		SendModInfo(&g_lpModList[dwCurModIndex]);

		OutputDebugString(L"2.5 ����ģ���Api�����API��Ϣ,ֻ�����ַ���������Api");
	
		
		//for (DWORD i = 0; i < _countof(g_szProcessingDlllist); i++)
		//{
		//	if (_wcsnicmp(szDllName,
		//		g_szProcessingDlllist[i].szDllName,
		//		g_szProcessingDlllist[i].dwCheckSize) == 0)
		//	{
		//		bIgnored = FALSE;
		//		break;
		//	}
		//}


		//if (bIgnored)	continue;


		//2.5 ����ģ���Api�����API��Ϣ,ֻ�����ַ���������Api
		for (DWORD	dwIndex = 0; dwIndex < pExp->NumberOfNames ; dwIndex++)
		{
			//Ԥ����,Ĭ�ϲ���Hook
		//	bIgnoringHook = TRUE;

			//��ַ�Ƿ���ģ���⣬��֪����û�������������
			if( (dpExpAddrOfFunc[wpExpAddrOfNameOri[dwIndex]]  + dwPageAddr) >= dwCurModBaseEnd ||
			(dpExpAddrOfFunc[wpExpAddrOfNameOri[dwIndex]]+ dwPageAddr) <= dwCurModBaseAddr)
				continue;

			VirtualQuery( (LPVOID)(dpExpAddrOfFunc[wpExpAddrOfNameOri[dwIndex]] + dwPageAddr),
					&stcMibOfApi,
					sizeof(MEMORY_BASIC_INFORMATION));
			
			//��Api�Ƿ��ִ��
			//	0xF0����˼���鿴MSDN�ĵ���ֻҪ�Ǵ���ִ�����ԣ������ڸ�4λ��λ
			//	ֻҪ��������&0xF0��ֻҪΪ1���Ϳ���˵�����������ִ��
			if( ((stcMibOfApi.AllocationProtect & 0xF0) == 0)   ||   stcMibOfApi.State != MEM_COMMIT)
				continue;
			

			//��������Ԥ�����������ڵ�
			dwCurApiIndex =  AllocApiFromList(&g_lpModList[dwCurModIndex]);

			if(dwCurApiIndex == -1) continue;

			lpCurApiList = g_lpModList[dwCurModIndex].lpApiList;

			lpCurApiList[dwCurApiIndex].bIsHooked		= FALSE;			//Ĭ��Api������
			lpCurApiList[dwCurApiIndex].dwOridinal		= wpExpAddrOfNameOri[dwIndex];
			lpCurApiList[dwCurApiIndex].dwModIndex		= dwCurModIndex;
			lpCurApiList[dwCurApiIndex].dwApiIndex		= dwCurApiIndex;
			lpCurApiList[dwCurApiIndex].dwApiRealAddr	= dpExpAddrOfFunc[wpExpAddrOfNameOri[dwIndex]] + dwPageAddr;
			
			dwWideCharSize = MultiByteToWideChar(CP_ACP, NULL, (CHAR*)(dwPageAddr + dpExpAddrOfName[dwIndex]), strlen((CHAR*)(dwPageAddr + dpExpAddrOfName[dwIndex])),
				NULL, NULL);

			MultiByteToWideChar(CP_ACP, NULL, (CHAR*)(dwPageAddr + dpExpAddrOfName[dwIndex]), 
				strlen((CHAR*)(dwPageAddr + dpExpAddrOfName[dwIndex])),
				lpCurApiList[dwCurApiIndex].szApiName, dwWideCharSize);

			lpCurApiList[dwCurApiIndex].lpShellBuf = (lpShellCode)VirtualAlloc(NULL, sizeof(ShellCode), 
				MEM_COMMIT, PAGE_EXECUTE_READWRITE);

			stcShellCode.dwPushApiIndex = dwCurApiIndex;
			stcShellCode.dwPushModIndex = dwCurModIndex;
			
			DWORD	dwTrapAddr = (DWORD)GetProcAddress((HMODULE)dwSelfModBaseStart, "TrappedApiCall");

	
			stcShellCode.dwCallAddr = (DWORD)dwTrapAddr - ((DWORD)lpCurApiList[dwCurApiIndex].lpShellBuf + offsetof(ShellCode, dwCallAddr) + 4);
			stcShellCode.dwJmpApiAddr = lpCurApiList[dwCurApiIndex].dwApiRealAddr - ((DWORD)lpCurApiList[dwCurApiIndex].lpShellBuf + offsetof(ShellCode, dwJmpApiAddr) + 4);
			
			CopyMemory(lpCurApiList[dwCurApiIndex].lpShellBuf, &stcShellCode, sizeof(ShellCode));

			//2.6 ����Api�����Ϣ(ע�������ȷ�����Hook����Ȼ��Api��ʵ��ַ�ᱻDetour�滻)
			SendApiInfo(&lpCurApiList[dwCurApiIndex]);

			

			//2.7 HOOK API��������һ������Ԥ�����ù����б�szFilteredDllList�е�Dll���ܱ�HOOK����֤��������ִ��Ч��
			//for (DWORD i = 0; i < _countof(g_szProcessingDlllist); i++)
			//{
			//	if (_wcsnicmp(g_lpModList[dwCurModIndex].szModName,
			//		g_szProcessingDlllist[i].szDllName,
			//		g_szProcessingDlllist[i].dwCheckSize) == 0)
			//		bIgnoringHook = FALSE;
			//}


			bIgnoringHook = FilterApi(g_lpModList[dwCurModIndex].szModName, 
				lpCurApiList[dwCurApiIndex].szApiName);


			if (bIgnoringHook == FALSE )
			{		
					wsprintf(szTest, L"ApiName = %s",  lpCurApiList[dwCurApiIndex].szApiName);
					OutputDebugString(szTest);

					DetourAttachEx((PVOID*)&lpCurApiList[dwCurApiIndex].dwApiRealAddr,
					lpCurApiList[dwCurApiIndex].lpShellBuf,
					(PDETOUR_TRAMPOLINE*)&dwNewAddr, NULL, NULL);
					lpCurApiList[dwCurApiIndex].bIsHooked = TRUE;
					lpCurApiList[dwCurApiIndex].lpShellBuf->dwJmpApiAddr = dwNewAddr - ((DWORD)lpCurApiList[dwCurApiIndex].lpShellBuf + offsetof(ShellCode, dwJmpApiAddr) + 4);
			
			}
		
		}//for


	}//for


	SendFinish(FINISH_MODMSG);

	OutputDebugString(L"DetourTransactionCommit ǰ");
	DetourTransactionCommit();
	OutputDebugString(L"DetourTransactionCommit ��");


	return TRUE;
}





BOOL	ScanModule2()
{
	//ɨ���ڴ��е�ģ��
	//ʹ��GetSystemInfo��VirtualQuery
	DWORD						dwPageAddr;
	DWORD						dwMinPageAddr;			//�û��ռ���С��ַ
	DWORD						dwMaxPageAddr;			//�û��ռ���ߵ�ַ
	MEMORY_BASIC_INFORMATION	stcMbi;
	SYSTEM_INFO					stcSi;
	PIMAGE_DOS_HEADER			pDosHeader;
	PIMAGE_NT_HEADERS			pNtHeader;
	PIMAGE_EXPORT_DIRECTORY		pExp;
	WCHAR						szDllName[MAX_NAME_LEN];
	DWORD						dwWideCharSize;
	DWORD						dwCurModIndex;			//��ǰModIndex
	PDWORD						dpExpAddrOfFunc;
	PDWORD						dpExpAddrOfName;
	PWORD						wpExpAddrOfNameOri;

	WCHAR						szTest[MAX_NAME_LEN] = { 0 };

	DWORD						dwObjectBaseAddr;
	WCHAR						szProgramName[MAX_PATH] = { 0 };


	//1.2 ��ȡϵͳ��Ϣ���õ��û��ռ�ķ�Χ�����ڱ���ģ��
	OutputDebugString(L"1.2 ��ȡϵͳ��Ϣ���õ��û��ռ�ķ�Χ�����ڱ���ģ��");

	GetSystemInfo(&stcSi);

	dwMinPageAddr = (DWORD)stcSi.lpMinimumApplicationAddress;
	dwMaxPageAddr = (DWORD)stcSi.lpMaximumApplicationAddress;
	dwPageAddr = dwMinPageAddr;

	
	//1.3 ��ȡ���������ģ����Ϣ����ӵ��б����ǲ����ڼ�ض���ʾ
	dwObjectBaseAddr = (DWORD)GetModuleHandle(NULL);
	
	GetModuleFileName((HMODULE)dwObjectBaseAddr, szProgramName, MAX_PATH);

	pDosHeader = (PIMAGE_DOS_HEADER)dwObjectBaseAddr;

	pNtHeader = (PIMAGE_NT_HEADERS)(pDosHeader->e_lfanew + dwObjectBaseAddr);


	dwCurModIndex = AddModuleToList(dwObjectBaseAddr, pNtHeader->OptionalHeader.SizeOfImage, PathFindFileName(szProgramName));

	SendModInfo(&g_lpModList[dwCurModIndex]);



	//2. ����ģ��
	for (; dwPageAddr < dwMaxPageAddr; dwPageAddr += stcMbi.RegionSize)
	{
		VirtualQuery((LPVOID)dwPageAddr, &stcMbi, sizeof(stcMbi));

		if (stcMbi.State != MEM_COMMIT || stcMbi.Protect & PAGE_GUARD)	continue;

		pDosHeader = (PIMAGE_DOS_HEADER)dwPageAddr;

		if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)	continue;

		pNtHeader = (PIMAGE_NT_HEADERS)(pDosHeader->e_lfanew + dwPageAddr);

		if (pNtHeader->Signature != IMAGE_NT_SIGNATURE)	continue;

		if (pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress == 0)	continue;

		pExp = (PIMAGE_EXPORT_DIRECTORY)(dwPageAddr + pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

		//���������ע��DLL��������
		if (dwPageAddr == dwSelfModBaseStart)	continue;

		ZeroMemory(szDllName, sizeof(WCHAR)*MAX_NAME_LEN);
		if (pExp->Name)
		{//����ģ���������⴦��������GetModuleFileName����ȡģ������

			dwWideCharSize = MultiByteToWideChar(CP_ACP, NULL, (CHAR*)(dwPageAddr + pExp->Name), strlen((CHAR*)(dwPageAddr + pExp->Name)),
				NULL, NULL);

			MultiByteToWideChar(CP_ACP, NULL, (CHAR*)(dwPageAddr + pExp->Name),
				strlen((CHAR*)(dwPageAddr + pExp->Name)),
				szDllName, dwWideCharSize);

			OutputDebugString(L"����ģ���������⴦��������GetModuleFileName����ȡģ������");
		}
		else
		{
			//�˴��Ĵ�����Ҫ�����Wow64ϵ�е�DLL�����ǵ����Ʋ����ø�API��ȡ,ֱ��������һ��ģ��
			if (GetModuleFileName((HMODULE)dwPageAddr, szDllName, MAX_NAME_LEN) == 0)
				continue;
		}

		OutputDebugString(szDllName);

		OutputDebugString(L"2.1 ���ģ����Ϣ");
		//2.1 ���ģ����Ϣ
		dwCurModIndex = AddModuleToList(dwPageAddr, pNtHeader->OptionalHeader.SizeOfImage, szDllName);
		dpExpAddrOfFunc = (PDWORD)(pExp->AddressOfFunctions + dwPageAddr);
		dpExpAddrOfName = (PDWORD)(pExp->AddressOfNames + dwPageAddr);
		wpExpAddrOfNameOri = (PWORD)(pExp->AddressOfNameOrdinals + dwPageAddr);
		g_lpModList[dwCurModIndex].dwApiListLen = pExp->NumberOfNames;

		//2.4 ����Mod�����Ϣ����ض�
		OutputDebugString(L"2.4 ����Mod�����Ϣ����ض�");

		SendModInfo(&g_lpModList[dwCurModIndex]);

	}//for


	SendFinish(FINISH_MODMSG);

	return TRUE;
}








//////////////////////////////////////////////////////////////////////////
//	��ӵ�ǰģ����Ϣ���б��У����б�ռ��Ѿ�ʹ����ϣ����������
//	������
//	DWORD	dwBaseAddr	- ģ���ַ
//	DWORD	dwImageSize	- ģ���С
//	WCHAR*	szModName	- ģ����
//	ע��	ʹ����g_dwPrivateIndexΪ���б��ȫ����������,�ҵ�һ����������ģ����Ϊ������Ϊ1	\
//	����Ϊ0��ģ��Ϊ����ģ����Ϣ
//	����ֵ:		�ɹ��򷵻ص�ǰ���б��е�����������Ϊ-1
//////////////////////////////////////////////////////////////////////////

DWORD	AddModuleToList(DWORD dwBaseAddr, DWORD dwImageSize, WCHAR* szModName)
{
	
	

	//���ռ䲻�㣬 ��չ����ռ�
	if (g_dwPrivateIndex >= g_dwModListLen)
	{
		lpModInfo	pTemp;
		//λ�ò�����Ҫ��չ���鳤��
		pTemp = (lpModInfo)VirtualAlloc(NULL, sizeof(ModInfo) * (g_dwModListLen + LIST_INC), MEM_COMMIT, PAGE_READWRITE);
		
		if (pTemp == NULL)	return -1;
	
		CopyMemory(pTemp, g_lpModList, sizeof(ModInfo)* g_dwModListLen);
		g_dwModListLen += LIST_INC;
		VirtualFree(g_lpModList, 0, MEM_RELEASE);
		g_lpModList = pTemp;
	}
		
	g_lpModList[g_dwPrivateIndex].dwBaseAddr	= dwBaseAddr;
	g_lpModList[g_dwPrivateIndex].dwImageSize	= dwImageSize;
	g_lpModList[g_dwPrivateIndex].dwModIndex	= g_dwPrivateIndex;
	g_lpModList[g_dwPrivateIndex].bIsFiltered	= FALSE;
	g_lpModList[g_dwPrivateIndex].bActive		= FALSE;
	g_lpModList[g_dwPrivateIndex].lpApiList		= NULL;
	g_lpModList[g_dwPrivateIndex].dwPrivateApiListIndex = 0;
	
	wcscpy_s(g_lpModList[g_dwPrivateIndex].szModName, szModName);
	
	g_dwPrivateIndex++;


	return g_dwPrivateIndex-1;
}


//////////////////////////////////////////////////////////////////////////
//	����Apilist�е�һ���ڵ㣬���������ű��ڲ���
//	ע��������չ���飬DetourAttach�����ᱣ��֮ǰδ��չ������
//	������
//	lpModInfo	lpCurModNode - ָ��ǰģ��ڵ�
//	����ֵ��	�ɹ��򷵻ص�ǰ���뵽�����������򷵻�-1
//////////////////////////////////////////////////////////////////////////

DWORD  AllocApiFromList(lpModInfo lpCurModNode)
{
	DWORD			dwApiIndex;
	DWORD			dwApiNum;
	lpApiInfo		pTemp;

	//���û������Apilist����������룬�������ú��������б���
	if (lpCurModNode->lpApiList == NULL)
	{
		if (lpCurModNode->dwApiListLen % LIST_API_INC)
		{
			dwApiNum = (lpCurModNode->dwApiListLen / LIST_API_INC + 1)*LIST_API_INC;
		}
		else
		{//������������ֹ����Ȳ���֪�ڴ��쳣���
			dwApiNum = lpCurModNode->dwApiListLen + LIST_API_INC;
		}

		//ע�⣬һ��Ҫ���뵽��ִ�е����ԣ���Ȼdetour�����ֱ�ӱ��쳣
		lpCurModNode->lpApiList = (lpApiInfo)VirtualAlloc(NULL, sizeof(ApiInfo) * dwApiNum, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		
		if (lpCurModNode->lpApiList == NULL) return -1;

		ZeroMemory(lpCurModNode->lpApiList, sizeof(ApiInfo)*dwApiNum);

		lpCurModNode->dwApiListLen = dwApiNum;

		lpCurModNode->dwPrivateApiListIndex = 0;

	}//if



	pTemp		= lpCurModNode->lpApiList;
	dwApiIndex	= lpCurModNode->dwPrivateApiListIndex;

	pTemp[dwApiIndex].dwModIndex = lpCurModNode->dwModIndex;
	lpCurModNode->dwPrivateApiListIndex++;

	return dwApiIndex;
}


//////////////////////////////////////////////////////////////////////////
//	��ʼ��ShellCode����дshellCode��Opcode
//	������	
//	lpShellCode lpShell - ָ��shellcode�ṹ��ָ��
//////////////////////////////////////////////////////////////////////////

VOID	InitApiShellStruct(lpShellCode lpShell)
{
	lpShell->byPushadOpc	= 0x60;
	lpShell->byPushEsp		= 0x54;
	lpShell->byPushOpc1		= 0x68;
	lpShell->byPushOpc2		= 0x68;
	lpShell->byCallOpc		= 0xE8;
	lpShell->byJmpOpcApi	= 0xE9;
	lpShell->byPopadOpc		= 0x61;
	
}



//////////////////////////////////////////////////////////////////////////
//	ж��Detour��hook����ԭԭ������ʹ������������
//////////////////////////////////////////////////////////////////////////

BOOL UnHookAllApi()
{
	lpModInfo					lpCurModNode;
	lpApiInfo					lpApiList;
	HANDLE						hSnapShot;
	THREADENTRY32				stcThreadInfo;


	//1. ʹ��detourˢ���߳�
	DetourTransactionBegin();
	DetourSetIgnoreTooSmall(TRUE);

	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());

	if (hSnapShot != INVALID_HANDLE_VALUE)
	{
		if (Thread32First(hSnapShot, &stcThreadInfo))
		{
			DetourUpdateThread(OpenThread(THREAD_QUERY_INFORMATION, FALSE, stcThreadInfo.th32ThreadID));
			while (Thread32Next(hSnapShot, &stcThreadInfo))
			{
				DetourUpdateThread(OpenThread(THREAD_QUERY_INFORMATION, FALSE, stcThreadInfo.th32ThreadID));
			}
		}
	}


	//2. ����ModList
	for (DWORD dwModIndex = 0; dwModIndex < g_dwModListLen; dwModIndex ++ )
	{
		lpCurModNode	= &g_lpModList[dwModIndex];
		lpApiList		= lpCurModNode->lpApiList;
		OutputDebugString(lpCurModNode->szModName);
		if(lpCurModNode->lpApiList == NULL) continue;

		//2.1 ������ӦMod��Apilist
		for (DWORD dwApiIndex = 0; dwApiIndex < lpCurModNode->dwApiListLen; dwApiIndex ++ )
		{

			if (lpApiList[dwApiIndex].bIsHooked)
			{
				DetourDetach((PVOID*)&lpApiList[dwApiIndex].dwApiRealAddr, lpApiList[dwApiIndex].lpShellBuf);
			}
		}
	}

	OutputDebugString(L"DetourDetach");

	if (DetourTransactionCommit() != NO_ERROR)
		return FALSE;
	

	return TRUE;

}


//////////////////////////////////////////////////////////////////////////
//	�ͷ��б�Ŀռ�
//	ע���ò���������UnHookApi֮��ִ�У��������ַ����쳣
//////////////////////////////////////////////////////////////////////////

BOOL FreeListMemory()
{
	lpModInfo	lpCurModNode;
	lpApiInfo	lpApiList;


	for (DWORD dwModIndex = 0; dwModIndex < g_dwModListLen; dwModIndex++)
	{
		lpCurModNode = &g_lpModList[dwModIndex];
		lpApiList = lpCurModNode->lpApiList;
		if (VirtualFree(lpApiList, 0, MEM_RELEASE) == 0) return FALSE;
	}
		
	if (VirtualFree(g_lpModList, 0, MEM_RELEASE) == 0)	return FALSE;

	return TRUE;

}

//////////////////////////////////////////////////////////////////////////
//	��ָ��ģ�����Hook������������API�Ĺ���
//	������
//	DWORD			dwModIndex	 - ģ������
//////////////////////////////////////////////////////////////////////////

VOID	HookModule(DWORD dwModIndex)
{
	//ɨ���ڴ��е�ģ��
	//ʹ��GetSystemInfo��VirtualQuery
	DWORD						dwPageAddr;



	MEMORY_BASIC_INFORMATION	stcMibOfApi;
	
	PIMAGE_DOS_HEADER			pDosHeader;
	PIMAGE_NT_HEADERS			pNtHeader;
	PIMAGE_EXPORT_DIRECTORY		pExp;

	DWORD						dwWideCharSize;
	DWORD						dwCurModIndex;			//��ǰModIndex
	DWORD						dwCurApiIndex;			//��ǰApiIndex
	DWORD						dwCurModBaseAddr;		//��ǰģ�����ʼ��ַ
	DWORD						dwCurModBaseEnd;		//��ǰģ���ĩ��ַ
	PDWORD						dpExpAddrOfFunc;
	PDWORD						dpExpAddrOfName;
	PWORD						wpExpAddrOfNameOri;

	lpApiInfo					lpCurApiList;
	ShellCode					stcShellCode;

	HANDLE						hSnapShot;
	THREADENTRY32				stcThreadInfo;
	BOOL						bIgnoringHook;
	DWORD						dwNewAddr;
	DWORD						dwTrapAddr;



	//1.  Detour�����߳�
	DetourTransactionBegin();
	DetourSetIgnoreTooSmall(TRUE);

	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());

	if (hSnapShot != INVALID_HANDLE_VALUE)
	{
		if (Thread32First(hSnapShot, &stcThreadInfo))
		{
			DetourUpdateThread(OpenThread(THREAD_QUERY_INFORMATION, FALSE, stcThreadInfo.th32ThreadID));
			while (Thread32Next(hSnapShot, &stcThreadInfo))
			{
				DetourUpdateThread(OpenThread(THREAD_QUERY_INFORMATION, FALSE, stcThreadInfo.th32ThreadID));
			}
		}
	}


	dwCurModIndex	= dwModIndex;
	dwPageAddr		= g_lpModList[dwCurModIndex].dwBaseAddr;
	//2.  ��ģ����е�����Ľ���

	pDosHeader = (PIMAGE_DOS_HEADER)dwPageAddr;

	pNtHeader = (PIMAGE_NT_HEADERS)(pDosHeader->e_lfanew + dwPageAddr);

	pExp = (PIMAGE_EXPORT_DIRECTORY)(dwPageAddr + pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);


	dpExpAddrOfFunc = (PDWORD)(pExp->AddressOfFunctions + dwPageAddr);
	dpExpAddrOfName = (PDWORD)(pExp->AddressOfNames + dwPageAddr);
	wpExpAddrOfNameOri = (PWORD)(pExp->AddressOfNameOrdinals + dwPageAddr);
	g_lpModList[dwCurModIndex].dwApiListLen = pExp->NumberOfNames;

	// ��ǰģ��ķ�Χ
	dwCurModBaseAddr	= dwPageAddr;
	dwCurModBaseEnd		= dwPageAddr + pNtHeader->OptionalHeader.SizeOfImage;


	//2.3 ��ʼ��Shellcode
	InitApiShellStruct(&stcShellCode);
	dwTrapAddr = (DWORD)GetProcAddress((HMODULE)dwSelfModBaseStart, "TrappedApiCall");


	for (DWORD i = 0; i < _countof(g_szFilteredDllList); i++)
	{
		if (_wcsnicmp(g_lpModList[dwCurModIndex].szModName,
			g_szFilteredDllList[i].szDllName,
			g_szFilteredDllList[i].dwCheckSize) == 0)
		{	
			return;
		}
	}


	//2.5 ����ģ���Api�����API��Ϣ,ֻ�����ַ���������Api
	for (DWORD dwIndex = 0; dwIndex < pExp->NumberOfNames; dwIndex++)
	{
		//Ԥ����,Ĭ��Bu����Hook
		bIgnoringHook = FALSE;

		//��ַ�Ƿ���ģ���⣬��֪����û�������������
		if ((dpExpAddrOfFunc[wpExpAddrOfNameOri[dwIndex]] + dwPageAddr) >= dwCurModBaseEnd ||
			(dpExpAddrOfFunc[wpExpAddrOfNameOri[dwIndex]] + dwPageAddr) <= dwCurModBaseAddr)
			continue;

		VirtualQuery((LPVOID)(dpExpAddrOfFunc[wpExpAddrOfNameOri[dwIndex]] + dwPageAddr),
			&stcMibOfApi,
			sizeof(MEMORY_BASIC_INFORMATION));

		//��Api�Ƿ��ִ��
		//	0xF0����˼���鿴MSDN�ĵ���ֻҪ�Ǵ���ִ�����ԣ������ڸ�4λ��λ
		//	ֻҪ��������&0xF0��ֻҪΪ1���Ϳ���˵�����������ִ��
		if (((stcMibOfApi.AllocationProtect & 0xF0) == 0) || stcMibOfApi.State != MEM_COMMIT)
			continue;


		//��������Ԥ�����������ڵ�
		dwCurApiIndex = AllocApiFromList(&g_lpModList[dwCurModIndex]);

		if (dwCurApiIndex == -1) continue;

		lpCurApiList = g_lpModList[dwCurModIndex].lpApiList;

		lpCurApiList[dwCurApiIndex].bIsHooked		= FALSE;			
		lpCurApiList[dwCurApiIndex].bIsFiltered		= FALSE;			//Ĭ�ϲ�����
		lpCurApiList[dwCurApiIndex].dwOridinal		= wpExpAddrOfNameOri[dwIndex];
		lpCurApiList[dwCurApiIndex].dwModIndex		= dwCurModIndex;
		lpCurApiList[dwCurApiIndex].dwApiIndex		= dwCurApiIndex;
		lpCurApiList[dwCurApiIndex].dwApiRealAddr	= dpExpAddrOfFunc[wpExpAddrOfNameOri[dwIndex]] + dwPageAddr;
		lpCurApiList[dwCurApiIndex].lpShellBuf		= NULL;

		dwWideCharSize = MultiByteToWideChar(CP_ACP, NULL, (CHAR*)(dwPageAddr + dpExpAddrOfName[dwIndex]), strlen((CHAR*)(dwPageAddr + dpExpAddrOfName[dwIndex])),
			NULL, NULL);

		MultiByteToWideChar(CP_ACP, NULL, (CHAR*)(dwPageAddr + dpExpAddrOfName[dwIndex]),
			strlen((CHAR*)(dwPageAddr + dpExpAddrOfName[dwIndex])),
			lpCurApiList[dwCurApiIndex].szApiName, dwWideCharSize);

		//2.6 ����Api�����Ϣ(ע�������ȷ�����Hook����Ȼ��Api��ʵ��ַ�ᱻDetour�滻)
		SendApiInfo(&lpCurApiList[dwCurApiIndex]);

	
		//	����API����

		bIgnoringHook = FilterApi( g_lpModList[dwCurModIndex].szModName,
			lpCurApiList[dwCurApiIndex].szApiName);
		


		if (bIgnoringHook == FALSE)
		{

			OutputDebugString(L"Api:");
			OutputDebugString(lpCurApiList[dwCurApiIndex].szApiName);

			lpCurApiList[dwCurApiIndex].lpShellBuf = (lpShellCode)VirtualAlloc(NULL, sizeof(ShellCode),
				MEM_COMMIT, PAGE_EXECUTE_READWRITE);

			stcShellCode.dwPushApiIndex = dwCurApiIndex;
			stcShellCode.dwPushModIndex = dwCurModIndex;


			stcShellCode.dwCallAddr = (DWORD)dwTrapAddr - ((DWORD)lpCurApiList[dwCurApiIndex].lpShellBuf + offsetof(ShellCode, dwCallAddr) + 4);
			stcShellCode.dwJmpApiAddr = lpCurApiList[dwCurApiIndex].dwApiRealAddr - ((DWORD)lpCurApiList[dwCurApiIndex].lpShellBuf + offsetof(ShellCode, dwJmpApiAddr) + 4);

			CopyMemory(lpCurApiList[dwCurApiIndex].lpShellBuf, &stcShellCode, sizeof(ShellCode));

			DetourAttachEx((PVOID*)&lpCurApiList[dwCurApiIndex].dwApiRealAddr,
				lpCurApiList[dwCurApiIndex].lpShellBuf,
				(PDETOUR_TRAMPOLINE*)&dwNewAddr, NULL, NULL);

			lpCurApiList[dwCurApiIndex].lpShellBuf->dwJmpApiAddr = dwNewAddr - ((DWORD)lpCurApiList[dwCurApiIndex].lpShellBuf + offsetof(ShellCode, dwJmpApiAddr) + 4);
			lpCurApiList[dwCurApiIndex].bIsHooked = TRUE;
		}
	}//for

	SendFinish(FINISH_APIMSG);	
	DetourTransactionCommit();
	g_lpModList[dwCurModIndex].bActive = TRUE;

}



//////////////////////////////////////////////////////////////////////////
//	��ָ��ģ���е�API����UnHook
//	������	DWORD	dwModIndex	 - ģ������
//	ע�⣺��_ApiInfo�е�IsHooked�ֶ����ж�
//////////////////////////////////////////////////////////////////////////
BOOL	UnHookModule(DWORD dwModIndex)
{
	lpModInfo					lpCurModNode;
	lpApiInfo					lpApiList;
	HANDLE						hSnapShot;
	THREADENTRY32				stcThreadInfo;


	//1. ʹ��detourˢ���߳�
	DetourTransactionBegin();
	DetourSetIgnoreTooSmall(TRUE);

	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());

	if (hSnapShot != INVALID_HANDLE_VALUE)
	{
		if (Thread32First(hSnapShot, &stcThreadInfo))
		{
			DetourUpdateThread(OpenThread(THREAD_QUERY_INFORMATION, FALSE, stcThreadInfo.th32ThreadID));
			while (Thread32Next(hSnapShot, &stcThreadInfo))
			{
				DetourUpdateThread(OpenThread(THREAD_QUERY_INFORMATION, FALSE, stcThreadInfo.th32ThreadID));
			}
		}
	}

		lpCurModNode	= &g_lpModList[dwModIndex];
		lpApiList		= lpCurModNode->lpApiList;

		//2.1 ������ӦMod��Apilist
		for (DWORD dwApiIndex = 0; dwApiIndex < lpCurModNode->dwApiListLen; dwApiIndex++)
		{
			if (lpApiList[dwApiIndex].bIsHooked)
			{
				DetourDetach((PVOID*)&lpApiList[dwApiIndex].dwApiRealAddr, lpApiList[dwApiIndex].lpShellBuf);
				lpApiList[dwApiIndex].bIsHooked = FALSE;
			}
		}
	


	if (DetourTransactionCommit() != NO_ERROR)
		return FALSE;


	return TRUE;
}




//////////////////////////////////////////////////////////////////////////
//	����ģ����Ϣ����ضˣ����뵱ǰMod�ڵ�Ľṹ�壬�ɸ�ģ��������ݰ���װ
//	��PcketModInfo��������һ��
//	������
//	lpModInfo lpCurModNode - ��ǰMod�ڵ�Ľṹ��
//////////////////////////////////////////////////////////////////////////


VOID SendModInfo(lpModInfo lpCurModNode)
{
	lpPacketModInfo lpCurModInfo;
	lpPacketInfo	lpCurPacket;
	DWORD			dwTotalLength;
	DWORD			dwBytesOfWritten;
	//�ȷ�װPacketModInfo
	lpCurModInfo = new PacketModInfo;
	lpCurModInfo->dwBaseAddr	= lpCurModNode->dwBaseAddr;
	lpCurModInfo->dwImageSize	= lpCurModNode->dwImageSize;
	lpCurModInfo->dwModIndex	= lpCurModNode->dwModIndex;
	
	//ע�⣬Ҫ��ʼ��dll�ַ������飬��Ȼ��������벻��������
	ZeroMemory(lpCurModInfo->szModName, sizeof(WCHAR)*MAX_NAME_LEN);

	//ע�⣺�����й����У�����Ĭ���Կ��ַ������д���,��ʡת������
	wcscpy_s(lpCurModInfo->szModName, lpCurModNode->szModName);

	//��һ����װPacketInfo��������Ϣ���ͺ����ݳ���
	dwTotalLength = sizeof(PacketModInfo) + sizeof(PacketInfo);
	lpCurPacket = (lpPacketInfo) new BYTE[dwTotalLength];
	lpCurPacket->dwType = MOD_MSG;
	lpCurPacket->dwLegth = sizeof(PacketModInfo);

	CopyMemory(lpCurPacket->Data, lpCurModInfo, lpCurPacket->dwLegth);

	WriteFile(g_hPipe2, lpCurPacket, dwTotalLength, &dwBytesOfWritten, NULL);

	
}


//////////////////////////////////////////////////////////////////////////
//	����Api��Ϣ����ضˣ����뵱ǰApi�ڵ���Ϣ����ģ����з������
//	������
//	lpApiInfo lpCurApiNode - Api�ڵ�Ľṹ��
//////////////////////////////////////////////////////////////////////////

VOID SendApiInfo(lpApiInfo lpCurApiNode)
{
	lpPacketApiInfo lpCurApiInfo;
	lpPacketInfo	lpCurPacket;
	DWORD			dwTotalLength;
	DWORD			dwBytesOfWritten;

	//�ȷ�װPacketApiInfo��������Api�Ĺؼ���Ϣ
	lpCurApiInfo = new PacketApiInfo;
	lpCurApiInfo->dwApiIndex	= lpCurApiNode->dwApiIndex;
	lpCurApiInfo->dwApiRealAddr = lpCurApiNode->dwApiRealAddr;
	lpCurApiInfo->dwModIndex	= lpCurApiNode->dwModIndex;
	lpCurApiInfo->dwOridinal	= lpCurApiNode->dwOridinal;
	
	wcscpy_s(lpCurApiInfo->szApiName, lpCurApiNode->szApiName);

	//��һ����װPacketInfo
	dwTotalLength = sizeof(PacketApiInfo) + sizeof(PacketInfo);
	lpCurPacket = (lpPacketInfo)new BYTE[dwTotalLength];

	lpCurPacket->dwType = API_MSG;
	lpCurPacket->dwLegth = sizeof(PacketApiInfo);

	CopyMemory(lpCurPacket->Data, lpCurApiInfo, lpCurPacket->dwLegth);

	WriteFile(g_hPipe2, lpCurPacket, dwTotalLength, &dwBytesOfWritten, NULL);
}



//////////////////////////////////////////////////////////////////////////
//	�жϸ�Api�Ƿ��ǹ��˵ģ��Ƚ���ģ����˼�⣬Ȼ����ż��Api����
//	������
//	DWORD dwModIndex - ��Api��Ӧ��Mod����
//	DWORD dwApiIndex - ��Api��Apilist�е�����
//	����ֵ��	���ǹ���Api���򷵻�TRUE������FALSE
//////////////////////////////////////////////////////////////////////////

BOOL IsFilteredApi(DWORD dwModIndex, DWORD dwApiIndex)
{
	lpApiInfo	lpCurApiNode;

	////��Api��Ӧ��ģ���Ƿ񱻹��ˣ�
	//if (g_lpModList[dwModIndex].bIsFiltered == TRUE)	return TRUE;

	lpCurApiNode = &g_lpModList[dwModIndex].lpApiList[dwApiIndex];

	if (lpCurApiNode->bIsFiltered == TRUE)	return TRUE;

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//	��װApi���ô�����Ϣ����Ϣ�������ص�ַ��ģ��������Api����(���ò���)
//	������
//	VOID* pStack		- ��Api�Ķ�ջָ��
//	DWORD dwModIndex	- ��Api��Ӧ��Mod����
//	DWORD dwApiIndex	- ��Api���������
//////////////////////////////////////////////////////////////////////////

VOID SendTrapInfo(VOID* pStack, DWORD dwModIndex, DWORD dwApiIndex, WCHAR* szParam)
{
	lpPacketTrapInfo	lpCurTrap;
	lpPacketInfo		lpCurPacket;
	DWORD				dwTotalLength;
	DWORD				dwBytesOfWritten;
	DWORD				dwParamLen;


	if (szParam != NULL)
		dwParamLen = (wcslen(szParam) + 1)*sizeof(WCHAR);
	else
		dwParamLen = 0;

	lpCurTrap = (lpPacketTrapInfo)new BYTE [sizeof(PacketTrapInfo) + dwParamLen];
	lpCurTrap->dwModIndex	= dwModIndex;
	lpCurTrap->dwApiIndex	= dwApiIndex;
	lpCurTrap->dwRetAddr	= *(PDWORD)pStack;
	lpCurTrap->dwLength		= dwParamLen;

	if (szParam != NULL)
		lstrcpyn((WCHAR*)lpCurTrap->byPara, szParam, wcslen(szParam) + 1);
	else
		lpCurTrap->byPara[0] = 0x0;
	

	dwTotalLength = sizeof(PacketTrapInfo) + sizeof(PacketInfo) + dwParamLen;
	lpCurPacket = (lpPacketInfo)new BYTE[dwTotalLength];

	lpCurPacket->dwType = TRAP_MSG;
	lpCurPacket->dwLegth = sizeof(PacketTrapInfo) + dwParamLen;
	
	CopyMemory(lpCurPacket->Data, lpCurTrap, lpCurPacket->dwLegth);

	WriteFile(g_hPipe2, lpCurPacket, dwTotalLength, &dwBytesOfWritten, NULL);
	delete lpCurPacket;
	delete lpCurTrap;
}


//////////////////////////////////////////////////////////////////////////
//	��������Ϣ����MOD���ݺ�API�����Ѿ�������ϣ������ü�ض˵�UI��������
//	���ԭ������ü�ض�ÿ���ڽ��յ����ݺ�ֱ�����ɣ��ᵼ�¿��ٺ�ֻ����ʾ
//	������Ϣ����������ֱ�ӽ�����ȫ���ڽ���UI���ɣ�Ч����Խϸ�
//////////////////////////////////////////////////////////////////////////

VOID SendFinish(DWORD	dwType)
{
	lpPacketInfo		lpCurPacket;
	DWORD				dwTotalLength;
	DWORD				dwBytesOfWritten;

	dwTotalLength = sizeof(PacketInfo);
	lpCurPacket = (lpPacketInfo)new BYTE[dwTotalLength];
	lpCurPacket->dwType = dwType;
	lpCurPacket->dwLegth = 0;

	WriteFile(g_hPipe2, lpCurPacket, dwTotalLength, &dwBytesOfWritten, NULL);
}





//////////////////////////////////////////////////////////////////////////
//	������������ȡApi�ĵ��ò��������ص�ַ
//	������
//	DWORD dwModIndex - ģ������������g_lpModList
//	DWORD dwApiIndex - Api���������ڶ�ӦMod�е�Apilist
//	VOID* pStack - Api��ջ��ָ��
//////////////////////////////////////////////////////////////////////////

extern "C"  VOID  __stdcall TrappedApiCall(DWORD dwModIndex, DWORD dwApiIndex, VOID* pStack)
{
	WCHAR*	szParam;
	if (IsHookCallAgain)				//����Ƿ�����TrappedApiCall()���е��ú�����Hook�������ѭ������
	{
		return	;						//�����⵽ѭ�����ã�����ֱ�ӷ���
	}
	else
	{
		IsHookCallAgain = TRUE;
	}

	pStack = (VOID*)((DWORD)pStack + 0x20);

	//���˼��
	if (IsFilteredApi(dwModIndex, dwApiIndex) == TRUE)
	{

		IsHookCallAgain = FALSE;
		return;
	}


	szParam = GetApiParam(g_lpModList[dwModIndex].szModName, g_lpModList[dwModIndex].lpApiList[dwApiIndex].szApiName,
		pStack);
	
	
	//���ʹ�����Ϣ����ض�
	SendTrapInfo(pStack, dwModIndex, dwApiIndex, szParam);

	if (szParam != NULL)
	{
		//OutputDebugString(szParam);
		VirtualFree(szParam, 0, MEM_RELEASE);
	}


	IsHookCallAgain = FALSE;
}




//////////////////////////////////////////////////////////////////////////
//	���¶�ָ��ģ�����Hook��Ŀ���Ǳ�������Ϊ��ģ������ͷ���API��Ϣ��ֻ
//	����DetourAttach
//	������
//	DWORD		dwModIndex		 - ָ��ģ�������
//////////////////////////////////////////////////////////////////////////


VOID	ReHookModule(DWORD dwModIndex)
{
	DWORD						dwCurModIndex;			//��ǰModIndex
	DWORD						dwCurApiIndex;			//��ǰApiIndex
	lpApiInfo					lpCurApiList;
	HANDLE						hSnapShot;
	THREADENTRY32				stcThreadInfo;
	BOOL						bIgnoringHook;
	DWORD						dwNewAddr;


	//1.  Detour�����߳�
	DetourTransactionBegin();
	DetourSetIgnoreTooSmall(TRUE);

	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());

	if (hSnapShot != INVALID_HANDLE_VALUE)
	{
		if (Thread32First(hSnapShot, &stcThreadInfo))
		{
			DetourUpdateThread(OpenThread(THREAD_QUERY_INFORMATION, FALSE, stcThreadInfo.th32ThreadID));
			while (Thread32Next(hSnapShot, &stcThreadInfo))
			{
				DetourUpdateThread(OpenThread(THREAD_QUERY_INFORMATION, FALSE, stcThreadInfo.th32ThreadID));
			}
		}
	}


	//2. ����ؼ�����
	dwCurModIndex	= dwModIndex;
	lpCurApiList	= g_lpModList[dwCurModIndex].lpApiList;


	for (DWORD i = 0; i < _countof(g_szFilteredDllList); i++)
	{
		if (_wcsnicmp(g_lpModList[dwCurModIndex].szModName,
			g_szFilteredDllList[i].szDllName,
			g_szFilteredDllList[i].dwCheckSize) == 0)
		{
			return;
		}
	}



	for (DWORD dwIndex = 0; dwIndex < g_lpModList[dwCurModIndex].dwPrivateApiListIndex; dwIndex++)
	{
		if( wcslen(lpCurApiList[dwIndex].szApiName) == 0)	break;

		bIgnoringHook = FALSE;

		dwCurApiIndex = dwIndex;
		//////////////////////////////////////////////////////////////////////////
		//	��������api
		//////////////////////////////////////////////////////////////////////////

		bIgnoringHook = FilterApi(g_lpModList[dwCurModIndex].szModName , 
			lpCurApiList[dwCurApiIndex].szApiName);


		if (bIgnoringHook == FALSE)
		{
			DetourAttachEx((PVOID*)&lpCurApiList[dwCurApiIndex].dwApiRealAddr,
				lpCurApiList[dwCurApiIndex].lpShellBuf,
				(PDETOUR_TRAMPOLINE*)&dwNewAddr, NULL, NULL);
			lpCurApiList[dwCurApiIndex].lpShellBuf->dwJmpApiAddr = dwNewAddr - ((DWORD)lpCurApiList[dwCurApiIndex].lpShellBuf + offsetof(ShellCode, dwJmpApiAddr) + 4);
			lpCurApiList[dwCurApiIndex].bIsHooked = TRUE;
		}


	}

	DetourTransactionCommit();
}



//////////////////////////////////////////////////////////////////////////
//	�ж�ָ����API�Ƿ���Ҫ����
//	������
//	WCHAR* szModName - API����ģ����
//	WCHAR* szApiName - api��
//	����ֵ�������򷵻�TRUE������FLASE
//////////////////////////////////////////////////////////////////////////

BOOL	FilterApi(WCHAR* szModName, WCHAR* szApiName)
{
	BOOL	bIgnoringHook = FALSE;
	BOOL	bExistInSS;
	WCHAR*	pOffset;
	WCHAR	szTest[MAX_PATH] = { 0 };

	bExistInSS = FALSE;

	//���ss�ļ����Ƿ���ڸ�ģ���������Ϣ����û�У�ֱ����Ϊȫ����API������HOOK
	for (DWORD dwModIndex = 0; dwModIndex < g_dwDefNUm; dwModIndex++)
	{
		if (_wcsnicmp(g_stcDef[dwModIndex].szDefFileObject, szModName, wcslen(g_stcDef[dwModIndex].szDefFileObject)) == 0)
		{//�ҵ�ģ��
			bExistInSS = TRUE;
			break;
		}
	}


	if (bExistInSS == FALSE)
	{//�����ڣ����ؿ�HOOK
		return TRUE;
	}

	//���ڸ�ģ��������ļ������˲���Ҫ������API
	for (DWORD dwModIndex = 0; dwModIndex < g_dwDefNUm ; dwModIndex++)
	{
		if (_wcsnicmp(g_stcDef[dwModIndex].szDefFileObject, szModName, wcslen(g_stcDef[dwModIndex].szDefFileObject)) == 0)
		{//�ҵ�ģ��
			if ((pOffset = wcsstr((WCHAR*)g_stcDef[dwModIndex].lpMapAddress, szApiName)) == NULL)
			{//û�ҵ�API
				bIgnoringHook = TRUE;
				return bIgnoringHook;
			}
			else
			{
				//�ҵ�ǰ׺����һ�����API���Ƿ���ȫƥ��
				if (_wcsnicmp(pOffset, szApiName, (DWORD)(wcsstr(pOffset, L"\r") - pOffset)) != 0)
				{
					bIgnoringHook = TRUE;
					return bIgnoringHook;
				}//if
			}//if
		}//if
		
	}//while

	return bIgnoringHook;

}


//BOOL	FilterApi(WCHAR* szModName, WCHAR* szApiName)
//{
//	BOOL	bIgnoringHook = FALSE;
//
//	//user32
//	if (wcscmp(L"gapfnScSendMessage", szApiName) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"OffsetRect", szApiName, wcslen(L"OffsetRect")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"IsRectEmpty", szApiName, wcslen(L"IsRectEmpty")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"GetWindowLong", szApiName, wcslen(L"GetWindowLong")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"GetParent", szApiName, wcslen(L"GetParent")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"IsWindow", szApiName, wcslen(L"IsWindow")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"SetWindowLong", szApiName, wcslen(L"SetWindowLong")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"GetKeyboardLayout", szApiName, wcslen(L"GetKeyboardLayout")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"SendMessage", szApiName, wcslen(L"SendMessage")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"GetClientRect", szApiName, wcslen(L"GetClientRect")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"GetWindow", szApiName, wcslen(L"GetWindow")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"PeekMessage", szApiName, wcslen(L"PeekMessage")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"IsDialogMessage", szApiName, wcslen(L"IsDialogMessage")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"GetSystemMetrics", szApiName, wcslen(L"IsDialogMessage")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"UnregisterClass", szApiName, wcslen(L"UnregisterClass")) == 0)
//		bIgnoringHook = TRUE;
//	
//	if (_wcsnicmp(L"CallWindowProc", szApiName, wcslen(L"CallWindowProc")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"GetProp", szApiName, wcslen(L"GetProp")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"SystemParametersInfo", szApiName, wcslen(L"SystemParametersInfo")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"GetDpiForSystem", szApiName, wcslen(L"GetDpiForSystem")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"GetSysColor", szApiName, wcslen(L"GetSysColor")) == 0)
//		bIgnoringHook = TRUE;
//	
//	if (_wcsnicmp(L"IsProcessDPIAware", szApiName, wcslen(L"IsProcessDPIAware")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"TranslateMessage", szApiName, wcslen(L"TranslateMessage")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"DispatchMessage", szApiName, wcslen(L"DispatchMessage")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"GetDlgCtrlID", szApiName, wcslen(L"GetDlgCtrlID")) == 0)
//		bIgnoringHook = TRUE;
//	
//	if (_wcsnicmp(L"GetMessage", szApiName, wcslen(L"GetMessage")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"IsThreadDesktopComposited", szApiName, wcslen(L"IsThreadDesktopComposited")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"SetProp", szApiName, wcslen(L"SetProp")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"PtInRect", szApiName, wcslen(L"PtInRect")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"ClientToScreen", szApiName, wcslen(L"ClientToScreen")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"CallMsgFilter", szApiName, wcslen(L"CallMsgFilter")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"IsTopLevelWindow", szApiName, wcslen(L"IsTopLevelWindow")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"WaitMessage", szApiName, wcslen(L"WaitMessage")) == 0)
//		bIgnoringHook = TRUE;
//
//
//	//msvcrt
//
//	if (_wcsnicmp(L"_osver", szApiName, wcslen(L"_osver")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"_environ", szApiName, wcslen(L"_environ")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"_iob", szApiName, wcslen(L"_iob")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"__threadid", szApiName, wcslen(L"__threadid")) == 0)
//		bIgnoringHook = TRUE;
//
//	//	__pioinfo
//	if (_wcsnicmp(L"__pioinfo", szApiName, wcslen(L"__pioinfo")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"_acmdln", szApiName, wcslen(L"_acmdln")) == 0)
//		bIgnoringHook = TRUE;
//
//
//
//
//	//kernel32
//
//	if (wcscmp(L"BaseFormatObjectAttributes", szApiName) == 0)
//		bIgnoringHook = TRUE;
//
//	if (wcscmp(L"DuplicateHandle", szApiName) == 0)
//		bIgnoringHook = TRUE;
//
//	if (wcscmp(L"HeapFree", szApiName) == 0)
//		bIgnoringHook = TRUE;
//
//	if (wcscmp(L"ReleaseMutex", szApiName) == 0)
//		bIgnoringHook = TRUE;
//
//	if (wcscmp(L"Sleep", szApiName) == 0)
//		bIgnoringHook = TRUE;
//	//kernel32
//	if (_wcsnicmp(L"OutputDebugString", szApiName, 17) == 0)
//		bIgnoringHook = TRUE;
//	//kernel32
//	if (_wcsnicmp(L"GetLastError", szApiName, 12) == 0)
//		bIgnoringHook = TRUE;
//	//kernel32
//	if (_wcsnicmp(L"wow64", szApiName, 5) == 0)
//		bIgnoringHook = TRUE;
//	//kernel32
//
//
//	if (_wcsnicmp(L"CreateEvent ", szApiName, wcslen(L"CreateEvent")) == 0)
//		bIgnoringHook = TRUE;
//
//
//
//	if (_wcsnicmp(L"GlobalFindAtom ", szApiName, wcslen(L"GlobalFindAtom")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"FlushInstructionCache ", szApiName, wcslen(L"FlushInstructionCache")) == 0)
//		bIgnoringHook = TRUE;
//	//kernel32
//	if (_wcsnicmp(L"AddRefActCtxWorker ", szApiName, wcslen(L"AddRefActCtxWorker")) == 0)
//		bIgnoringHook = TRUE;
//	//kernel32
//	if (_wcsnicmp(L"SetLastError", szApiName, wcslen(L"SetLastError")) == 0)
//		bIgnoringHook = TRUE;
//	//kernel32
//	if (_wcsnicmp(L"tls", szApiName, wcslen(L"tls")) == 0)
//		bIgnoringHook = TRUE;
//	//kernel32
//	if (_wcsnicmp(L"Rtl", szApiName, wcslen(L"Rtl")) == 0)
//		bIgnoringHook = TRUE;
//	//kernel32
//	if (_wcsnicmp(L"RaiseException", szApiName, wcslen(L"RaiseException")) == 0)
//		bIgnoringHook = TRUE;
//	//kernel32
//	if (_wcsnicmp(L"WaitFor", szApiName, wcslen(L"WaitFor")) == 0)
//		bIgnoringHook = TRUE;
//	//kernel32
//	if (_wcsnicmp(L"VirtualProtect", szApiName, wcslen(L"VirtualProtect")) == 0)
//		bIgnoringHook = TRUE;
//	//kernelbase
//	if (_wcsnicmp(L"HeapValidate", szApiName, wcslen(L"HeapValidate")) == 0)
//		bIgnoringHook = TRUE;
//	//ws2_32.dll
//	if (_wcsnicmp(L"WSASetLastError", szApiName, wcslen(L"WSASetLastError")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"WSAGetLastError ", szApiName, wcslen(L"WSASetLastError")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"_toupper", szApiName, wcslen(L"_toupper")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"_tolower", szApiName, wcslen(L"_tolower")) == 0)
//		bIgnoringHook = TRUE;
//
//	if (_wcsnicmp(L"_CrtIsValidHeapPointer", szApiName, wcslen(L"_CrtIsValidHeapPointer")) == 0)
//		bIgnoringHook = TRUE;
//
//
//	return bIgnoringHook;
//
//
//}