#pragma once

#include "Video.h"
#include "Audio.h"

extern unsigned char m_One_Frame_Buf[MAX_ONE_FRAME_SIZE];
extern TsPes m_video_tspes;
extern TsPes m_audio_tspes;
extern unsigned int WritePacketNum;               //一共的包数目


int Write_Pat(unsigned char * buf);
int Write_Pmt(unsigned char * buf);
int Take_Out_Pes(TsPes * tspes, unsigned long time_pts, unsigned int frametype, unsigned int *videoframetype); //0 视频 ，1音频
int Take_Out_Pes(TsPes * tspes, unsigned long time_pts, unsigned int frametype, unsigned int *videoframetype,unsigned char *pData,int iDataSize); //0 视频 ，1音频

int WriteAdaptive_flags_Head(Ts_Adaptation_field  * ts_adaptation_field,unsigned int Videopts);              //填写自适应段标志帧头的
int WriteAdaptive_flags_Tail(Ts_Adaptation_field  * ts_adaptation_field);                                    //填写自适应段标志帧尾的

int WriteBuf2File(unsigned int framerate);
int WriteBuf2TsFile(unsigned int framerate,int iStreamType,unsigned char *pData,int iDataSize,unsigned long lTimeStamp);

int PES2TS(TsPes * ts_pes,unsigned int Video_Audio_PID ,Ts_Adaptation_field * ts_adaptation_field_Head ,Ts_Adaptation_field * ts_adaptation_field_Tail,
		   unsigned long  Videopts,unsigned long Adudiopts);

int CreateAdaptive_Ts(Ts_Adaptation_field * ts_adaptation_field,unsigned char * buf,unsigned int AdaptiveLength);

int WriteH264Buff2File(unsigned int framerate);