#pragma once

#include <Windows.h>
#include <string.h>
#include <stdlib.h>
#include <shlwapi.h>
#include <aplib.h>
#include <jcalg1.h>
#include <stdint.h>  
#pragma comment(lib,"aplib.lib")
#pragma comment(lib,"shlwapi.lib")


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

	//���鹦��
	BOOL	bIsVirtualizeCode;		//�������⻯
	WCHAR	szSample[MAX_PATH];
}SelectionInfo, *PSelectionInfo;



typedef struct _PEInfo
{
	DWORD					dwOEP;				// ��ڵ�
	DWORD					dwImageBase;		// ӳ���ַ
	DWORD					dwSectionAlign;		// �ڴ��������
	DWORD					dwFileAlign;		// �ļ���������
	DWORD					dwSizeOfImage;		// �����С
	DWORD					dwNumOfSections;	// �ڱ�����
	PIMAGE_DATA_DIRECTORY	pDataDir;			// ����Ŀ¼ָ��
	PIMAGE_SECTION_HEADER	pSectionHeader;		// ���α�ͷ��ָ��
	IMAGE_DATA_DIRECTORY	stcPERelocDir;		// �ض�λ����Ϣ
	IMAGE_DATA_DIRECTORY	stcPEImportDir;		// �������Ϣ
	IMAGE_DATA_DIRECTORY	stcPEResDir;		// ��Դ����Ϣ
	IMAGE_DATA_DIRECTORY	stcPETlsDir;		// tls����Ϣ
	IMAGE_DATA_DIRECTORY	stcIATDir;			// IAT��Ϣ
	DWORD					dwNewResAddr;		// �ع�res��ĵ�ַ���ļ�ƫ�ƣ���ReBuildRes���������
	DWORD					dwNewResSize;		// ת�ƺ����Դͷ+�ؼ����ݵĴ�С == ����Դ���ε���ʵ�ߴ�
	DWORD					dwResHeaderSize;	// ��Դͷ�Ĵ�С
	DWORD					dwTlsOffset;		// tls�������������εľ��루RVA��
	DWORD					dwTlsSectionRVA;	// tls�������ε���ʼRVA
	BOOL					bIsDll;				// �Ƿ�Ϊdll�ļ�
	DWORD					dwDllName;			// ����dll�Ļ����򱣴�dll��name
	LPBYTE					pExportBuf;			// ��Ϊ������ı������ݣ���ΪԤ�����ʹ���������ܣ��Ӷ�Ӱ������shellֲ�����
	PDWORD					pExpAddrOfName;		// �����ŵ�����AddressOfName
	LPBYTE					pOriginalSecTable;	// �����žɵĽڱ�
	LPBYTE					pCompressInfo;		// ����ѹ����Ϣ�Ľṹ������
	BOOL					bTls;				// �Ƿ����tls��
	DWORD					dwTlsModStart;		// tlsģ�����ݵ���ʼVA
	DWORD					dwTlsModEnd;		// tlsģ�����ݵ�ĩVA
	DWORD					dwTlsIndexValue;	// tls����ֵ
	DWORD					dwCodeBase;			// ����λ�ַ
	DWORD					dwCodeSize;			// ����δ�С���ڴ����ȣ�
	DWORD					dwCodeRawSize;		// ����εĴ�С���ļ����ȣ�
	DWORD					dwSizeOfHeader;		// Ŀ������PEͷ��С
	LPBYTE					pNewIATAddr;		// ת��IAT�ĵ�ַ
	DWORD					dwNewIATSize;		// ת��IAT�Ĵ�С
	DWORD					dwOrigIATBase;		// ԭʼIAT���ڶε���ʼRVA
	DWORD					dwOrigIATSize;		// ԭʼIAT���ڶεĳ���
	LPBYTE					pNewRelocAddr;		// ת���ض�λ��ĵ�ַ
	DWORD					dwNewRelocSize;		// ת���ض�λ��Ĵ�С
	DWORD					dwCodeMemCRC32;		// ����ε�CRC32ֵ
}PEInfo , *pPEInfo;



