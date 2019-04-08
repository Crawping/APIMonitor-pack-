// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� SAERPACKBASE_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// SAERPACKBASE_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef SAERPACKBASE_EXPORTS
#define SAERPACKBASE_API __declspec(dllexport)
#else
#define SAERPACKBASE_API __declspec(dllimport)
#endif

#include "OperationPE.h"
#include "CodeConfusedEngine.h"

typedef struct _SHELL_DATA
{
	DWORD					dwPEOEP;			//������ڵ�
	DWORD					dwOldOEP;			//ԭ����OEP
	DWORD					dwImageBase;		//PE�ļ�Ĭ��ӳ���ַ
	DWORD					dwIATSectionBase;	//IAT���ڶλ�ַ
	DWORD					dwIATSectionSize;	//IAT���ڶδ�С
	DWORD					dwCodeBase;			//����λ�ַ
	DWORD					dwCodeSize;			//����δ�С���ڴ����ȣ�
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


// ����һ��������API��������������ִ�мӿǲ���
extern "C" SAERPACKBASE_API BOOL PackBase(LPWSTR strPath, PSelectionInfo pSelect);

// �ӿ�ʱ���õ��ĺ�������
extern VOID		Pretreatment(LPBYTE pCodeStart, LPBYTE pCodeEnd, COperationPE* pObejctPE, pPEInfo pPeInfo, PSelectionInfo pSelect);
extern DWORD	Implantation( LPWSTR pFileName, DWORD dwFileBufSize, COperationPE* pObjectPE, PEInfo stcPeInfo, PSHELL_DATA pGlobalVar, PSelectionInfo pSelect);
