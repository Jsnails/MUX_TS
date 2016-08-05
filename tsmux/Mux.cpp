#include "Mux.h"

unsigned char m_One_Frame_Buf[MAX_ONE_FRAME_SIZE];
TsPes m_video_tspes;
TsPes m_audio_tspes;
unsigned int WritePacketNum = 0;
unsigned long  Timestamp_video = 0;    //һ֡��Ƶ����ʱ��
unsigned long  Timestamp_audio = 0;    //һ֡��Ƶ����ʱ�� 

int Write_Pat(unsigned char * buf)
{
	WriteStruct_Pat(buf);
	return WriteFile(pVideo_Audio_Ts_File,(char *)buf,TS_PACKET_SIZE);
}

int Write_Pmt(unsigned char * buf)
{
	WriteStruct_Pmt(buf);
	return WriteFile(pVideo_Audio_Ts_File,(char *)buf,TS_PACKET_SIZE);
}

int Take_Out_Pes(TsPes * tspes ,unsigned long time_pts,unsigned int frametype,unsigned int * videoframetype)
{
	unsigned int pes_pos = 0;
	if (frametype == 0x00) //��Ƶ
	{
		pes_pos = H2642PES(tspes,time_pts,videoframetype);
	}
	else                   //��Ƶ
	{
		pes_pos = AAC2PES(tspes,time_pts);  
	}
	return pes_pos;
}

int Take_Out_Pes(TsPes * tspes, unsigned long time_pts, unsigned int frametype, unsigned int *videoframetype, unsigned char *pData, int iDataSize)
{
	unsigned int pes_pos = 0;
	if (frametype == 0x00) //��Ƶ
	{
		pes_pos = H2642PES(tspes, time_pts, videoframetype,pData,iDataSize);
	}
	else                   //��Ƶ
	{
		pes_pos = AAC2PES(tspes, time_pts,pData,iDataSize);
	}
	return pes_pos;
}

int WriteAdaptive_flags_Head(Ts_Adaptation_field  * ts_adaptation_field,unsigned int Videopts)
{
	//��д����Ӧ��
	ts_adaptation_field->discontinuty_indicator = 0;
	ts_adaptation_field->random_access_indicator = 0;
	ts_adaptation_field->elementary_stream_priority_indicator = 0;
	ts_adaptation_field->PCR_flag = 1;                                          //ֻ�õ����
	ts_adaptation_field->OPCR_flag = 0;
	ts_adaptation_field->splicing_point_flag = 0;
	ts_adaptation_field->transport_private_data_flag = 0;
	ts_adaptation_field->adaptation_field_extension_flag = 0;

	//��Ҫ�Լ���
	ts_adaptation_field->pcr  = Videopts * 300;
	ts_adaptation_field->adaptation_field_length = 7;                          //ռ��7λ

	ts_adaptation_field->opcr = 0;
	ts_adaptation_field->splice_countdown = 0;
	ts_adaptation_field->private_data_len = 0;
	return 1;
}

int WriteAdaptive_flags_Tail(Ts_Adaptation_field  * ts_adaptation_field)
{
	//��д����Ӧ��
	ts_adaptation_field->discontinuty_indicator = 0;
	ts_adaptation_field->random_access_indicator = 0;
	ts_adaptation_field->elementary_stream_priority_indicator = 0;
	ts_adaptation_field->PCR_flag = 0;                                          //ֻ�õ����
	ts_adaptation_field->OPCR_flag = 0;
	ts_adaptation_field->splicing_point_flag = 0;
	ts_adaptation_field->transport_private_data_flag = 0;
	ts_adaptation_field->adaptation_field_extension_flag = 0;

	//��Ҫ�Լ���
	ts_adaptation_field->pcr  = 0;
	ts_adaptation_field->adaptation_field_length = 1;                          //ռ��1λ��־���õ�λ

	ts_adaptation_field->opcr = 0;
	ts_adaptation_field->splice_countdown = 0;
	ts_adaptation_field->private_data_len = 0;                    
	return 1;
}

