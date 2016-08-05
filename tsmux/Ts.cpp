#include "Ts.h"

Continuity_Counter continuity_counter;     //�����ͼ�����

int WriteStruct_Packetheader(unsigned char * Buf , unsigned int PID,unsigned char play_init,unsigned char ada_field_C)
{
	TsPacketHeader tspacketheader;

	tspacketheader.sync_byte = TS_SYNC_BYTE;
	tspacketheader.tras_error = 0x00;
	tspacketheader.play_init = play_init;
	tspacketheader.tras_prio = 0x00;
	tspacketheader.PID = PID;
	tspacketheader.tras_scramb = 0x00;
	tspacketheader.ada_field_C = ada_field_C;

	if (PID == TS_PAT_PID)             //����pat�İ�
	{
		tspacketheader.conti_cter = (continuity_counter.continuity_counter_pat %16);
		continuity_counter.continuity_counter_pat ++;
	}
	else if (PID == TS_PMT_PID)        //����pmt�İ�
	{
		tspacketheader.conti_cter = (continuity_counter.continuity_counter_pmt %16);
		continuity_counter.continuity_counter_pmt ++;
	}
	else if (PID == TS_H264_PID )      //����H264�İ�
	{
		tspacketheader.conti_cter = (continuity_counter.continuity_counter_video %16);
		continuity_counter.continuity_counter_video ++;
	}
	else if (PID == TS_AAC_PID)        //����MP3�İ�
	{
		tspacketheader.conti_cter = (continuity_counter.continuity_counter_audio %16);
		continuity_counter.continuity_counter_audio ++;
	}
	else                               //���������������չ
	{
		printf("continuity_counter error packet\n");
		getchar();
	}

	Buf[0] = tspacketheader.sync_byte;
	Buf[1] = tspacketheader.tras_error << 7 | tspacketheader.play_init << 6  | tspacketheader.tras_prio << 5 | ((tspacketheader.PID >> 8) & 0x1f);
	Buf[2] = (tspacketheader. PID & 0x00ff);
	Buf[3] = tspacketheader.tras_scramb << 6 | tspacketheader.ada_field_C << 4 | tspacketheader.conti_cter;
	return 4;
}

int WriteStruct_Pat(unsigned char * Buf)
{
	unsigned int pat_pos = 0;  
	TsPat tspat;
	unsigned int PAT_CRC = 0xFFFFFFFF;

	memset(Buf,0xFF,TS_PACKET_SIZE);

	tspat.table_id = 0x00;
	tspat.section_syntax_indicator = 0x01;
	tspat.zero = 0x00;
	tspat.reserved_1 = 0x03;                                               //����Ϊ11��
	tspat.section_length = 0x0d;                                           //pat�ṹ�峤�� 16���ֽڼ�ȥ�����3���ֽ�
	tspat.transport_stream_id = 0x01;
	tspat.reserved_2 = 0x03;                                               //����Ϊ11��
	tspat.version_number = 0x00;
	tspat.current_next_indicator = 0x01;                                   //��ǰ��pat ��Ч
	tspat.section_number = 0x00;
	tspat.last_section_number = 0x00;
	tspat.program_number = 0x01;
	tspat.reserved_3 = 0x07;                                               //����Ϊ111��
	tspat.program_map_PID = TS_PMT_PID;                                    //PMT��PID
	tspat.CRC_32 = PAT_CRC;                                                //��������м���һ���㷨ֵ ���趨һ�����ֵ

	pat_pos += WriteStruct_Packetheader(Buf,TS_PAT_PID,0x01,0x01);  //PID = 0x00,��Ч���ص�Ԫ��ʼָʾ��_play_init = 0x01, ada_field_C,0x01,������Ч���� ��
	Buf[4] = 0;                                                     //����Ӧ�εĳ���Ϊ0
	pat_pos ++;

	Buf[pat_pos] = tspat.table_id;
	Buf[pat_pos + 1] = tspat.section_syntax_indicator << 7 | tspat.zero  << 6 | tspat.reserved_1 << 4 | ((tspat.section_length >> 8) & 0x0F);
	Buf[pat_pos + 2] = tspat.section_length & 0x00FF;
	Buf[pat_pos + 3] = tspat.transport_stream_id >> 8;
	Buf[pat_pos + 4] = tspat.transport_stream_id & 0x00FF;
	Buf[pat_pos + 5] = tspat.reserved_2 << 6 | tspat.version_number << 1 | tspat.current_next_indicator;
	Buf[pat_pos + 6] = tspat.section_number;
	Buf[pat_pos + 7] = tspat.last_section_number;
	Buf[pat_pos + 8] = tspat.program_number>>8;
	Buf[pat_pos + 9] = tspat.program_number & 0x00FF;
	Buf[pat_pos + 10]= tspat.reserved_3 << 5 | ((tspat.program_map_PID >> 8) & 0x0F);
	Buf[pat_pos + 11]= tspat.program_map_PID & 0x00FF;
	pat_pos += 12;

	PAT_CRC = Zwg_ntohl(calc_crc32(Buf + 5, pat_pos - 5));
	memcpy(Buf + pat_pos, (unsigned char *)&PAT_CRC, 4);
	pat_pos += 4; 

	return 188;
}

