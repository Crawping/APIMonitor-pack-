// SaerPackBase.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "SaerPackBase.h"
#include "resource.h"


SAERPACKBASE_API BOOL PackBase(LPWSTR strPath, PSelectionInfo pSelect)
{
	COperationPE	stcObjectPE; //��������� PE�������
	PEInfo			stcPeInfo = { 0 }; // PE��Ϣ
	HANDLE  hFile_In;
	HANDLE  hFile_Out;
	DWORD   dwFileSize;
	LPBYTE  pFileImage;
	WCHAR   szOutPath[MAX_PATH] = { 0 };
	// 1. ��������ļ�·��

	LPWSTR strSuffix = PathFindExtension(strPath);				// ��ȡ�ļ��ĺ�׺��
	wcsncpy_s(szOutPath, MAX_PATH, strPath, wcslen(strPath));	// ����Ŀ���ļ�·����szOutPath
	PathRemoveExtension(szOutPath);								// ��szOutPath�б���·���ĺ�׺��ȥ��
	wcscat_s(szOutPath, MAX_PATH, L"_Pack");					// ��·����󸽼ӡ�_Pack��
	wcscat_s(szOutPath, MAX_PATH, strSuffix);					// ��·����󸽼Ӹոձ���ĺ�׺��
	LPWSTR strFileName = PathFindFileName(szOutPath);			// ��ȡ�޸ĺ���ļ���
	OutputDebugString(L"��������ļ�·��");

	//1.2 ��ȡ��Դ��ʽ��DLL�Ĵ�С
	// 1. ����Դ�ж�ȡ�ļ�����
	HRSRC   hREC = NULL; // ��Դ����
	HGLOBAL hREC_Handle = NULL; // ��Դ���
	DWORD   dwShellFileSize = NULL; // �ļ���С
	LPVOID  pResData = NULL; // ��Դ����ָ��
	HMODULE hModule = GetModuleHandle(L"SaerPackBase.dll");



	if (!(hREC = FindResource(hModule, MAKEINTRESOURCE(IDR_SHELL1), L"SHELL")))  return FALSE;
	if (!(hREC_Handle = LoadResource(hModule, hREC)))							 return FALSE;
	if (!(pResData = LockResource(hREC_Handle)))								 return FALSE;
	if (!(dwShellFileSize = SizeofResource(hModule, hREC)))							 return FALSE;



	// 2. ��ȡ�ļ���Ϣ����ӳ����ڴ���
	if (INVALID_HANDLE_VALUE == (hFile_In = CreateFile(strPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)))
	{
		return false;
	}

	if (INVALID_FILE_SIZE == (dwFileSize = GetFileSize(hFile_In, NULL)))
	{
		CloseHandle(hFile_In);
		return false;
	}

	//15000�Ŀռ�ʹΪ��������׼����
	if (!(pFileImage = (LPBYTE)VirtualAlloc(NULL, dwFileSize * 5 + dwShellFileSize + 15000, MEM_COMMIT, PAGE_READWRITE)))
	{
		CloseHandle(hFile_In);
		return false;
	}

	DWORD dwRet;


	if (!ReadFile(hFile_In, pFileImage, dwFileSize, &dwRet, NULL))
	{
		CloseHandle(hFile_In);
		VirtualFree(pFileImage, 0, MEM_RELEASE);
		return false;
	}

	OutputDebugString(L"��ȡ�ļ���Ϣ����ӳ����ڴ���");


	// 3. ��ȡPE�ļ���Ϣ

	ZeroMemory(&stcPeInfo, sizeof(PEInfo));

	if (stcObjectPE.GetPEInfo(pFileImage, dwFileSize, &stcPeInfo) == FALSE)
		return FALSE;
	
	OutputDebugString(L"��ȡPE�ļ���Ϣ");


	// 4. ��ȡĿ���ļ�����ε���ʼ������Ϣ
	//    ��ȡ��һ�����ε������Ϣ����������ܣ�Ĭ�ϵ�һ������Ϊ����Σ�
	DWORD	dwVirtualSize;
	if (!strncmp((CHAR*)stcPeInfo.pSectionHeader->Name, ".textbss", strlen(".textbss")))
	{//����һ�����β���textbss��������һ������
		stcPeInfo.pSectionHeader = stcPeInfo.pSectionHeader + 1;
	}
	

	if (stcPeInfo.pSectionHeader->Misc.VirtualSize % stcPeInfo.dwSectionAlign)
	{
		dwVirtualSize = (stcPeInfo.pSectionHeader->Misc.VirtualSize / stcPeInfo.dwSectionAlign + 1) * stcPeInfo.dwSectionAlign;
	}
	else
	{
		dwVirtualSize = (stcPeInfo.pSectionHeader->Misc.VirtualSize / stcPeInfo.dwSectionAlign) * stcPeInfo.dwSectionAlign;
	}

	LPBYTE pRawStart		= (LPBYTE)(stcPeInfo.pSectionHeader->PointerToRawData + (DWORD)pFileImage);
	LPBYTE pRawEnd			= (LPBYTE)((DWORD)pRawStart + stcPeInfo.pSectionHeader->SizeOfRawData);
	LPBYTE pVirtualStart	= (LPBYTE)(stcPeInfo.pSectionHeader->VirtualAddress);
	LPBYTE pVirtualEnd		= (LPBYTE)((DWORD)pVirtualStart + dwVirtualSize);

	stcPeInfo.dwCodeSize	= (DWORD)pVirtualEnd - (DWORD)pVirtualStart;	//���ڴ����ȶ����Ĵ�С
	stcPeInfo.dwCodeBase	= (DWORD)pVirtualStart;

	OutputDebugString(L"����β���");


	// 5. ���ļ�����Ԥ����
	//ѹ�����Σ��޸����α�,��������
	Pretreatment(pRawStart, pRawEnd, &stcObjectPE ,&stcPeInfo,  pSelect);

	//5.1 �������ļ���С����Ϊ����������������Σ�ʹ��ԭ�ļ���������仯
	dwFileSize = stcObjectPE.GetFileRawSize();

	OutputDebugString(L"�ļ�����Ԥ����");

	// 6. ֲ��Shell
	DWORD        dwShellSize = 0;
	SHELL_DATA	 stcParam = { 0 };
	stcParam.dwImageBase		= stcPeInfo.dwImageBase;
	stcParam.dwOldOEP			= stcPeInfo.dwOEP;			//ԭ����OEP
	stcParam.dwCodeBase			= stcPeInfo.dwCodeBase;
	stcParam.dwCodeSize			= stcPeInfo.dwCodeSize;
	stcParam.dwCodeRawSize		= stcPeInfo.dwCodeRawSize;
	stcParam.stcPEImportDir		= stcPeInfo.stcPEImportDir;
	stcParam.stcPERelocDir		= stcPeInfo.stcPERelocDir;
	stcParam.stcPEResDir		= stcPeInfo.stcPEResDir;
	stcParam.stcPETlsDir		= stcPeInfo.stcPETlsDir;
	stcParam.stcIATDir			= stcPeInfo.stcIATDir;
	stcParam.dwNumOfSections	= stcPeInfo.dwNumOfSections;
	//�����û����ò���
	CopyMemory(&stcParam.stcConfig, pSelect, sizeof(SelectionInfo));
	
	dwShellSize = Implantation(strFileName, dwFileSize, &stcObjectPE, stcPeInfo, &stcParam, pSelect);
	
	OutputDebugString(L"ֲ��Shell");

	// 7.�������Ҫ�ı�
	stcObjectPE.CleanDir();


	// 8. ����CRC32У����
	stcObjectPE.CalAndSaveCRC(dwShellSize + dwFileSize);
	

	OutputDebugString(L"�������Ҫ�ı�");

	// 9. ��������ɺ�Ľ��д�뵽���ļ���
	DWORD dwError;
	if (INVALID_HANDLE_VALUE != (hFile_Out = CreateFile(szOutPath, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL)))
	{
		DWORD dwRet = 0;
	
		WriteFile(hFile_Out, pFileImage, dwShellSize + dwFileSize, &dwRet, NULL);
		dwError = GetLastError();
	}




	
	OutputDebugString(L"��������ɺ�Ľ��д�뵽���ļ���");

	// 9. �ͷ������Դ������
	CloseHandle(hFile_In);
	CloseHandle(hFile_Out);
	VirtualFree(pFileImage, 0, MEM_RELEASE);

	OutputDebugString(L"�ͷ������Դ������");

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//	Ԥ�����޸Ĵ��������Ϊ�ɶ���д��ִ��
//	�Դ���ν��м򵥵��ֻ����,����ѹ������
//	������
//	LPBYTE				pCodeStart		����ε���ʼ��ַ
//	LPBYTE				pCodeEnd		����ε�ĩ��ַ
//	COperationPE*		pObejctPE		���������PE������
//	pPEInfo				pPeInfo			[out]�ؼ���Ϣ�ṹ
//	PSelectionInfo		pSelect			������Ϣ
//////////////////////////////////////////////////////////////////////////


VOID	Pretreatment(LPBYTE pCodeStart, LPBYTE pCodeEnd, COperationPE* pObjectPE, pPEInfo pPeInfo, PSelectionInfo pSelect)
{
	// 1.1���ݵ���������
	
	if (pPeInfo->bIsDll)
	{
		pObjectPE->GetExportBuf(pPeInfo);
	}

	// 1.2 ������˽�б����е�PeInfo�ṹ��
	pObjectPE->GetPEInfo(NULL, NULL, pPeInfo);

	// 1.3 �������������tls����
	pObjectPE->ReturnTlsModuleAddress(&pPeInfo->dwTlsModStart, &pPeInfo->dwTlsModEnd, &pPeInfo->dwTlsIndexValue); 

	// 1.4 ת�������

		DWORD	dwIATBase;
		DWORD	dwIATSize;
		LPBYTE	pNewAddr;
		DWORD	dwNewIATSize = 0;

		//ע�⣬�ڴ˴���õ�IAT���ڶβ�����ȷ��
		pObjectPE->FindSectionOfIAT(&dwIATBase, &dwIATSize);

		
		pPeInfo->dwOrigIATBase = dwIATBase;
		pPeInfo->dwOrigIATSize = dwIATSize;

		if (pSelect->bIsTransferIAT)
		{
			dwIATSize = pObjectPE->AlignSize(dwIATSize, pPeInfo->dwSectionAlign);
			pNewAddr = new BYTE[dwIATSize];
			ZeroMemory(pNewAddr, dwIATSize);
			dwNewIATSize = pObjectPE->MoveImportTable((DWORD)pNewAddr);
			pObjectPE->CleanImportTable();
			pPeInfo->pNewIATAddr = pNewAddr;
			pPeInfo->dwNewIATSize = dwNewIATSize;
		}


	// 1.5 ת���ض�λ��
	if (pPeInfo->stcPERelocDir.VirtualAddress)
		if(pSelect->bIsTransferReloc)
	{//��������ض�λ��Ļ�
		LPBYTE	pNewRelocAddr = new BYTE[pPeInfo->stcPERelocDir.Size];
		DWORD	dwNewRelocSize;

		ZeroMemory(pNewRelocAddr, pPeInfo->stcPERelocDir.Size);
		dwNewRelocSize = pObjectPE->MoveRelocTable((DWORD)pNewRelocAddr);
		pPeInfo->pNewRelocAddr	= pNewRelocAddr;
		pPeInfo->dwNewRelocSize = dwNewRelocSize;
		pObjectPE->CleanRelocTable();
	}


	//2. ������������
	pCode_Flow_Node			pCodeFlowHeader = NULL;
	WCHAR					szModPrefix[MAX_PATH];
	WCHAR					szConfig[MAX_PATH];
	DWORD					dwAvg;
	DWORD					dwNumOfNode;
	DWORD					dwNumOfMod;
	DWORD					dwRawSize;
	PDWORD					pdArrayMod;
	IMAGE_SECTION_HEADER	stcNewSectionOfJunkCode;
	LPBYTE					pNewSection = NULL;

	pSample_Array			pSampleArray = NULL;
	DWORD					dwTotalCtr;


	if (pSelect->bIsVirtualizeCode)
	{
		//2.1 ����ദ��
		pCodeFlowHeader = DrawCodeFlow(pObjectPE, pPeInfo, pCodeStart, pCodeEnd - pCodeStart);

		//2.2 �������ָ��ģ��ƽ������

		ZeroMemory(szModPrefix, sizeof(WCHAR)*MAX_PATH);

		if (GetConfigPath(szConfig, MAX_PATH) == FALSE)
		{
			MessageBox(NULL, L"GetConfigPath Error", L"Error", NULL);
			exit(0);
		}

		dwAvg = CalcAverageVal(szConfig, szModPrefix, &dwNumOfMod, &pdArrayMod);

		if (dwAvg == -1)
		{
			MessageBox(NULL, L"CalcAverageVal Error", L"Error", NULL);
			exit(0);
		}

		//2.3 ���㻨ָ�����γ��ȣ��������Ŀ�����

		dwNumOfNode = GetNumOfNode(pCodeFlowHeader);

		if (dwNumOfNode != 0)
		{

			if (dwNumOfNode >= 100)
			{//�������δ�С,��Щ����Ľڵ��ﵽ�������
				dwNumOfNode = 100;
			}

			pNewSection = pObjectPE->AddSection(L".pack4", dwNumOfNode * dwAvg,
				IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_MEM_EXECUTE,
				&stcNewSectionOfJunkCode,
				&dwRawSize);

			pObjectPE->GetPEInfo(NULL, NULL, pPeInfo);

			//2.4 ������

			//��ȡ��������
			pSampleArray = AnalyseSample(pSelect->szSample, &dwTotalCtr);


			if (ConfuseCode(pCodeFlowHeader, pObjectPE, pPeInfo, pNewSection, &stcNewSectionOfJunkCode,
				dwNumOfMod, szModPrefix, pdArrayMod, pSampleArray, dwTotalCtr) == FALSE)
			{
				MessageBox(NULL, L"ConfuseCode Error", L"Error", NULL);
				exit(0);
			}
		}//if

		ReleaseCodeFlow(pCodeFlowHeader);
		ReleaseSampleArray(pSampleArray, dwTotalCtr);
	}

	// 3. ����ָ������
	//	3.1 �����ڴ��д���ε�У��ֵ - CRC32
	LPBYTE	pCode = NULL;



	pCode = (LPBYTE)VirtualAlloc(NULL, pPeInfo->dwCodeSize, MEM_COMMIT, PAGE_READWRITE);
	if (pCode != NULL)
	{

		ZeroMemory(pCode, pPeInfo->dwCodeSize);

		CopyMemory(pCode, pCodeStart, pCodeEnd - pCodeStart);
			
		pObjectPE->CalMemCRC(pCode, pPeInfo->dwCodeSize, pPeInfo);

		VirtualFree(pCode, 0, MEM_RELEASE);
	}

	//	�����Ժ��ת����IAT����
	pPeInfo->dwCodeRawSize = pCodeEnd - pCodeStart;

	
	for (DWORD i=0; pCodeStart + i < pCodeEnd; i++)
	{
		pCodeStart[i] ^= i ;
	}

	

	// 4. ������θ����Ͽ�д����
	PDWORD pChara = &(pPeInfo->pSectionHeader->Characteristics);
	*pChara = *pChara | IMAGE_SCN_MEM_WRITE;

	// 5. �ؽ���Դ��
	if(pPeInfo->stcPEResDir.Size != 0)
	pObjectPE->ReBuildRes(pPeInfo);

	// 6.ѹ������
	// ������tls���򲻽���ѹ��
	if(pSelect->bIsCompression)
	if(!pPeInfo->bTls)
	pObjectPE->CompressSection(pPeInfo, pSelect);



}


//////////////////////////////////////////////////////////////////////////
//	ֲ��Shell�������������������
//	������
//	DWORD			dwFileBufSize			����������
//	COperationPE*	pObjectPE				���������PE������ָ��
//	PEInfo			stcPeInfo				���������PE�ؼ���Ϣ
//	PSHELL_DATA		pGlobalVar				���ݸ�Shell��ȫ�ֱ����ṹ��ָ��
//////////////////////////////////////////////////////////////////////////

DWORD	Implantation(
	LPWSTR pFileName,
	DWORD dwFileBufSize,
	COperationPE* pObjectPE, 
	PEInfo stcPeInfo, 
	PSHELL_DATA pGlobalVar,
	PSelectionInfo pSelect)
{
	// 1. ����Դ�ж�ȡ�ļ�����
	HRSRC   hREC = NULL; // ��Դ����
	HGLOBAL hREC_Handle = NULL; // ��Դ���
	DWORD   dwShellSize = NULL; // �ļ���С
	LPVOID  pResData = NULL; // ��Դ����ָ��
	HMODULE hModule = GetModuleHandle(L"SaerPackBase.dll");
	WCHAR*	szDirPath = new WCHAR[MAX_PATH];
	HANDLE	hFile;
	HMODULE	hMod;
	DWORD	dwBytesOfWritten;
	DWORD	dwDllOEP;		//dll����������OEP������������û����Ԥ����ѡ������ȨΪ��ڵ��ʹ�ø÷�����


	if (!(hREC = FindResource(hModule, MAKEINTRESOURCE(IDR_SHELL1), L"SHELL")))  return FALSE;
	if (!(hREC_Handle = LoadResource(hModule, hREC)))							 return FALSE;
	if (!(pResData = LockResource(hREC_Handle)))								 return FALSE;
	if (!(dwShellSize = SizeofResource(hModule, hREC)))							 return FALSE;


	//////////////////////////////////////////////////////////////////////////
	//	����Ϊ����ӵĴ��룬����BUG���Ӵ˴������޸�
	// 1.2 ����Դ��DLL�ͷ�Ϊ�ļ���ʽ�������ص��ڴ��л�ȡ������������OEP

	GetModuleFileName(NULL, szDirPath, MAX_PATH);
	PathRemoveFileSpec(szDirPath);
	BOOL bRootDir = PathIsRoot(szDirPath);

	if (bRootDir)
	{//���Ǹ�Ŀ¼�����üӷ�б��
		wcscat_s(szDirPath, MAX_PATH, L"Temp.dll");
	}
	else
	{
		wcscat_s(szDirPath, MAX_PATH, L"\\Temp.dll");
	}

	hFile = CreateFile(szDirPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == NULL)
		exit(0);

	WriteFile(hFile, pResData, dwShellSize, &dwBytesOfWritten, NULL);

	CloseHandle(hFile);

	hMod = LoadLibrary(szDirPath);

	PSHELL_DATA pTempShell = (PSHELL_DATA)GetProcAddress(hMod, "g_stcShellData");

	dwDllOEP = pTempShell->dwPEOEP - (DWORD)hMod;

	FreeLibrary(hMod);

	DeleteFile(szDirPath);

	//////////////////////////////////////////////////////////////////////////



	// 2. ��ȡShell���ֵĹؼ���Ϣ
	COperationPE  stcShellPE;
	PEInfo        stcShellPeInfo;
	LPBYTE        pData = new BYTE[dwShellSize];


	// 2.1 ��Stub���Ƶ���ʱ����������ֹ�ظ�����
	CopyMemory(pData, pResData, dwShellSize);
	// 2.2 ��ȡStub��PE��Ϣ
	stcShellPE.GetPEInfo(pData, dwShellSize, &stcShellPeInfo);

	// 2.3 �������ε������Ϣ��Ĭ�ϵ�һ������Ϊ����Σ�
	LPBYTE pCodeBuf			= (LPBYTE)(stcShellPeInfo.pSectionHeader->PointerToRawData + (DWORD)pData);
	DWORD dwCodeBufSize		= stcShellPeInfo.pSectionHeader->SizeOfRawData;
	DWORD dwCodeRealSize	= stcShellPeInfo.pSectionHeader->SizeOfRawData;

	//����dwCodeBufSize
	if (dwCodeBufSize % stcShellPeInfo.dwSectionAlign)
	{
		dwCodeBufSize = (dwCodeBufSize / stcShellPeInfo.dwSectionAlign + 1) * stcShellPeInfo.dwSectionAlign;
	}
	else
	{
		dwCodeBufSize = (dwCodeBufSize / stcShellPeInfo.dwSectionAlign ) * stcShellPeInfo.dwSectionAlign;
	}


	//2.4  �����ض�λ�ε������Ϣ
	DWORD	dwRelocSize;
	LPBYTE	pRelocBuf = stcShellPE.RVAToOffset(stcShellPeInfo.stcPERelocDir.VirtualAddress)  + pData;
	dwRelocSize = stcShellPeInfo.stcPERelocDir.Size;

	if (stcShellPeInfo.stcPERelocDir.Size % stcShellPeInfo.dwSectionAlign)
	{
		dwRelocSize = (dwRelocSize / stcShellPeInfo.dwSectionAlign + 1) * stcShellPeInfo.dwSectionAlign;
	}
	else
	{
		dwRelocSize = (dwRelocSize / stcShellPeInfo.dwSectionAlign ) * stcShellPeInfo.dwSectionAlign;
	}

	// 2.5 ����tls�ε������Ϣ
	DWORD	dwTlsSectionStartRVA;
	DWORD	dwTlsSize				= 0;
	LPBYTE	pTlsBuf					= NULL;
	if (stcPeInfo.stcPETlsDir.VirtualAddress)
	{//�ж��Ƿ����tls��	


		//���ҵ�tls�ε���ʼλ��

		stcShellPE.FindSectionOfTls(&stcShellPeInfo, &dwTlsSectionStartRVA, &dwTlsSize);

		pTlsBuf = stcShellPE.RVAToOffset(dwTlsSectionStartRVA) + pData;
		
		if (dwTlsSize % stcShellPeInfo.dwSectionAlign)
		{
			dwTlsSize = (dwTlsSize / stcShellPeInfo.dwSectionAlign + 1) * stcShellPeInfo.dwSectionAlign;
		}
		else
		{
			dwTlsSize = (dwTlsSize / stcShellPeInfo.dwSectionAlign) * stcShellPeInfo.dwSectionAlign;
		}

		pGlobalVar->bTlsExist = TRUE;

	}
	else
	{
		pGlobalVar->bTlsExist = FALSE;
	}


	// 2.6 ����idata����Ϣ
	PIMAGE_SECTION_HEADER	pSecondSectionHeader = (stcShellPeInfo.pSectionHeader + 1);
	DWORD					dwDataSectionSize = pSecondSectionHeader->Misc.VirtualSize;
	DWORD					dwDataSectionRealSize = pSecondSectionHeader->Misc.VirtualSize;
	LPBYTE					pDataBuf = pSecondSectionHeader->PointerToRawData + pData;

	if (dwDataSectionSize % stcShellPeInfo.dwSectionAlign)
	{
		dwDataSectionSize = (dwDataSectionSize / stcShellPeInfo.dwSectionAlign + 1) * stcShellPeInfo.dwSectionAlign;
	}
	else
	{
		dwDataSectionSize = (dwDataSectionSize / stcShellPeInfo.dwSectionAlign) * stcShellPeInfo.dwSectionAlign;
	}
	

	//ע�⣬����Ŀǰshellû�е��������idata�β����ڣ��Ͳ�������!!!!!!!
	dwDataSectionSize = dwDataSectionRealSize = 0;


	// 2.7 ��ԭIAT��Ϣ������shell��ȫ�ֱ�����
	pGlobalVar->dwIATSectionBase = stcPeInfo.dwOrigIATBase;
	pGlobalVar->dwIATSectionSize = stcPeInfo.dwOrigIATSize;



	// 2.8 ������������dll����������еĻ�
	DWORD	dwExportTableSize	= 0;
	LPBYTE	pExportBuf			= NULL;
	if (stcPeInfo.bIsDll)
	{
		dwExportTableSize	= stcPeInfo.pDataDir[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
		pExportBuf			= stcPeInfo.pExportBuf;
	}


	// 2.9 ������������ɵĽڱ��Ա��ѹʱ�ָ�������Ϣ,����ѹ�����εĹؼ���Ϣ����
	DWORD	dwCompressInfoSize = 0;
	DWORD	dwOriginalSecTableSize = 0;
	if (pSelect->bIsCompression)
	{
		if (!stcPeInfo.bTls)
		{
			dwCompressInfoSize = stcPeInfo.dwNumOfSections * sizeof(ComPressInfo);
			dwOriginalSecTableSize = stcPeInfo.dwNumOfSections * sizeof(IMAGE_SECTION_HEADER);
		}
	}
	

	// 2.10 ��ת�����IAT����
	DWORD	dwNewIATSize = stcPeInfo.dwNewIATSize;
	

	// 2.11 ��ת������ض�λ����
	DWORD	dwNewRelocSize = stcPeInfo.dwNewRelocSize;

	// 3. �������
	DWORD	dwNewSectionRawSize		= 0;		//pack2�ε�rawsize
	DWORD	dwNewSectionRealSize	= 0;		//pack2�ε���ʵ��С
	IMAGE_SECTION_HEADER  stcNewSection = { 0 };


	dwNewSectionRealSize = dwRelocSize + dwCodeBufSize + dwTlsSize + dwDataSectionSize + dwExportTableSize + dwOriginalSecTableSize +
							dwCompressInfoSize + dwNewIATSize + dwNewRelocSize;


	LPBYTE pNewSectionData = pObjectPE->AddSection(L".pack2", dwNewSectionRealSize,
		IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_MEM_EXECUTE, 
		&stcNewSection, &dwNewSectionRawSize);


	// 4. ��Stub���ֽ��е��ض�λ����
	//    �µļ��ص�ַ = (�����ε���ʼRVA - Shell��".Text"���ε���ʼRVA) + ӳ���ַ
	DWORD dwLoadImageAddr = (stcNewSection.VirtualAddress - stcShellPeInfo.pSectionHeader->VirtualAddress) + stcPeInfo.dwImageBase;
	stcShellPE.RECReloc(dwLoadImageAddr, &stcNewSection);

	// 4.1 ����tls�������ԣ���ģ������ʼֹ��ַ��������ַ��������
	if (pGlobalVar->bTlsExist)
	{
		//������shell��tls����
		stcShellPE.ChangeModuleData(stcPeInfo.dwTlsModStart, stcPeInfo.dwTlsModEnd, stcPeInfo.dwTlsIndexValue);
	}

	// 5. д�����ò���
	// 5.1 ��ȡShell�ĵ���������ַ

	PSHELL_DATA pShellGlobalVarAddr;
	CHAR		szDllName[MAX_PATH] = { 0 };

	//��ģ������ִ�Unicodeת��Ϊ���ֽ�
	WideCharToMultiByte(CP_ACP, NULL, pFileName, -1, szDllName, _countof(szDllName), NULL, FALSE);
	CopyMemory(pGlobalVar->szDllName, szDllName, strlen(szDllName));
	pShellGlobalVarAddr = (PSHELL_DATA)stcShellPE.GetExpVarAddr(L"g_stcShellData");

	if (pGlobalVar->bTlsExist)
	{
		pGlobalVar->stcPETlsShellDir.VirtualAddress = stcShellPeInfo.stcPETlsDir.VirtualAddress - stcShellPeInfo.pSectionHeader->VirtualAddress + stcNewSection.VirtualAddress;
		pGlobalVar->stcPETlsShellDir.Size = stcShellPeInfo.stcPETlsDir.Size;
	}

	if (stcPeInfo.bIsDll)
	{
		pGlobalVar->bDll = TRUE;
	}
	else
	{
		pGlobalVar->bDll = FALSE;
	}
	
	// 5.2 ����ڱ��ѹ���ṹ���RVA
	pGlobalVar->dwOriginalSecRva	= pObjectPE->OffsetToRVA(stcNewSection.PointerToRawData + dwRelocSize + dwCodeBufSize + dwTlsSize + dwDataSectionSize + dwExportTableSize);
	pGlobalVar->dwCompressInfoRva	= pObjectPE->OffsetToRVA(stcNewSection.PointerToRawData + dwRelocSize + dwCodeBufSize + dwTlsSize + dwDataSectionSize + dwExportTableSize + dwOriginalSecTableSize);
	pGlobalVar->dwNewIATRva			= pObjectPE->OffsetToRVA(stcNewSection.PointerToRawData + dwRelocSize + dwCodeBufSize + dwTlsSize + dwDataSectionSize + dwExportTableSize + dwOriginalSecTableSize +
										dwCompressInfoSize);
	pGlobalVar->dwNewRelocRva		= pObjectPE->OffsetToRVA(stcNewSection.PointerToRawData + dwRelocSize + dwCodeBufSize + dwTlsSize + dwDataSectionSize + dwExportTableSize + dwOriginalSecTableSize +
										dwCompressInfoSize + dwNewIATSize);

	// 5.3 �������ε�CRC32ֵ
	pGlobalVar->dwCodeMemCRC32		= stcPeInfo.dwCodeMemCRC32;

	CopyMemory(pShellGlobalVarAddr, pGlobalVar, sizeof(SHELL_DATA));



	// 6. ��Shell���Ƶ���������

	LPBYTE	pCombinedBuf = new BYTE[dwNewSectionRawSize];
	DWORD	dwCombinedOffset = 0;
	memset(pCombinedBuf, 0x0, dwNewSectionRawSize);

	// 6.1 ���ƴ����
	//�˴�����ǰ�code�ε���ʵ��Сд���룬��Ҫ�ö����Ĵ�С�����ܻᵼ�·���Խ�����
	CopyMemory(pCombinedBuf + dwCombinedOffset, pCodeBuf, dwCodeRealSize);
	dwCombinedOffset += dwCodeBufSize;

	// 6.2 ����idata��
	//�˴�����ǰ�idata�ε���ʵ��Сд���룬��Ҫ�ö����Ĵ�С�����ܻᵼ�·���Խ�����
	//CopyMemory(pCombinedBuf + dwCombinedOffset, pDataBuf, dwDataSectionRealSize);
	//dwCombinedOffset += dwDataSectionSize;
	
	// 6.3 ����tls��
	if (pGlobalVar->bTlsExist)
	{//��tls���ڵĻ�
		CopyMemory(pCombinedBuf + dwCombinedOffset, pTlsBuf, dwTlsSize);
		dwCombinedOffset += dwTlsSize;
	}
	

	// 6.4 �����ض�λ��
	//�˴�����ǰ��ض�λ�ε���ʵ��Сд���룬��Ҫ�ö����Ĵ�С�����ܻᵼ�·���Խ�����
	CopyMemory(pCombinedBuf + dwCombinedOffset, pRelocBuf, stcShellPeInfo.stcPERelocDir.Size);
	dwCombinedOffset += dwRelocSize;


	// 6.5 ���Ƶ�����
	if (stcPeInfo.bIsDll)
	{
		//�ض�λ������
		pObjectPE->RelocExportTable(pObjectPE->OffsetToRVA(stcNewSection.PointerToRawData + dwCodeBufSize + dwTlsSize + dwDataSectionSize + dwRelocSize),
			(PIMAGE_EXPORT_DIRECTORY)pExportBuf);

		CopyMemory(pCombinedBuf + dwCombinedOffset, pExportBuf, dwExportTableSize);
		dwCombinedOffset += dwExportTableSize;
	}

	// 6.6 ��ѹ�����ݺͽڱ��Ƶ�shell�Ŀռ���
	if (pSelect->bIsCompression)
	if (!stcPeInfo.bTls)
	{
		// 6������������Ľڱ�
		CopyMemory(pCombinedBuf + dwCombinedOffset, stcPeInfo.pOriginalSecTable, dwOriginalSecTableSize);
		dwCombinedOffset += dwOriginalSecTableSize;

		// �������������ѹ�����νṹ
		
		CopyMemory(pCombinedBuf + dwCombinedOffset, stcPeInfo.pCompressInfo, dwCompressInfoSize);
		dwCombinedOffset += dwCompressInfoSize;
		
	}

	// 6.7����ת�����IAT��
	if (pSelect->bIsTransferIAT)
	{
		CopyMemory(pCombinedBuf + dwCombinedOffset, stcPeInfo.pNewIATAddr, dwNewIATSize);
		dwCombinedOffset += dwNewIATSize;
	}

	// 6.8����ת������ض�λ��
	if (pSelect->bIsTransferReloc)
	{
		CopyMemory(pCombinedBuf + dwCombinedOffset, stcPeInfo.pNewRelocAddr, dwNewRelocSize);
		dwCombinedOffset += dwNewRelocSize;
	}



	//�����λ���������ת���������οռ�
	//dwCombinedOffset�൱���������εĴ�С
	CopyMemory(pNewSectionData, pCombinedBuf, dwCombinedOffset);




	// 7. �޸���Դ��
	// �����������Ҫת��
	// ����Դ��Ĵ������жϴ���
	DWORD	dwNewResSectionSize = 0;
	if (stcPeInfo.stcPEResDir.Size != 0)
	dwNewResSectionSize = pObjectPE->FixRes(pObjectPE, &stcPeInfo);


	// 8. ���㲢������OEP
	DWORD	dwNewOEP = 0;
	DWORD	dwShellOEP;
	//dwShellOEP = stcShellPeInfo.dwOEP;		//shell��dll�е�OEP
	dwShellOEP = dwDllOEP;

	// 8.1 ������OEP
	DWORD dwShellCodeRVA	= stcShellPeInfo.pSectionHeader->VirtualAddress;
	DWORD dwNewSectionRVA	= stcNewSection.VirtualAddress;

	dwNewOEP = (dwShellOEP - dwShellCodeRVA) + dwNewSectionRVA;

	// 8.2 ������OEP
	pObjectPE->SetOEP(dwNewOEP);

	// 8.3 ��������������ض�λ������Ŀ¼��
	pObjectPE->SetDir(IMAGE_DIRECTORY_ENTRY_BASERELOC,
		pObjectPE->OffsetToRVA(stcNewSection.PointerToRawData + dwCodeBufSize + dwTlsSize + dwDataSectionSize),
		stcShellPeInfo.stcPERelocDir.Size);

	// 8.4 �������������tls������Ŀ¼��
	if (pGlobalVar->bTlsExist)
	{
		pObjectPE->SetDir(IMAGE_DIRECTORY_ENTRY_TLS,
			stcShellPeInfo.stcPETlsDir.VirtualAddress - stcShellPeInfo.pSectionHeader->VirtualAddress + stcNewSection.VirtualAddress,
			stcShellPeInfo.stcPETlsDir.Size);
	}

	// 8.5 ������������ĵ���������Ŀ¼��
	if (stcPeInfo.bIsDll)
	{
		pObjectPE->SetDir(IMAGE_DIRECTORY_ENTRY_EXPORT,
			pObjectPE->OffsetToRVA(stcNewSection.PointerToRawData +\
				dwCodeBufSize + dwTlsSize + dwDataSectionSize + dwRelocSize),
			stcPeInfo.pDataDir[IMAGE_DIRECTORY_ENTRY_EXPORT].Size);
	}


	// 9. �ͷ���Դ����������
	delete[] pData;
	delete[] pCombinedBuf;
	FreeResource(hREC_Handle);
	return dwNewSectionRawSize + dwNewResSectionSize;


}