int CreateAdaptive_Ts(Ts_Adaptation_field * ts_adaptation_field,unsigned char * buf,unsigned int AdaptiveLength)
{
	unsigned int CurrentAdaptiveLength = 1;                                 //��ǰ�Ѿ��õ�����Ӧ�γ���  
	unsigned char Adaptiveflags = 0;                                        //����Ӧ�εı�־
	unsigned int adaptive_pos = 0;

	//��д����Ӧ�ֶ�
	if (ts_adaptation_field->adaptation_field_length > 0)
	{
		adaptive_pos += 1;                                                  //����Ӧ�ε�һЩ��־��ռ�õ�1���ֽ�
		CurrentAdaptiveLength += 1;

		if (ts_adaptation_field->discontinuty_indicator)
		{
			Adaptiveflags |= 0x80;
		}
		if (ts_adaptation_field->random_access_indicator)
		{
			Adaptiveflags |= 0x40;
		}
		if (ts_adaptation_field->elementary_stream_priority_indicator)
		{
			Adaptiveflags |= 0x20;
		}
		if (ts_adaptation_field->PCR_flag)
		{
			unsigned long long pcr_base;
			unsigned int pcr_ext;

			pcr_base = (ts_adaptation_field->pcr / 300);
			pcr_ext = (ts_adaptation_field->pcr % 300);

			Adaptiveflags |= 0x10;

			buf[adaptive_pos + 0] = (pcr_base >> 25) & 0xff;
			buf[adaptive_pos + 1] = (pcr_base >> 17) & 0xff;
			buf[adaptive_pos + 2] = (pcr_base >> 9) & 0xff;
			buf[adaptive_pos + 3] = (pcr_base >> 1) & 0xff;
			buf[adaptive_pos + 4] = pcr_base << 7 | pcr_ext >> 8 | 0x7e;
			buf[adaptive_pos + 5] = (pcr_ext) & 0xff;
			adaptive_pos += 6;

			CurrentAdaptiveLength += 6;
		}
		if (ts_adaptation_field->OPCR_flag)
		{
			unsigned long long opcr_base;
			unsigned int opcr_ext;

			opcr_base = (ts_adaptation_field->opcr / 300);
			opcr_ext = (ts_adaptation_field->opcr % 300);

			Adaptiveflags |= 0x08;

			buf[adaptive_pos + 0] = (opcr_base >> 25) & 0xff;
			buf[adaptive_pos + 1] = (opcr_base >> 17) & 0xff;
			buf[adaptive_pos + 2] = (opcr_base >> 9) & 0xff;
			buf[adaptive_pos + 3] = (opcr_base >> 1) & 0xff;
			buf[adaptive_pos + 4] = ((opcr_base << 7) & 0x80) | ((opcr_ext >> 8) & 0x01);
			buf[adaptive_pos + 5] = (opcr_ext) & 0xff;
			adaptive_pos += 6;
			CurrentAdaptiveLength += 6;
		}
		if (ts_adaptation_field->splicing_point_flag)
		{
			buf[adaptive_pos] = ts_adaptation_field->splice_countdown;

			Adaptiveflags |= 0x04;

			adaptive_pos += 1;
			CurrentAdaptiveLength += 1;
		}
		if (ts_adaptation_field->private_data_len > 0)
		{
			Adaptiveflags |= 0x02;
			if (1+ ts_adaptation_field->private_data_len > AdaptiveLength - CurrentAdaptiveLength)
			{
				printf("private_data_len error !\n");
				return getchar();
			}
			else
			{
				buf[adaptive_pos] = ts_adaptation_field->private_data_len;
				adaptive_pos += 1;
				memcpy (buf + adaptive_pos, ts_adaptation_field->private_data, ts_adaptation_field->private_data_len);
				adaptive_pos += ts_adaptation_field->private_data_len;

				CurrentAdaptiveLength += (1 + ts_adaptation_field->private_data_len) ;
			}
		}
		if (ts_adaptation_field->adaptation_field_extension_flag)
		{
			Adaptiveflags |= 0x01;
			buf[adaptive_pos + 1] = 1;
			buf[adaptive_pos + 2] = 0;
			CurrentAdaptiveLength += 2;
		}
		buf[0] = Adaptiveflags;                        //����־�����ڴ�
	}
	return 1;
}