int WriteStruct_Pmt(unsigned char * Buf)
{
	unsigned int pmt_pos = 0;  
	TsPmt tspmt;
	unsigned long long PMT_CRC = 0xFFFFFFFF; 
	int len = 0;

	memset(Buf,0xFF,TS_PACKET_SIZE);

	tspmt.table_id = 0x02;
	tspmt.section_syntax_indicator = 0x01;
	tspmt.zero = 0x00;
	tspmt.reserved_1 = 0x03;
	tspmt.section_length = 0x17;                                           //PMT�ṹ�峤�� 16 + 5 + 5���ֽڼ�ȥ�����3���ֽ�
	tspmt.program_number = 01;                                             //ֻ��һ����Ŀ
	tspmt.reserved_2 = 0x03;
	tspmt.version_number = 0x00;
	tspmt.current_next_indicator = 0x01;                                   //��ǰ��PMT��Ч
	tspmt.section_number = 0x00;
	tspmt.last_section_number = 0x00;
	tspmt.reserved_3 = 0x07;
	tspmt.PCR_PID = TS_H264_PID ;                                          //��ƵPID                                   
	tspmt.reserved_4 = 0x0F;
	tspmt.program_info_length = 0x00;                                      //������ ��Ŀ��Ϣ����
	tspmt.stream_type_video = PMT_STREAM_TYPE_VIDEO;                       //��Ƶ������
	tspmt.reserved_5_video = 0x07;
	tspmt.elementary_PID_video = TS_H264_PID;                              //��Ƶ��PID
	tspmt.reserved_6_video= 0x0F;
	tspmt.ES_info_length_video = 0x00;                                     //��Ƶ�޸���������Ϣ
	tspmt.stream_type_audio = PMT_STREAM_TYPE_AUDIO;                       //��Ƶ����
	tspmt.reserved_5_audio = 0x07;
	tspmt.elementary_PID_audio = TS_AAC_PID;                               //��ƵPID 
	tspmt.reserved_6_audio = 0x0F;
	tspmt.ES_info_length_audio = 0x00;                                     //��Ƶ�޸���������Ϣ
	tspmt.CRC_32 = PMT_CRC; 

	pmt_pos += WriteStruct_Packetheader(Buf,TS_PMT_PID,0x01,0x01);           //PID = TS_PMT_PID,��Ч���ص�Ԫ��ʼָʾ��_play_init = 0x01, ada_field_C,0x01,������Ч���أ�

	Buf[4] = 0;                                                      //����Ӧ�εĳ���Ϊ0
	pmt_pos ++;

	Buf[pmt_pos + 0] = tspmt.table_id;
	Buf[pmt_pos + 1] = tspmt.section_syntax_indicator << 7 | tspmt.zero  << 6 | tspmt.reserved_1 << 4 | ((tspmt.section_length >> 8) & 0x0F);
	Buf[pmt_pos + 2] = tspmt.section_length & 0x00FF;
	Buf[pmt_pos + 3] = tspmt.program_number >> 8;
	Buf[pmt_pos + 4] = tspmt.program_number & 0x00FF;
	Buf[pmt_pos + 5] = tspmt.reserved_2 << 6 | tspmt.version_number << 1 | tspmt.current_next_indicator;
	Buf[pmt_pos + 6] = tspmt.section_number;
	Buf[pmt_pos + 7] = tspmt.last_section_number;
	Buf[pmt_pos + 8] = tspmt.reserved_3 << 5  | ((tspmt.PCR_PID >> 8) & 0x1F);
	Buf[pmt_pos + 9] = tspmt.PCR_PID & 0x0FF;
	Buf[pmt_pos + 10]= tspmt.reserved_4 << 4 | ((tspmt.program_info_length >> 8) & 0x0F);
	Buf[pmt_pos + 11]= tspmt.program_info_length & 0xFF;
	Buf[pmt_pos + 12]= tspmt.stream_type_video;                               //��Ƶ����stream_type
	Buf[pmt_pos + 13]= tspmt.reserved_5_video << 5 | ((tspmt.elementary_PID_video >> 8 ) & 0x1F);
	Buf[pmt_pos + 14]= tspmt.elementary_PID_video & 0x00FF;
	Buf[pmt_pos + 15]= tspmt.reserved_6_video<< 4 | ((tspmt.ES_info_length_video >> 8) & 0x0F);
	Buf[pmt_pos + 16]= tspmt.ES_info_length_video & 0x0FF;
	Buf[pmt_pos + 17]= tspmt.stream_type_audio;                               //��Ƶ����stream_type
	Buf[pmt_pos + 18]= tspmt.reserved_5_audio<< 5 | ((tspmt.elementary_PID_audio >> 8 ) & 0x1F);
	Buf[pmt_pos + 19]= tspmt.elementary_PID_audio & 0x00FF;
	Buf[pmt_pos + 20]= tspmt.reserved_6_audio << 4 | ((tspmt.ES_info_length_audio >> 8) & 0x0F);
	Buf[pmt_pos + 21]= tspmt.ES_info_length_audio & 0x0FF;
	pmt_pos += 22;

	len  = pmt_pos - 8 + 4;
	len = len > 0xffff ? 0:len;
	Buf[6] = 0xb0 | (len >> 8);
	Buf[7] = len;

	PMT_CRC = Zwg_ntohl(calc_crc32(Buf + 5, pmt_pos - 5));
	memcpy(Buf + pmt_pos , (unsigned char  *)&PMT_CRC, 4);
	pmt_pos += 4;

	return 188;
}

