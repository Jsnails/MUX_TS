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

//ts 包头
typedef struct Tag_PacketHeader
{
	unsigned char sync_byte         :8  ;         //同步字节, 固定为0x47,表示后面的是一个TS分组
	unsigned char tras_error        :1  ;         //传输误码指示符   
	unsigned char play_init         :1  ;         //有效荷载单元起始指示符
	unsigned char tras_prio	        :1  ;         //传输优先, 1表示高优先级,传输机制可能用到，解码用不着
	unsigned int  PID               :13 ;         //PID
	unsigned char tras_scramb       :2  ;         //传输加扰控制
	unsigned char ada_field_C       :2  ;         //自适应控制 01仅含有效负载，10仅含调整字段，11含有调整字段和有效负载，先调整字段然后有效负载。为00解码器不进行处理 
	unsigned char conti_cter        :4  ;         //连续计数器 一个4bit的计数器，范围0-15
}TsPacketHeader; 

//PAT结构体：节目相关表
typedef struct Tag_TsPat
{
	unsigned char table_id :8 ;                  //固定为0x00 ，标志是该表是PAT
	unsigned char section_syntax_indicator: 1;   //段语法标志位，固定为1
	unsigned char zero : 1;                      //0 
	unsigned char reserved_1 : 2;                //保留位
	unsigned int section_length : 12 ;           //表示这个字节后面有用的字节数，包括CRC32
	unsigned int transport_stream_id : 16 ;      //该传输流的ID，区别于一个网络中其它多路复用的流
	unsigned char reserved_2 : 2;                //保留位
	unsigned char version_number : 5 ;           //范围0-31，表示PAT的版本号
	unsigned char current_next_indicator : 1 ;   //发送的PAT是当前有效还是下一个PAT有效
	unsigned char section_number : 8 ;           //分段的号码。PAT可能分为多段传输，第一段为00，以后每个分段加1，最多可能有256个分段
	unsigned char last_section_number : 8 ;      //最后一个分段的号码
	unsigned int program_number  : 16 ;          //节目号
	unsigned char reserved_3  : 3  ;             //保留位
	unsigned int network_PID :13 ;               //网络信息表（NIT）的PID,节目号为0时对应的PID为network_PID,本例中不含有 networke_pid
	unsigned int program_map_PID : 13;           //节目映射表的PID，节目号大于0时对应的PID，每个节目对应一个
	unsigned int CRC_32  : 32;             //CRC32校验码
}TsPat; 

//PMT结构体：节目映射表
typedef struct Tag_TsPmt
{
	unsigned char table_id  : 8;                 //固定为0x02, 表示PMT表
	unsigned char section_syntax_indicator : 1;  //固定为0x01
	unsigned char zero: 1;                       //0x00
	unsigned char reserved_1 : 2;                //0x03
	unsigned int section_length: 12;             //首先两位bit置为00，它指示段的byte数，由段长度域开始，包含CRC。
	unsigned int program_number : 16;            // 指出该节目对应于可应用的Program map PID
	unsigned char reserved_2: 2;                 //0x03
	unsigned char version_number: 5;             //指出TS流中Program map section的版本号
	unsigned char current_next_indicator: 1;     //当该位置1时，当前传送的Program map section可用；当该位置0时，指示当前传送的Program map section不可用，下一个TS流的Program map section有效。
	unsigned char section_number : 8;            //固定为0x00
	unsigned char last_section_number: 8;        //固定为0x00
	unsigned char reserved_3 : 3;                //0x07
	unsigned int PCR_PID : 13;                   //指明TS包的PID值，该TS包含有PCR域，该PCR值对应于由节目号指定的对应节目。如果对于私有数据流的节目定义与PCR无关，这个域的值将为0x1FFF。
	unsigned char reserved_4 : 4;                //预留为0x0F
	unsigned int program_info_length : 12;       //前两位bit为00。该域指出跟随其后对节目信息的描述的byte数。
	unsigned char stream_type_video : 8;         //指示特定PID的节目元素包的类型。该处PID由elementary PID指定
	unsigned char reserved_5_video: 3;           //0x07
	unsigned int elementary_PID_video: 13;       //该域指示TS包的PID值。这些TS包含有相关的节目元素
	unsigned char reserved_6_video : 4;          //0x0F
	unsigned int ES_info_length_video : 12;      //前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数
	unsigned char stream_type_audio : 8;         //指示特定PID的节目元素包的类型。该处PID由elementary PID指定
	unsigned char reserved_5_audio: 3;           //0x07
	unsigned int elementary_PID_audio: 13;       //该域指示TS包的PID值。这些TS包含有相关的节目元素
	unsigned char reserved_6_audio : 4;          //0x0F
	unsigned int ES_info_length_audio : 12;      //前两位bit为00。该域指示跟随其后的描述相关节目元素的byte数
	unsigned long long CRC_32: 32;               //CRC32校验码
}TsPmt; 