int PES2TS(TsPes * ts_pes,unsigned int Video_Audio_PID ,Ts_Adaptation_field * ts_adaptation_field_Head ,Ts_Adaptation_field * ts_adaptation_field_Tail,
		   unsigned long  Videopts,unsigned long Adudiopts)
{
	TsPacketHeader ts_header;
	unsigned int ts_pos = 0;
	unsigned int FirstPacketLoadLength = 0 ;                                   //��Ƭ���ĵ�һ�����ĸ��س���
	unsigned int NeafPacketCount = 0;                                          //��Ƭ���ĸ���
	unsigned int AdaptiveLength = 0;                                           //Ҫ��д0XFF�ĳ���
	unsigned char * NeafBuf = NULL;                                            //��Ƭ�� �ܸ��ص�ָ��
	unsigned char TSbuf[TS_PACKET_SIZE];

	memset(TSbuf,0,TS_PACKET_SIZE); 
	FirstPacketLoadLength = 188 - 4 - 1 - ts_adaptation_field_Head->adaptation_field_length - 14; //�����Ƭ���ĵ�һ�����ĸ��س���
	NeafPacketCount += 1;                                                                   //��һ����Ƭ��  

	//һ���������
	if (ts_pes->Pes_Packet_Length_Beyond < FirstPacketLoadLength)                           //������ sps ��pps ��sei��
	{
		memset(TSbuf,0xFF,TS_PACKET_SIZE);
		WriteStruct_Packetheader(TSbuf,Video_Audio_PID,0x01,0x03);                          //PID = TS_H264_PID,��Ч���ص�Ԫ��ʼָʾ��_play_init = 0x01, ada_field_C,0x03,���е����ֶκ���Ч���� ��
		ts_pos += 4;
		TSbuf[ts_pos + 0] = 184 - ts_pes->Pes_Packet_Length_Beyond - 9 - 5 - 1 ;
		TSbuf[ts_pos + 1] = 0x00;
		ts_pos += 2; 
		memset(TSbuf + ts_pos,0xFF,(184 - ts_pes->Pes_Packet_Length_Beyond - 9 - 5 - 2));
		ts_pos += (184 - ts_pes->Pes_Packet_Length_Beyond - 9 - 5 - 2);

		TSbuf[ts_pos + 0] = (ts_pes->packet_start_code_prefix >> 16) & 0xFF;
		TSbuf[ts_pos + 1] = (ts_pes->packet_start_code_prefix >> 8) & 0xFF; 
		TSbuf[ts_pos + 2] = ts_pes->packet_start_code_prefix & 0xFF;
		TSbuf[ts_pos + 3] = ts_pes->stream_id;
		TSbuf[ts_pos + 4] = ((ts_pes->PES_packet_length) >> 8) & 0xFF;
		TSbuf[ts_pos + 5] = (ts_pes->PES_packet_length) & 0xFF;
		TSbuf[ts_pos + 6] = ts_pes->marker_bit << 6 | ts_pes->PES_scrambling_control << 4 | ts_pes->PES_priority << 3 |
			ts_pes->data_alignment_indicator << 2 | ts_pes->copyright << 1 |ts_pes->original_or_copy;
		TSbuf[ts_pos + 7] = ts_pes->PTS_DTS_flags << 6 |ts_pes->ESCR_flag << 5 | ts_pes->ES_rate_flag << 4 |
			ts_pes->DSM_trick_mode_flag << 3 | ts_pes->additional_copy_info_flag << 2 | ts_pes->PES_CRC_flag << 1 | ts_pes->PES_extension_flag;
		TSbuf[ts_pos + 8] = ts_pes->PES_header_data_length;
		ts_pos += 9;

		if (ts_pes->stream_id == TS_H264_STREAM_ID)
		{
			TSbuf[ts_pos + 0] = (((0x3 << 4) | ((Videopts>> 29) & 0x0E) | 0x01) & 0xff);
			TSbuf[ts_pos + 1]= (((((Videopts >> 14) & 0xfffe) | 0x01) >> 8) & 0xff);
			TSbuf[ts_pos + 2]= ((((Videopts >> 14) & 0xfffe) | 0x01) & 0xff);
			TSbuf[ts_pos + 3]= (((((Videopts << 1) & 0xfffe) | 0x01) >> 8) & 0xff);
			TSbuf[ts_pos + 4]= ((((Videopts << 1) & 0xfffe) | 0x01) & 0xff);
			ts_pos += 5;

		}
		else if (ts_pes->stream_id == TS_AAC_STREAM_ID)
		{
			TSbuf[ts_pos + 0] = (((0x3 << 4) | ((Adudiopts>> 29) & 0x0E) | 0x01) & 0xff);
			TSbuf[ts_pos + 1]= (((((Adudiopts >> 14) & 0xfffe) | 0x01) >> 8) & 0xff);
			TSbuf[ts_pos + 2]= ((((Adudiopts >> 14) & 0xfffe) | 0x01) & 0xff);
			TSbuf[ts_pos + 3]= (((((Adudiopts << 1) & 0xfffe) | 0x01) >> 8) & 0xff);
			TSbuf[ts_pos + 4]= ((((Adudiopts << 1) & 0xfffe) | 0x01) & 0xff);
			ts_pos += 5;
		}
		else
		{
			printf("ts_pes->stream_id  error 0x%x \n",ts_pes->stream_id);
			return getchar();
		}
		memcpy(TSbuf + ts_pos,ts_pes->Es,ts_pes->Pes_Packet_Length_Beyond);  

		//����д���ļ�
		fwrite(TSbuf,188,1,pVideo_Audio_Ts_File);                               //��һ������д���ļ�
		WritePacketNum ++;                                                      //�Ѿ�д���ļ��İ�����++
		return WritePacketNum;
	}
	
	NeafPacketCount += (ts_pes->Pes_Packet_Length_Beyond - FirstPacketLoadLength)/ 184;     
	NeafPacketCount += 1;                                                                   //���һ����Ƭ��
	AdaptiveLength = 188 - 4 - 1 - ((ts_pes->Pes_Packet_Length_Beyond - FirstPacketLoadLength)% 184)  ;  //Ҫ��д0XFF�ĳ���
	if ((WritePacketNum % 40) == 0)                                                         //ÿ40������һ�� pat,һ��pmt
	{
		Write_Pat(m_One_Frame_Buf);                                                         //����PAT
		Write_Pmt(m_One_Frame_Buf);                                                         //����PMT
	}
	//��ʼ�����һ����,��Ƭ���ĸ�������Ҳ�������� 
	WriteStruct_Packetheader(TSbuf,Video_Audio_PID,0x01,0x03);                              //PID = TS_H264_PID,��Ч���ص�Ԫ��ʼָʾ��_play_init = 0x01, ada_field_C,0x03,���е����ֶκ���Ч���� ��
	ts_pos += 4;
	TSbuf[ts_pos] = ts_adaptation_field_Head->adaptation_field_length;                      //����Ӧ�ֶεĳ��ȣ��Լ���д��
	ts_pos += 1;                                                       

	CreateAdaptive_Ts(ts_adaptation_field_Head,TSbuf + ts_pos,(188 - 4 - 1 - 14));          //��д����Ӧ�ֶ�
	ts_pos += ts_adaptation_field_Head->adaptation_field_length;                            //��д����Ӧ������Ҫ�ĳ���

	TSbuf[ts_pos + 0] = (ts_pes->packet_start_code_prefix >> 16) & 0xFF;
	TSbuf[ts_pos + 1] = (ts_pes->packet_start_code_prefix >> 8) & 0xFF; 
	TSbuf[ts_pos + 2] = ts_pes->packet_start_code_prefix & 0xFF;
	TSbuf[ts_pos + 3] = ts_pes->stream_id;
	TSbuf[ts_pos + 4] = ((ts_pes->PES_packet_length) >> 8) & 0xFF;
	TSbuf[ts_pos + 5] = (ts_pes->PES_packet_length) & 0xFF;
	TSbuf[ts_pos + 6] = ts_pes->marker_bit << 6 | ts_pes->PES_scrambling_control << 4 | ts_pes->PES_priority << 3 |
		ts_pes->data_alignment_indicator << 2 | ts_pes->copyright << 1 |ts_pes->original_or_copy;
	TSbuf[ts_pos + 7] = ts_pes->PTS_DTS_flags << 6 |ts_pes->ESCR_flag << 5 | ts_pes->ES_rate_flag << 4 |
		ts_pes->DSM_trick_mode_flag << 3 | ts_pes->additional_copy_info_flag << 2 | ts_pes->PES_CRC_flag << 1 | ts_pes->PES_extension_flag;
	TSbuf[ts_pos + 8] = ts_pes->PES_header_data_length;
	ts_pos += 9;

	if (ts_pes->stream_id == TS_H264_STREAM_ID)
	{
		TSbuf[ts_pos + 0] = (((0x3 << 4) | ((Videopts>> 29) & 0x0E) | 0x01) & 0xff);
		TSbuf[ts_pos + 1]= (((((Videopts >> 14) & 0xfffe) | 0x01) >> 8) & 0xff);
		TSbuf[ts_pos + 2]= ((((Videopts >> 14) & 0xfffe) | 0x01) & 0xff);
		TSbuf[ts_pos + 3]= (((((Videopts << 1) & 0xfffe) | 0x01) >> 8) & 0xff);
		TSbuf[ts_pos + 4]= ((((Videopts << 1) & 0xfffe) | 0x01) & 0xff);
		ts_pos += 5;

	}
	else if (ts_pes->stream_id == TS_AAC_STREAM_ID)
	{
		TSbuf[ts_pos + 0] = (((0x3 << 4) | ((Adudiopts>> 29) & 0x0E) | 0x01) & 0xff);
		TSbuf[ts_pos + 1]= (((((Adudiopts >> 14) & 0xfffe) | 0x01) >> 8) & 0xff);
		TSbuf[ts_pos + 2]= ((((Adudiopts >> 14) & 0xfffe) | 0x01) & 0xff);
		TSbuf[ts_pos + 3]= (((((Adudiopts << 1) & 0xfffe) | 0x01) >> 8) & 0xff);
		TSbuf[ts_pos + 4]= ((((Adudiopts << 1) & 0xfffe) | 0x01) & 0xff);
		ts_pos += 5;
	}
	else
	{
		printf("ts_pes->stream_id  error 0x%x \n",ts_pes->stream_id);
		return getchar();
	}

	NeafBuf = ts_pes->Es ;
	memcpy(TSbuf + ts_pos,NeafBuf,FirstPacketLoadLength);  

	NeafBuf += FirstPacketLoadLength;
	ts_pes->Pes_Packet_Length_Beyond -= FirstPacketLoadLength;
	//����д���ļ�
	fwrite(TSbuf,188,1,pVideo_Audio_Ts_File);                               //��һ������д���ļ�
	WritePacketNum ++;                                                      //�Ѿ�д���ļ��İ�����++

	while(ts_pes->Pes_Packet_Length_Beyond)
	{
		ts_pos = 0;
		memset(TSbuf,0,TS_PACKET_SIZE); 

		if ((WritePacketNum % 40) == 0)                                                         //ÿ40������һ�� pat,һ��pmt
		{
			Write_Pat(m_One_Frame_Buf);                                                         //����PAT
			Write_Pmt(m_One_Frame_Buf);                                                         //����PMT
		}
		if(ts_pes->Pes_Packet_Length_Beyond >= 184)
		{
			//�����м��   
			WriteStruct_Packetheader(TSbuf,Video_Audio_PID,0x00,0x01);     //PID = TS_H264_PID,������Ч���ص�Ԫ��ʼָʾ��_play_init = 0x00, ada_field_C,0x01,������Ч���أ�    
			ts_pos += 4;
            memcpy(TSbuf + ts_pos,NeafBuf,184); 
			NeafBuf += 184;
			ts_pes->Pes_Packet_Length_Beyond -= 184;
			fwrite(TSbuf,188,1,pVideo_Audio_Ts_File); 
		}
		else
		{
			if(ts_pes->Pes_Packet_Length_Beyond == 183||ts_pes->Pes_Packet_Length_Beyond == 182)
			{
				if ((WritePacketNum % 40) == 0)                                                         //ÿ40������һ�� pat,һ��pmt
				{
					Write_Pat(m_One_Frame_Buf);                                                         //����PAT
					Write_Pmt(m_One_Frame_Buf);                                                         //����PMT
				}

				WriteStruct_Packetheader(TSbuf,Video_Audio_PID,0x00,0x03);   //PID = TS_H264_PID,������Ч���ص�Ԫ��ʼָʾ��_play_init = 0x00, ada_field_C,0x03,���е����ֶκ���Ч���أ�
				ts_pos += 4;
				TSbuf[ts_pos + 0] = 0x01;
				TSbuf[ts_pos + 1] = 0x00;
				ts_pos += 2;
				memcpy(TSbuf + ts_pos,NeafBuf,182); 
				  
				NeafBuf += 182;
				ts_pes->Pes_Packet_Length_Beyond -= 182;
				fwrite(TSbuf,188,1,pVideo_Audio_Ts_File); 
			}
			else
			{
				if ((WritePacketNum % 40) == 0)                                                         //ÿ40������һ�� pat,һ��pmt
				{
					Write_Pat(m_One_Frame_Buf);                                                         //����PAT
					Write_Pmt(m_One_Frame_Buf);                                                         //����PMT
				}

				WriteStruct_Packetheader(TSbuf,Video_Audio_PID,0x00,0x03);  //PID = TS_H264_PID,������Ч���ص�Ԫ��ʼָʾ��_play_init = 0x00, ada_field_C,0x03,���е����ֶκ���Ч���أ�
				ts_pos += 4;
				TSbuf[ts_pos + 0] = 184-ts_pes->Pes_Packet_Length_Beyond-1 ;
				TSbuf[ts_pos + 1] = 0x00;
				ts_pos += 2;
				memset(TSbuf + ts_pos,0xFF,(184 - ts_pes->Pes_Packet_Length_Beyond - 2)); 
				ts_pos += (184-ts_pes->Pes_Packet_Length_Beyond-2);
				memcpy(TSbuf + ts_pos,NeafBuf,ts_pes->Pes_Packet_Length_Beyond);
				ts_pes->Pes_Packet_Length_Beyond = 0;
				fwrite(TSbuf,188,1,pVideo_Audio_Ts_File);   //��һ������д���ļ�
				WritePacketNum ++;  
			}
		}	
		WritePacketNum ++;  
	}

	return WritePacketNum ;
}

