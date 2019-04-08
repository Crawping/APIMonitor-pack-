#include "stdafx.h"
#include "OperationPE.h"


COperationPE::COperationPE()
{
	ZeroMemory(&m_stcPeInfo, sizeof(PEInfo));
}


COperationPE::~COperationPE()
{
}


DWORD COperationPE::RVAToOffset(DWORD dwRVA)
{
	PIMAGE_SECTION_HEADER	pSectionHeader		= IMAGE_FIRST_SECTION(m_pNtHeader);
	DWORD					dwNumberOfSections	= m_pNtHeader->FileHeader.NumberOfSections;

	for (DWORD i = 0; i < dwNumberOfSections ; i++ ,pSectionHeader++)
	{
		if (pSectionHeader->VirtualAddress <= dwRVA && dwRVA < pSectionHeader->Misc.VirtualSize + pSectionHeader->VirtualAddress)
		{
			return	dwRVA - pSectionHeader->VirtualAddress + pSectionHeader->PointerToRawData;
		}
	}

	return	NULL;
}


DWORD	COperationPE::OffsetToRVA(DWORD dwRawPointer)
{
	PIMAGE_SECTION_HEADER	pSectionHeader = IMAGE_FIRST_SECTION(m_pNtHeader);
	DWORD					dwNumberOfSections = m_pNtHeader->FileHeader.NumberOfSections;

	for (DWORD i = 0; i < dwNumberOfSections; i++, pSectionHeader++)
	{
		if (pSectionHeader->PointerToRawData <= dwRawPointer && 
			dwRawPointer < pSectionHeader->SizeOfRawData + pSectionHeader->PointerToRawData)
		{
			return	dwRawPointer - pSectionHeader->PointerToRawData + pSectionHeader->VirtualAddress;
		}
	}

	return	NULL;
}


//////////////////////////////////////////////////////////////////////////
//	��ȡ��ǰ������ļ�ָ��
//	����ֵ��
//	DWORD �ļ�ָ��
//////////////////////////////////////////////////////////////////////////

DWORD	COperationPE::GetFileAddr()
{
	return m_dwFileDataAddr;
}

//////////////////////////////////////////////////////////////////////////
//	��ȡ��ǰ������ļ���С
//	����ֵ��
//	DWORD �ļ���С
//////////////////////////////////////////////////////////////////////////
DWORD	COperationPE::GetFileRawSize()
{
	return m_dwFileDataSize;
}


//////////////////////////////////////////////////////////////////////////
//	��ȡpFileBuf�������еĹؼ�PE��Ϣ
//	������
//	LPBYTE pFileBuf			Ŀ�����Ļ�����,�����Ǹ�������PE�ļ�ӳ��Ļ�����
//	DWORD dwFileSize		Ŀ�������ļ���С����FileAlign���룩
//	pPEInfo pObjectPEInfo	[out]Ŀ���ļ��Ĺؼ���Ϣ�ṹ��
//////////////////////////////////////////////////////////////////////////

