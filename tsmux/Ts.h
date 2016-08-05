#pragma once

#include "FileIo.h"
#include "crc.h"

#define TS_PACKET_HEADER               4
#define TS_PACKET_SIZE                 188
#define TS_SYNC_BYTE                   0x47
#define TS_PAT_PID                     0x00
#define TS_PMT_PID                     0xFFF
#define TS_H264_PID                    0x100
#define TS_AAC_PID                     0x101
#define TS_H264_STREAM_ID              0xE0
#define TS_AAC_STREAM_ID               0xC0
#define PMT_STREAM_TYPE_VIDEO          0x1B
#define PMT_STREAM_TYPE_AUDIO          0x0F
#define MAX_ONE_FRAME_SIZE             1024 * 1024

//ts ��ͷ
typedef struct Tag_PacketHeader
{
	unsigned char sync_byte         :8  ;         //ͬ���ֽ�, �̶�Ϊ0x47,��ʾ�������һ��TS����
	unsigned char tras_error        :1  ;         //��������ָʾ��   
	unsigned char play_init         :1  ;         //��Ч���ص�Ԫ��ʼָʾ��
	unsigned char tras_prio	        :1  ;         //��������, 1��ʾ�����ȼ�,������ƿ����õ��������ò���
	unsigned int  PID               :13 ;         //PID
	unsigned char tras_scramb       :2  ;         //������ſ���
	unsigned char ada_field_C       :2  ;         //����Ӧ���� 01������Ч���أ�10���������ֶΣ�11���е����ֶκ���Ч���أ��ȵ����ֶ�Ȼ����Ч���ء�Ϊ00�����������д��� 
	unsigned char conti_cter        :4  ;         //���������� һ��4bit�ļ���������Χ0-15
}TsPacketHeader; 

//PAT�ṹ�壺��Ŀ��ر�
typedef struct Tag_TsPat
{
	unsigned char table_id :8 ;                  //�̶�Ϊ0x00 ����־�Ǹñ���PAT
	unsigned char section_syntax_indicator: 1;   //���﷨��־λ���̶�Ϊ1
	unsigned char zero : 1;                      //0 
	unsigned char reserved_1 : 2;                //����λ
	unsigned int section_length : 12 ;           //��ʾ����ֽں������õ��ֽ���������CRC32
	unsigned int transport_stream_id : 16 ;      //�ô�������ID��������һ��������������·���õ���
	unsigned char reserved_2 : 2;                //����λ
	unsigned char version_number : 5 ;           //��Χ0-31����ʾPAT�İ汾��
	unsigned char current_next_indicator : 1 ;   //���͵�PAT�ǵ�ǰ��Ч������һ��PAT��Ч
	unsigned char section_number : 8 ;           //�ֶεĺ��롣PAT���ܷ�Ϊ��δ��䣬��һ��Ϊ00���Ժ�ÿ���ֶμ�1����������256���ֶ�
	unsigned char last_section_number : 8 ;      //���һ���ֶεĺ���
	unsigned int program_number  : 16 ;          //��Ŀ��
	unsigned char reserved_3  : 3  ;             //����λ
	unsigned int network_PID :13 ;               //������Ϣ��NIT����PID,��Ŀ��Ϊ0ʱ��Ӧ��PIDΪnetwork_PID,�����в����� networke_pid
	unsigned int program_map_PID : 13;           //��Ŀӳ����PID����Ŀ�Ŵ���0ʱ��Ӧ��PID��ÿ����Ŀ��Ӧһ��
	unsigned int CRC_32  : 32;             //CRC32У����
}TsPat; 

