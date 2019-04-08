#pragma once

#include <Windows.h>
#include <shlwapi.h>
#pragma comment(lib,"shlwapi.lib")


#define  MAX_DEF_FILENAME_LENGTH 40


typedef struct _DefFileInfo
{
	WCHAR  szDefFileObject[MAX_PATH];			//ģ����
	VOID*  lpMapAddress;						//ӳ���ַ
	DWORD  dwMapSize;							//ռ�ô�С
} DefFileInfo, *lpDefFileInfo;


BOOL	DoMapFile(WCHAR* szFileName, WCHAR* szDir);
VOID	UnmapSSFiles();
BOOL	ToNextLine(WCHAR* &pCh);
WCHAR*	GetApiParam(WCHAR* szDll, WCHAR* szApi, VOID* pStack);
BOOL	CreateSSMapFile();

BOOL	GetHookModel();
