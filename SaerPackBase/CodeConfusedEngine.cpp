#include "stdafx.h"
#include "CodeConfusedEngine.h"


//////////////////////////////////////////////////////////////////////////
//	��������������з����,����ת��callָ��д��������
//	������
//	COperationPE* pObjectPE		- Ŀ������PE������ָ��
//	PEInfo stcPeInfo			- Ŀ������PE�ؼ���Ϣ�ṹ��
//	CONST LPBYTE  pStart		- �����ָ��
//	DWORD dwSize				- ����γ���
//////////////////////////////////////////////////////////////////////////

pCode_Flow_Node DrawCodeFlow(COperationPE* pObjectPE, pPEInfo pPeInfo, CONST LPBYTE pStart, DWORD dwSize)
{
	pCode_Flow_Node pCodeFlow = new Code_Flow_Node;
	pCode_Flow_Node pCodeFlowHeader = NULL, *pCodeFlowNode = &pCodeFlowHeader;
	LPBYTE			pCurInpPtr;
	LPBYTE			pCurr = pStart;
	LPBYTE			pFileAddr = (LPBYTE)pObjectPE->GetFileAddr();
	DWORD			dwRva;
	DWORD			dwRaw;
	DWORD			dwOffset;
	pImport_Node	pImpHeader;
	DWORD			dwFunIndex;
	PDWORD			pdValInMem;
	//���������
	pImpHeader = DrawIATNode(pObjectPE, pPeInfo);

	if (pImpHeader == NULL)
		return NULL;
	

	// ��ʼ�����������
	ud_t ud_obj;
	ud_init(&ud_obj);
	ud_set_input_buffer(&ud_obj, pStart, dwSize);
	ud_set_mode(&ud_obj, 32);
	ud_set_syntax(&ud_obj, UD_SYN_INTEL);

	while (ud_disassemble(&ud_obj) != 0)
	{
		if (ud_obj.mnemonic == UD_Iinvalid)
			continue;
		
		// �ж��Ƿ�����ת��ַ
		switch (ud_obj.mnemonic)
		{
		case UD_Ijo:
		case UD_Ijno:
		case UD_Ijb:
		case UD_Ijae:
		case UD_Ijz:
		case UD_Ijnz:
		case UD_Ijbe:
		case UD_Ija:
		case UD_Ijs:
		case UD_Ijns:
		case UD_Ijp:
		case UD_Ijnp:
		case UD_Ijl:
		case UD_Ijge:
		case UD_Ijle:
		case UD_Ijg:
		case UD_Ijcxz:
		case UD_Ijecxz:
		case UD_Ijrcxz:
		{
			*pCodeFlowNode = new Code_Flow_Node;
			ZeroMemory(*pCodeFlowNode, sizeof(Code_Flow_Node));

			(*pCodeFlowNode)->bFar = FALSE;

			dwRva = pObjectPE->OffsetToRVA((DWORD)(pCurr - pFileAddr));
			(*pCodeFlowNode)->dwMemoryAddress = pPeInfo->dwImageBase + dwRva;
			(*pCodeFlowNode)->pFileAddress = pCurr;
			(*pCodeFlowNode)->dwType = JmpIns_Type_Jcc;
			(*pCodeFlowNode)->dwInsLen = ud_obj.inp_ctr;
			(*pCodeFlowNode)->pNext = NULL;
			(*pCodeFlowNode)->bConfused = FALSE;

			pCurInpPtr = (LPBYTE)ud_insn_ptr(&ud_obj);

			if (pCurInpPtr[0] == 0x0F)
			{//32bit
				dwOffset = ud_obj.operand[0].lval.udword;
				(*pCodeFlowNode)->dwOffset = dwOffset;
			
				if (dwOffset >= 0x80000000)
				{//������
					(*pCodeFlowNode)->bGoDown = FALSE;
					dwOffset = ~dwOffset;
					dwOffset++;
					(*pCodeFlowNode)->dwGotoMemoryAddress = (*pCodeFlowNode)->dwMemoryAddress + ud_obj.inp_ctr \
						- dwOffset;

					dwRaw = pObjectPE->RVAToOffset((*pCodeFlowNode)->dwGotoMemoryAddress - pPeInfo->dwImageBase);
					(*pCodeFlowNode)->pGotoFileAddress = dwRaw + pFileAddr;

				}
				else
				{//������
					(*pCodeFlowNode)->bGoDown = TRUE;
					(*pCodeFlowNode)->dwGotoMemoryAddress = (*pCodeFlowNode)->dwMemoryAddress + ud_obj.inp_ctr\
						+ dwOffset;

					dwRaw = pObjectPE->RVAToOffset((*pCodeFlowNode)->dwGotoMemoryAddress - pPeInfo->dwImageBase);
					(*pCodeFlowNode)->pGotoFileAddress = dwRaw + pFileAddr;
				}//if-else

				pCodeFlowNode = &((*pCodeFlowNode)->pNext);
			}
			else
			{//8bit
				//����
				delete *pCodeFlowNode;
				*pCodeFlowNode = NULL;
			}

		}
		break;
		case UD_Ijmp:
		{
			*pCodeFlowNode = new Code_Flow_Node;
			ZeroMemory(*pCodeFlowNode, sizeof(Code_Flow_Node));


			(*pCodeFlowNode)->bFar = FALSE;

			dwRva = pObjectPE->OffsetToRVA((DWORD)(pCurr - pFileAddr));
			(*pCodeFlowNode)->dwMemoryAddress = pPeInfo->dwImageBase + dwRva;
			(*pCodeFlowNode)->pFileAddress = pCurr;
			(*pCodeFlowNode)->dwType = JmpIns_Type_Jmp;
			(*pCodeFlowNode)->dwInsLen = ud_obj.inp_ctr;
			(*pCodeFlowNode)->pNext = NULL;
			(*pCodeFlowNode)->bConfused = FALSE;

			pCurInpPtr = (LPBYTE)ud_insn_ptr(&ud_obj);

			if (pCurInpPtr[0] == 0xE9)
			{//32bit

				(*pCodeFlowNode)->dwOffset = ud_obj.operand[0].lval.udword;
				dwOffset = (*pCodeFlowNode)->dwOffset;

				if (dwOffset >= 0x80000000)
				{//������
					(*pCodeFlowNode)->bGoDown = FALSE;
					dwOffset = ~dwOffset;
					dwOffset++;
					(*pCodeFlowNode)->dwGotoMemoryAddress = (*pCodeFlowNode)->dwMemoryAddress + ud_obj.inp_ctr \
						- dwOffset;

					dwRaw = pObjectPE->RVAToOffset((*pCodeFlowNode)->dwGotoMemoryAddress - pPeInfo->dwImageBase);
					(*pCodeFlowNode)->pGotoFileAddress = dwRaw + pFileAddr;

				}
				else
				{//������
	
					(*pCodeFlowNode)->bGoDown = TRUE;
					(*pCodeFlowNode)->dwGotoMemoryAddress = (*pCodeFlowNode)->dwMemoryAddress + ud_obj.inp_ctr \
						+ dwOffset;

					dwRaw = pObjectPE->RVAToOffset((*pCodeFlowNode)->dwGotoMemoryAddress - pPeInfo->dwImageBase);
					(*pCodeFlowNode)->pGotoFileAddress = dwRaw + pFileAddr;
				}//if-else

				pCodeFlowNode = &((*pCodeFlowNode)->pNext);

			}
			else
			{//8bit
				//����
				delete *pCodeFlowNode;
				*pCodeFlowNode = NULL;
			}
		}
		break;
		case UD_Icall:
		{
			*pCodeFlowNode = new Code_Flow_Node;
			ZeroMemory(*pCodeFlowNode, sizeof(Code_Flow_Node));

			(*pCodeFlowNode)->bFar = FALSE;

			dwRva = pObjectPE->OffsetToRVA((DWORD)(pCurr - pFileAddr));
			(*pCodeFlowNode)->dwMemoryAddress = pPeInfo->dwImageBase + dwRva;
			(*pCodeFlowNode)->pFileAddress = pCurr;
			(*pCodeFlowNode)->dwType = JmpIns_Type_Call;
			(*pCodeFlowNode)->dwInsLen = ud_obj.inp_ctr;
			(*pCodeFlowNode)->pNext = NULL;
			(*pCodeFlowNode)->bConfused = FALSE;

			dwOffset = ud_obj.operand[0].lval.udword;
			(*pCodeFlowNode)->dwOffset = dwOffset;

			pCurInpPtr = (LPBYTE)ud_insn_ptr(&ud_obj);

			if (pCurInpPtr[0] == 0xE8)
			{//E8+ƫ��

				if (dwOffset >= 0x80000000)
				{//������
					(*pCodeFlowNode)->bGoDown = FALSE;
					dwOffset = ~dwOffset;
					dwOffset++;
					(*pCodeFlowNode)->dwGotoMemoryAddress = (*pCodeFlowNode)->dwMemoryAddress + ud_obj.inp_ctr \
						- dwOffset;

					dwRaw = pObjectPE->RVAToOffset((*pCodeFlowNode)->dwGotoMemoryAddress - pPeInfo->dwImageBase);
					(*pCodeFlowNode)->pGotoFileAddress = dwRaw + pFileAddr;

				}
				else
				{//������		
					(*pCodeFlowNode)->bGoDown = TRUE;
					(*pCodeFlowNode)->dwGotoMemoryAddress = (*pCodeFlowNode)->dwMemoryAddress + ud_obj.inp_ctr \
						+ dwOffset;

					dwRaw = pObjectPE->RVAToOffset((*pCodeFlowNode)->dwGotoMemoryAddress - pPeInfo->dwImageBase);
					(*pCodeFlowNode)->pGotoFileAddress = dwRaw + pFileAddr;
				}//if-else

				
				(*pCodeFlowNode)->pImpNode =  AnalyseDisp(pImpHeader, 
					(*pCodeFlowNode)->pGotoFileAddress,
					&dwFunIndex);
				(*pCodeFlowNode)->dwFunIndex = dwFunIndex;
		
				pCodeFlowNode = &((*pCodeFlowNode)->pNext);
			}
			else
			{//FF15+���Ե�ַ
				//���ԣ����Ե�ַ�޷����ɻ�ָ�������Ҫ�ض�λ��BUG�������Ժ�Ľ�
				delete *pCodeFlowNode;
				*pCodeFlowNode = NULL;	
			}//if-else

		}
		break;
		}//switch

		pCurr += ud_obj.inp_ctr;
	}//while

	return pCodeFlowHeader;
}


