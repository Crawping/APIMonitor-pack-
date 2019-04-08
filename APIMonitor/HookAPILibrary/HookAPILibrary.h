#pragma once
#include <windows.h>
#include "./src/detours.h"
#include <stddef.h>
#include <tlhelp32.h>
#include <stdlib.h>
#include "ApiParam.h"
#include <Shlwapi.h>
#pragma comment(lib,"shlwapi.lib")


#define			MAX_NAME_LEN                100
#define			MOD_MSG						0x1
#define			API_MSG						0x2
#define			TRAP_MSG					0x3
#define			FINISH_MODMSG				0x4					//	����Ϣ���ͱ�ʾ��ģ�����ݺ�API�����Ѿ�������ϣ�UI���Խ������ɹ���
#define			FINISH_APIMSG				0x6						
#define			LIST_INC					50					//	��������
#define			LIST_API_INC				10					//	API��������
#define			FILTER_MSG					0x7					//	����ָ����API
#define			HOOK_MSG					0x5					//	Hookָ����ģ��



typedef struct _ShellCode ShellCode, *lpShellCode;
typedef	struct _ApiInfo ApiInfo, *lpApiInfo;
typedef struct _ModInfo ModInfo, *lpModInfo;

//����ָ����߳�
DWORD  WINAPI ThreadProc(LPVOID lpPara);
//ģ�����߳�
DWORD  WINAPI ThreadProc2(LPVOID lpPara);
//���ӹܵ�
HANDLE ConnectToNamePipe(WCHAR* szNamePipe);


//ɨ��ģ��
BOOL	ScanModule();
BOOL	ScanModule2();


//����API
BOOL	FilterApi(WCHAR* szModName, WCHAR* szApiName);

//Hook��UnHook
VOID	ReHookModule(DWORD dwModIndex);
VOID	HookModule(DWORD dwModIndex);
BOOL	UnHookModule(DWORD dwModIndex);

//���Mod��Ϣ
DWORD	AddModuleToList(DWORD dwBaseAddr, DWORD dwImageSize, WCHAR* szModName);

//�����Mod��Api�Ľڵ���Ϣ
DWORD	AllocApiFromList(lpModInfo lpCurModNode);


//��ʼ��ShellCode
VOID	InitApiShellStruct(lpShellCode lpShell);

//shellcode�еĴ�������
 extern "C"	  VOID  __stdcall TrappedApiCall(DWORD dwModIndex, DWORD dwApiIndex, VOID* pStack);

//ж��Detour��Hook�����ͷſռ�
BOOL UnHookAllApi();

//����List�ռ�
BOOL FreeListMemory();


//ͨ��ģ��
VOID SendModInfo(lpModInfo lpCurModNode);
VOID SendApiInfo(lpApiInfo lpCurApiNode);
VOID SendTrapInfo(VOID* pStack, DWORD dwModIndex, DWORD dwApiIndex, WCHAR* szParam);
VOID SendFinish(DWORD	dwType);

//�����жϺ���
BOOL IsFilteredApi(DWORD dwModIndex, DWORD dwApiIndex);

#pragma   pack(1) 
//shellCode�Ľṹ�壬���ڹ���
typedef struct _ShellCode
{
	BYTE   byPushadOpc;			// 0x60 pushad

	BYTE   byPushEsp;			// 0x54 push esp
	BYTE   byPushOpc2;			// 0x68 push (dword)
	DWORD  dwPushApiIndex;		// Api Index
	BYTE   byPushOpc1;			// 0x68 push (dword)
	DWORD  dwPushModIndex;		// Mod Index
	BYTE   byCallOpc;			// 0xE8 call (dword)
	DWORD  dwCallAddr;			// address of "TrappedApiCall"

	BYTE   byPopadOpc;			// 0x61 popad

	BYTE   byJmpOpcApi;			// 0xE9 jmp (dword)
	DWORD  dwJmpApiAddr;		// jmp to the real Api function
}ShellCode, *lpShellCode;