BOOL COperationPE::GetPEInfo(LPBYTE pFileBuf, DWORD dwFileSize, pPEInfo pObjectPEInfo)
{

	if (m_stcPeInfo.dwOEP)
	{
		//�ָ���һ������,��Ϊ֮ǰ���textbss������
		pObjectPEInfo->pSectionHeader = IMAGE_FIRST_SECTION(m_pNtHeader);
		//�������������������������Ҫ�ô���
		pObjectPEInfo->dwNumOfSections = m_pNtHeader->FileHeader.NumberOfSections;
		memcpy_s(&m_stcPeInfo, sizeof(PEInfo), pObjectPEInfo, sizeof(PEInfo));
		return	TRUE;
	}
	else
	{
		if (!pFileBuf)
			return FALSE;
		m_dwFileDataAddr = (DWORD)pFileBuf;
		m_dwFileDataSize = (DWORD)dwFileSize;
	}




	m_pDosHeader	= (PIMAGE_DOS_HEADER)pFileBuf;
	m_pNtHeader		= (PIMAGE_NT_HEADERS)(m_pDosHeader->e_lfanew + pFileBuf);
	m_bCRC32Table	= FALSE;
	pObjectPEInfo->dwOEP				= m_pNtHeader->OptionalHeader.AddressOfEntryPoint;
	pObjectPEInfo->dwImageBase			= m_pNtHeader->OptionalHeader.ImageBase;
	pObjectPEInfo->dwSizeOfImage		= m_pNtHeader->OptionalHeader.SizeOfImage;
	pObjectPEInfo->pDataDir				= m_pNtHeader->OptionalHeader.DataDirectory;
	pObjectPEInfo->dwNumOfSections		= m_pNtHeader->FileHeader.NumberOfSections;
	pObjectPEInfo->pSectionHeader		= IMAGE_FIRST_SECTION(m_pNtHeader);
	pObjectPEInfo->stcPEImportDir		= IMAGE_DATA_DIRECTORY(pObjectPEInfo->pDataDir[IMAGE_DIRECTORY_ENTRY_IMPORT]);
	pObjectPEInfo->stcPERelocDir		= IMAGE_DATA_DIRECTORY(pObjectPEInfo->pDataDir[IMAGE_DIRECTORY_ENTRY_BASERELOC]);
	pObjectPEInfo->stcPEResDir			= IMAGE_DATA_DIRECTORY(pObjectPEInfo->pDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE]);
	pObjectPEInfo->stcPETlsDir			= IMAGE_DATA_DIRECTORY(pObjectPEInfo->pDataDir[IMAGE_DIRECTORY_ENTRY_TLS]);
	pObjectPEInfo->stcIATDir			= IMAGE_DATA_DIRECTORY(pObjectPEInfo->pDataDir[IMAGE_DIRECTORY_ENTRY_IAT]);
	pObjectPEInfo->dwSectionAlign		= m_pNtHeader->OptionalHeader.SectionAlignment;
	pObjectPEInfo->dwFileAlign			= m_pNtHeader->OptionalHeader.FileAlignment;
	pObjectPEInfo->dwTlsOffset			= 0;
	pObjectPEInfo->dwTlsSectionRVA		= 0;
	pObjectPEInfo->dwSizeOfHeader		= m_pNtHeader->OptionalHeader.SizeOfHeaders;
	pObjectPEInfo->pExpAddrOfName		= NULL;
	pObjectPEInfo->pOriginalSecTable	= NULL;
	pObjectPEInfo->dwOrigIATBase		= NULL;
	pObjectPEInfo->dwOrigIATSize		= NULL;
	pObjectPEInfo->pNewIATAddr			= NULL;
	pObjectPEInfo->dwNewIATSize			= NULL;
	pObjectPEInfo->pNewRelocAddr		= NULL;
	pObjectPEInfo->dwNewRelocSize		= NULL;
	pObjectPEInfo->dwCodeMemCRC32		= NULL;
	pObjectPEInfo->dwCodeBase			= NULL;
	pObjectPEInfo->dwCodeSize			= 0;
	pObjectPEInfo->dwCodeRawSize		= 0;


	if ((m_pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) || (m_pNtHeader->Signature != IMAGE_NT_SIGNATURE))
	{
		// �ⲻ��һ����Ч��PE�ļ�
		return FALSE;
	}

	if (m_pNtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64 ||
		m_pNtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
	{
		MessageBox(NULL, L"��֧��x64����ļӿǲ�����", L"����", NULL);
		exit(0);
	}



	if ((m_pNtHeader->FileHeader.Characteristics & 0x2000) == IMAGE_FILE_DLL)
	{//�ж��Ƿ�Ϊdll�ļ�
		pObjectPEInfo->bIsDll = TRUE;
	}
	else
	{
		pObjectPEInfo->bIsDll = FALSE;
	}

	if (pObjectPEInfo->stcPETlsDir.VirtualAddress != 0)
	{//�ж��Ƿ���tls��
		pObjectPEInfo->bTls = TRUE;
	}
	else
	{
		pObjectPEInfo->bTls = FALSE;
	}


	memcpy_s(&m_stcPeInfo, sizeof(PEInfo), pObjectPEInfo, sizeof(PEInfo));

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//	�ض�λ����ֻ����Shell��COperationPE������ã��޸�Shell�������е��ض�λ��,
//	��tls��������⴦��
//	������
//	DWORD					dwLoadImageBase			�µ�����RVA���������������ĳ�Ա��- Shell��"Text"RVA + ��������ImageBase
//	PIMAGE_SECTION_HEADER	pObjectPeNewSection		���������������ͷ�ṹ��
//////////////////////////////////////////////////////////////////////////

VOID COperationPE::RECReloc(DWORD dwLoadImageBase, PIMAGE_SECTION_HEADER	pObjectPeNewSection)
{
	typedef struct _TYPEOFFSET
	{
		WORD offset : 12;			//ƫ��ֵ
		WORD Type : 4;			//�ض�λ����(��ʽ)
	}TYPEOFFSET, *PTYPEOFFSET;

	PIMAGE_BASE_RELOCATION	pReloc = (PIMAGE_BASE_RELOCATION)(m_dwFileDataAddr + RVAToOffset(m_stcPeInfo.stcPERelocDir.VirtualAddress));
	DWORD					dwRelocOfItemNum;
	DWORD					dwRVA;
	DWORD					dwItemAddressOfReloc;
	DWORD					dwTestAddr;
	PTYPEOFFSET				pOffset;
	DWORD					dwTestRVA;
	DWORD					dwEndOfReloc = (DWORD)pReloc + m_stcPeInfo.stcPERelocDir.Size;

	while (pReloc->VirtualAddress)
	{
	
		pOffset = (PTYPEOFFSET)((DWORD)pReloc + sizeof(IMAGE_BASE_RELOCATION));

		dwRelocOfItemNum = (pReloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / 2;

		for (DWORD i = 0; i < dwRelocOfItemNum; i++, pOffset++)
		{

		
			if (pOffset->Type  != IMAGE_REL_BASED_HIGHLOW)
			continue;



			dwRVA = pReloc->VirtualAddress + pOffset->offset;

			*(DWORD*)(RVAToOffset(dwRVA) + (DWORD)m_dwFileDataAddr) +=( dwLoadImageBase - m_stcPeInfo.dwImageBase);
			dwTestRVA  = RVAToOffset(dwRVA);
			dwTestAddr = RVAToOffset(dwRVA) + (DWORD)m_dwFileDataAddr;
			dwItemAddressOfReloc = *(DWORD*)(RVAToOffset(dwRVA) + (DWORD)m_dwFileDataAddr);//������

		}

		//������RVA + dwAlignBlock * SectionAlign
	/*	pReloc->VirtualAddress = dwAlignBlock * m_pNtHeader->OptionalHeader.SectionAlignment + pObjectPeNewSection->VirtualAddress;*/

		//��Ϊshellû��textbss�Σ�ֱ�Ӽ�ȥ0x1000�ͺ���
		pReloc->VirtualAddress = pReloc->VirtualAddress - 0x1000 + pObjectPeNewSection->VirtualAddress;

		if (dwEndOfReloc == (DWORD)pOffset)
			return;

		pReloc = (PIMAGE_BASE_RELOCATION)((DWORD)pReloc + pReloc->SizeOfBlock);

	


	}

}


//////////////////////////////////////////////////////////////////////////
//	��FOA��ʽ��ȡȫ�ֱ����ĵ�ַ
//	������
//	LPCTSTR strVarName		�뵱ǰ�ַ�����ͬ���͵��ַ�����ȫ�ֱ���������
//////////////////////////////////////////////////////////////////////////

LPBYTE COperationPE::GetExpVarAddr(LPWSTR strVarName)
{
	// 1����ȡ�������ַ����������strVarNameתΪASCII��ʽ������ԱȲ���
	PCHAR strTempName;
	PDWORD pNameAddr;
	PDWORD pFunAddr;
	CHAR szVarName[MAX_PATH] = { 0 };
	IMAGE_DATA_DIRECTORY	stcExport;
	stcExport = IMAGE_DATA_DIRECTORY(m_pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);
	PIMAGE_EXPORT_DIRECTORY lpExport = (PIMAGE_EXPORT_DIRECTORY)(m_dwFileDataAddr + RVAToOffset(stcExport.VirtualAddress));
	WideCharToMultiByte(CP_ACP, NULL, strVarName, -1, szVarName, _countof(szVarName), NULL, FALSE);

	// 2��ѭ����ȡ�����������������������������szVarName���ȶԣ������ͬ����ȡ�����Ӧ�ĺ�����ַ
	for (DWORD i = 0; i < lpExport->NumberOfNames; i++)
	{
		 pNameAddr = (PDWORD)(m_dwFileDataAddr + RVAToOffset(lpExport->AddressOfNames + i));
		 strTempName = (PCHAR)(m_dwFileDataAddr + RVAToOffset(*pNameAddr));

		if (!strcmp(szVarName, strTempName))
		{
			pFunAddr = (PDWORD)(m_dwFileDataAddr + RVAToOffset(lpExport->AddressOfFunctions + i));
			return (LPBYTE)(m_dwFileDataAddr + RVAToOffset(*pFunAddr));
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//				���һ���µ�����
//	������
//	LPCTSTR strName						��������
//	DWORD dwSize						�����ε�ʵ�ʳߴ�
//	DWORD dwCharac						���ε�����
//	PIMAGE_SECTION_HEADER pNewSection	[out]�����ε�ͷ�ṹ
//	PDWORD pSizeOfRaw					[out]�����ε�SizeOfRawData
//	����ֵ								���ε���ʵ��ַ�����ں���д������
//////////////////////////////////////////////////////////////////////////

LPBYTE COperationPE::AddSection(LPWSTR strName, DWORD dwSize, DWORD dwCharac, PIMAGE_SECTION_HEADER pNewSection, PDWORD pSizeOfRaw)
{
	DWORD	dwDosSize			= m_pDosHeader->e_lfanew;
	DWORD	dwPeSize			= sizeof(m_pNtHeader->Signature) + m_pNtHeader->FileHeader.SizeOfOptionalHeader + sizeof(IMAGE_FILE_HEADER);
	DWORD	dwNumOfSections		= m_pNtHeader->FileHeader.NumberOfSections;
	DWORD	dwSectionTableSize  = dwNumOfSections * sizeof(IMAGE_SECTION_HEADER);
	DWORD	dwFileAlign			= m_pNtHeader->OptionalHeader.FileAlignment;
	DWORD	dwSectionAlign		= m_pNtHeader->OptionalHeader.SectionAlignment;
	CHAR	szVarName[8] = { 0 };
	IMAGE_SECTION_HEADER	stcLastSectionHeader = 	m_stcPeInfo.pSectionHeader[dwNumOfSections - 1];
	IMAGE_SECTION_HEADER	stcNewSectionHeader;
	DWORD	dwNewSectionVirtualAddress;
	DWORD	dwNewSectionPointerToRaw;
	DWORD	dwNewSectionVirtualSize;
	DWORD	dwNewSectionRawSize;

	if (dwSize % dwSectionAlign)
	{
		dwNewSectionVirtualSize = (dwSize / dwSectionAlign + 1) * dwSectionAlign;
	}
	else
	{
		dwNewSectionVirtualSize = (dwSize / dwSectionAlign) * dwSectionAlign;
	}

	if (dwSize % dwFileAlign)
	{
		dwNewSectionRawSize = (dwSize / dwFileAlign + 1) * dwFileAlign;
	}
	else
	{
		dwNewSectionRawSize = (dwSize / dwFileAlign ) * dwFileAlign;
	}

	*pSizeOfRaw = dwNewSectionRawSize;
	dwNewSectionPointerToRaw	= stcLastSectionHeader.PointerToRawData + stcLastSectionHeader.SizeOfRawData;


	if (stcLastSectionHeader.Misc.VirtualSize % dwSectionAlign)
	{
		dwNewSectionVirtualAddress = (stcLastSectionHeader.Misc.VirtualSize / dwSectionAlign + 1) * dwSectionAlign + stcLastSectionHeader.VirtualAddress;

	}
	else
	{
		dwNewSectionVirtualAddress = (stcLastSectionHeader.Misc.VirtualSize / dwSectionAlign ) * dwSectionAlign + stcLastSectionHeader.VirtualAddress;
	}


	WideCharToMultiByte(CP_ACP, NULL, strName, -1, szVarName, _countof(szVarName), NULL, FALSE);
	memset(&stcNewSectionHeader, 0x0, sizeof(IMAGE_SECTION_HEADER));
	memcpy_s(stcNewSectionHeader.Name, 8, szVarName, strlen(szVarName));
	stcNewSectionHeader.Misc.VirtualSize	 = dwNewSectionVirtualSize;
	stcNewSectionHeader.VirtualAddress		 = dwNewSectionVirtualAddress;
	stcNewSectionHeader.SizeOfRawData		 = dwNewSectionRawSize;
	stcNewSectionHeader.PointerToRawData	 = dwNewSectionPointerToRaw;
	stcNewSectionHeader.Characteristics		 = dwCharac;
	memcpy_s(&m_stcPeInfo.pSectionHeader[dwNumOfSections], sizeof(IMAGE_SECTION_HEADER),
	&stcNewSectionHeader, sizeof(IMAGE_SECTION_HEADER));

	m_pNtHeader->OptionalHeader.SizeOfImage += dwNewSectionVirtualSize;

	m_pNtHeader->FileHeader.NumberOfSections++;

	memcpy_s(pNewSection, sizeof(IMAGE_SECTION_HEADER), &stcNewSectionHeader, sizeof(IMAGE_SECTION_HEADER));

	//�����ļ�RS
	m_dwFileDataSize += *pSizeOfRaw;

	return	(LPBYTE)(m_dwFileDataAddr + dwNewSectionPointerToRaw);

}

//////////////////////////////////////////////////////////////////////////
//	�����µ�OEP
//	������
//	DWORD dwOEP			�µ���ڵ�		
//////////////////////////////////////////////////////////////////////////

VOID COperationPE::SetOEP(DWORD dwOEP)
{
	m_pNtHeader->OptionalHeader.AddressOfEntryPoint = dwOEP;
}



//////////////////////////////////////////////////////////////////////////
//	������Ŀ¼�����������Ҫ���������Դ���ض�λ��������
//////////////////////////////////////////////////////////////////////////

VOID COperationPE::CleanDir()
{
	DWORD dwCount = 15;
	for (DWORD i = 0; i < dwCount; i++)
	{
		if (i != IMAGE_DIRECTORY_ENTRY_EXPORT &&
			i != IMAGE_DIRECTORY_ENTRY_RESOURCE &&
			i != IMAGE_DIRECTORY_ENTRY_BASERELOC&&
			i != IMAGE_DIRECTORY_ENTRY_TLS)
		{
			m_pNtHeader->OptionalHeader.DataDirectory[i].VirtualAddress = 0;
			m_pNtHeader->OptionalHeader.DataDirectory[i].Size = 0;
		}
	}
}



VOID COperationPE::FindSectionOfIAT(PDWORD dwIATBase, PDWORD dwSize)
{
	PIMAGE_SECTION_HEADER	pFirstSectionHeader = IMAGE_FIRST_SECTION(m_pNtHeader);
	DWORD					dwNumOfSections		= m_pNtHeader->FileHeader.NumberOfSections;


	for (DWORD i = 0;
		i < dwNumOfSections;
		i++, pFirstSectionHeader++)
	{
		if (m_stcPeInfo.stcPEImportDir.VirtualAddress >= pFirstSectionHeader->VirtualAddress &&
			m_stcPeInfo.stcPEImportDir.VirtualAddress < pFirstSectionHeader->VirtualAddress + pFirstSectionHeader->Misc.VirtualSize)
		{
			*dwIATBase = pFirstSectionHeader->VirtualAddress;
			*dwSize = pFirstSectionHeader->Misc.VirtualSize;
			break;
		}

	}

}


//////////////////////////////////////////////////////////////////////////
//	��ȡ��ԴĿ¼ͷ�͹ؼ����ݺϲ���һ����������
//	�����������Ǵ�����Դͷ���������ӹؼ�����
//	��ע��ָ�򻺳����ı��������PEInfo�ṹ���У�	
//	������
//	pPEInfo pObjectPE					//���������PE�ṹָ��
//  BUG: ���Ƕ�û����Դ��ĳ������жϴ���
//////////////////////////////////////////////////////////////////////////

VOID	COperationPE::ReBuildRes(pPEInfo pObjectPE)
{
	IMAGE_DATA_DIRECTORY	stcResDir = IMAGE_DATA_DIRECTORY(pObjectPE->pDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE]);
	DWORD					dwResHeaderEndRVA;
	DWORD					dwResHeaderSize;
	DWORD					dwNewResOffset	= 0;
	LPBYTE					pNewResAddr;
	LPBYTE					pResStartAddr	= (LPBYTE)(m_dwFileDataAddr + RVAToOffset(stcResDir.VirtualAddress));

	pObjectPE->dwNewResSize = 0;

	dwResHeaderEndRVA = FindResourceHeader(pResStartAddr, pResStartAddr, pObjectPE->dwSizeOfImage);
	
	//��Դͷ��С	= ��Դͷĩַ	-	��Դͷ��ʼ��ַ
	dwResHeaderSize = dwResHeaderEndRVA - stcResDir.VirtualAddress;
	
	pObjectPE->dwResHeaderSize = dwResHeaderSize;

	pObjectPE->dwNewResAddr = (DWORD)VirtualAlloc(NULL, stcResDir.Size * 2, MEM_COMMIT, PAGE_READWRITE);
	
	pNewResAddr				= (LPBYTE)pObjectPE->dwNewResAddr;

	ZeroMemory((PVOID)pNewResAddr, stcResDir.Size * 2);

	CopyMemory((PVOID)pNewResAddr,
				(PVOID)pResStartAddr,
				dwResHeaderSize);
	pObjectPE->dwNewResSize += dwResHeaderSize;

	pNewResAddr += dwResHeaderSize;


	//��ȡͼ�� 
	MoveObjectRes(pResStartAddr, (DWORD)RT_ICON, pNewResAddr, &dwNewResOffset);
	pObjectPE->dwNewResSize += dwNewResOffset;
	//��ȡͼ����
	pNewResAddr += dwNewResOffset;
	MoveObjectRes(pResStartAddr, (DWORD)RT_GROUP_ICON, pNewResAddr, &dwNewResOffset);
	pObjectPE->dwNewResSize += dwNewResOffset;
	//��ȡ�汾��Ϣ
	pNewResAddr += dwNewResOffset;
	MoveObjectRes(pResStartAddr, (DWORD)RT_VERSION, pNewResAddr, &dwNewResOffset);
	pObjectPE->dwNewResSize += dwNewResOffset;

	//Manifest
	pNewResAddr += dwNewResOffset;
	MoveObjectRes(pResStartAddr, (DWORD)RT_MANIFEST, pNewResAddr, &dwNewResOffset);
	pObjectPE->dwNewResSize += dwNewResOffset;



}


//////////////////////////////////////////////////////////////////////////
//	�ҵ���ԴĿ¼ͷ��ĩβRVA
//	������
//	LPBYTE	pResHeaderAddr					��Դͷ�ĵ�ַ
//	LPBYTE  pResAddr						��ǰ��ԴĿ¼����ʼ��ַ
//	DWORD	dwMinRVA						������ݵ�RVA
//////////////////////////////////////////////////////////////////////////

DWORD	COperationPE::FindResourceHeader(LPBYTE pResHeaderAddr, LPBYTE pResAddr, DWORD	dwMinRVA)
{
	PIMAGE_RESOURCE_DIRECTORY		pResDir = (PIMAGE_RESOURCE_DIRECTORY)pResAddr;
	DWORD							dwNumOfID;
	DWORD							dwNumOfName;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY	pResDirEntry;
	PIMAGE_RESOURCE_DATA_ENTRY		pResDataEntry;
	DWORD							dwReturnedMinRVA;


	dwNumOfID = pResDir->NumberOfIdEntries;
	dwNumOfName = pResDir->NumberOfNamedEntries;


	pResDirEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pResDir + sizeof(IMAGE_RESOURCE_DIRECTORY));
	


	if (dwNumOfName)
	{
		for (DWORD i = 0; i < dwNumOfName ; i++)
		{
			if (pResDirEntry->DataIsDirectory)
			{
				dwReturnedMinRVA = FindResourceHeader(pResHeaderAddr,pResDirEntry->OffsetToDirectory + pResHeaderAddr, dwMinRVA);
				if (dwReturnedMinRVA < dwMinRVA)	dwMinRVA = dwReturnedMinRVA;

			}
			else
			{
				pResDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)(pResDirEntry->OffsetToData + pResHeaderAddr);
				
				if (pResDataEntry->OffsetToData < dwMinRVA)	return pResDataEntry->OffsetToData;
				
			}//if

			pResDirEntry += 1;
		}//for
	}



	if (dwNumOfID)
	{
		for (DWORD i = 0; i < dwNumOfID; i++)
		{
			if (pResDirEntry->DataIsDirectory)
			{
				dwReturnedMinRVA =  FindResourceHeader(pResHeaderAddr,pResDirEntry->OffsetToDirectory + pResHeaderAddr, dwMinRVA);
				
				if (dwReturnedMinRVA < dwMinRVA)	dwMinRVA = dwReturnedMinRVA;

			}
			else
			{
				pResDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)(pResDirEntry->OffsetToData + pResHeaderAddr);

				if (pResDataEntry->OffsetToData < dwMinRVA)	return pResDataEntry->OffsetToData;

			}//if

			pResDirEntry += 1;
		}//for
	}

	return	dwMinRVA;

}


//////////////////////////////////////////////////////////////////////////
//	�ҵ�ָ�����͵���Դ������д������Ļ�������
//	������
//	LPBYTE pResAddr				��Դ�ε�ptr
//	DWORD  dwType				��Ҫ��ȡ��ָ������
//	LPBYTE pDataBuf				д��Ļ�����
//	PDWORD dwBufSize			[out]д����ܴ�С
//////////////////////////////////////////////////////////////////////////

VOID	COperationPE::MoveObjectRes(LPBYTE pResAddr, DWORD dwType, LPBYTE pDataBuf, PDWORD dwBufSize)
{
	PIMAGE_RESOURCE_DIRECTORY		pResTypeDir = (PIMAGE_RESOURCE_DIRECTORY)pResAddr;
	PIMAGE_RESOURCE_DIRECTORY		pResNameIdDir;
	PIMAGE_RESOURCE_DIRECTORY		pResLanguageDir;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY	pResTypeDirEntry;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY	pResNameIdEntry;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY	pResLanguageEntry;
	PIMAGE_RESOURCE_DATA_ENTRY		pResDataEntry;
	DWORD							dwNumOfType		= 0;
	DWORD							dwNumOfNameId	= 0;
	DWORD							dwNumOfLanguage = 0;
	DWORD							dwWrittenBytes	= 0;
	LPBYTE							pTempBuf = NULL;

	dwNumOfType += pResTypeDir->NumberOfIdEntries;
	dwNumOfType += pResTypeDir->NumberOfNamedEntries;


	pResTypeDirEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResTypeDir + 1);

	for (DWORD dwTypeIndex = 0; dwTypeIndex < dwNumOfType; dwTypeIndex++, pResTypeDirEntry++)
	{//��һ��Ŀ¼�ṹ

		if (!pResTypeDirEntry->NameIsString)
		{
			if (pResTypeDirEntry->Name == dwType)
			{
				pResNameIdDir = (PIMAGE_RESOURCE_DIRECTORY)(pResTypeDirEntry->OffsetToDirectory + pResAddr);

				pResNameIdEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResNameIdDir + 1);

				dwNumOfNameId = pResNameIdDir->NumberOfNamedEntries + pResNameIdDir->NumberOfIdEntries;

				for (DWORD dwNameIdIndex = 0; dwNameIdIndex < dwNumOfNameId; dwNameIdIndex++, pResNameIdEntry++)
				{//�ڶ���Ŀ¼�ṹ
					//if (!pResNameIdEntry->NameIsString)
					//{
						pResLanguageDir = (PIMAGE_RESOURCE_DIRECTORY)(pResNameIdEntry->OffsetToDirectory + pResAddr);
						dwNumOfLanguage = pResLanguageDir->NumberOfNamedEntries + pResLanguageDir->NumberOfIdEntries;
						pResLanguageEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResLanguageDir + 1);

						for (DWORD dwLanguageIndex = 0; dwLanguageIndex < dwNumOfLanguage; dwLanguageIndex++, pResLanguageEntry++)
						{//������Ŀ¼
							//if (!pResLanguageEntry->NameIsString)
							//{
								//��ȡ��Դ����
								
								pResDataEntry	= (PIMAGE_RESOURCE_DATA_ENTRY)(pResLanguageEntry->OffsetToData + pResAddr);

								pTempBuf		= (LPBYTE)RVAToOffset(pResDataEntry->OffsetToData) + m_dwFileDataAddr;

								CopyMemory(pDataBuf + dwWrittenBytes, pTempBuf, pResDataEntry->Size);

								//ZeroMemory(pTempBuf, pResDataEntry->Size);

								dwWrittenBytes += pResDataEntry->Size;

						//	}
						}
					//}
				}
			}
		}
	}

	*dwBufSize = dwWrittenBytes;

}

