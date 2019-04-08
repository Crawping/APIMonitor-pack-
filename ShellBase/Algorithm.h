#pragma once

#include <windows.h>
#include <stdint.h>  



bool	kmpMatch(char * s, int sLength, char * p, int pLength, int *prefix);
void	kmpPrefixFunction(char *p, int length, int *prefix);
VOID	MakeCRC32Table();											//����CRC32���
DWORD	CalcuCRC(UCHAR *string, uint32_t size);						//����CRC32ֵ