#include "stdafx.h"
#include "ProcessingTransmit.h"




ProcessingList::ProcessingList()
{//ΪModlist�����ڴ�ռ�
	m_pModList		= (lpModList)VirtualAlloc(NULL, sizeof(ModList) * LIST_INC, MEM_COMMIT, PAGE_READWRITE);
	m_dwModListLen	= LIST_INC;
	ZeroMemory(m_pModList, LIST_INC * sizeof(ModList));
	m_pLog			= NULL;
}

ProcessingList::~ProcessingList()
{

	delete[] m_pModList;
}



//////////////////////////////////////////////////////////////////////////
//	���ģ����Ϣ����Ӧ��Listλ���У����������е�ָ��λ��
//	������
//	lpPacketModInfo		lpData			-	ָ��һ��ģ����Ϣ�Ľṹ��
//////////////////////////////////////////////////////////////////////////

BOOL ProcessingList::AddModInfo(lpPacketModInfo lpData)
{
	DWORD	dwModIndex = lpData->dwModIndex;

	//���ж�ModIndex�ǲ����ڷ�Χ֮��
	if (lpData->dwModIndex >= m_dwModListLen)
	{
		lpModList	pTemp;
		//λ�ò�����Ҫ��չ���鳤��
		pTemp = (lpModList)VirtualAlloc(NULL, sizeof(ModList) * (m_dwModListLen + LIST_INC), MEM_COMMIT, PAGE_READWRITE);
		
		if (!pTemp)	return FALSE;

		ZeroMemory(pTemp, sizeof(ModList) * (m_dwModListLen + LIST_INC));

		CopyMemory(pTemp, m_pModList, sizeof(ModList)* m_dwModListLen);

		VirtualFree(m_pModList, 0, MEM_RELEASE);

		m_pModList = pTemp;

		m_dwModListLen += LIST_INC;
	}

	
	CopyMemory(&m_pModList[dwModIndex], lpData, sizeof(PacketModInfo));
	
	//�����ģ���ڱ���������ʹ��
	m_pModList[dwModIndex].bActive = TRUE;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//	����Api��Ϣ��ӵ�Modlist����apilist��
//	������
//	lpPacketApiInfo				lpData	 - ����API��Ϣ�Ľṹ��ָ��
//////////////////////////////////////////////////////////////////////////

BOOL	ProcessingList::AddApiInfo(lpPacketApiInfo lpData)
{
	DWORD				dwModIndex		= lpData->dwModIndex;
	DWORD				dwApiIndex		= lpData->dwApiIndex;
	lpPacketApiInfo		lpApiList		= NULL;
	DWORD				dwApiListLen;
	
	if (!lpData)		return	FALSE;

	if (m_pModList[dwModIndex].lpApiList == NULL)
	{
		m_pModList[dwModIndex].lpApiList = (lpPacketApiInfo)VirtualAlloc(NULL, sizeof(PacketApiInfo) * LIST_INC,
			MEM_COMMIT, PAGE_READWRITE);;
		m_pModList[dwModIndex].dwApiListLen		= LIST_INC;

		ZeroMemory(m_pModList[dwModIndex].lpApiList, sizeof(PacketApiInfo) * LIST_INC);
	}

	//����ռ䲻�㣬��Ҫ����
	if (m_pModList[dwModIndex].dwApiListLen <= lpData->dwApiIndex)
	{
		dwApiListLen = m_pModList[dwModIndex].dwApiListLen;
		lpApiList = (lpPacketApiInfo)VirtualAlloc(NULL, sizeof(PacketApiInfo) * (dwApiListLen + LIST_INC),
					MEM_COMMIT, PAGE_READWRITE);

		if (!lpApiList)	return FALSE;

		m_pModList[dwModIndex].dwApiListLen += LIST_INC;
		
		ZeroMemory(lpApiList, sizeof(PacketApiInfo) * (dwApiListLen + LIST_INC));

		CopyMemory(lpApiList, m_pModList[dwModIndex].lpApiList, sizeof(PacketApiInfo) * dwApiListLen);

		VirtualFree(m_pModList[dwModIndex].lpApiList, 0, MEM_RELEASE);

		m_pModList[dwModIndex].lpApiList = lpApiList;
	}

	lpApiList = m_pModList[dwModIndex].lpApiList;

	CopyMemory(&lpApiList[dwApiIndex], lpData, sizeof(PacketApiInfo));

	lpApiList[dwApiIndex].bActive = TRUE;

	return	TRUE;
}


//////////////////////////////////////////////////////////////////////////
//	���ݰ����ˣ����ɺ���
//	������
//	lpPacketInfo		lpData - ���ݰ�������
//	����ֵ����Ϣ���ͣ�ʹ�ⲿ���ú������Ը��ݸ�����ʵ��UI����
//////////////////////////////////////////////////////////////////////////

DWORD	ProcessingList::GetMsgInfo(lpPacketInfo	lpData)
{
	WCHAR				szTrap[MAX_BUF_SIZE];
	lpTrapInfo			lpCurTrap;

	switch (lpData->dwType)
	{
	case MOD_MSG:	//ģ����Ϣ
		AddModInfo((lpPacketModInfo)lpData->Data);
		OutputDebugString(L"ģ�����OK");
		break;
	case API_MSG:	//Api��Ϣ
		AddApiInfo((lpPacketApiInfo)lpData->Data);
		break;
	case TRAP_MSG:	//Api������Ϣ
		ZeroMemory(szTrap, sizeof(WCHAR)*MAX_BUF_SIZE);
		lpCurTrap = (lpTrapInfo)lpData->Data;
		
		wsprintf(szTrap, L"Mod:%s ApiIndex:%s RetAddr:%x", m_pModList[lpCurTrap->dwModIndex].szModName,
			m_pModList[lpCurTrap->dwModIndex].lpApiList[lpCurTrap->dwApiIndex].szApiName, 
			lpCurTrap->dwRetAddr);
		

		break;
	default:
		break;
	}

	return lpData->dwType;

}


//////////////////////////////////////////////////////////////////////////
//	��DLL�ε�ĳ��ģ�鷢��Hook����UnHook��ָ��
//	������
//	HANDLE		hPipe		- �ܵ����
//	WCHAR*		szModName	- ģ����
//	BOOL		bHook		- �Ƿ�HOOK
//////////////////////////////////////////////////////////////////////////

VOID	ProcessingList::SendHookMod(HANDLE hPipe, WCHAR* szModName, BOOL bHook)
{
	lpPacketInfo	lpPacket = NULL;
	HookMod			stcHookInfo;
	DWORD			dwTotalLength;
	DWORD			dwBytesOfWritten;

	for (DWORD dwIndex = 0; dwIndex < m_dwModListLen; dwIndex++)
		if (wcscmp(szModName, m_pModList[dwIndex].szModName) == 0)
		{
			stcHookInfo.bHook = bHook;
			stcHookInfo.dwModIndex = dwIndex;
			dwTotalLength = sizeof(HookMod) + sizeof(PacketInfo);
			lpPacket = (lpPacketInfo)new BYTE[dwTotalLength];
			lpPacket->dwType = HOOK_MSG;
			lpPacket->dwLegth = sizeof(HookMod);
			CopyMemory(lpPacket->Data, &stcHookInfo, lpPacket->dwLegth);
			
			WriteFile(hPipe, lpPacket, dwTotalLength, &dwBytesOfWritten, NULL);
			break;
		}

}


//////////////////////////////////////////////////////////////////////////
//	�������õ�ַ����ȡ����õ�ģ����
//	������
//	DWORD dwInvokedAddr	 - ���õ�ַ
//	����ֵ��	ȫ��ModList�е�ModName��ַ
//////////////////////////////////////////////////////////////////////////

WCHAR*	ProcessingList::GetInvokedModName(DWORD dwInvokedAddr)
{
	for (DWORD	dwIndex = 0; dwIndex < m_dwModListLen; dwIndex++)
	{
		if (m_pModList[dwIndex].dwBaseAddr <= dwInvokedAddr && 
			dwInvokedAddr < (m_pModList[dwIndex].dwBaseAddr + m_pModList[dwIndex].dwImageSize))
		{
			return m_pModList[dwIndex].szModName;
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//	��ȡָ�����Ե�Api����
//	������
//	DWORD dwModIndex	 - ģ������
//	DWORD dwApiIndex	 - Api����
//	����ֵ��	Api���Ƶĵ�ַ
//////////////////////////////////////////////////////////////////////////

WCHAR*	ProcessingList::GetApiName(DWORD dwModIndex, DWORD dwApiIndex)
{
	if (dwApiIndex >= m_pModList[dwModIndex].dwApiListLen)	return NULL;
	return m_pModList[dwModIndex].lpApiList[dwApiIndex].szApiName;
}


//////////////////////////////////////////////////////////////////////////
//	���͹���Api�����ݰ�
//	������
//	HANDLE			hPipe		- �ܵ����
//	DWORD			dwModIndex	- ģ������
//	DWORD			dwApiIndex	- Api����
//	BOOL			bFiltered	- �Ƿ����
//////////////////////////////////////////////////////////////////////////

VOID	ProcessingList::SendFilteredApi(HANDLE hPipe, DWORD dwModIndex, DWORD dwApiIndex, BOOL bFiltered)
{
	lpPacketInfo	lpPacket = NULL;
	FilteredInfo	stcApi;
	DWORD			dwTotalLength;
	DWORD			dwBytesOfWritten;


	stcApi.dwModIndex = dwModIndex;
	stcApi.dwApiIndex = dwApiIndex;
	stcApi.bFilterd		= bFiltered;

	dwTotalLength = sizeof(FilteredInfo) + sizeof(PacketInfo);
	lpPacket = (lpPacketInfo)new BYTE[dwTotalLength];
	lpPacket->dwType = FILTER_MSG;
	lpPacket->dwLegth = sizeof(FilteredInfo);
	CopyMemory(lpPacket->Data, &stcApi, lpPacket->dwLegth);

	WriteFile(hPipe, lpPacket, dwTotalLength, &dwBytesOfWritten, NULL);
	
}



//////////////////////////////////////////////////////////////////////////
//	������ģ����õĺ���ͳ�ƽ��б��У������ۻ����������Ȳ����Զ���չ
//	������
//	CHAR* szApiName - ������
//////////////////////////////////////////////////////////////////////////


VOID	ProcessingList::InsertOfBinary(CHAR* szApiName)
{
	DWORD			dwIndex;
	lpLogBinary		lpTemp;

	if (m_pLog == NULL)
	{
		m_pLog = (lpLogBinary)VirtualAlloc(NULL, LIST_INC * sizeof(LogBinary), MEM_COMMIT, PAGE_READWRITE);
		ZeroMemory(m_pLog, LIST_INC * sizeof(LogBinary));
		m_dwLogListLen = LIST_INC;
	}

	for (dwIndex = 0; dwIndex < m_dwLogListLen; dwIndex++)
	{
		if (strlen( m_pLog[dwIndex].szApiName) == 0)	break;
		
		if (strcmp(szApiName, m_pLog[dwIndex].szApiName) == 0)
		{
			m_pLog[dwIndex].dwCount++;
			return;
		}
	}

	//�б��Ȳ��㣬����չ
	if (dwIndex == m_dwLogListLen)
	{
		m_dwModListLen += LIST_INC;
		lpTemp = (lpLogBinary)VirtualAlloc(NULL, m_dwModListLen  * sizeof(LogBinary), MEM_COMMIT, PAGE_READWRITE);
		ZeroMemory(lpTemp, sizeof(LogBinary)*m_dwModListLen);
		CopyMemory(lpTemp, m_pLog, dwIndex * sizeof(LogBinary));
		VirtualFree(m_pLog, 0, MEM_RELEASE);
		m_pLog = lpTemp;
	}

	//�����³�Ա
	strcpy_s(m_pLog[dwIndex].szApiName, szApiName);
	m_pLog[dwIndex].dwCount++;


}


//////////////////////////////////////////////////////////////////////////
//	���ط�װ���ݵĻ������������б�Ĵ�С(bytes)
//	������
//	DWORD*		pdSize		- [out]���б���ֽڴ�С
//	����ֵ��	����log�Ķ����ƻ�����
//////////////////////////////////////////////////////////////////////////

DWORD	ProcessingList::GetLogInfo(DWORD* pdSize)
{
	DWORD		dwIndex;
	LPBYTE		lpData = NULL;
	DWORD		dwOffset = 0;

	if (m_pLog == NULL)
	{
		*pdSize = 0x0;
		return NULL;
	}

	for (dwIndex = 0; dwIndex < m_dwLogListLen; dwIndex++)
	{
		if (strlen(m_pLog[dwIndex].szApiName) == 0)	break;
	}

	// Signature | ItemCount | szApiname1 | invokedCount | szApiname2 |invokedCount
	*pdSize = sizeof(DWORD) +  sizeof(DWORD) +   dwIndex * sizeof(LogBinary) ;

	lpData = (LPBYTE)VirtualAlloc(NULL, *pdSize, MEM_COMMIT, PAGE_READWRITE);

	//����Signature
	*(PDWORD)lpData = SIGNATURE_LOG;
	dwOffset += sizeof(DWORD);

	//����ItemCount
	CopyMemory(lpData+ dwOffset, &dwIndex, sizeof(DWORD));
	dwOffset += sizeof(DWORD);

	for (DWORD i = 0; i < dwIndex; i++)
	{
		strcpy_s((CHAR*)(lpData + dwOffset),  strlen(m_pLog[i].szApiName)+1,  m_pLog[i].szApiName);
		dwOffset += (strlen(m_pLog[i].szApiName) + 1);
		CopyMemory(lpData + dwOffset, &m_pLog[i].dwCount, sizeof(DWORD));
		dwOffset += sizeof(DWORD);
	}
	
	*pdSize = dwOffset;

	return (DWORD)lpData;

}