/*�ļ�����д��ts�ļ�*/
int WriteBuf2File(unsigned int framerate)
{
	unsigned long  Timestamp_video = 0;    //һ֡��Ƶ����ʱ��
	unsigned long  Timestamp_audio = 0;    //һ֡��Ƶ����ʱ�� 
	unsigned int   audiosamplerate = 0;    //��Ƶ������
	unsigned int   videoframetype =  0;    //��Ƶ֡����
	Ts_Adaptation_field  ts_adaptation_field_Head ; 
	Ts_Adaptation_field  ts_adaptation_field_Tail ;
	unsigned int WritePacketNum;

	//����AAC�ļ�����Ϣ,���adtsͷ
	ADTS_HEADER adtsheader;
	Detach_Head_Aac(&adtsheader,m_One_Frame_Buf); 
	if (adtsheader.sf_index == 0x04)
	{
		//audiosamplerate = 44100;
		audiosamplerate = 8000;
	}
	else if (adtsheader.sf_index == 0x03)
	{
		audiosamplerate = 48000;
	}
	//����Ƶ�ļ��ƶ�����ͷ
	if (fseek(pAudio_Aac_File, 0, 0) < 0) //�ɹ�������0��ʧ�ܷ���-1
	{
		printf("fseek : pAudio_Aac_File Error\n");
		return getchar();
	}

	//��ʼѭ��д������Ƶ����
	for (;;)
	{
		if (/*�ļ���ȡ���*/(decode_video_done && decode_audio_done))
		{
			break;
		}
		//��Ƶ�ļ���ȡ���
		if (decode_audio_done)
		{
			Take_Out_Pes(&m_audio_tspes ,Timestamp_audio,0x01,NULL);
			if (m_audio_tspes.Pes_Packet_Length_Beyond != 0)
			{
				printf("PES_AUDIO  :  SIZE = %d\n",m_audio_tspes.Pes_Packet_Length_Beyond);
				//��д����Ӧ�α�־
				WriteAdaptive_flags_Tail(&ts_adaptation_field_Head); //��д����Ӧ�α�־  ,����ע�� ��Ƶ���Ͳ�Ҫ��pcr ���Զ���֡β�������
				WriteAdaptive_flags_Tail(&ts_adaptation_field_Tail); //��д����Ӧ�α�־֡β
				PES2TS(&m_audio_tspes,TS_AAC_PID ,&ts_adaptation_field_Head ,&ts_adaptation_field_Tail,Timestamp_video,Timestamp_audio);
				//����һ֡��Ƶ����ʱ��
				Timestamp_audio += 1024*1000* 90/audiosamplerate;
			}
			continue;
		}
		//��Ƶ�ļ���ȡ���
		if (decode_video_done)
		{
			Take_Out_Pes(&m_video_tspes ,Timestamp_video,0x00,&videoframetype);
			if (m_video_tspes.Pes_Packet_Length_Beyond != 0)
			{
				printf("PES_VIDEO  :  SIZE = %d\n",m_video_tspes.Pes_Packet_Length_Beyond);
				if (videoframetype == FRAME_I || videoframetype == FRAME_P || videoframetype == FRAME_B)
				{
					//��д����Ӧ�α�־
					WriteAdaptive_flags_Head(&ts_adaptation_field_Head,Timestamp_video); //��д����Ӧ�α�־֡ͷ
					WriteAdaptive_flags_Tail(&ts_adaptation_field_Tail); //��д����Ӧ�α�־֡β
					//����һ֡��Ƶ����ʱ��
					PES2TS(&m_video_tspes,TS_H264_PID ,&ts_adaptation_field_Head ,&ts_adaptation_field_Tail,Timestamp_video,Timestamp_audio);
					Timestamp_video += 1000* 90/framerate;
				}
				else
				{
					//��д����Ӧ�α�־
					WriteAdaptive_flags_Tail(&ts_adaptation_field_Head); //��д����Ӧ�α�־  ,����ע�� ����֡���Ͳ�Ҫ��pcr ���Զ���֡β�������
					WriteAdaptive_flags_Tail(&ts_adaptation_field_Tail); //��д����Ӧ�α�־֡β
					PES2TS(&m_video_tspes,TS_H264_PID ,&ts_adaptation_field_Head ,&ts_adaptation_field_Tail,Timestamp_video,Timestamp_audio);
				}
			}
			continue;
		}

		/* write interleaved audio and video frames */
		if ( Timestamp_audio > Timestamp_video )
		{
			Take_Out_Pes(&m_video_tspes ,Timestamp_video,0x00,&videoframetype);
			if (m_video_tspes.Pes_Packet_Length_Beyond != 0)
			{
				printf("PES_VIDEO  :  SIZE = %d\n",m_video_tspes.Pes_Packet_Length_Beyond);
				if (videoframetype == FRAME_I || videoframetype == FRAME_P || videoframetype == FRAME_B)
				{
					//��д����Ӧ�α�־
					WriteAdaptive_flags_Head(&ts_adaptation_field_Head,Timestamp_video); //��д����Ӧ�α�־֡ͷ
					WriteAdaptive_flags_Tail(&ts_adaptation_field_Tail); //��д����Ӧ�α�־֡β
					//����һ֡��Ƶ����ʱ��
					PES2TS(&m_video_tspes,TS_H264_PID ,&ts_adaptation_field_Head ,&ts_adaptation_field_Tail,Timestamp_video,Timestamp_audio);
					Timestamp_video += 1000* 90/framerate;   //90khz
				}
				else
				{
					//��д����Ӧ�α�־
					WriteAdaptive_flags_Tail(&ts_adaptation_field_Head); //��д����Ӧ�α�־  ,����ע�� ����֡���Ͳ�Ҫ��pcr ���Զ���֡β�������
					WriteAdaptive_flags_Tail(&ts_adaptation_field_Tail); //��д����Ӧ�α�־֡β
					PES2TS(&m_video_tspes,TS_H264_PID ,&ts_adaptation_field_Head ,&ts_adaptation_field_Tail,Timestamp_video,Timestamp_audio);
				}
			}
		}
		else
		{
			Take_Out_Pes(&m_audio_tspes ,Timestamp_audio,0x01,NULL);
			if (m_audio_tspes.Pes_Packet_Length_Beyond != 0)
			{
				printf("PES_AUDIO  :  SIZE = %d\n",m_audio_tspes.Pes_Packet_Length_Beyond);
				//��д����Ӧ�α�־
				WriteAdaptive_flags_Tail(&ts_adaptation_field_Head); //��д����Ӧ�α�־  ,����ע�� ��Ƶ���Ͳ�Ҫ��pcr ���Զ���֡β�������
				WriteAdaptive_flags_Tail(&ts_adaptation_field_Tail); //��д����Ӧ�α�־֡β
				PES2TS(&m_audio_tspes,TS_AAC_PID ,&ts_adaptation_field_Head ,&ts_adaptation_field_Tail,Timestamp_video,Timestamp_audio);
				//����һ֡��Ƶ����ʱ��
				Timestamp_audio += 1024*1000* 90/audiosamplerate;
			}
		}
	}
	return 1;
}

