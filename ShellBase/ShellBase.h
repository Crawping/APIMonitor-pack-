#pragma once

#ifdef SHELLBASE_EXPORTS
#define SHELL_API __declspec(dllexport)
#else
#define SHELL_API __declspec(dllimport)
#endif

#include <windows.h>
#include <Winternl.h>
#include <stdint.h> 
#include "Algorithm.h"


#define		COMPRESS_APLIB				0x0
#define		COMPRESS_JCALG1_FAST		0x1
#define		COMPRESS_JCALG1_SMALL		0x2

typedef struct _ComPressInfo
{
	DWORD	OriginalRva;		//ԭ���ε�RVA
	DWORD	CompressRva;		//ѹ�����RVA
	DWORD	CompressSize;		//ѹ����С
	LPBYTE	pData;				//ѹ������ָ��
}ComPressInfo, *PComPressInfo;


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

}SelectionInfo, *PSelectionInfo;


extern"C"  typedef struct _SHELL_DATA
{
	DWORD					dwPEOEP;			//������ڵ�
	DWORD					dwOldOEP;			//ԭ����OEP
	DWORD					dwImageBase;		//PE�ļ�Ĭ��ӳ���ַ
	DWORD					dwIATSectionBase;	//IAT���ڶλ�ַ
	DWORD					dwIATSectionSize;	//IAT���ڶδ�С
	DWORD					dwCodeBase;			//����λ�ַ
	DWORD					dwCodeSize;			//����δ�С
	DWORD					dwCodeRawSize;		//����δ�С���ļ����ȣ�
	DWORD					dwNumOfSections;	//���θ���
	IMAGE_DATA_DIRECTORY	stcPERelocDir;		//�ض�λ����Ϣ
	IMAGE_DATA_DIRECTORY	stcPEImportDir;		//�������Ϣ
	IMAGE_DATA_DIRECTORY	stcIATDir;			//IAT��Ϣ
	IMAGE_DATA_DIRECTORY	stcPEResDir;		//��Դ����Ϣ
	IMAGE_DATA_DIRECTORY	stcPETlsDir;		//tls����Ϣ
	IMAGE_DATA_DIRECTORY	stcPETlsShellDir;	//����������shell��tls����Ϣ
	BOOL					bTlsExist;			//ʹ����tls
	DWORD					dwOriginalSecRva;	//��������ԭʼ�ڱ��RVA
	DWORD					dwCompressInfoRva;	//��������ѹ���ṹ�������RVA
	DWORD					dwNewIATRva;		//��������ת�����IAT��RVA
	DWORD					dwNewRelocRva;		//��������ת������ض�λ���RVA
	BOOL					bDll;				//���������Ƿ�ΪDLL
	SelectionInfo			stcConfig;			//�ӿǲ�����������Ϣ
	DWORD					dwCodeMemCRC32;		//����ε�CRC32ֵ
	CHAR					szDllName[MAX_PATH];
}SHELL_DATA, *PSHELL_DATA;

//����ShellData�ṹ�����
extern"C" SHELL_API SHELL_DATA g_stcShellData;


typedef DWORD(WINAPI *fnGetProcAddress)(_In_ HMODULE hModule, _In_ LPCSTR lpProcName);
typedef HMODULE(WINAPI *fnLoadLibraryA)(_In_ LPCSTR lpLibFileName);
typedef HMODULE(WINAPI *fnGetModuleHandleA)(_In_opt_ LPCSTR lpModuleName);
typedef BOOL(WINAPI *fnVirtualProtect)(_In_ LPVOID lpAddress, _In_ SIZE_T dwSize, _In_ DWORD flNewProtect, _Out_ PDWORD lpflOldProtect);
typedef LPVOID(WINAPI *fnVirtualAlloc)(_In_opt_ LPVOID lpAddress, _In_ SIZE_T dwSize, _In_ DWORD flAllocationType, _In_ DWORD flProtect);
typedef void(WINAPI *fnExitProcess)(_In_ UINT uExitCode);
typedef int(WINAPI *fnMessageBox)(HWND hWnd, LPSTR lpText, LPSTR lpCaption, UINT uType);
typedef void(WINAPI* fnOutPutDebugString)(_In_opt_ LPCSTR lpOutputString);
typedef	BOOL(WINAPI* fnVirtualFree)(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);
typedef DWORD(WINAPI* fnGetModuleFileNameA)(_In_opt_ HMODULE hModule, _Out_writes_to_(nSize, ((return < nSize) ? (return +1) : nSize)) LPSTR lpFilename,
	_In_ DWORD nSize);

typedef HANDLE (WINAPI* fnCreateFileA)(
	_In_ LPCSTR lpFileName,
	_In_ DWORD dwDesiredAccess,
	_In_ DWORD dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_ DWORD dwCreationDisposition,
	_In_ DWORD dwFlagsAndAttributes,
	_In_opt_ HANDLE hTemplateFile
);