//////////////////////////////////////////////////////////////////////////
//	�޸�����Դ�ε�ƫ�ƣ�ֻ����Դͷ���йؼ����ݵ��޸ģ����ı���Դ����
//	������
//	LPBYTE	pNewResAddr				�������Դͷ�Ļ���������ʼ��ַ
//	DWORD	dwType					ָ������
//	DWORD	dwCurrentRVA			��ǰ���йؼ���Դ���ݵ���ʼRVA����Ϊ���㹫ʽ�Ļ�ַ��
//	DWORD	dwDataOffset			��ǰƫ����
//	PDWORD	dwReturnedDataSize		[out]��ǰ�����Ĺؼ����ݵĴ�С
//////////////////////////////////////////////////////////////////////////

VOID	COperationPE::FixResDataEntry(LPBYTE pNewResAddr, DWORD dwType, DWORD dwCurrentRVA, DWORD dwDataOffset, PDWORD	dwReturnedDataSize)
{
	PIMAGE_RESOURCE_DIRECTORY		pResTypeDir = (PIMAGE_RESOURCE_DIRECTORY)pNewResAddr;
	PIMAGE_RESOURCE_DIRECTORY		pResNameIdDir;
	PIMAGE_RESOURCE_DIRECTORY		pResLanguageDir;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY	pResTypeDirEntry;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY	pResNameIdEntry;
	PIMAGE_RESOURCE_DIRECTORY_ENTRY	pResLanguageEntry;
	PIMAGE_RESOURCE_DATA_ENTRY		pResDataEntry;
	DWORD							dwNumOfType = 0;
	DWORD							dwNumOfNameId = 0;
	DWORD							dwNumOfLanguage = 0;
	DWORD							dwWrittenBytes = 0;
	BOOL							bChanged = FALSE;

	dwNumOfType += pResTypeDir->NumberOfIdEntries;
	dwNumOfType += pResTypeDir->NumberOfNamedEntries;


	pResTypeDirEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResTypeDir + 1);

	for (DWORD dwTypeIndex = 0; dwTypeIndex < dwNumOfType; dwTypeIndex++, pResTypeDirEntry++)
	{//��һ��Ŀ¼�ṹ

		if (!pResTypeDirEntry->NameIsString)
		{
			if (pResTypeDirEntry->Name == dwType)
			{
				pResNameIdDir = (PIMAGE_RESOURCE_DIRECTORY)(pResTypeDirEntry->OffsetToDirectory + pNewResAddr);

				pResNameIdEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResNameIdDir + 1);

				dwNumOfNameId = pResNameIdDir->NumberOfNamedEntries + pResNameIdDir->NumberOfIdEntries;

				for (DWORD dwNameIdIndex = 0; dwNameIdIndex < dwNumOfNameId; dwNameIdIndex++, pResNameIdEntry++)
				{//�ڶ���Ŀ¼�ṹ
					//if (!pResNameIdEntry->NameIsString)
					//{
						pResLanguageDir = (PIMAGE_RESOURCE_DIRECTORY)(pResNameIdEntry->OffsetToDirectory + pNewResAddr);
						dwNumOfLanguage = pResLanguageDir->NumberOfNamedEntries + pResLanguageDir->NumberOfIdEntries;
						pResLanguageEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResLanguageDir + 1);

						for (DWORD dwLanguageIndex = 0; dwLanguageIndex < dwNumOfLanguage; dwLanguageIndex++, pResLanguageEntry++)
						{//������Ŀ¼
							//if (!pResLanguageEntry->NameIsString)
							//{
							//��ȡ��Դ����

								pResDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)(pResLanguageEntry->OffsetToData + pNewResAddr);

								//if (!bChanged)
							//	{//��֤Data_Entry����Դ����RVAֻ�޸�һ��
									pResDataEntry->OffsetToData = dwCurrentRVA + dwDataOffset;
									//bChanged = TRUE;
							//	}
								dwWrittenBytes += pResDataEntry->Size;
								dwDataOffset += pResDataEntry->Size;
							//}
						}
					//}
				}
			}
		}
	}

	*dwReturnedDataSize = dwWrittenBytes;

}