//ģ����Ϣ�ṹ��
typedef struct _ModInfo
{
	DWORD		dwBaseAddr;
	BOOL		bIsFiltered;
	DWORD		dwImageSize;
	DWORD		dwModIndex;
	WCHAR		szModName[MAX_NAME_LEN];
	DWORD		dwPrivateApiListIndex;			//	����Api�б������
	DWORD		dwApiListLen;					//	��ǰApi�б�ĳ���
	lpApiInfo	lpApiList;
	BOOL		bActive;						//	�Ƿ��Ѿ���ȡ��api��Ϣ,�ñ�־�����ж��Ѿ���hook����ģ�飬��������������ͷ���api��Ϣ����ض�
}ModInfo, *lpModInfo;

//api��Ϣ�ṹ��
typedef	struct _ApiInfo
{
	DWORD			dwApiRealAddr;
	BOOL			bIsHooked;		
	BOOL			bIsFiltered;
	DWORD			dwModIndex;
	DWORD			dwApiIndex;
	DWORD			dwOridinal;							
	WCHAR			szApiName[MAX_NAME_LEN];
	lpShellCode		lpShellBuf;					//ShellCode
}ApiInfo, *lpApiInfo;


//����API�Ľṹ��
typedef struct  _FilteredInfo
{
	DWORD	dwModIndex;
	DWORD	dwApiIndex;
	BOOL	bFilterd;
}FilteredInfo, *lpFilteredInfo;





//////////////////////////////////////////////////////////////////////////
//			ͨ�Žṹ��
//////////////////////////////////////////////////////////////////////////

//��װģ����Ϣ�����ݰ��ṹ��
typedef struct _PacketModInfo
{
	DWORD	dwBaseAddr;
	DWORD	dwModIndex;
	DWORD	dwImageSize;
	WCHAR	szModName[MAX_NAME_LEN];
}PacketModInfo, *lpPacketModInfo;



//��װApi��Ϣ�����ݰ��ṹ��
typedef	struct _PacketApiInfo
{
	DWORD	dwApiRealAddr;
	DWORD	dwModIndex;
	DWORD	dwApiIndex;					//	Api���������������
	DWORD	dwOridinal;					//	���λΪ1�Ļ�������Ϊ����ŵ���
	WCHAR	szApiName[MAX_NAME_LEN];	//	���еĻ����������ݰ��а��������򣬲���װ��Ϊ�˽�Լ�ռ�
}PacketApiInfo, *lpPacketApiInfo;



//Api���������ݰ��ṹ��
typedef struct _PacketTrapInfo
{
	DWORD	dwRetAddr;					// Api�ķ��ص�ַ
	DWORD	dwModIndex;	
	DWORD	dwApiIndex;
	//����ѡ�������
	DWORD	dwLength;					//�����ܳ���
	BYTE	byPara[1];					//������,��ȡҪע��˳��
}PacketTrapInfo, *lpPacketTrapInfo;


//���ݰ���װ�ṹ
typedef	struct _PacketInfo
{
	DWORD	dwType;						//���ͣ����궨��x_MSG
	DWORD	dwLegth;					//���ݳ���
	BYTE	Data[1];					//���ݻ�����
}PacketInfo, *lpPacketInfo;


typedef struct _StringFilteredDllList
{
	WCHAR	szDllName[12];		//Dll����ǰ׺
	DWORD	dwCheckSize;		//ƥ�䳤��(��˼�ǣ�����Ҫȫ��ƥ�䣬ƥ��ͷ�����ؼ��ּ��ɹ�����һϵ�е�Dll)
} StringFilteredDllList, *lpStringFilteredDllList;

//���ռ�ض�HOOKָ���Э��
typedef struct  _HookMod
{
	DWORD	dwModIndex;
	BOOL	bHook;
}HookMod, *lpHookMod;