//////////////////////////////////////////////////////////////////////////
//	��ȡ�����ļ�����ȡ��ָ��ģ���ļ�����ǰ׺����ȡģ������\
//	�����ģ�峤��ƽ��ֵ
//	������
//	LPWSTR ConfigPath		- �����ļ�·��
//	LPWSTR ModPath			- [out]ģ���ļ���ǰ׺����������·��
//	PDWORD pdModNum			- [out]ģ���ļ�������
//	PDWORD* pdArrayMod		- [out]��˳�򱣴�ÿ��ģ��Ĵ�С���������ʹ��
//	����ֵ��
//	DWORD - ģ�峤��ƽ��ֵ	 ��������-1����������
//////////////////////////////////////////////////////////////////////////


DWORD	CalcAverageVal(LPWSTR ConfigPath, LPWSTR ModPath, PDWORD pdModNum, PDWORD* pdArrayMod)
{
	WCHAR	szModPath[MAX_PATH];
	WCHAR	szModPrefix_w[MAX_PATH];
	CHAR	szModPrefix[MAX_PATH];
	DWORD	dwPrefixLen;		
	DWORD	dwFileSize;
	DWORD	dwNumOfByteRead;
	DWORD	dwNumOfMod;		//��ָ��ģ������
	DWORD	dwAverage = 0;		//ƽ��ֵ
	PDWORD	ArrayMod;	//����ģ���С������
	LPBYTE	pFileBuf;
	HANDLE hFile = CreateFile(ConfigPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == NULL)
		return	-1;

	dwFileSize = GetFileSize(hFile, NULL);

	pFileBuf = new BYTE[dwFileSize];

	ZeroMemory(pFileBuf, dwFileSize);

	ZeroMemory(szModPrefix_w, MAX_PATH* sizeof(WCHAR));

	ZeroMemory(szModPrefix, MAX_PATH);

	ReadFile(hFile, pFileBuf, dwFileSize, &dwNumOfByteRead, NULL);

	strcpy_s(szModPrefix, (PCHAR)pFileBuf);

	dwPrefixLen = strlen((PCHAR)pFileBuf);

	MultiByteToWideChar(CP_ACP, NULL, szModPrefix,
		dwPrefixLen,
		szModPrefix_w,
		dwPrefixLen);


	//��ȡģ������
	pFileBuf += (dwPrefixLen + 1);

	dwNumOfMod = *((PDWORD)(pFileBuf));

	*pdModNum = dwNumOfMod;

	ArrayMod = new DWORD[dwNumOfMod];

	ZeroMemory(ArrayMod, sizeof(DWORD)*dwNumOfMod);


	//��ȡÿһ��ģ���ļ��ĳ���

	for (DWORD dwIndex = 0; dwIndex < dwNumOfMod; dwIndex++)
	{
		pFileBuf += sizeof(DWORD);

		dwAverage += *((PDWORD)pFileBuf);

		ArrayMod[dwIndex] = *((PDWORD)pFileBuf);

	}
	
	dwAverage = dwAverage / dwNumOfMod;

	if (dwAverage % 10)
	{
		dwAverage = (dwAverage / 10 + 1) * 10;
	}



	HMODULE hModule = GetModuleHandle(L"SaerPackBase.dll");

	ZeroMemory(szModPath, sizeof(WCHAR)*MAX_PATH);

	GetModuleFileName(hModule, szModPath, MAX_PATH);

	//��ȡ�ļ�·��ǰ׺
	PathRemoveFileSpec(szModPath);

	//�ж��Ƿ�Ϊ��Ŀ¼
	BOOL bRootDir = PathIsRoot(szModPath);

	if (bRootDir)
	{//���Ǹ�Ŀ¼�����üӷ�б��

		wsprintf(ModPath, L"%s\\JunkCodeMod\\%s", szModPath, szModPrefix_w);
	}
	else
	{
		
		wsprintf(ModPath, L"%s\\JunkCodeMod\\%s", szModPath, szModPrefix_w);
	}

	CloseHandle(hFile);

	*pdArrayMod = ArrayMod;

	return dwAverage;

}