//PMT�ṹ�壺��Ŀӳ���
typedef struct Tag_TsPmt
{
	unsigned char table_id  : 8;                 //�̶�Ϊ0x02, ��ʾPMT��
	unsigned char section_syntax_indicator : 1;  //�̶�Ϊ0x01
	unsigned char zero: 1;                       //0x00
	unsigned char reserved_1 : 2;                //0x03
	unsigned int section_length: 12;             //������λbit��Ϊ00����ָʾ�ε�byte�����ɶγ�����ʼ������CRC��
	unsigned int program_number : 16;            // ָ���ý�Ŀ��Ӧ�ڿ�Ӧ�õ�Program map PID
	unsigned char reserved_2: 2;                 //0x03
	unsigned char version_number: 5;             //ָ��TS����Program map section�İ汾��
	unsigned char current_next_indicator: 1;     //����λ��1ʱ����ǰ���͵�Program map section���ã�����λ��0ʱ��ָʾ��ǰ���͵�Program map section�����ã���һ��TS����Program map section��Ч��
	unsigned char section_number : 8;            //�̶�Ϊ0x00
	unsigned char last_section_number: 8;        //�̶�Ϊ0x00
	unsigned char reserved_3 : 3;                //0x07
	unsigned int PCR_PID : 13;                   //ָ��TS����PIDֵ����TS������PCR�򣬸�PCRֵ��Ӧ���ɽ�Ŀ��ָ���Ķ�Ӧ��Ŀ���������˽���������Ľ�Ŀ������PCR�޹أ�������ֵ��Ϊ0x1FFF��
	unsigned char reserved_4 : 4;                //Ԥ��Ϊ0x0F
	unsigned int program_info_length : 12;       //ǰ��λbitΪ00������ָ���������Խ�Ŀ��Ϣ��������byte����
	unsigned char stream_type_video : 8;         //ָʾ�ض�PID�Ľ�ĿԪ�ذ������͡��ô�PID��elementary PIDָ��
	unsigned char reserved_5_video: 3;           //0x07
	unsigned int elementary_PID_video: 13;       //����ָʾTS����PIDֵ����ЩTS��������صĽ�ĿԪ��
	unsigned char reserved_6_video : 4;          //0x0F
	unsigned int ES_info_length_video : 12;      //ǰ��λbitΪ00������ָʾ��������������ؽ�ĿԪ�ص�byte��
	unsigned char stream_type_audio : 8;         //ָʾ�ض�PID�Ľ�ĿԪ�ذ������͡��ô�PID��elementary PIDָ��
	unsigned char reserved_5_audio: 3;           //0x07
	unsigned int elementary_PID_audio: 13;       //����ָʾTS����PIDֵ����ЩTS��������صĽ�ĿԪ��
	unsigned char reserved_6_audio : 4;          //0x0F
	unsigned int ES_info_length_audio : 12;      //ǰ��λbitΪ00������ָʾ��������������ؽ�ĿԪ�ص�byte��
	unsigned long long CRC_32: 32;               //CRC32У����
}TsPmt; 

//�����Լ�����,Ҳ����˵ �ж��ٸ� pat��������pmt�� ������MP3 �������� h264����0x00 - 0x0f ��Ȼ���ۻص�0x00 ���ڲ鿴����                            
typedef struct Tag_Continuity_Counter
{
	unsigned char continuity_counter_pat;
	unsigned char continuity_counter_pmt;
	unsigned char continuity_counter_video;
	unsigned char continuity_counter_audio;
}Continuity_Counter;

//����Ӧ�α�־
typedef struct Tag_Ts_Adaptation_field
{
	unsigned char discontinuty_indicator:1;                //1������ǰ����������Ĳ�����״̬Ϊ��
	unsigned char random_access_indicator:1;               //������һ������ͬPID��PES����Ӧ�ú���PTS�ֶκ�һ��ԭʼ�����ʵ�
	unsigned char elementary_stream_priority_indicator:1;  //���ȼ�
	unsigned char PCR_flag:1;                              //����pcr�ֶ�
	unsigned char OPCR_flag:1;                             //����opcr�ֶ�
	unsigned char splicing_point_flag:1;                   //ƴ�ӵ��־       
	unsigned char transport_private_data_flag:1;           //˽���ֽ�
	unsigned char adaptation_field_extension_flag:1;       //�����ֶ�����չ

	unsigned char adaptation_field_length;                 //����Ӧ�γ���
	unsigned long long  pcr;                               //����Ӧ�����õ��ĵ�pcr
	unsigned long long  opcr;                              //����Ӧ�����õ��ĵ�opcr
	unsigned char splice_countdown;
	unsigned char private_data_len;
	unsigned char private_data [256];
}Ts_Adaptation_field;

