#include "Audio.h"

unsigned int decode_audio_done = 0;

int Detach_Head_Aac(ADTS_HEADER * adtsheader,unsigned char *Adts_Headr_Buf)
{
	unsigned int readsize = 0;
	readsize = ReadFile(pAudio_Aac_File ,Adts_Headr_Buf,ADTS_HEADER_LENGTH);
	if (readsize < 0)
	{
		printf("ReadFile : pAudio_Aac_File ERROR\n");
		return getchar();
	}

	if (readsize == 0)
	{
		return readsize;
	}

	if ((Adts_Headr_Buf[0] == 0xFF)&&((Adts_Headr_Buf[1] & 0xF0) == 0xF0))    //syncword 12个1
	{
		adtsheader->syncword = (Adts_Headr_Buf[0] << 4 )  | (Adts_Headr_Buf[1]  >> 4);
		adtsheader->id = ((unsigned int) Adts_Headr_Buf[1] & 0x08) >> 3;
		adtsheader->layer = ((unsigned int) Adts_Headr_Buf[1] & 0x06) >> 1;
		adtsheader->protection_absent = (unsigned int) Adts_Headr_Buf[1] & 0x01;
		adtsheader->profile = ((unsigned int) Adts_Headr_Buf[2] & 0xc0) >> 6;
		adtsheader->sf_index = ((unsigned int) Adts_Headr_Buf[2] & 0x3c) >> 2;
		adtsheader->private_bit = ((unsigned int) Adts_Headr_Buf[2] & 0x02) >> 1;
		adtsheader->channel_configuration = ((((unsigned int) Adts_Headr_Buf[2] & 0x01) << 2) | (((unsigned int) Adts_Headr_Buf[3] & 0xc0) >> 6));
		adtsheader->original = ((unsigned int) Adts_Headr_Buf[3] & 0x20) >> 5;
		adtsheader->home = ((unsigned int) Adts_Headr_Buf[3] & 0x10) >> 4;
		adtsheader->copyright_identification_bit = ((unsigned int) Adts_Headr_Buf[3] & 0x08) >> 3;
		adtsheader->copyright_identification_start = (unsigned int) Adts_Headr_Buf[3] & 0x04 >> 2;		
		adtsheader->aac_frame_length = (((((unsigned int) Adts_Headr_Buf[3]) & 0x03) << 11) | (((unsigned int) Adts_Headr_Buf[4] & 0xFF) << 3)| ((unsigned int) Adts_Headr_Buf[5] & 0xE0) >> 5) ;
		adtsheader->adts_buffer_fullness = (((unsigned int) Adts_Headr_Buf[5] & 0x1f) << 6 | ((unsigned int) Adts_Headr_Buf[6] & 0xfc) >> 2);
		adtsheader->no_raw_data_blocks_in_frame = ((unsigned int) Adts_Headr_Buf[6] & 0x03);
	}
	else 
	{
		printf("ADTS_HEADER : BUF ERROR\n");
		getchar();
	}
	return readsize;
}

int Read_One_Aac_Frame(unsigned char * buf)
{
	ADTS_HEADER  adts_header ;
	unsigned int readsize = 0;

	//读取ADTS头
	if (!Detach_Head_Aac(&adts_header,buf))
	{
		decode_audio_done = 1;
		return 0;
	}
	//将data填入bufz中
	readsize = ReadFile(pAudio_Aac_File ,buf + ADTS_HEADER_LENGTH ,adts_header.aac_frame_length - ADTS_HEADER_LENGTH);
	if (readsize != adts_header.aac_frame_length - ADTS_HEADER_LENGTH)
	{
		printf("READ ADTS_DATA : BUF LENGTH ERROR\n");
		return -1;
	}
	return adts_header.aac_frame_length;
}

int AAC2PES(TsPes * tsaacpes,unsigned long Adudiopts)
{
	unsigned int aacpes_pos = 0;
	unsigned int OneFrameLen_AAC = 0;

	//读取出一帧数据
	OneFrameLen_AAC = Read_One_Aac_Frame(tsaacpes->Es);
	aacpes_pos += OneFrameLen_AAC ;

	tsaacpes->packet_start_code_prefix = 0x000001;
	tsaacpes->stream_id = TS_AAC_STREAM_ID;                                //E0~EF表示是视频的,C0~DF是音频,H264-- E0
	tsaacpes->PES_packet_length = 0 ; // OneFrameLen_AAC + 8 ;             //一帧数据的长度 不包含 PES包头 ,8自适应段的长度
	tsaacpes->Pes_Packet_Length_Beyond = OneFrameLen_AAC;                  //= OneFrameLen_aac;     //这里读错了一帧  
	if (OneFrameLen_AAC > 0xFFFF)                                          //如果一帧数据的大小超出界限
	{
		tsaacpes->PES_packet_length = 0x00;
		tsaacpes->Pes_Packet_Length_Beyond = OneFrameLen_AAC;  
		aacpes_pos += 16;
	}
	else
	{
		tsaacpes->PES_packet_length = 0x00;
		tsaacpes->Pes_Packet_Length_Beyond = OneFrameLen_AAC;  
		aacpes_pos += 14;
	}
	tsaacpes->marker_bit = 0x02;
	tsaacpes->PES_scrambling_control = 0x00;                               //人选字段 存在，不加扰
	tsaacpes->PES_priority = 0x00;
	tsaacpes->data_alignment_indicator = 0x00;
	tsaacpes->copyright = 0x00;
	tsaacpes->original_or_copy = 0x00;
	tsaacpes->PTS_DTS_flags = 0x02;                                        //10'：PTS字段存在,DTS不存在
	tsaacpes->ESCR_flag = 0x00;
	tsaacpes->ES_rate_flag = 0x00;
	tsaacpes->DSM_trick_mode_flag = 0x00;
	tsaacpes->additional_copy_info_flag = 0x00;
	tsaacpes->PES_CRC_flag = 0x00;
	tsaacpes->PES_extension_flag = 0x00;
	tsaacpes->PES_header_data_length = 0x05;                               //后面的数据 包括了PTS所占的字节数

	//清 0 
	tsaacpes->tsptsdts.pts_32_30  = 0;
	tsaacpes->tsptsdts.pts_29_15 = 0;
	tsaacpes->tsptsdts.pts_14_0 = 0;

	tsaacpes->tsptsdts.reserved_1 = 0x03;                                 //填写 pts信息
	// Adudiopts大于30bit，使用最高三位 
	if(Adudiopts > 0x7FFFFFFF)
	{
		tsaacpes->tsptsdts.pts_32_30 = (Adudiopts >> 30) & 0x07;                 
		tsaacpes->tsptsdts.marker_bit1 = 0x01;
	}
	else 
	{
		tsaacpes->tsptsdts.marker_bit1 = 0;
	}
	// Videopts大于15bit，使用更多的位来存储
	if(Adudiopts > 0x7FFF)
	{
		tsaacpes->tsptsdts.pts_29_15 = (Adudiopts >> 15) & 0x007FFF ;
		tsaacpes->tsptsdts.marker_bit2 = 0x01;
	}
	else
	{
		tsaacpes->tsptsdts.marker_bit2 = 0;
	}
	//使用最后15位
	tsaacpes->tsptsdts.pts_14_0 = Adudiopts & 0x007FFF;
	tsaacpes->tsptsdts.marker_bit3 = 0x01;

	return aacpes_pos;
}