//////////////////////////////////////////////////////////////////////////
//	�޸�����Դ�εĹؼ����ݣ��޸�˳��ͼ��-��ͼ����-���汾��Ϣ
//	ע�⣺���޹ؼ���Դ���ͣ���ö�ֻ������Դͷ
//	������
//	COperationPE*	pObjectPE			���������PE������
//	pPEInfo			pObjectPEInfo		���������PE�ؼ���Ϣ�ṹ��
//	����ֵ			�����εĴ�С
//////////////////////////////////////////////////////////////////////////



DWORD	COperationPE::FixRes(COperationPE* pObjectPE, pPEInfo pObjectPEInfo)
{
	// 1.���������
	IMAGE_SECTION_HEADER	stcNewSectionHeader;
	DWORD					dwSizeOfRawData;
	DWORD					dwOffset		= 0;
	DWORD					dwReturnedSize	= 0;
	LPBYTE					pNewSectionAddr;

	pNewSectionAddr	= pObjectPE->AddSection(L".rsrc", pObjectPEInfo->dwNewResSize, IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE,
		&stcNewSectionHeader, &dwSizeOfRawData);

	CopyMemory((PVOID)pNewSectionAddr, (PVOID)pObjectPEInfo->dwNewResAddr, pObjectPEInfo->dwNewResSize);



	// 2.��˳���޸��������͵���Դ
	// 2.1 ͼ��
	FixResDataEntry(pNewSectionAddr, (DWORD)RT_ICON, stcNewSectionHeader.VirtualAddress + pObjectPEInfo->dwResHeaderSize,
					dwOffset, &dwReturnedSize);

	// 2.2 ͼ����
	dwOffset += dwReturnedSize;
	FixResDataEntry(pNewSectionAddr, (DWORD)RT_GROUP_ICON, stcNewSectionHeader.VirtualAddress + pObjectPEInfo->dwResHeaderSize,
					dwOffset, &dwReturnedSize);

	// 2.3 �汾��Ϣ
	dwOffset += dwReturnedSize;
	FixResDataEntry(pNewSectionAddr, (DWORD)RT_VERSION, stcNewSectionHeader.VirtualAddress + pObjectPEInfo->dwResHeaderSize,
		dwOffset, &dwReturnedSize);

	//2.5 Manifest
	dwOffset += dwReturnedSize;
	FixResDataEntry(pNewSectionAddr, (DWORD)RT_MANIFEST, stcNewSectionHeader.VirtualAddress + pObjectPEInfo->dwResHeaderSize,
		dwOffset, &dwReturnedSize);



	PIMAGE_DATA_DIRECTORY	pResDir = &(pObjectPE->m_stcPeInfo.pDataDir[IMAGE_DIRECTORY_ENTRY_RESOURCE]);


	//�ô�����Բ����ڹؼ���Դ���͵�����£�dwOffsetΪ0
	if (dwOffset)
	{
		pResDir->VirtualAddress = stcNewSectionHeader.VirtualAddress;
		pResDir->Size = pObjectPEInfo->dwNewResSize;
		
	}
	else
	{
		memset(pResDir, 0x0, sizeof(IMAGE_DATA_DIRECTORY));
	}

	return	dwSizeOfRawData;


}


