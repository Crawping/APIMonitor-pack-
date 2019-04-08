#include "stdafx.h"
#include "Algorithm.h"



uint32_t			 crc32_table[256];	// CRC32������
BOOL				 g_Crc32Table = FALSE;//�Ƿ�������CRC32���




//////////////////////////////////////////////////////////////////////////
//	KMPƥ���㷨���ӿ�
//	������
//	char *		s		- ���������ݿ�ָ��
//	int			sLength	- ���ݿ��С
//	char *		p		- ƥ��������ָ��
//	int			pLength	- ��������С
//	int *		prefix	- ǰ׺ָ��
//	���ҵ���һ��ƥ�䴮�������̷���TRUE
//////////////////////////////////////////////////////////////////////////
bool kmpMatch(char * s, int sLength, char * p, int pLength, int *prefix)
{
	DWORD	dwOffset;
	int pPoint = 0;
	for (int i = 0; i <= sLength - pLength; i++)
	{


		while (pPoint != 0 && (s[i] != p[pPoint]))
		{
			pPoint = prefix[pPoint - 1];
		}
		if (s[i] == p[pPoint])
		{
			pPoint++;
			if (pPoint == pLength)
			{
				dwOffset = i - pPoint + 1;
				if ((s + dwOffset) != p)
				{
					//printf("�ҵ���ȷ��ƥ��ֵ Base: 0x%08X ƥ�䴮: 0x%08X \n", dwOffset + s, p);

					return true;
				}

				//pPoint = 0;//��һ����sƥ����ַ���,���ܳ�Ϊ��һ��ƥ���ַ�����һ����  
				pPoint = prefix[pPoint - 1];//��һ����sƥ����ַ���,Ҳ�ܳ�Ϊ��һ��ƥ���ַ�����һ����  
			}
		}
	}
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//	��ȡǰ׺
//	������
//	char *		p		- ������ָ��
//	int			length	- ����������
//	int *		prefix	- ǰ׺ָ��
//////////////////////////////////////////////////////////////////////////


void kmpPrefixFunction(char *p, int length, int *prefix)
{
	prefix[0] = 0;
	int k = 0;//ǰ׺�ĳ���  
	for (int i = 1; i < length; i++)
	{
		while (k > 0 && p[k] != p[i])
		{
			k = prefix[k - 1];
		}
		if (p[k] == p[i])//˵��p[0...k-1]��k����ƥ����  
		{
			k = k + 1;
		}
		prefix[i] = k;
	}
}





//////////////////////////////////////////////////////////////////////////
//	����CRC32��񣬼�����������CRC32ֵ
//	������
//	UCHAR *				string		- ������ָ��
//	uint32_t			size		- ��������С
//	����ֵ�� CRC32
//////////////////////////////////////////////////////////////////////////

DWORD	CalcuCRC(UCHAR *string, uint32_t size)
{
	//1. ����CRC32 ���

	if (g_Crc32Table == FALSE)	MakeCRC32Table();




	//2. ����CRC32ֵ
	uint32_t crc = 0xFFFFFFFF;

	while (size--)
		crc = (crc >> 8) ^ (crc32_table[(crc ^ *string++) & 0xff]);


	return crc;
}




///////////////////////////////////////////////////////////////////////////
//	����CRC32���
//////////////////////////////////////////////////////////////////////////


VOID	MakeCRC32Table()
{
	uint32_t c;
	int i = 0;
	int bit = 0;

	for (i = 0; i < 256; i++)
	{
		c = (uint32_t)i;

		for (bit = 0; bit < 8; bit++)
		{
			if (c & 1)
			{
				c = (c >> 1) ^ (0xEDB88320);
			}
			else
			{
				c = c >> 1;
			}

		}
		crc32_table[i] = c;
	}

	g_Crc32Table = TRUE;

}