/*ʵʱ��д��ts�ļ�*/
int WriteBuf2TsFile(unsigned int framerate, int iStreamType, unsigned char *pData, int iDataSize, unsigned long lTimeStamp)
{
	unsigned int   audiosamplerate = 8000;    //��Ƶ������
	unsigned int   videoframetype = 0;    //��Ƶ֡����
	Ts_Adaptation_field  ts_adaptation_field_Head;
	Ts_Adaptation_field  ts_adaptation_field_Tail;
	unsigned int WritePacketNum;

	if (0 == iStreamType)
	{
		Take_Out_Pes(&m_audio_tspes, Timestamp_audio, 0x01, NULL,pData,iDataSize);
		if (m_audio_tspes.Pes_Packet_Length_Beyond != 0)
		{
			printf("PES_AUDIO  :  SIZE = %d\n", m_audio_tspes.Pes_Packet_Length_Beyond);
			//��д����Ӧ�α�־
			WriteAdaptive_flags_Tail(&ts_adaptation_field_Head); //��д����Ӧ�α�־  ,����ע�� ��Ƶ���Ͳ�Ҫ��pcr ���Զ���֡β�������
			WriteAdaptive_flags_Tail(&ts_adaptation_field_Tail); //��д����Ӧ�α�־֡β
			PES2TS(&m_audio_tspes, TS_AAC_PID, &ts_adaptation_field_Head, &ts_adaptation_field_Tail, Timestamp_video, Timestamp_audio);
			Timestamp_audio += 1024 * 1000 * 90 / 8000;
			//����һ֡��Ƶ����ʱ��
		}
	}
	else if (1 == iStreamType)
	{
		Take_Out_Pes(&m_video_tspes, Timestamp_video, 0x00, &videoframetype,pData,iDataSize);
		if (m_video_tspes.Pes_Packet_Length_Beyond != 0)
		{
		    printf("PES_VIDEO  :  SIZE = %d\n", m_video_tspes.Pes_Packet_Length_Beyond);
			if (videoframetype == FRAME_I || videoframetype == FRAME_P || videoframetype == FRAME_B)
			{
				//��д����Ӧ�α�־
				WriteAdaptive_flags_Head(&ts_adaptation_field_Head, Timestamp_video); //��д����Ӧ�α�־֡ͷ
				WriteAdaptive_flags_Tail(&ts_adaptation_field_Tail); //��д����Ӧ�α�־֡β
				//����һ֡��Ƶ����ʱ��
				PES2TS(&m_video_tspes, TS_H264_PID, &ts_adaptation_field_Head, &ts_adaptation_field_Tail, Timestamp_video, Timestamp_audio);
				Timestamp_video += 1000 * 90 / framerate;
			}
			else
			{
				//��д����Ӧ�α�־
				WriteAdaptive_flags_Tail(&ts_adaptation_field_Head); //��д����Ӧ�α�־  ,����ע�� ����֡���Ͳ�Ҫ��pcr ���Զ���֡β�������
				WriteAdaptive_flags_Tail(&ts_adaptation_field_Tail); //��д����Ӧ�α�־֡β
				PES2TS(&m_video_tspes, TS_H264_PID, &ts_adaptation_field_Head, &ts_adaptation_field_Tail, Timestamp_video, Timestamp_audio);
			}
		}
	}
	return 1;
}

