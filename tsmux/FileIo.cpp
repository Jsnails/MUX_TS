#include "FileIo.h"

FILE * pVideo_H264_File = NULL;
FILE * pAudio_Aac_File = NULL;
FILE * pVideo_Audio_Ts_File = NULL;


FILE * OpenFile(char * FileName,char * OpenMode)
{
	FILE * pFile = NULL;
	pFile = fopen(FileName, OpenMode);
	if (NULL != pFile)
	{
		fclose(pFile);
		fopen(FileName, OpenMode);//文件清空
		fclose(pFile);
		pFile = NULL;
	}
	else
	{
		printf("打开输入输出文件失败!\n");
	}
	pFile = fopen(FileName, OpenMode);
	return pFile;
}

void  CloseFile(FILE * pFile)
{
	fclose(pFile);
}

int ReadFile(FILE * pFile ,unsigned char * Buffer,int BufferSize)
{
	return fread(Buffer,1,BufferSize,pFile);
}

int WriteFile(FILE * pFile ,char * Buffer,int BufferSize)
{
	int WriteSize = 0;

    WriteSize = fwrite(Buffer, 1, BufferSize, pFile);
	return WriteSize;
}