//////////////////////////////////////////////////////////////////////////
//	���tls�����ڶε���ʼRVA�͸öε���ʵ��С
//	������
//	pPEInfo pObjectPEInfo								PE�ؼ���Ϣ�ṹ��
//	PDWORD	dwTlsSectionStartRVA						[out]tls�ε���ʼRVA
//	PDWORD	dwSectionRealSize							[out]tls�ε���ʵ��С
//////////////////////////////////////////////////////////////////////////

BOOL	COperationPE::FindSectionOfTls(pPEInfo pObjectPEInfo, PDWORD dwTlsSectionStartRVA, PDWORD dwSectionRealSize)
{
	//Ҫ��PE�ؼ��ṹ�еģ���Ϊ��Щ������ܻ���textbss��Ϊ��һ������
	PIMAGE_SECTION_HEADER	pSectionHeader = pObjectPEInfo->pSectionHeader;
	DWORD					dwNumOfSections = m_pNtHeader->FileHeader.NumberOfSections;
	DWORD					dwCodeEndRVA;
	if (!pObjectPEInfo->stcPETlsDir.VirtualAddress)		return FALSE;
	

	//Ĭ�ϵ�һ������Ϊ�����
	if (pSectionHeader->Misc.VirtualSize % pObjectPEInfo->dwSectionAlign)
	{
		dwCodeEndRVA = (pSectionHeader->Misc.VirtualSize / pObjectPEInfo->dwSectionAlign + 1) * pObjectPEInfo->dwSectionAlign +
						pSectionHeader->VirtualAddress;
	}
	

	for (DWORD i = 0; i < dwNumOfSections ; i++ , pSectionHeader ++)
	{
		if (!strcmp((CHAR*)pSectionHeader->Name, ".tls"))
		{
			*dwTlsSectionStartRVA	= pSectionHeader->VirtualAddress;
			*dwSectionRealSize		= pSectionHeader->Misc.VirtualSize;
			return TRUE;
		}
	}

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//	���ö�Ӧ���͵�����Ŀ¼�����
//	������
//	DWORD dwType			����������
//	DWORD dwVirtualAddress	��ӦRVA
//	DWORD dwSize			��Ӧ��ʵ��С
//////////////////////////////////////////////////////////////////////////

VOID	COperationPE::SetDir(DWORD dwType, DWORD dwVirtualAddress, DWORD dwSize)
{
	PIMAGE_DATA_DIRECTORY pRelocDir = &(m_pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]);
	PIMAGE_DATA_DIRECTORY pDir = NULL;
	switch (dwType)
	{
	case IMAGE_DIRECTORY_ENTRY_BASERELOC:
		pDir = &(m_pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]);
		break;
	case IMAGE_DIRECTORY_ENTRY_TLS:
		pDir = &(m_pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS]);
		break;
	case IMAGE_DIRECTORY_ENTRY_IAT:
		pDir = &(m_pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT]);
		break;
	case  IMAGE_DIRECTORY_ENTRY_EXPORT:
		pDir = &(m_pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);
		break;
	default:
		break;
	}

	if (!pDir)		return;

	pDir->VirtualAddress = dwVirtualAddress;
	pDir->Size			 = dwSize;


}


//////////////////////////////////////////////////////////////////////////
//	�����������tls���������ǵ�shell����������
//	������
//	DWORD	dwStartAddr				��������tls��ģ��������ʼ��ַ
//	DWORD	dwEndAddr				��������tls��ģ������ĩ��ַ
//	DWORD	dwIndexValue			��������tls
//////////////////////////////////////////////////////////////////////////

BOOL	COperationPE::ChangeModuleData(DWORD dwStartAddr, DWORD dwEndAddr, DWORD dwIndexValue)
{
	PIMAGE_TLS_DIRECTORY	pTls		=  (PIMAGE_TLS_DIRECTORY)(m_dwFileDataAddr +  RVAToOffset( m_stcPeInfo.stcPETlsDir.VirtualAddress));
	
	if (!pTls)		return FALSE;

	*(PDWORD)(RVAToOffset(pTls->AddressOfIndex - m_stcPeInfo.dwImageBase) + m_dwFileDataAddr) = dwIndexValue;

	pTls->StartAddressOfRawData = dwStartAddr;
	pTls->EndAddressOfRawData	= dwEndAddr;

	return TRUE;

}

//////////////////////////////////////////////////////////////////////////
//	���Ŀ������ģ�����ݼ������ݴ�С
//	����ֵ��	���������tls��
//////////////////////////////////////////////////////////////////////////

VOID	COperationPE::ReturnTlsModuleAddress(PDWORD dwStartAddr, PDWORD dwEndAddr, PDWORD dwIndexValue)
{
	PIMAGE_TLS_DIRECTORY	pTls	= (PIMAGE_TLS_DIRECTORY)(m_dwFileDataAddr + RVAToOffset(m_stcPeInfo.stcPETlsDir.VirtualAddress));

	*dwStartAddr	= pTls->StartAddressOfRawData;
	*dwEndAddr		= pTls->EndAddressOfRawData;
	*dwIndexValue	= *(PDWORD)(RVAToOffset(pTls->AddressOfIndex - m_stcPeInfo.dwImageBase) + m_dwFileDataAddr);
	
}


//////////////////////////////////////////////////////////////////////////
//	��ȡ������Ļ������ʹ�С,��AddOfName������Copy����
//	������
//	PDWORD		dwSize			[out]�������С
//	����ֵ��					����������
//////////////////////////////////////////////////////////////////////////

