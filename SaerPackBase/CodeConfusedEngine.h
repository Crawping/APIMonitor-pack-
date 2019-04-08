#pragma once

#include <windows.h>
#include <udis86.h>
#include <time.h>
#include "OperationPE.h"

//��ת��callָ�������ṹ

#define JmpIns_Type_Jcc			0x0
#define JmpIns_Type_Jmp			0x1
#define JmpIns_Type_Call		0x2

typedef struct _Import_Node Import_Node, *pImport_Node;

typedef struct _Code_Flow_Node
{
	struct _Code_Flow_Node *pNext;					//��һ���ڵ�
	BOOL					bGoDown;				//�Ƿ�������
	DWORD					dwBits;					//��ת��Χ
	DWORD					dwType;					//ָ������
	BOOL					bFar;					//�Ƿ���Զ��
	DWORD					dwMemoryAddress;		//��ǰ�ڴ��ַ
	LPBYTE					pFileAddress;			//��ǰ�ļ���ַ
	DWORD					dwGotoMemoryAddress;	//��ת����ڴ��ַ
	LPBYTE					pGotoFileAddress;		//��ת����ļ���ַ
	DWORD					dwInsLen;				//ָ���
	pImport_Node			pImpNode;				//��IAT�еĽڵ���Ϣ
	DWORD					dwFunIndex;				//�ڵ㺯���������
	DWORD					dwFinalMemoryAddress;	//��ָ����ڴ��ַ
	DWORD					dwFinalFileAddress;		//��ָ����ļ���ַ
	BOOL					bConfused;				//�Ƿ�����
	union
	{
		BYTE bOffset;
		WORD wOffset;
		DWORD dwOffset;
	};//ƫ��
}Code_Flow_Node, *pCode_Flow_Node;


typedef struct _Import_Node
{
	CHAR*					szDllName;			//ģ����
	DWORD					dwIATMemoryAddr;	//IAT���ڴ��ַ
	DWORD					dwIATFileAddr;		//IAT���ļ���ַ
	DWORD					dwNumOfItem;		//���뺯��������
	PCHAR *					pdFunTable;			//��������
	struct _Import_Node*	pNext;
}Import_Node, *pImport_Node;



//�����ļ���ʹ�õĴ洢�ṹ
typedef	struct _Sample_Array
{
	DWORD	dwInvokedCtr;						//�����õĴ���
	PCHAR	szFunc;								//������
}Sample_Array, *pSample_Array;



//////////////////////////////////////////////////////////////////////////
//	����ദ��ӿ�
//////////////////////////////////////////////////////////////////////////

pCode_Flow_Node DrawCodeFlow(COperationPE* pObjectPE, pPEInfo pPeInfo, CONST LPBYTE pStart, DWORD dwSize);

//////////////////////////////////////////////////////////////////////////
//	��ָ��ģ���ļ��ӿ�
//////////////////////////////////////////////////////////////////////////

DWORD	CalcAverageVal(LPWSTR ConfigPath, LPWSTR ModPath, PDWORD pdModNum, PDWORD* pdArrayMod);
BOOL	GetConfigPath(LPWSTR ConfigPath, DWORD dwConfigSize);
DWORD	GetNumOfNode(pCode_Flow_Node pHeader);
BOOL	ConfuseCode(pCode_Flow_Node pHeader, 
		COperationPE* pObjectPE,
		pPEInfo pPeInfo,
		LPBYTE pNewSection,
		PIMAGE_SECTION_HEADER	pSecHeader,
		DWORD	dwNumOfMod,
		LPWSTR	szModPrefix,
		PDWORD	pdArrayMod,
		pSample_Array	pSampleArray,
		DWORD	dwTotalCtr);
LPBYTE	GetSelectedModAddr(LPWSTR szSelected, PDWORD pdSizeOfMod);
LPBYTE	RandomMod(LPWSTR szModPrefix, DWORD dwNumOfMod, PDWORD pdRemainSize, PDWORD	pdArrayMod,
	PDWORD	pdCurSize);

VOID	ReleaseCodeFlow(pCode_Flow_Node pHeader);


//////////////////////////////////////////////////////////////////////////
//		�������ӿ�
//////////////////////////////////////////////////////////////////////////

pImport_Node	DrawIATNode(COperationPE* pObjectPE, pPEInfo pPeInfo);
pImport_Node	AnalyseDisp(pImport_Node pImpHeader,LPBYTE pFileAddr, PDWORD pdIndex);


//////////////////////////////////////////////////////////////////////////
//	�����ļ�����ӿ�
//////////////////////////////////////////////////////////////////////////

#define			SIGNATURE_LOG				0x00005150

pSample_Array	AnalyseSample(LPWSTR szSample, PDWORD pdTotalCtr);
DWORD			CalcTotalInvokedCtr(pSample_Array pSampleArray, DWORD dwTotalCtr);
BOOL			RandomProbability(pSample_Array pSampleArray, DWORD dwTotalCtr,pImport_Node pImpNode,DWORD	dwFuncIndex);
VOID			ReleaseSampleArray(pSample_Array pSampleArray, DWORD dwTotalCtr);