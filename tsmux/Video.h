#pragma once

#include "Ts.h"
#include "Mybs.h"

#define  MAX_VIDEO_TAG_BUF_SIZE   1024 * 1024
#define  VIDEO_TAG_HEADER_LENGTH  11

extern unsigned int decode_video_done;


//H264һ֡���ݵĽṹ��
typedef struct Tag_NALU_t
{
	unsigned char forbidden_bit;           //! Should always be FALSE
	unsigned char nal_reference_idc;       //! NALU_PRIORITY_xxxx
	unsigned char nal_unit_type;           //! NALU_TYPE_xxxx  
	unsigned int  startcodeprefix_len;      //! ǰ׺�ֽ���
	unsigned int  len;                     //! ����nal ͷ��nal ���ȣ��ӵ�һ��00000001����һ��000000001�ĳ���
	unsigned int  max_size;                //! ����һ��nal �ĳ���
	unsigned char * buf;                   //! ����nal ͷ��nal ����
	unsigned char Frametype;               //! ֡����
	unsigned int  lost_packets;            //! Ԥ��
} NALU_t;

//nal����
enum nal_unit_type_e
{
	NAL_UNKNOWN     = 0,
	NAL_SLICE       = 1,
	NAL_SLICE_DPA   = 2,
	NAL_SLICE_DPB   = 3,
	NAL_SLICE_DPC   = 4,
	NAL_SLICE_IDR   = 5,    /* ref_idc != 0 */
	NAL_SEI         = 6,    /* ref_idc == 0 */
	NAL_SPS         = 7,
	NAL_PPS         = 8
	/* ref_idc == 0 for 6,9,10,11,12 */
};

//֡����
enum Frametype_e
{
	FRAME_I  = 15,
	FRAME_P  = 16,
	FRAME_B  = 17
};


NALU_t *AllocNALU(int buffersize);										  //����nal ��Դ
void FreeNALU(NALU_t * n);												  //�ͷ�nal ��Դ 
int FindStartCode2 (unsigned char *Buf);								  //�ж�nal ǰ׺�Ƿ�Ϊ3���ֽ�
int FindStartCode3 (unsigned char *Buf);								  //�ж�nal ǰ׺�Ƿ�Ϊ4���ֽ�
int GetAnnexbNALU(NALU_t *nalu);										  //��дnal ���ݺ�ͷ
int GetAnnexbNALU(NALU_t *nalu,void *pData,int iDataSize);                //��дnal ���ݺ�ͷ
int GetFrameType(NALU_t * n);											  //��ȡ֡����
int Read_One_H264_Frame(unsigned char * buf, unsigned int * videoframetype);
int Read_One_H264_Frame(unsigned char * buf, unsigned int * videoframetype,void *pData,int iDataSize);
int H2642PES(TsPes * tsh264pes, unsigned long Videopts, unsigned int * videoframetype);
int H2642PES(TsPes * tsh264pes, unsigned long Videopts, unsigned int * videoframetype, unsigned char*pData, int iDataSize);