class COperationPE
{
public:
	COperationPE();
	~COperationPE();
	


public:
	DWORD	RVAToOffset(DWORD	dwRVA);
	DWORD	OffsetToRVA(DWORD	dwRawPointer);
	BOOL	GetPEInfo(LPBYTE pFileBuf,DWORD dwFileSize, pPEInfo	pObjectPEInfo);
	LPBYTE	GetExpVarAddr(LPWSTR strVarName);
	LPBYTE	AddSection(LPWSTR strName, DWORD dwSize, DWORD dwCharac, PIMAGE_SECTION_HEADER pNewSection,  PDWORD pSizeOfRaw);
	VOID	RECReloc(DWORD dwLoadImageBase, PIMAGE_SECTION_HEADER	pObjectPeNewSection);
	VOID	SetOEP(DWORD dwOEP);
	VOID	SetDir(DWORD dwType, DWORD dwVirtualAddress, DWORD dwSize);
	VOID	CleanDir();
	VOID	FindSectionOfIAT(PDWORD dwIATBase, PDWORD  dwSize);
	VOID	ReBuildRes(pPEInfo pObjectPE);
	DWORD	FixRes(COperationPE* pObjectPE, pPEInfo pObjectPEInfo);
	BOOL	FindSectionOfTls(pPEInfo pObjectPEInfo, PDWORD dwTlsSectionStartRVA, PDWORD dwSectionRealSize);
	BOOL	ChangeModuleData(DWORD dwStartAddr, DWORD dwEndAddr, DWORD dwIndexValue);
	VOID	ReturnTlsModuleAddress(PDWORD dwStartAddr, PDWORD dwEndAddr, PDWORD dwIndexValue);
	VOID	GetExportBuf( pPEInfo pPeInfo);
	BOOL	RelocExportTable(DWORD dwNewExportRVA, PIMAGE_EXPORT_DIRECTORY	pNewExp);
	VOID	CompressSection(pPEInfo pObjectPE, PSelectionInfo pSelect);
	DWORD	AlignSize(DWORD dwSize, DWORD dwAlign);
	DWORD	MoveImportTable(DWORD dwNewAddr);
	BOOL	CleanImportTable();
	DWORD	MoveRelocTable(DWORD dwNewAddr);
	BOOL	CleanRelocTable();
	VOID	CalAndSaveCRC(DWORD dwFileSize);
	VOID	CalMemCRC(LPBYTE pCodeBase, DWORD dwSize, pPEInfo pObjectPE);
	DWORD	GetFileAddr();
	DWORD	GetFileRawSize();
private:
	DWORD	CalcuCRC(UCHAR *string, uint32_t size);
	VOID	MakeCRC32Table();
	LPBYTE	CompressDataOfAplib(LPBYTE pData, DWORD dwSize, PDWORD dwPackedSize);
	LPBYTE	ComressDataOfJCALG1(LPBYTE pData, DWORD dwSize, PDWORD dwPackedSize);
	DWORD	FindResourceHeader(LPBYTE pResHeaderAddr ,LPBYTE pResAddr,  DWORD	dwMinRVA);
	VOID	MoveObjectRes(LPBYTE pResAddr, DWORD dwType, LPBYTE pDataBuf, PDWORD dwBufSize);
	VOID	FixResDataEntry(LPBYTE	pNewResAddr, DWORD dwType,  DWORD dwCurrentRVA, DWORD dwDataOffset, PDWORD	dwReturnedDataSize);
private:
	DWORD				 m_dwFileDataAddr;	// Ŀ���ļ����ڻ������ĵ�ַ
	DWORD				 m_dwFileDataSize;	// Ŀ���ļ���С
	PIMAGE_DOS_HEADER	 m_pDosHeader;		// DOSͷָ��
	PIMAGE_NT_HEADERS	 m_pNtHeader;		// NTͷָ��
	PEInfo				 m_stcPeInfo;		// PE�ؼ���Ϣ
	BOOL				 m_bCRC32Table;		// �Ƿ�������CRC32���
	uint32_t			 crc32_table[256];	// CRC32������
};




