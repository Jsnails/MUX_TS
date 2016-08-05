#pragma once

#include "Information.h"

#define INPUTH264FILENAME     "a.h264"
#define INPUTAACFILENAME      "b.aac"
#define OUTPUTTSFILENAME      "h264_aac.ts"


extern FILE * pVideo_H264_File;
extern FILE * pAudio_Aac_File;
extern FILE * pVideo_Audio_Ts_File;

FILE * OpenFile(char * FileName,char * OpenMode);                        //打开文件
void   CloseFile(FILE * pFile);                                          //关闭文件
int    ReadFile(FILE * pFile ,unsigned char * Buffer,int BufferSize);    //读取操作
int    WriteFile(FILE * pFile ,char * Buffer,int BufferSize);            //文件写操作