int AAC2PES(TsPes * tsaacpes, unsigned long Adudiopts, unsigned char *pData, int iDataSize)
{
	unsigned int aacpes_pos = 0;
	unsigned int OneFrameLen_AAC = 0;

	//读取出一帧数据
	OneFrameLen_AAC = iDataSize; //Read_One_Aac_Frame(tsaacpes->Es);
	memcpy(tsaacpes->Es, pData, iDataSize);

	aacpes_pos += OneFrameLen_AAC;

	tsaacpes->packet_start_code_prefix = 0x000001;
	tsaacpes->stream_id = TS_AAC_STREAM_ID;                                //E0~EF表示是视频的,C0~DF是音频,H264-- E0
	tsaacpes->PES_packet_length = 0; // OneFrameLen_AAC + 8 ;             //一帧数据的长度 不包含 PES包头 ,8自适应段的长度
	tsaacpes->Pes_Packet_Length_Beyond = OneFrameLen_AAC;                  //= OneFrameLen_aac;     //这里读错了一帧  
	if (OneFrameLen_AAC > 0xFFFF)                                          //如果一帧数据的大小超出界限
	{
		tsaacpes->PES_packet_length = 0x00;
		tsaacpes->Pes_Packet_Length_Beyond = OneFrameLen_AAC;
		aacpes_pos += 16;
	}
	else
	{
		tsaacpes->PES_packet_length = 0x00;
		tsaacpes->Pes_Packet_Length_Beyond = OneFrameLen_AAC;
		aacpes_pos += 14;
	}
	tsaacpes->marker_bit = 0x02;
	tsaacpes->PES_scrambling_control = 0x00;                               //人选字段 存在，不加扰
	tsaacpes->PES_priority = 0x00;
	tsaacpes->data_alignment_indicator = 0x00;
	tsaacpes->copyright = 0x00;
	tsaacpes->original_or_copy = 0x00;
	tsaacpes->PTS_DTS_flags = 0x02;                                        //10'：PTS字段存在,DTS不存在
	tsaacpes->ESCR_flag = 0x00;
	tsaacpes->ES_rate_flag = 0x00;
	tsaacpes->DSM_trick_mode_flag = 0x00;
	tsaacpes->additional_copy_info_flag = 0x00;
	tsaacpes->PES_CRC_flag = 0x00;
	tsaacpes->PES_extension_flag = 0x00;
	tsaacpes->PES_header_data_length = 0x05;                               //后面的数据 包括了PTS所占的字节数

	//清 0 
	tsaacpes->tsptsdts.pts_32_30 = 0;
	tsaacpes->tsptsdts.pts_29_15 = 0;
	tsaacpes->tsptsdts.pts_14_0 = 0;

	tsaacpes->tsptsdts.reserved_1 = 0x03;                                 //填写 pts信息
	// Adudiopts大于30bit，使用最高三位 
	if (Adudiopts > 0x7FFFFFFF)
	{
		tsaacpes->tsptsdts.pts_32_30 = (Adudiopts >> 30) & 0x07;
		tsaacpes->tsptsdts.marker_bit1 = 0x01;
	}
	else
	{
		tsaacpes->tsptsdts.marker_bit1 = 0;
	}
	// Videopts大于15bit，使用更多的位来存储
	if (Adudiopts > 0x7FFF)
	{
		tsaacpes->tsptsdts.pts_29_15 = (Adudiopts >> 15) & 0x007FFF;
		tsaacpes->tsptsdts.marker_bit2 = 0x01;
	}
	else
	{
		tsaacpes->tsptsdts.marker_bit2 = 0;
	}
	//使用最后15位
	tsaacpes->tsptsdts.pts_14_0 = Adudiopts & 0x007FFF;
	tsaacpes->tsptsdts.marker_bit3 = 0x01;

	return aacpes_pos;
}