typedef DWORD (WINAPI* fnGetFileSize)(
	_In_ HANDLE hFile,
	_Out_opt_ LPDWORD lpFileSizeHigh
);

typedef BOOL (WINAPI* fnWriteFile)(
	_In_ HANDLE hFile,
	_In_reads_bytes_opt_(nNumberOfBytesToWrite) LPCVOID lpBuffer,
	_In_ DWORD nNumberOfBytesToWrite,
	_Out_opt_ LPDWORD lpNumberOfBytesWritten,
	_Inout_opt_ LPOVERLAPPED lpOverlapped
);



typedef BOOL (WINAPI* fnReadFile)(
	_In_ HANDLE hFile,
	_Out_writes_bytes_to_opt_(nNumberOfBytesToRead, *lpNumberOfBytesRead) __out_data_source(FILE) LPVOID lpBuffer,
	_In_ DWORD nNumberOfBytesToRead,
	_Out_opt_ LPDWORD lpNumberOfBytesRead,
	_Inout_opt_ LPOVERLAPPED lpOverlapped
);


typedef BOOL (WINAPI* fnCloseHandle)(
	_In_ _Post_ptr_invalid_ HANDLE hObject
);


typedef void*(__cdecl *fnMemset)(
	_Out_writes_bytes_all_(_Size) void*  _Dst,
	_In_                          int    _Val,
	_In_                          size_t _Size);

typedef void*(__cdecl *fnMemcpy)(
	_Out_writes_bytes_all_(_Size) void* _Dst,
	_In_reads_bytes_(_Size)       void const* _Src,
	_In_                          size_t      _Size);


typedef VOID (WINAPI* fnGetSystemInfo)(
	_Out_ LPSYSTEM_INFO lpSystemInfo
);


typedef SIZE_T (WINAPI* fnVirtualQuery)(
	_In_opt_ LPCVOID lpAddress,
	_Out_writes_bytes_to_(dwLength, return) PMEMORY_BASIC_INFORMATION lpBuffer,
	_In_ SIZE_T dwLength
);





//////////////////////////////////////////////////////////////////////////
//	Anti-Dump����ṹ��
//////////////////////////////////////////////////////////////////////////

typedef struct _MYPEB_LDR_DATA
{
	PVOID					Reserved1[3];
	LIST_ENTRY              InLoadOrderModuleList;
	LIST_ENTRY              InMemoryOrderModuleList;
	LIST_ENTRY              InInitializationOrderModuleList;
} MYPEB_LDR_DATA, *PMYPEB_LDR_DATA;

typedef struct _MYLDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY              InLoadOrderModuleList;
	LIST_ENTRY              InMemoryOrderModuleList;
	LIST_ENTRY              InInitializationOrderModuleList;
	PVOID                   BaseAddress;
	PVOID                   EntryPoint;
	ULONG                   SizeOfImage;
	UNICODE_STRING          FullDllName;
	UNICODE_STRING          BaseDllName;
	ULONG                   Flags;
	SHORT                   LoadCount;
	SHORT                   TlsIndex;
	LIST_ENTRY              HashTableEntry;
	ULONG                   TimeDateStamp;
} MYLDR_DATA_TABLE_ENTRY, *PMYLDR_DATA_TABLE_ENTRY;

typedef struct _MYRTL_USER_PROCESS_PARAMETERS
{
	ULONG                   MaximumLength;
	ULONG                   Length;
	ULONG                   Flags;
	ULONG                   DebugFlags;
	PVOID                   ConsoleHandle;
	ULONG                   ConsoleFlags;
	HANDLE                  StdInputHandle;
	HANDLE                  StdOutputHandle;
	HANDLE                  StdErrorHandle;
	UNICODE_STRING          CurrentDirectoryPath;
	HANDLE                  CurrentDirectoryHandle;
	UNICODE_STRING          DllPath;
	UNICODE_STRING          ImagePathName;
	UNICODE_STRING          CommandLine;
	PVOID                   Environment;
	ULONG                   StartingPositionLeft;
	ULONG                   StartingPositionTop;
	ULONG                   Width;
	ULONG                   Height;
	ULONG                   CharWidth;
	ULONG                   CharHeight;
	ULONG                   ConsoleTextAttributes;
	ULONG                   WindowFlags;
	ULONG                   ShowWindowFlags;
	UNICODE_STRING          WindowTitle;
	UNICODE_STRING          DesktopName;
	UNICODE_STRING          ShellInfo;
	UNICODE_STRING          RuntimeData;
}MYRTL_USER_PROCESS_PARAMETERS, *PMYRTL_USER_PROCESS_PARAMETERS;