//PTS_DTS�ṹ�壺���������ö��� ��11��
typedef struct Tag_TsPtsDts
{
	unsigned char reserved_1 : 4;
	unsigned char pts_32_30  : 3;                //��ʾʱ���
	unsigned char marker_bit1: 1;
	unsigned int  pts_29_15 : 15;
	unsigned char marker_bit2 : 1;
	unsigned int  pts_14_0 : 15;
	unsigned char marker_bit3 :1 ;
	unsigned char reserved_2 : 4;
	unsigned char dts_32_30: 3;                  //����ʱ���
	unsigned char marker_bit4 :1;
	unsigned int  dts_29_15 :15;
	unsigned char marker_bit5: 1;
	unsigned int  dts_14_0 :15;
	unsigned char marker_bit6 :1 ;
}TsPtsDts;

//PES���ṹ�壬����PES��ͷ��ES���� ,ͷ 19 ���ֽ�
typedef struct Tag_TsPes
{
	unsigned int   packet_start_code_prefix : 24;//��ʼ��0x000001
	unsigned char  stream_id : 8;                //�����������ͺͱ��
	unsigned int   PES_packet_length : 16;       //������,����֡���ݵĳ��ȣ�����Ϊ0,Ҫ�Լ���,����16λ�������������Ҫ�Լ���
	unsigned char  marker_bit:2;                 //�����ǣ�'10'
	unsigned char  PES_scrambling_control:2;     //pes����Ч�غɵļ��ŷ�ʽ
	unsigned char  PES_priority:1;               //��Ч���ص����ȼ�
	unsigned char  data_alignment_indicator:1;   //�������Ϊ1����PES����ͷ�����������Ƶ����Ƶsyncword��ʼ�Ĵ��롣
	unsigned char  copyright:1;                  //1:����Ȩ������0������
	unsigned char  original_or_copy:1;           //1;��Ч������ԭʼ�ģ�0����Ч����ʱ������
	unsigned char  PTS_DTS_flags:2;              //'10'��PTS�ֶδ��ڣ���11����PTD��DTS�����ڣ���00������û�У���01�������á�
	unsigned char  ESCR_flag:1;                  //1:escr��׼�ֶ� �� escr��չ�ֶξ����ڣ�0�����κ�escr�ֶδ���
	unsigned char  ES_rate_flag:1;               //1:es_rate�ֶδ��ڣ�0 ��������
	unsigned char  DSM_trick_mode_flag:1;        //1;8�����ؽӷ�ʽ�ֶδ��ڣ�0 ��������
	unsigned char  additional_copy_info_flag:1;  //1:additional_copy_info���ڣ�0: ������
	unsigned char  PES_CRC_flag:1;               //1:crc�ֶδ��ڣ�0��������
	unsigned char  PES_extension_flag:1;         //1����չ�ֶδ��ڣ�0��������
	unsigned char  PES_header_data_length :8;    //�������ݵĳ��ȣ�
	TsPtsDts       tsptsdts;                     //ptsdts�ṹ�����10���ֽ�
	unsigned char  Es[MAX_ONE_FRAME_SIZE];       //һ֡ ԭʼ����
	unsigned int   Pes_Packet_Length_Beyond;     //���PES_packet_length�Ĵ�С��������һ֡���ݵĳ��������������
}TsPes;

extern Continuity_Counter continuity_counter;     //�����ͼ�����

int WriteStruct_Packetheader(unsigned char * Buf , unsigned int PID,unsigned char play_init,unsigned char ada_field_C);
int WriteStruct_Pat(unsigned char * Buf);
int WriteStruct_Pmt(unsigned char * Buf);