VOID	COperationPE::GetExportBuf( pPEInfo pPeInfo)
{
	DWORD dwSize					= pPeInfo->pDataDir[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

	PIMAGE_EXPORT_DIRECTORY	pExp	= (PIMAGE_EXPORT_DIRECTORY)(RVAToOffset(m_stcPeInfo.pDataDir[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress) + m_dwFileDataAddr);

	PDWORD	pName					= (PDWORD)(RVAToOffset(pExp->AddressOfNames) + m_dwFileDataAddr);

	DWORD	dwNumOfName				= pExp->NumberOfNames;

	pPeInfo->pExpAddrOfName		= new DWORD[dwNumOfName + 1];

	ZeroMemory(pPeInfo->pExpAddrOfName, sizeof(DWORD)*(dwNumOfName + 1));

	//����AddOfName������
	while (dwNumOfName >0)
	{
		CopyMemory(pPeInfo->pExpAddrOfName, pName, dwNumOfName * sizeof(DWORD));
		dwNumOfName--;
	}


	pPeInfo->pExportBuf = new BYTE[dwSize];

	CopyMemory(pPeInfo->pExportBuf,
		(LPBYTE)(RVAToOffset(m_stcPeInfo.pDataDir[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress) + m_dwFileDataAddr),
		dwSize);

}


//////////////////////////////////////////////////////////////////////////
//	���ݾɵ�������ֶ�ƫ���ض�λ�±�ĸ��ֶ�ƫ��
//	������
//	DWORD					dwNewExportRVA				�µ������RVA
//	PIMAGE_EXPORT_DIRECTORY	pNewExp						�µ��������ڵ�ַ(��δ�޸��������ֶ����Ǿɵ�RVA��ַ)
//////////////////////////////////////////////////////////////////////////

BOOL	COperationPE::RelocExportTable(DWORD dwNewExportRVA, PIMAGE_EXPORT_DIRECTORY	pNewExp)
{
	DWORD					dwOldExpRVA = m_stcPeInfo.pDataDir[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

	DWORD					dwNumOfName;

	if (!dwOldExpRVA || !dwNewExportRVA  || !pNewExp)	return FALSE;


	pNewExp->AddressOfFunctions		= dwNewExportRVA + pNewExp->AddressOfFunctions - dwOldExpRVA;

	pNewExp->AddressOfNameOrdinals	= dwNewExportRVA + pNewExp->AddressOfNameOrdinals - dwOldExpRVA;

	pNewExp->Name					= dwNewExportRVA + pNewExp->Name - dwOldExpRVA;

	dwNumOfName						= 0;

	PDWORD					pAddrOfName = (PDWORD)(pNewExp->AddressOfNames - dwOldExpRVA + (DWORD)pNewExp);
		
	for(dwNumOfName = 0; dwNumOfName <pNewExp->NumberOfNames; dwNumOfName++)
	{
		*pAddrOfName = m_stcPeInfo.pExpAddrOfName[dwNumOfName] + dwNewExportRVA - dwOldExpRVA;
	
		pAddrOfName++;

	}

	pNewExp->AddressOfNames = dwNewExportRVA + pNewExp->AddressOfNames - dwOldExpRVA;

	return	TRUE;
}


//////////////////////////////////////////////////////////////////////////
//	�����ݽ���ѹ����ѹ������Aplib
//	������
//	LPBYTE	pData			��ѹ�����ݵĻ�����
//	DWORD	dwSize			��ѹ�����ݴ�С
//	PDWORD	dwPackedSize	[out]ѹ��������ݴ�С
//	����ֵ��				ѹ�����ݵĻ�����
//////////////////////////////////////////////////////////////////////////

LPBYTE	COperationPE::CompressDataOfAplib(LPBYTE pData, DWORD dwSize, PDWORD dwPackedSize)
{
	DWORD	dwPackedFileSize;
	DWORD	dwMemWorkPlace;
	DWORD	dwPackedRealSize;

	dwPackedFileSize = aP_max_packed_size(dwSize);

	dwMemWorkPlace = aP_workmem_size(dwSize);

	LPBYTE	pPackedFile = new BYTE[dwPackedFileSize];

	LPBYTE	pWorkMem = new BYTE[dwMemWorkPlace];

	ZeroMemory(pWorkMem, dwMemWorkPlace);

	ZeroMemory(pPackedFile, dwPackedFileSize);


	dwPackedRealSize = aPsafe_pack(pData, pPackedFile, dwSize, pWorkMem, NULL, NULL);

	*dwPackedSize = dwPackedRealSize;

	delete[] pWorkMem;

	if (dwPackedRealSize == APLIB_ERROR)
	{
		delete[] pPackedFile;
		return NULL;
	}


	return pPackedFile;
}

//////////////////////////////////////////////////////////////////////////
//	���������ν���ѹ�������ں�Ϊһ�����Σ�ѹ�����һ������Ϊռλ����
//	��ѹ�������е����ιؼ���Ϣ������һ���ṹ���У���ָ���������������PE�ṹ����
//	���Զ�tls��������⴦��
//	������
//	pPEInfo					pObjectPE	���������PE�ؼ���Ϣ�ṹ��
//////////////////////////////////////////////////////////////////////////

VOID	COperationPE::CompressSection(pPEInfo pObjectPE, PSelectionInfo pSelect)
{
	PIMAGE_SECTION_HEADER	pSecHeader		= IMAGE_FIRST_SECTION(m_pNtHeader);
	DWORD					dwCompressSecRva;
	DWORD					dwOccupiedSecEndRva;
	DWORD					dwNumOfSection	= m_pNtHeader->FileHeader.NumberOfSections;
	DWORD					dwLastSecVS		= pSecHeader[dwNumOfSection - 1].Misc.VirtualSize;
	DWORD					dwLastSecRva	= pSecHeader[dwNumOfSection - 1].VirtualAddress;
	DWORD					dwCompressedSize;
	
	// 1.1�����ռλ���ε���Ϣ
	if (dwLastSecVS % pObjectPE->dwSectionAlign)
	{
		dwCompressSecRva = (dwLastSecVS / pObjectPE->dwSectionAlign + 1)*pObjectPE->dwSectionAlign + dwLastSecRva;
	}
	else
	{
		dwCompressSecRva = (dwLastSecVS / pObjectPE->dwSectionAlign)*pObjectPE->dwSectionAlign + dwLastSecRva;
	}
	
	dwOccupiedSecEndRva = dwCompressSecRva;

	// 1.2����ڱ�
	PIMAGE_SECTION_HEADER	pOriginalSecTable = new IMAGE_SECTION_HEADER[dwNumOfSection];

	CopyMemory(pOriginalSecTable, pSecHeader, dwNumOfSection * sizeof(IMAGE_SECTION_HEADER));
	
	pObjectPE->pOriginalSecTable = (LPBYTE)pOriginalSecTable;


	
	// 2.ѹ������
	PComPressInfo	pSecComPressInfo = new ComPressInfo[dwNumOfSection];
	pObjectPE->pCompressInfo = (LPBYTE)pSecComPressInfo;

	for (DWORD dwIndex = 0; dwIndex < dwNumOfSection; dwIndex++)
	{
		if (pSecHeader[dwIndex].SizeOfRawData  == 0)
		{
			ZeroMemory(&pSecComPressInfo[dwIndex], sizeof(ComPressInfo));
			continue;
		}

		if(pSelect->dwCompressionType == COMPRESS_APLIB)
		pSecComPressInfo[dwIndex].pData = CompressDataOfAplib((LPBYTE)(pSecHeader[dwIndex].PointerToRawData + m_dwFileDataAddr),
											pSecHeader[dwIndex].SizeOfRawData,
											&dwCompressedSize);

		if (pSelect->dwCompressionType == COMPRESS_JCALG1_FAST  || pSelect->dwCompressionType == COMPRESS_JCALG1_SMALL)
			pSecComPressInfo[dwIndex].pData = ComressDataOfJCALG1((LPBYTE)(pSecHeader[dwIndex].PointerToRawData + m_dwFileDataAddr),
				pSecHeader[dwIndex].SizeOfRawData,
				&dwCompressedSize);


		if (pSecComPressInfo[dwIndex].pData == NULL)
		{
			MessageBox(NULL, L"ѹ��ʧ�ܣ����򼴽��˳�!", L"����", NULL);
			exit(0);
		}

		pSecComPressInfo[dwIndex].CompressSize	= dwCompressedSize;

		pSecComPressInfo[dwIndex].OriginalRva	= pSecHeader[dwIndex].VirtualAddress;

		pSecComPressInfo[dwIndex].CompressRva	= dwCompressSecRva;

		//���ļ����ȶ�����Ϊ�˽�ʡ�ռ䣬���ܺ��ڻ������⣬�ȱ���

		if (dwCompressedSize % pObjectPE->dwFileAlign)
		{
			dwCompressSecRva += ((dwCompressedSize / pObjectPE->dwFileAlign + 1)*pObjectPE->dwFileAlign);
		}
		else
		{
			dwCompressSecRva += ((dwCompressedSize / pObjectPE->dwFileAlign)*pObjectPE->dwFileAlign);
		}
		
	}


	// 3.1 �޸�PEͷ��ռλ����+ѹ������

	m_pNtHeader->FileHeader.NumberOfSections = 2;
	
	// PS:��ס����Ҫ�޸ľ����С

	ZeroMemory(pSecHeader, dwNumOfSection * sizeof(IMAGE_SECTION_HEADER));

	//д��ռλ������Ϣ
	//bug,Ҫ��ռλ�ε�ƫ��ΪPEͷ�Ĵ�С
	pSecHeader[0].PointerToRawData	= pObjectPE->dwSizeOfHeader;
	pSecHeader[0].SizeOfRawData		= 0;
	pSecHeader[0].VirtualAddress	= pObjectPE->dwSectionAlign;
	pSecHeader[0].Misc.VirtualSize	= dwOccupiedSecEndRva - pObjectPE->dwSectionAlign;
	pSecHeader[0].Characteristics	= IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE;
	memcpy(pSecHeader[0].Name, ".pack0", strlen(".pack0"));
	
	//д��ѹ��������Ϣ
	//bug
	pSecHeader[1].PointerToRawData	= pObjectPE->dwSizeOfHeader;

	if ((dwCompressSecRva - dwOccupiedSecEndRva) % pObjectPE->dwSectionAlign)
	{
		pSecHeader[1].Misc.VirtualSize	= ((dwCompressSecRva - dwOccupiedSecEndRva) / pObjectPE->dwSectionAlign + 1)*pObjectPE->dwSectionAlign;
	}
	else
	{
		pSecHeader[1].Misc.VirtualSize	= ((dwCompressSecRva - dwOccupiedSecEndRva) / pObjectPE->dwSectionAlign)*pObjectPE->dwSectionAlign;
	}


	if ((dwCompressSecRva - dwOccupiedSecEndRva) % pObjectPE->dwFileAlign)
	{
		pSecHeader[1].SizeOfRawData = ((dwCompressSecRva - dwOccupiedSecEndRva) / pObjectPE->dwFileAlign + 1)*pObjectPE->dwFileAlign;
	}
	else
	{
		pSecHeader[1].SizeOfRawData = ((dwCompressSecRva - dwOccupiedSecEndRva) / pObjectPE->dwFileAlign)*pObjectPE->dwFileAlign;
	}



//	pSecHeader[1].SizeOfRawData			= pSecHeader[1].Misc.VirtualSize;
	pSecHeader[1].VirtualAddress		= dwOccupiedSecEndRva;
	pSecHeader[1].Characteristics		= IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
	memcpy(pSecHeader[1].Name, ".pack1", strlen(".pack1"));

	//��ѹ����Ϣд��
	for (DWORD dwIndex = 0; dwIndex < dwNumOfSection ; dwIndex++)
	{
		CopyMemory((PVOID)(m_dwFileDataAddr + RVAToOffset(pSecComPressInfo[dwIndex].CompressRva)),
			pSecComPressInfo[dwIndex].pData,
			pSecComPressInfo[dwIndex].CompressSize);
	}



	m_pNtHeader->OptionalHeader.SizeOfImage = pObjectPE->dwSectionAlign +
		AlignSize(pSecHeader[0].Misc.VirtualSize, pObjectPE->dwSectionAlign) +
		AlignSize(pSecHeader[1].Misc.VirtualSize, pObjectPE->dwSectionAlign);


}


//////////////////////////////////////////////////////////////////////////
//	ʹ������ָ�������ȶ���
//	������
//	DWORD		dwSize		���ݳ���
//	DWORD		dwAlign		��������

DWORD	 COperationPE::AlignSize(DWORD dwSize, DWORD dwAlign)
{

	if (dwSize % dwAlign)
	{
		return (dwSize / dwAlign + 1)*dwAlign;
	}
	else
	{
		return (dwSize / dwAlign)*dwAlign;
	}

}


//////////////////////////////////////////////////////////////////////////
//	���ض��Ľṹ�屣�浼���,���ת��
//	������
//	DWORD			dwNewAddr		�����Ž���֮�����Ļ�����
//	����ֵ��						�ñ�Ĵ�С
//////////////////////////////////////////////////////////////////////////

DWORD	COperationPE::MoveImportTable(DWORD dwNewAddr)
{
	PIMAGE_IMPORT_DESCRIPTOR	pImport = (PIMAGE_IMPORT_DESCRIPTOR)(RVAToOffset(m_stcPeInfo.stcPEImportDir.VirtualAddress)
											+ m_dwFileDataAddr);
	PIMAGE_THUNK_DATA			pThunk;
	PIMAGE_IMPORT_BY_NAME		pImportByName;

	DWORD						dwCounter;
	LPBYTE						pTemp = (LPBYTE)dwNewAddr;
	PDWORD						pFunNum;
	CHAR*						szDllName;
	

	if (!dwNewAddr)	return NULL;


	while (pImport->Name)
	{
		dwCounter = 0;

		*(PDWORD)pTemp = pImport->FirstThunk;

		pTemp += sizeof(DWORD);

		szDllName = (CHAR*)(m_dwFileDataAddr + RVAToOffset(pImport->Name));

		*pTemp =  (BYTE)strlen(szDllName);

		pTemp++;

		CopyMemory(pTemp, szDllName, strlen(szDllName)+1 );

		pTemp += strlen(szDllName) + 1;

		pFunNum = (PDWORD)pTemp;

		pTemp += sizeof(DWORD);

		//�����⴦��
		if (pImport->OriginalFirstThunk == 0)
		{
			pThunk = (PIMAGE_THUNK_DATA)(m_dwFileDataAddr + RVAToOffset(pImport->FirstThunk));
		}
		else
		{
			pThunk = (PIMAGE_THUNK_DATA)(m_dwFileDataAddr + RVAToOffset(pImport->OriginalFirstThunk));
		}


		while (pThunk->u1.AddressOfData)
		{

			if (IMAGE_SNAP_BY_ORDINAL(pThunk->u1.Ordinal))
			{//���
				*pTemp = 0x0;
				pTemp++;
				*(PDWORD)pTemp = pThunk->u1.Ordinal & 0x7FFFFFFF;
				pTemp += sizeof(DWORD);
			}
			else
			{//�ַ���
				pImportByName = (PIMAGE_IMPORT_BY_NAME)(RVAToOffset( pThunk->u1.AddressOfData) + m_dwFileDataAddr);

				*pTemp = (BYTE)strlen(pImportByName->Name);

				pTemp++;

				CopyMemory(pTemp, pImportByName->Name, strlen(pImportByName->Name)+1);
		
				pTemp += strlen(pImportByName->Name) + 1;

			}

			dwCounter++;
			pThunk++;

		}


		*pFunNum = dwCounter;
		pImport++;
	}

	//������־
	*(PDWORD)pTemp = 0x0;		
	pTemp += sizeof(DWORD);

	return	(DWORD)pTemp - dwNewAddr;

}



//////////////////////////////////////////////////////////////////////////
//	��յ����
//////////////////////////////////////////////////////////////////////////

BOOL	COperationPE::CleanImportTable()
{
	PIMAGE_IMPORT_DESCRIPTOR	pImport = (PIMAGE_IMPORT_DESCRIPTOR)(RVAToOffset(m_stcPeInfo.stcPEImportDir.VirtualAddress)
		+ m_dwFileDataAddr);
	PIMAGE_THUNK_DATA			pThunk;
	PIMAGE_IMPORT_BY_NAME		pImportByName;
	CHAR*						szDllName;
	DWORD						dwOffset = 0;

	if (!m_stcPeInfo.stcPEImportDir.VirtualAddress)	return FALSE;
	

	while (pImport->Name)
	{
		szDllName = (CHAR*)(RVAToOffset(pImport->Name) + m_dwFileDataAddr);

		ZeroMemory(szDllName, strlen(szDllName));

		if(pImport->OriginalFirstThunk)
		{
			pThunk = (PIMAGE_THUNK_DATA)(RVAToOffset(pImport->OriginalFirstThunk) + m_dwFileDataAddr);
			while (pThunk->u1.AddressOfData)
			{
				if (IMAGE_SNAP_BY_ORDINAL(pThunk->u1.Ordinal))
				{//���
					ZeroMemory(pThunk, sizeof(DWORD));
				}
				else
				{//�ַ���
					pImportByName = (PIMAGE_IMPORT_BY_NAME)(RVAToOffset(pThunk->u1.AddressOfData) + m_dwFileDataAddr);
					ZeroMemory(pImportByName->Name, strlen(pImportByName->Name));
					pImportByName->Hint = 0x0;
					ZeroMemory(pThunk, sizeof(DWORD));
				}
				pThunk++;
			}
		}//if


			pThunk = (PIMAGE_THUNK_DATA)(RVAToOffset(pImport->FirstThunk) + m_dwFileDataAddr);
			while (pThunk->u1.AddressOfData)
			{
				if (IMAGE_SNAP_BY_ORDINAL(pThunk->u1.Ordinal))
				{//���
					ZeroMemory(pThunk, sizeof(DWORD));
				}
				else
				{//�ַ���
					dwOffset = RVAToOffset(pThunk->u1.AddressOfData);
					if(dwOffset == 0)	break;
					pImportByName = (PIMAGE_IMPORT_BY_NAME)(RVAToOffset(pThunk->u1.AddressOfData) + m_dwFileDataAddr);
					ZeroMemory(pImportByName, strlen(pImportByName->Name) +sizeof(WORD));	
					ZeroMemory(pThunk, sizeof(DWORD));
				}

				pThunk++;
			}
		
			ZeroMemory(pImport, sizeof(IMAGE_IMPORT_DESCRIPTOR));
			pImport++;
	}


	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//	ת���ض�λ�����ա���������ܡ��������еĽṹ��֯
//	������
//	DWORD		dwNewAddr			����ת�����ݵĻ�������ַ
//	����ֵ��						��������С
//////////////////////////////////////////////////////////////////////////

DWORD	COperationPE::MoveRelocTable(DWORD dwNewAddr)
{
	typedef struct _TYPEOFFSET
	{
		WORD offset : 12;			//ƫ��ֵ
		WORD Type : 4;			//�ض�λ����(��ʽ)
	}TYPEOFFSET, *PTYPEOFFSET;


	PIMAGE_BASE_RELOCATION	pReloc = (PIMAGE_BASE_RELOCATION)(m_dwFileDataAddr + RVAToOffset(m_stcPeInfo.stcPERelocDir.VirtualAddress));
	DWORD					dwRelocOfItemNum;
	PTYPEOFFSET				pOffset;
	DWORD					dwNewItemOffset = 0;
	DWORD					dwTemp			= 0;
	LPBYTE					pData			= (LPBYTE)dwNewAddr;


	while (pReloc->VirtualAddress)
	{

		dwRelocOfItemNum	= (pReloc->SizeOfBlock - 8) / 2;
		pOffset				= (PTYPEOFFSET)((DWORD)pReloc + sizeof(IMAGE_BASE_RELOCATION));
		
		for (DWORD dwIndex = 0; dwIndex < dwRelocOfItemNum; dwIndex++, pOffset++)
		{
			if (pOffset->Type == IMAGE_REL_BASED_HIGHLOW)
			{
				dwNewItemOffset = pOffset->offset + pReloc->VirtualAddress - dwTemp;

				if (dwNewItemOffset > 0xfff)
				{//����ǵ�һ���ض�λ��Ļ�
					*pData = IMAGE_REL_BASED_HIGHLOW;
					pData++;
					*(PDWORD)pData = dwNewItemOffset;
					pData += sizeof(DWORD);
					
				}
				else
				{

					*(PWORD)pData = (WORD)dwNewItemOffset;
					pData += sizeof(WORD);
				}

				dwTemp += dwNewItemOffset;
			}//if


		}//for

		*(PWORD)pData = 0x0; //��Ϊһҳ�Ľ�����
		pData += sizeof(WORD);
		dwTemp = 0;
		dwNewItemOffset = 0;

		pReloc = (PIMAGE_BASE_RELOCATION)(pReloc->SizeOfBlock + (DWORD)pReloc);
	}//while

	*(PWORD)pData = 0x0;
	pData += sizeof(WORD);

	return (DWORD)pData - dwNewAddr;

}

//////////////////////////////////////////////////////////////////////////
//	����ض�λ��
//////////////////////////////////////////////////////////////////////////

BOOL	COperationPE::CleanRelocTable()
{
	typedef struct _TYPEOFFSET
	{
		WORD offset : 12;			//ƫ��ֵ
		WORD Type : 4;			//�ض�λ����(��ʽ)
	}TYPEOFFSET, *PTYPEOFFSET;


	PIMAGE_BASE_RELOCATION	pReloc = (PIMAGE_BASE_RELOCATION)(m_dwFileDataAddr + RVAToOffset(m_stcPeInfo.stcPERelocDir.VirtualAddress));
	DWORD					dwRelocOfItemNum;
	DWORD					dwSizeOfBlock;
	PTYPEOFFSET				pOffset;
	

	if (!m_stcPeInfo.stcPERelocDir.VirtualAddress)	return FALSE;
	

	while (pReloc->VirtualAddress)
	{
		dwRelocOfItemNum = (pReloc->SizeOfBlock - 8) / 2;
		pOffset = (PTYPEOFFSET)((DWORD)pReloc + sizeof(IMAGE_BASE_RELOCATION));
		dwSizeOfBlock = pReloc->SizeOfBlock;

		for (DWORD dwIndex = 0; dwIndex < dwRelocOfItemNum; dwIndex++, pOffset++)
		{
			pOffset->offset = 0x0;
			pOffset->Type	= 0x0;
		}

		memset(pReloc, 0, sizeof(IMAGE_BASE_RELOCATION));

		pReloc = (PIMAGE_BASE_RELOCATION)(dwSizeOfBlock + (DWORD)pReloc);
	}

	m_stcPeInfo.pDataDir[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = 0x0;
	m_stcPeInfo.pDataDir[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = 0x0;


	return TRUE;
}


void * _stdcall AllocFunc(DWORD nMemSize)
{
	return (VOID *)GlobalAlloc(GMEM_FIXED, nMemSize);
}

bool _stdcall DeallocFunc(void *pBuffer)
{
	GlobalFree((HGLOBAL)pBuffer);
	return true;
}

bool _stdcall CallbackFunc(DWORD pSourcePos, DWORD pDestinationPos)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//	�����ݽ���ѹ����ѹ������JCALG1
//	������
//	LPBYTE					pData			ԭʼ���ݵĻ�����
//	DWORD					dwSize			ԭʼ���ݻ������Ĵ�С
//	PDWORD					dwPackedSize	[out]ѹ�����ݵĴ�С
//	����ֵ��								ѹ�����ݵĻ�����
//////////////////////////////////////////////////////////////////////////

LPBYTE	COperationPE::ComressDataOfJCALG1(LPBYTE pData, DWORD dwSize, PDWORD dwPackedSize)
{
	DWORD	dwCompressedSize;
	DWORD	dwWindowSize = 4 * 1024;
	DWORD	dwMemSize = JCALG1_GetNeededBufferSize(dwSize);
	LPBYTE pBuffer = new BYTE[dwMemSize + 1];

	ZeroMemory(pBuffer, dwMemSize);
	dwCompressedSize =
		JCALG1_Compress((void *)pData, dwSize, (void *)pBuffer, dwWindowSize, &AllocFunc, &DeallocFunc, &CallbackFunc, 0);

	if (!dwCompressedSize)
	{	
		delete[] pBuffer;
		return NULL;
	}

	*dwPackedSize = dwCompressedSize;
	return pBuffer;
}



//////////////////////////////////////////////////////////////////////////
//	����CRC32���
//////////////////////////////////////////////////////////////////////////

VOID	COperationPE::MakeCRC32Table()
{
	uint32_t c;
	int i = 0;
	int bit = 0;

	for (i = 0; i < 256; i++)
	{
		c = (uint32_t)i;

		for (bit = 0; bit < 8; bit++)
		{
			if (c & 1)
			{
				c = (c >> 1) ^ (0xEDB88320);
			}
			else
			{
				c = c >> 1;
			}

		}
		crc32_table[i] = c;
	}



}


//////////////////////////////////////////////////////////////////////////
//	������������CRC32ֵ
//	������
//	UCHAR *		string	- ���ݿ�ָ��
//	uint32_t	size	- ���ݵĴ�С
//	����ֵ������õ�CRC32ֵ
//////////////////////////////////////////////////////////////////////////


DWORD	COperationPE::CalcuCRC(UCHAR *string, uint32_t size)
{
	//��������Ϊ-1
	uint32_t crc = 0xFFFFFFFF;


	while (size--)
		crc = (crc >> 8) ^ (crc32_table[(crc ^ *string++) & 0xff]);

	return crc;

}



//////////////////////////////////////////////////////////////////////////
//	����CRCֵ�������浽PE��ʶǰ4��byte��
//	������
//	DWORD				dwFileSize	- �ļ���С
//	ע����Ҫ��ȥDOSͷ��DOS Stub�Ĵ�С��ֻ����PEͷ֮�������
//	
//////////////////////////////////////////////////////////////////////////

VOID	COperationPE::CalAndSaveCRC(DWORD dwFileSize)
{
	DWORD	dwCrc32;	//�����ֵ

	//1. ����CRC32���
	if(m_bCRC32Table == FALSE)
		MakeCRC32Table();

	//2. ����PEͷ֮�������
	dwCrc32 = CalcuCRC((UCHAR*)(m_pDosHeader->e_lfanew + m_dwFileDataAddr), dwFileSize - m_pDosHeader->e_lfanew);

	//3. ����CRC32ֵд��PEͷ��ʶǰ4���ֽ�
	*(PDWORD)((DWORD)m_pNtHeader - 4) = dwCrc32;

}


//////////////////////////////////////////////////////////////////////////
//	Ϊ�ڴ��еĴ��������CRCУ��ֵ,��������ֵ�����ڳ�Ա������
//	����CRC32���
//	������
//	LPBYTE		pCodeBase		- ����ε�������
//	DWORD		dwSize			- ���ڴ����ȶ����Ĵ���δ�С
//////////////////////////////////////////////////////////////////////////

VOID	COperationPE::CalMemCRC(LPBYTE pCodeBase, DWORD dwSize, pPEInfo pObjectPE)
{

	//1. ����CRC32���
	if (m_bCRC32Table == FALSE)
		MakeCRC32Table();

	//2. ��������CRC32ֵ
	pObjectPE->dwCodeMemCRC32 = CalcuCRC((UCHAR*)pCodeBase, dwSize);

}