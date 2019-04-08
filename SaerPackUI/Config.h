#pragma once
#include <winternl.h>

//ѹ������
#define		COMPRESS_APLIB				0x0
#define		COMPRESS_JCALG1_FAST		0x1
#define		COMPRESS_JCALG1_SMALL		0x2
 

typedef struct _SelectionInfo
{
	//��������
	BOOL	bIsCompression;
	DWORD	dwCompressionType;
	BOOL	bIsEncryption;
	BOOL	bIsTransferIAT;
	BOOL	bIsTransferReloc;

	//AntiCrack����
	BOOL	bIsApiRedirect;			//IAT�ض���
	BOOL	bIsAntiDebugging;		//������
	BOOL	bIsAntiDump;			//��Dump
	BOOL	bIsFusedCracker;		//��������
	BOOL	bIsFileVerification;	//�ļ�У��
	BOOL	bIsMemVerification;		//�ڴ�У��
	BOOL	bIsAntiOD;				//��OD

	//���鹦��
	BOOL	bIsVirtualizeCode;		//�������⻯
	WCHAR	szSample[MAX_PATH];

}SelectionInfo, *PSelectionInfo;

//typedef struct _MYPEB_LDR_DATA
//{
//	PVOID					Reserved1[3];
//	LIST_ENTRY              InLoadOrderModuleList;
//	LIST_ENTRY              InMemoryOrderModuleList;
//	LIST_ENTRY              InInitializationOrderModuleList;
//} MYPEB_LDR_DATA, *PMYPEB_LDR_DATA;
//
//
//typedef struct _MYLDR_DATA_TABLE_ENTRY
//{
//	LIST_ENTRY              InLoadOrderModuleList;
//	LIST_ENTRY              InMemoryOrderModuleList;
//	LIST_ENTRY              InInitializationOrderModuleList;
//	PVOID                   BaseAddress;
//	PVOID                   EntryPoint;
//	ULONG                   SizeOfImage;
//	UNICODE_STRING          FullDllName;
//	UNICODE_STRING          BaseDllName;
//	ULONG                   Flags;
//	SHORT                   LoadCount;
//	SHORT                   TlsIndex;
//	LIST_ENTRY              HashTableEntry;
//	ULONG                   TimeDateStamp;
//} MYLDR_DATA_TABLE_ENTRY, *PMYLDR_DATA_TABLE_ENTRY;


//////////////////////////////////////////////////////////////////////////
//	�ӿ��ܽӿ�
//	������
//	_In_ LPWSTR				- �ӿ��ļ��ľ��Ե�ַ
//	 _In_ PSelectionInfo	- ������Ϣ��ѹ�������ܡ�ת����AntiCrack��ѡ��ȣ�
//////////////////////////////////////////////////////////////////////////

typedef	BOOL(*fnPackBase)	(_In_ LPWSTR, _In_ PSelectionInfo);