//////////////////////////////////////////////////////////////////////////
//	��ȡ�����ļ��ľ���·��
//	������
//	LPWSTR ConfigPath		- [in&out]�ļ�·��
//	DWORD dwSize			- ����������
//	����ֵ��
//	TRUE - ��ȡ�ɹ�,���򷵻�FALSE
//////////////////////////////////////////////////////////////////////////

BOOL	GetConfigPath(LPWSTR ConfigPath, DWORD dwSize)
{
	HMODULE hModule = GetModuleHandle(L"SaerPackBase.dll");
	
	if (ConfigPath == NULL || hModule == NULL)
		return FALSE;

	ZeroMemory(ConfigPath, sizeof(WCHAR)*dwSize);

	if (GetModuleFileName(hModule, ConfigPath, dwSize) == NULL)
		return FALSE;

	//��ȡ�ļ�·��ǰ׺
	if (PathRemoveFileSpec(ConfigPath) == NULL)
		return FALSE;

	//�ж��Ƿ�Ϊ��Ŀ¼
	BOOL bRootDir = PathIsRoot(ConfigPath);

	if (bRootDir)
	{//���Ǹ�Ŀ¼�����üӷ�б��
		wcscat_s(ConfigPath,dwSize, L"JunkCodeMod\\ModInfo.ini");	
	}
	else
	{
		wcscat_s(ConfigPath, dwSize, L"\\JunkCodeMod\\ModInfo.ini");
	}


	return TRUE;

}