int WriteH264Buff2File(unsigned int framerate)
{
	unsigned long  Timestamp_video = 0;    //һ֡��Ƶ����ʱ��
	unsigned long  Timestamp_audio = 0;    //һ֡��Ƶ����ʱ�� 
	unsigned int   audiosamplerate = 0;    //��Ƶ������
	unsigned int   videoframetype = 0;     //��Ƶ֡����
	Ts_Adaptation_field  ts_adaptation_field_Head;
	Ts_Adaptation_field  ts_adaptation_field_Tail;
	unsigned int WritePacketNum;


	while (!decode_audio_done)
	{
		Take_Out_Pes(&m_video_tspes, Timestamp_video, 0x00, &videoframetype);
		if (m_video_tspes.Pes_Packet_Length_Beyond != 0)
		{
			printf("PES_VIDEO  :  SIZE = %d\n", m_video_tspes.Pes_Packet_Length_Beyond);
			if (videoframetype == FRAME_I || videoframetype == FRAME_P || videoframetype == FRAME_B)
			{
				//��д����Ӧ�α�־
				WriteAdaptive_flags_Head(&ts_adaptation_field_Head, Timestamp_video); //��д����Ӧ�α�־֡ͷ
				WriteAdaptive_flags_Tail(&ts_adaptation_field_Tail); //��д����Ӧ�α�־֡β
				//����һ֡��Ƶ����ʱ��
				PES2TS(&m_video_tspes, TS_H264_PID, &ts_adaptation_field_Head, &ts_adaptation_field_Tail, Timestamp_video, Timestamp_audio);
				Timestamp_video += 1000 * 90 / framerate;   //90khz
			}
			else
			{
				//��д����Ӧ�α�־
				WriteAdaptive_flags_Tail(&ts_adaptation_field_Head); //��д����Ӧ�α�־  ,����ע�� ����֡���Ͳ�Ҫ��pcr ���Զ���֡β�������
				WriteAdaptive_flags_Tail(&ts_adaptation_field_Tail); //��д����Ӧ�α�־֡β
				PES2TS(&m_video_tspes, TS_H264_PID, &ts_adaptation_field_Head, &ts_adaptation_field_Tail, Timestamp_video, Timestamp_audio);
			}
		}
	}
	return 1;
}