//连续性计数器,也就是说 有多少个 pat包，几个pmt包 ，几个MP3 包，几个 h264包，0x00 - 0x0f ，然后折回到0x00 用于查看丢包                            
typedef struct Tag_Continuity_Counter
{
	unsigned char continuity_counter_pat;
	unsigned char continuity_counter_pmt;
	unsigned char continuity_counter_video;
	unsigned char continuity_counter_audio;
}Continuity_Counter;

//自适应段标志
typedef struct Tag_Ts_Adaptation_field
{
	unsigned char discontinuty_indicator:1;                //1表明当前传送流分组的不连续状态为真
	unsigned char random_access_indicator:1;               //表明下一个有相同PID的PES分组应该含有PTS字段和一个原始流访问点
	unsigned char elementary_stream_priority_indicator:1;  //优先级
	unsigned char PCR_flag:1;                              //包含pcr字段
	unsigned char OPCR_flag:1;                             //包含opcr字段
	unsigned char splicing_point_flag:1;                   //拼接点标志       
	unsigned char transport_private_data_flag:1;           //私用字节
	unsigned char adaptation_field_extension_flag:1;       //调整字段有扩展

	unsigned char adaptation_field_length;                 //自适应段长度
	unsigned long long  pcr;                               //自适应段中用到的的pcr
	unsigned long long  opcr;                              //自适应段中用到的的opcr
	unsigned char splice_countdown;
	unsigned char private_data_len;
	unsigned char private_data [256];
}Ts_Adaptation_field;

//PTS_DTS结构体：本程序设置都有 “11”
typedef struct Tag_TsPtsDts
{
	unsigned char reserved_1 : 4;
	unsigned char pts_32_30  : 3;                //显示时间戳
	unsigned char marker_bit1: 1;
	unsigned int  pts_29_15 : 15;
	unsigned char marker_bit2 : 1;
	unsigned int  pts_14_0 : 15;
	unsigned char marker_bit3 :1 ;
	unsigned char reserved_2 : 4;
	unsigned char dts_32_30: 3;                  //解码时间戳
	unsigned char marker_bit4 :1;
	unsigned int  dts_29_15 :15;
	unsigned char marker_bit5: 1;
	unsigned int  dts_14_0 :15;
	unsigned char marker_bit6 :1 ;
}TsPtsDts;

//PES包结构体，包括PES包头和ES数据 ,头 19 个字节
typedef struct Tag_TsPes
{
	unsigned int   packet_start_code_prefix : 24;//起始：0x000001
	unsigned char  stream_id : 8;                //基本流的类型和编号
	unsigned int   PES_packet_length : 16;       //包长度,就是帧数据的长度，可能为0,要自己算,做多16位，如果超出则需要自己算
	unsigned char  marker_bit:2;                 //必须是：'10'
	unsigned char  PES_scrambling_control:2;     //pes包有效载荷的加扰方式
	unsigned char  PES_priority:1;               //有效负载的优先级
	unsigned char  data_alignment_indicator:1;   //如果设置为1表明PES包的头后面紧跟着视频或音频syncword开始的代码。
	unsigned char  copyright:1;                  //1:靠版权保护，0：不靠
	unsigned char  original_or_copy:1;           //1;有效负载是原始的，0：有效负载时拷贝的
	unsigned char  PTS_DTS_flags:2;              //'10'：PTS字段存在，‘11’：PTD和DTS都存在，‘00’：都没有，‘01’：禁用。
	unsigned char  ESCR_flag:1;                  //1:escr基准字段 和 escr扩展字段均存在，0：无任何escr字段存在
	unsigned char  ES_rate_flag:1;               //1:es_rate字段存在，0 ：不存在
	unsigned char  DSM_trick_mode_flag:1;        //1;8比特特接方式字段存在，0 ：不存在
	unsigned char  additional_copy_info_flag:1;  //1:additional_copy_info存在，0: 不存在
	unsigned char  PES_CRC_flag:1;               //1:crc字段存在，0：不存在
	unsigned char  PES_extension_flag:1;         //1：扩展字段存在，0：不存在
	unsigned char  PES_header_data_length :8;    //后面数据的长度，
	TsPtsDts       tsptsdts;                     //ptsdts结构体对象，10个字节
	unsigned char  Es[MAX_ONE_FRAME_SIZE];       //一帧 原始数据
	unsigned int   Pes_Packet_Length_Beyond;     //如果PES_packet_length的大小不能满足一帧数据的长度则用这个代替
}TsPes;

extern Continuity_Counter continuity_counter;     //包类型计数器

int WriteStruct_Packetheader(unsigned char * Buf , unsigned int PID,unsigned char play_init,unsigned char ada_field_C);
int WriteStruct_Pat(unsigned char * Buf);
int WriteStruct_Pmt(unsigned char * Buf);