//////////////////////////////////////////////////////////////////////////
//	��ȡ����ڵ���
//	������
//	pCode_Flow_Node pHeader		- ͷ���
//	����ֵ��
//	�ڵ������,�������㻨ָ�����εĴ�С��ָ���ƽ������*�ڵ���
//////////////////////////////////////////////////////////////////////////

DWORD	GetNumOfNode(pCode_Flow_Node pHeader)
{
	DWORD dwNum = 0;

	if (pHeader == NULL)
		return dwNum;

	do 
	{
		dwNum++;
		pHeader = pHeader->pNext;
	} while (pHeader);

	return dwNum;

}

//////////////////////////////////////////////////////////////////////////
//	�������д洢����ת�ڵ�����������
//	������
//	pCode_Flow_Node pHeader		- �����ͷ���
//	COperationPE* pObjectPE		- Ŀ������PE������
//	LPBYTE pNewSection			- �����ε�������ָ��
//	PIMAGE_SECTION_HEADER	pSecHeader	 - �����ε�ͷ�ṹ
//	DWORD	dwNumOfMod			- ģ�������
//	LPWSTR	szModPrefix			- ģ��·����ǰ׺
//	PDWORD	pdArrayMod			- ģ���С������
//	����ֵ��
//	TRUE - �ɹ� ,or else
//	ע����ÿ���ڵ����1/3��������ʳ�ȡ
//////////////////////////////////////////////////////////////////////////

