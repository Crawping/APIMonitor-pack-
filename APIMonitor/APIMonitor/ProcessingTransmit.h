#pragma once

#include <windows.h>

#define			MAX_BUF_SIZE				1024		//	��Ϣ���յĻ�������С
#define			MAX_NAME_LEN                50			//	API or MOD ���ֳ���
#define			MOD_MSG						0x1			//	ģ����Ϣ����
#define			API_MSG						0x2			//	API��Ϣ����
#define			TRAP_MSG					0x3			//	API��������
#define			FINISH_MODMSG				0x4			//	����Ϣ���ͱ�ʾ��ģ�����ݺ�API�����Ѿ�������ϣ�UI���Խ������ɹ���
#define			HOOK_MSG					0x5			//	��ض˶�ָ��ģ�鷢��HOOKָ��
#define			FINISH_APIMSG				0x6			//	ָ��ģ���API�����Ѿ�������ϣ�����UI��ʾ	
#define			FILTER_MSG					0x7			//	����ָ����API
#define			SIGNATURE_LOG				0x00005150

#define			FILE_DIR					L"HookRecord"


#define			LIST_INC					0x50		//	��������

#pragma   pack(1) 

//���ݰ���װ�ṹ
typedef	struct _PacketInfo
{
	DWORD	dwType;						//���ͣ����궨��x_MSG
	DWORD	dwLegth;					//���ݳ���
	BYTE	Data[1];					//���ݻ�����
}PacketInfo, *lpPacketInfo;

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
	WCHAR	szApiName[MAX_NAME_LEN];	
	BOOL	bActive;					//	��API�Ƿ񱻼���
}PacketApiInfo, *lpPacketApiInfo;



//Api���������ݰ��ṹ��
typedef struct _TrapInfo
{
	DWORD	dwRetAddr;						// Api�ķ��ص�ַ
	DWORD	dwModIndex;
	DWORD	dwApiIndex;
	//����ѡ�������
	DWORD	dwLength;					//�����ܳ���
	BYTE	byPara[1];					//������,��ȡҪע��˳��
}TrapInfo, *lpTrapInfo;


//����API�Ľṹ��
typedef struct  _FilteredInfo
{
	DWORD	dwModIndex;
	DWORD	dwApiIndex;
	BOOL	bFilterd;

}FilteredInfo, *lpFilteredInfo;


//��Ĳ����ṹ��
typedef struct _ModList
{
	DWORD				dwBaseAddr;
	DWORD				dwModIndex;
	DWORD				dwImageSize;
	WCHAR				szModName[MAX_NAME_LEN];
	BOOL				bActive;						//�����������ģ���Ƿ񱻼���,Ĭ��ΪFALSE��Ŀ�ĸ�UI���ж��Ƿ����ʾ
	DWORD				dwApiListLen;					//������
	lpPacketApiInfo		lpApiList;						//����API�������Ϣ
}ModList, *lpModList;



//Mod Tree Ctl�ؼ���ʹ�õ����ݰ��ṹ
typedef struct  _HookMod
{
	DWORD	dwModIndex;
	BOOL	bHook;
}HookMod , *lpHookMod;



// д��LogBunary�����������ļ��еĽṹ��
typedef struct _LogBinary
{
	CHAR	szApiName[MAX_NAME_LEN];	//����ģ����õĺ�����
	DWORD	dwCount;					//���ô���
}LogBinary, *lpLogBinary;





class ProcessingList
{
public:
	ProcessingList();
	~ProcessingList();

	BOOL	AddModInfo(lpPacketModInfo	lpData);
	BOOL	AddApiInfo(lpPacketApiInfo	lpData);
	DWORD	GetMsgInfo(lpPacketInfo		lpData);
	VOID	SendHookMod(HANDLE hPipe, WCHAR* szModName,BOOL bHook);
	VOID	SendFilteredApi(HANDLE hPipe, DWORD dwModIndex, DWORD dwApiIndex, BOOL bFiltered);
	WCHAR*	GetInvokedModName(DWORD dwInvokedAddr);
	WCHAR*	GetApiName(DWORD dwModIndex, DWORD dwApiIndex);
	VOID	InsertOfBinary(CHAR* szApiName);
	DWORD	GetLogInfo(DWORD* pdSize);
	

public:
	lpModList		m_pModList;
	DWORD			m_dwModListLen;		//ģ������ĳ���
	lpLogBinary		m_pLog;				//�����������ļ�������ָ��
	DWORD			m_dwLogListLen;
};

