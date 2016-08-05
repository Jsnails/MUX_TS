#include "Mycrc_32.h"

unsigned long CalculateCRC_32( unsigned char * aData, unsigned long aSize )
{
	unsigned long crc32 = 0;
	unsigned long tabitem;

	while(aSize--)
	{
		tabitem=( crc32 >> 24 )^ *aData++;
		crc32 = ( crc32 << 8 ) ^ crctable[tabitem];
	}
	return crc32;
}