BOOL	ConfuseCode(pCode_Flow_Node pHeader,
	COperationPE* pObjectPE, 
	pPEInfo pPeInfo,
	LPBYTE pNewSection, 
	PIMAGE_SECTION_HEADER	pSecHeader,
	DWORD	dwNumOfMod,
	LPWSTR	szModPrefix,
	PDWORD	pdArrayMod,
	pSample_Array	pSampleArray,
	DWORD	dwTotalCtr)
{
	DWORD			dwRemainSize;
	DWORD			dwPeFileAddr;
	DWORD			dwCurSize;
	LPBYTE			lpFileBuf;
	LPBYTE			lpCurBuf;
	DWORD			dwOffset;
	pCode_Flow_Node	pCurNode;
	WCHAR			szTest[MAX_PATH];
	BOOL			bPassByRand = TRUE;		//������� 1/3�ĸ���
	if (pObjectPE == NULL || pPeInfo == NULL || pNewSection == NULL || pdArrayMod == NULL  \
		|| szModPrefix == NULL)
		return FALSE;

	pCurNode	 = pHeader;
	dwPeFileAddr = pObjectPE->GetFileAddr();
	dwRemainSize = pSecHeader->SizeOfRawData;
	lpCurBuf	 = pNewSection;

	while (dwRemainSize >= 5 && pCurNode != NULL)
	{

		bPassByRand = RandomProbability(pSampleArray, dwTotalCtr, pCurNode->pImpNode, pCurNode->dwFunIndex);

		if (bPassByRand)
		{//�������
			lpFileBuf = RandomMod(szModPrefix, dwNumOfMod, &dwRemainSize, pdArrayMod, &dwCurSize);

			if (lpFileBuf)
			{//�ռ��㹻

				//����ָ��
				CopyMemory(lpCurBuf, lpFileBuf, dwCurSize);
				//lpCurBuf += dwCurSize;

				//���㻨ָ����ڴ��ַ���ļ���ַ
				pCurNode->dwFinalFileAddress = (DWORD)lpCurBuf;
				pCurNode->dwFinalMemoryAddress = pObjectPE->OffsetToRVA((DWORD)lpCurBuf - dwPeFileAddr) \
					+ pPeInfo->dwImageBase;

	
				if (pObjectPE->OffsetToRVA((DWORD)lpCurBuf - dwPeFileAddr) == 0)
				{//BUG����ʱ�Ҳ���,����ΪlpCurBuf �Ƚϴ󣬵���OffsetToRVAԽ�緵��0
					return FALSE;
				}


				//����ƫ��
				//��ת����ָ��
				dwOffset = pObjectPE->OffsetToRVA((DWORD)lpCurBuf - dwPeFileAddr) - \
					(pCurNode->dwMemoryAddress - pPeInfo->dwImageBase);

				if (pObjectPE->OffsetToRVA((DWORD)lpCurBuf - dwPeFileAddr) == 0)
				{//BUG����ʱ�Ҳ���,����ΪlpCurBuf �Ƚϴ󣬵���OffsetToRVAԽ�緵��0
					return FALSE;
				}


				dwOffset -= pCurNode->dwInsLen;
				*((PDWORD)(pCurNode->pFileAddress + 1)) = dwOffset;

				//��ָ����ת��Ŀ�ĵ�ַ
				lpCurBuf += dwCurSize;
				dwOffset = (pCurNode->dwGotoMemoryAddress - pPeInfo->dwImageBase) \
					- pObjectPE->OffsetToRVA((DWORD)lpCurBuf - dwPeFileAddr);

				dwOffset -= pCurNode->dwInsLen;
				*((PDWORD)(lpCurBuf + 1)) = dwOffset;
				*lpCurBuf = 0xE9;

				//��������ת��ַ,�ƶ�ָ��
				lpCurBuf += 0x5;
				pCurNode->bConfused = TRUE;
				delete lpFileBuf;

				dwCurSize += 0x5;

			}
			else
			{//�ռ䲻��,�����Դ��һ����תָ��
				//����ƫ��

				//���㻨ָ����ڴ��ַ���ļ���ַ
				pCurNode->dwFinalFileAddress = (DWORD)lpCurBuf;
				pCurNode->dwFinalMemoryAddress = pObjectPE->OffsetToRVA((DWORD)lpCurBuf - dwPeFileAddr) \
					+ pPeInfo->dwImageBase;

				if (pCurNode->dwFinalMemoryAddress == 0x400000)
				{
					__asm int 3
				}


				//��ת����ָ��,ֻ��һ����תָ��
				dwOffset = pObjectPE->OffsetToRVA((DWORD)lpCurBuf - dwPeFileAddr) - \
					(pCurNode->dwMemoryAddress - pPeInfo->dwImageBase);

				dwOffset -= pCurNode->dwInsLen;
				*((PDWORD)(pCurNode->pFileAddress + 1)) = dwOffset;
				//��ָ����ת��Ŀ�ĵ�ַ
				dwOffset = (pCurNode->dwGotoMemoryAddress - pPeInfo->dwImageBase) \
					- pObjectPE->OffsetToRVA((DWORD)lpCurBuf - dwPeFileAddr);

				dwOffset -= pCurNode->dwInsLen;
				*((PDWORD)(lpCurBuf + 1)) = dwOffset;
				*lpCurBuf = 0xE9;

				lpCurBuf += 0x5;
				pCurNode->bConfused = TRUE;

				//dwCurSize = 0x5;

			}//if-else

			//�۳�ʹ�õĿռ��С
			//RandomMod ���ص�dwCurSize ������ָ��ĩ��ַ��תָ��ĳ���
			dwRemainSize -= dwCurSize;
			
		}//if

		//ÿ���ڵ���1/3�ĸ��ʱ���ȡ
		pCurNode = pCurNode->pNext;
	
	}//while

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//	��ȡ��ָ��ģ��Ļ������ͳ���
//	������
//	LPWSTR szSelected		- ָ��ģ��ľ���·��
//	PDWORD pdSizeOfMod		-[out]��ģ��Ĵ�С
//	����ֵ��
//	�������ݵĻ�������ʧ�ܷ���NULL
//////////////////////////////////////////////////////////////////////////

LPBYTE	GetSelectedModAddr(LPWSTR szSelected, PDWORD pdSizeOfMod)
{
	if (szSelected == NULL)
		return NULL;

	DWORD	dwFileSize;
	DWORD	dwNumOfByteRead;
	HANDLE	hFile = CreateFile(szSelected, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	LPBYTE	pFileBuf;
	LPBYTE	pRetnBuf;

	if (hFile == NULL)
		return NULL;

	dwFileSize = GetFileSize(hFile, NULL);

	pFileBuf = new BYTE[dwFileSize];

	ZeroMemory(pFileBuf, dwFileSize);

	ReadFile(hFile, pFileBuf, dwFileSize, &dwNumOfByteRead, NULL);
	
	CloseHandle(hFile);

	*pdSizeOfMod = *((PDWORD)pFileBuf);

	pFileBuf += sizeof(DWORD);

	//���¸������ݣ�������������ͷſռ�
	dwFileSize = *pdSizeOfMod;

	if (dwFileSize % 10)
	{
		dwFileSize = (dwFileSize / 10 + 1) * 10;
	}

	pRetnBuf = new BYTE[dwFileSize];

	ZeroMemory(pRetnBuf, dwFileSize);

	CopyMemory(pRetnBuf, pFileBuf, *pdSizeOfMod);

	delete (pFileBuf - sizeof(DWORD));

	return pRetnBuf;
}


//////////////////////////////////////////////////////////////////////////
//	�����ȡһ����ָ��ģ�壬����ȡ��ģ��ռ���ڵ�ǰʣ��ռ䣬�� \
//	����˳���������ȡ��ǰ����ռ�ÿռ��ģ�壬�����Ҳ���������NULL
//	������
//	LPWSTR szModPrefix		- ģ�����·����ǰ׺
//	DWORD dwNumOfMod		- ģ������
//	PDWORD pdRemainSize		- [in&out]��ǰʣ��ռ�
//	PDWORD	pdArrayMod		- ģ���С������
//	PDWORD	pdCurSize		- [out]��ǰѡ��ģ��Ĵ�С
//	����ֵ��
//	���ɹ����򷵻�ģ��Ļ����������򷵻�NULL
//////////////////////////////////////////////////////////////////////////

LPBYTE	RandomMod(LPWSTR szModPrefix, DWORD dwNumOfMod, PDWORD pdRemainSize, PDWORD	pdArrayMod,
	PDWORD	pdCurSize)
{
	WCHAR	szTemp[MAX_PATH];
	DWORD	dwSize;
	LPBYTE	pFileBuF;
	DWORD	dwLowsetIndex;
	DWORD	dwTempVal;

	if (szModPrefix == NULL)
		return NULL;

	ZeroMemory(szTemp, sizeof(WCHAR)*MAX_PATH);
	wsprintf(szTemp, L"%s%d", szModPrefix, rand()%dwNumOfMod);

	pFileBuF = GetSelectedModAddr(szTemp, &dwSize);

	if (dwSize + 5 <= *pdRemainSize)
	{
		*pdCurSize = dwSize;
		return pFileBuF;
	}

	delete pFileBuF;
	


	for (DWORD dwIndex = 0; dwIndex < dwNumOfMod; dwIndex++)
	{
		if (dwIndex == 0)
		{
			dwTempVal = pdArrayMod[dwIndex];
			dwLowsetIndex = dwIndex;
			continue;
		}

		if (dwTempVal > pdArrayMod[dwIndex])
		{
			dwLowsetIndex = dwIndex;
			dwTempVal = pdArrayMod[dwIndex];
		}
	}


	//˵��ָ��ֻ������n����תָ�n��ȷ��
	if (dwTempVal + 5 > *pdRemainSize)
	{
		*pdCurSize = 0x5;
		return NULL;
	}


	ZeroMemory(szTemp, sizeof(WCHAR)*MAX_PATH);
	wsprintf(szTemp, L"%s%d", szModPrefix, dwLowsetIndex);

	pFileBuF = GetSelectedModAddr(szTemp, &dwSize);

	//*pdRemainSize -= (dwTempVal + 5);
	*pdCurSize = dwTempVal;

	return pFileBuF;

}


//////////////////////////////////////////////////////////////////////////
//	��������������ȡ�ؼ���Ϣ����ڵ��У�������"����ʶ��"ģ��ʹ��
//	������
//	COperationPE* pObjectPE		- Ŀ������PE������ָ��
//	PEInfo stcPeInfo			- Ŀ������PE�ؼ���Ϣ�ṹ��
//	����ֵ��
//	��������ͷ������NULL
//////////////////////////////////////////////////////////////////////////

pImport_Node	DrawIATNode(COperationPE* pObjectPE, pPEInfo pPeInfo)
{

	if (pObjectPE == NULL || pPeInfo == NULL)
		return NULL;

	PIMAGE_THUNK_DATA			pThunk, pTempThunk;
	PIMAGE_IMPORT_BY_NAME		pImportByName;
	DWORD						dwIndex;
	DWORD						dwCounter;
	PDWORD						pFunNum;
	CHAR*						szDllName;
	CHAR*						szFunName;
	PIMAGE_IMPORT_DESCRIPTOR	pImport;
	DWORD						dwFileAddr;
	pImport_Node				pImpCodeHeader = NULL, *pTempCode = &pImpCodeHeader;


	dwFileAddr = pObjectPE->GetFileAddr();
	
	pImport = (PIMAGE_IMPORT_DESCRIPTOR)(pObjectPE->RVAToOffset(pPeInfo->stcPEImportDir.VirtualAddress)
		+ dwFileAddr);


	while (pImport->Name)
	{
		*pTempCode = new Import_Node;

		//����ģ����
		szDllName = (CHAR*)(pObjectPE->RVAToOffset(pImport->Name) + dwFileAddr);

		(*pTempCode)->szDllName = new CHAR[strlen(szDllName) + 1];

		strncpy((*pTempCode)->szDllName, szDllName, strlen(szDllName) + 1);

		//�����ڴ��ַ���ļ���ַ
		(*pTempCode)->dwIATMemoryAddr = pImport->FirstThunk + pPeInfo->dwImageBase;

		(*pTempCode)->dwIATFileAddr = dwFileAddr + pObjectPE->RVAToOffset(pImport->FirstThunk);


		//�����⴦��
		if (pImport->OriginalFirstThunk == 0)
		{
			pThunk = (PIMAGE_THUNK_DATA)(dwFileAddr + pObjectPE->RVAToOffset(pImport->FirstThunk));
		}
		else
		{
			pThunk = (PIMAGE_THUNK_DATA)(dwFileAddr + pObjectPE->RVAToOffset(pImport->OriginalFirstThunk));
		}

		//��������
		pTempThunk = pThunk;
		dwCounter = 0;
		
		while (pTempThunk->u1.AddressOfData)
		{
			dwCounter++;
			pTempThunk++;
		}
		

		//���ɺ�������
		(*pTempCode)->dwNumOfItem = dwCounter;

		(*pTempCode)->pdFunTable = new PCHAR[dwCounter];

		ZeroMemory((*pTempCode)->pdFunTable, sizeof(PCHAR)*dwCounter);

		dwIndex = 0;


		while (pThunk->u1.AddressOfData)
		{

			if (IMAGE_SNAP_BY_ORDINAL(pThunk->u1.Ordinal))
			{//���ֱ����0����
				(*pTempCode)->pdFunTable[dwIndex] = 0x0;
			}
			else
			{//�ַ���
				pImportByName = (PIMAGE_IMPORT_BY_NAME)(pObjectPE->RVAToOffset(pThunk->u1.AddressOfData)\
					+ dwFileAddr);

				szFunName = (CHAR*)new BYTE[strlen(pImportByName->Name) + 1];
				
				CopyMemory(szFunName, pImportByName->Name, strlen(pImportByName->Name) + 1);

				(*pTempCode)->pdFunTable[dwIndex] = (PCHAR)szFunName;

			}//if - else

			dwIndex++;
			pThunk++;

		}//while


		(*pTempCode)->pNext = NULL;

		pTempCode = &((*pTempCode)->pNext);

		pImport++;
	}//while

	return pImpCodeHeader;
}


//////////////////////////////////////////////////////////////////////////
//	����Displacement���Ƿ�λ��IAT�У���λ�ڣ����ҳ���ģ�����뺯����
//	������
//	pImport_Node pImpHeader		- ����ͷ
//	LPBYTE  pFileAddr			- ��һ�����ݣ���udis86�����Ƿ�ΪFF25
//	PDWORD	pdIndex				-[out]�ýڵ�ĺ������ж�Ӧ������������
//	����ֵ��
//	��������ģ��Ľڵ����亯��������,����NULL
//////////////////////////////////////////////////////////////////////////

pImport_Node	AnalyseDisp(pImport_Node pImpHeader, LPBYTE pFileAddr,  PDWORD pdIndex)
{

	if (pFileAddr == NULL || pImpHeader == NULL)
		return NULL;

	pImport_Node	pImpTemp = pImpHeader;
	DWORD			dwMemAddrEnd;		
	DWORD			dwFileAddrEnd;
	DWORD			dwDisp;
	ud_t			ud_obj;
	LPBYTE			pCurInpPtr;
	

	ud_init(&ud_obj);
	ud_set_input_buffer(&ud_obj, pFileAddr, 0x6);
	ud_set_mode(&ud_obj, 32);
	ud_set_syntax(&ud_obj, UD_SYN_INTEL);

	while (ud_disassemble(&ud_obj) != 0)
	{
		if (ud_obj.mnemonic == UD_Iinvalid)
			return NULL;

		switch (ud_obj.mnemonic)
		{
		case UD_Ijmp:
			pCurInpPtr = (LPBYTE)ud_insn_ptr(&ud_obj);
			if (pCurInpPtr[0] == 0xFF && pCurInpPtr[1] == 0x25)
			{//FF25 - JMP ��ת��ֱ����ȡ��disp
				dwDisp = ud_obj.operand[0].lval.udword;
			}
			break;
		default:
			return NULL;
		}//switch

	}//while

	

	while (pImpTemp)
	{
		dwFileAddrEnd	= pImpTemp->dwIATFileAddr + 4 * (pImpTemp->dwNumOfItem - 1);
		dwMemAddrEnd	= pImpTemp->dwIATMemoryAddr + 4 * (pImpTemp->dwNumOfItem -1);

		if (dwDisp >= pImpTemp->dwIATMemoryAddr && dwDisp <= dwMemAddrEnd)
		{
			*pdIndex = (dwDisp - pImpTemp->dwIATMemoryAddr) / 4;

			break;
		}

		pImpTemp = pImpTemp->pNext;
	}


	return pImpTemp;
}


//////////////////////////////////////////////////////////////////////////
//	�ͷ�����
//	������
//	pCode_Flow_Node pHeader		- ����ͷ
//////////////////////////////////////////////////////////////////////////

VOID	ReleaseCodeFlow(pCode_Flow_Node pHeader)
{
	if (pHeader == NULL)
		return;

	pCode_Flow_Node pTemp;

	while (pHeader)
	{
		pTemp = pHeader->pNext;

	/*	if (pHeader->pImpNode)
		{
			delete pHeader->pImpNode->pdFunTable;
			delete pHeader->pImpNode;
		}*/

		delete pHeader;
		pHeader = pTemp;
	}

}


//////////////////////////////////////////////////////////////////////////
//	���������ļ��������鷽ʽ����
//	������
//	LPWSTR			szSample		- �������ļ�·��
//	PDWORD			pdTotalCtr		- ��Ϣ�������
//	����ֵ��
//	�����������ʼ��ַ������NULL
//////////////////////////////////////////////////////////////////////////


pSample_Array	AnalyseSample(LPWSTR szSample, PDWORD	pdTotalCtr)
{
	HANDLE				hFile;
	LPBYTE				pFileBuf;
	DWORD				dwFileSize;
	DWORD				dwNumOfBytesRead;
	DWORD				dwNumOfBlock;			//��Ϣ�������
	DWORD				dwLenOfFunc;
	DWORD				dwTotalCtr;
	pSample_Array		pSample;
	
	if (wcslen(szSample)== 0x0)
	{
		return NULL;
	}

	hFile = CreateFile(szSample, GENERIC_WRITE | GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);


	if (hFile == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	dwFileSize = GetFileSize(hFile, NULL);

	pFileBuf = new BYTE[dwFileSize];

	ZeroMemory(pFileBuf, dwFileSize);

	ReadFile(hFile, pFileBuf, dwFileSize, &dwNumOfBytesRead, NULL);

	if (   *((PDWORD)pFileBuf)     != SIGNATURE_LOG )
	{
		CloseHandle(hFile);
		return NULL;
	}

	pFileBuf += sizeof(DWORD);

	dwNumOfBlock = *((PDWORD)pFileBuf);

	
	pSample = new Sample_Array[dwNumOfBlock];

	ZeroMemory(pSample, sizeof(Sample_Array)* dwNumOfBlock);


	for (DWORD dwIndex = 0; dwIndex < dwNumOfBlock ; dwIndex++)
	{
		pFileBuf += sizeof(DWORD);

		dwLenOfFunc = strlen((PCHAR)pFileBuf);

		pSample[dwIndex].szFunc = (PCHAR)new BYTE[dwLenOfFunc + 1];

		strncpy_s(pSample[dwIndex].szFunc, dwLenOfFunc +1, (PCHAR)pFileBuf, dwLenOfFunc + 1);
	
		pFileBuf += (dwLenOfFunc + 1);

		pSample[dwIndex].dwInvokedCtr = *((PDWORD)pFileBuf);

		//dwTotalCtr += pSample[dwIndex].dwInvokedCtr;

	}

	*pdTotalCtr = dwNumOfBlock;

	CloseHandle(hFile);

	return pSample;
	
}


//////////////////////////////////////////////////////////////////////////
//	��������ܽӿ�
//	�������������ݵ�ǰ����ڵ��Ƿ�Ϊ�����ⲿ���������ǵĻ���������������ݽ���
//	ƥ�䣬ƥ��ɹ��󣬻��ʵ��ĵ���������ĸ���
//	��û�е����ⲿ����������͸���������
//	������
//	pSample_Array			pSampleArray	- �����ļ������ָ��
//	DWORD					dwTotalCtr		- ��Ϣ�������
//	pImport_Node			pImpNode		- �����ڵ���Ϣ
//	DWORD					dwFuncIndex		- �������е�����
//	����ֵ��
//	����������ʱ��鵽���򷵻�TRUE, or else
//////////////////////////////////////////////////////////////////////////

BOOL	RandomProbability(pSample_Array pSampleArray, DWORD dwTotalCtr, pImport_Node pImpNode, DWORD dwFuncIndex)
{
	WCHAR	szTest[MAX_PATH];
	static DWORD	dwTime = 0;

	if (pImpNode == NULL || pSampleArray == NULL)
	{//����͸���1/4����
		 
		srand(time(NULL) + dwTime);

		dwTime++;

		if (rand() % 16 < 4)
			return TRUE;
		else
			return FALSE;
	}

	//��ָ��Ϊ�����ⲿ����
	DWORD	dwInvokedTotalCtr;
	BOOL	bInvoked = FALSE;			//�Ƿ񱻵�����
	DWORD	dwIndex;

	for (dwIndex = 0; dwIndex < dwTotalCtr ; dwIndex++)
	{
		if (StrCmpA(pSampleArray[dwIndex].szFunc, pImpNode->pdFunTable[dwFuncIndex]) == NULL)
		{
			bInvoked = TRUE;
			break;
		}
	}


	if (bInvoked == FALSE)
	{//û���ڲ����б����õ���ֱ�Ӱ�1/3����
		srand(time(NULL));
		if (rand() % 9 < 3)
			return TRUE;
		else
			return FALSE;
	}

	dwInvokedTotalCtr = CalcTotalInvokedCtr(pSampleArray, dwTotalCtr);

	DWORD	dwCurApiInvokedCtr;
	DWORD	dwNumerator;			//����
	DWORD	dwDenominator;			//��ĸ


	dwCurApiInvokedCtr = pSampleArray[dwIndex].dwInvokedCtr;

	// ��ǰAPI���ô��� / �ܵ��ô���  + 1/3
	
	dwDenominator = 3 * dwInvokedTotalCtr;
	dwNumerator = dwInvokedTotalCtr + 3 * dwCurApiInvokedCtr;

	if (dwNumerator >= dwDenominator)
	{
		//���Ӵ��ڷ�ĸ��ֱ��ͨ��֤
		return TRUE;
	}

	if (rand() % dwDenominator < dwNumerator)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}

//////////////////////////////////////////////////////////////////////////
//	��������������У����õ��ܴ��������Լ���ÿһ��API�ĵ���ռ��
//	������
//	pSample_Array		pSampleArray		- ��������
//	DWORD				dwTotalCtr			- ���鳤��
//	����ֵ��
//	�ɹ����ص����ܴ��������� -1
//////////////////////////////////////////////////////////////////////////

DWORD	CalcTotalInvokedCtr(pSample_Array pSampleArray, DWORD dwTotalCtr)
{
	DWORD	dwTotalInvokedCtr = 0;

	if (pSampleArray == NULL)
	{
		return -1;
	}


	for (DWORD dwIndex = 0; dwIndex < dwTotalCtr ; dwIndex++)
	{
		dwTotalInvokedCtr += pSampleArray[dwIndex].dwInvokedCtr;
	}

	return dwTotalInvokedCtr;


}

//////////////////////////////////////////////////////////////////////////
//	�ͷ���������
//////////////////////////////////////////////////////////////////////////

VOID	ReleaseSampleArray(pSample_Array pSampleArray, DWORD dwTotalCtr)
{
	if (pSampleArray == NULL)
		return;


	for (DWORD dwIndex = 0; dwIndex <dwTotalCtr ; dwIndex++)
	{
		delete pSampleArray[dwIndex].szFunc;
	}

	delete pSampleArray;


}