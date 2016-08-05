#include "Video.h"

unsigned int decode_video_done = 0;

NALU_t *AllocNALU(int buffersize)
{
	NALU_t *n;

	if ((n = (NALU_t*)calloc (1, sizeof(NALU_t))) == NULL)
	{
		printf("AllocNALU Error: Allocate Meory To NALU_t Failed ");
		getchar();
	}

	n->max_size = buffersize;									//Assign buffer size 

	if ((n->buf = (unsigned char*)calloc (buffersize, sizeof (char))) == NULL)
	{
		free (n);
		printf ("AllocNALU Error: Allocate Meory To NALU_t Buffer Failed ");
		getchar();
	}
	return n;
}

void FreeNALU(NALU_t *n)
{
	if (n)
	{
		if (n->buf)
		{
			free(n->buf);
			n->buf=NULL;
		}
		free (n);
	}
}

int FindStartCode2 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=1)               //Check whether buf is 0x000001
	{
		return 0;
	}
	else 
	{
		return 1;
	}
}

int FindStartCode3 (unsigned char *Buf)
{
	if(Buf[0]!=0 || Buf[1]!=0 || Buf[2] !=0 || Buf[3] !=1)  //Check whether buf is 0x00000001
	{
		return 0;
	}
	else 
	{
		return 1;
	}
}

int GetAnnexbNALU (NALU_t * nalu)
{
	int pos = 0;                  //һ��nal����һ��nal �����ƶ���ָ��
	int StartCodeFound  = 0;      //�Ƿ��ҵ���һ��nal ��ǰ׺
	int rewind = 0;               //�ж� ǰ׺��ռ�ֽ��� 3�� 4
	unsigned char * Buf = NULL;
	static int info2 =0 ;
	static int info3 =0 ;

	if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL) 
	{
		printf ("GetAnnexbNALU Error: Could not allocate Buf memory\n");
	}

	nalu->startcodeprefix_len = 3;      //��ʼ��ǰ׺λ�����ֽ�

	if (3 != fread (Buf, 1, 3, pVideo_H264_File))//���ļ���ȡ�����ֽڵ�buf
	{
		free(Buf);
		return 0;
	}
	info2 = FindStartCode2 (Buf);       //Check whether Buf is 0x000001
	if(info2 != 1) 
	{
		//If Buf is not 0x000001,then read one more byte
		if(1 != fread(Buf + 3, 1, 1, pVideo_H264_File))
		{
			free(Buf);
			return 0;
		}
		info3 = FindStartCode3 (Buf);   //Check whether Buf is 0x00000001
		if (info3 != 1)                 //If not the return -1
		{ 
			free(Buf);
			return -1;
		}
		else 
		{
			//If Buf is 0x00000001,set the prefix length to 4 bytes
			pos = 4;
			nalu->startcodeprefix_len = 4;
		}
	} 
	else
	{
		//If Buf is 0x000001,set the prefix length to 3 bytes
		pos = 3;
		nalu->startcodeprefix_len = 3;
	}
	//Ѱ����һ���ַ�����λ�� �� Ѱ��һ��nal ��һ��0000001 ����һ��00000001
	StartCodeFound = 0;
	info2 = 0;
	info3 = 0;
	while (!StartCodeFound)
	{
		if (feof (pVideo_H264_File))                                 //��������ļ���β
		{
			nalu->len = (pos-1) - nalu->startcodeprefix_len;  //��0 ��ʼ
			memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);     
			nalu->forbidden_bit = nalu->buf[0] & 0x80;      // 1 bit--10000000
			nalu->nal_reference_idc = nalu->buf[0] & 0x60;  // 2 bit--01100000
			nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;    // 5 bit--00011111
			free(Buf);
			return ((info3 == 1)? 4 : 3);
		}
		Buf[pos++] = fgetc (pVideo_H264_File);                       //Read one char to the Buffer һ���ֽ�һ���ֽڴ��ļ������
		info3 = FindStartCode3(&Buf[pos-4]);		        //Check whether Buf is 0x00000001 
		if(info3 != 1)
		{
			info2 = FindStartCode2(&Buf[pos-3]);            //Check whether Buf is 0x000001
		}
		StartCodeFound = (info2 == 1 || info3 == 1);        //����ҵ���һ��ǰ׺
	}

	rewind = (info3 == 1)? -4 : -3;

	if (0 != fseek (pVideo_H264_File, rewind, SEEK_CUR))			    //���ļ��ڲ�ָ���ƶ��� nal ��ĩβ
	{
		free(Buf);
		printf("GetAnnexbNALU Error: Cannot fseek in the bit stream file");
	}

	nalu->len = (pos + rewind) -  nalu->startcodeprefix_len;       //���ð���nal ͷ�����ݳ���
	memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);//����һ��nal ���ݵ�������
	nalu->forbidden_bit = nalu->buf[0] & 0x80;                     //1 bit  ����nal ͷ
	nalu->nal_reference_idc = nalu->buf[0] & 0x60;                 // 2 bit
	nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;                   // 5 bit
	free(Buf);
	return ((info3 == 1)? 4 : 3);                                               
}

int GetAnnexbNALU(NALU_t *nalu, void *pData, int iDataSize)
{
	int pos = iDataSize;                  //һ��nal����һ��nal �����ƶ���ָ��
	int StartCodeFound = 0;               //�Ƿ��ҵ���һ��nal ��ǰ׺
	int rewind = 0;                       //ǰ׺��ռ�ֽ��� 3�� 4
	unsigned char * Buf = NULL;
	static int info2 = 0;
	static int info3 = 0;

	if ((Buf = (unsigned char*)calloc(nalu->max_size, sizeof(char))) == NULL)
	{
		printf("GetAnnexbNALU Error: Could not allocate Buf memory\n");
	}
	memcpy(Buf, pData, iDataSize);

	nalu->startcodeprefix_len = 4;								  //��ʼ��ǰ׺λ�����ֽ�
	nalu->len = pos- nalu->startcodeprefix_len;					  //���ð���nal ͷ�����ݳ���
	memcpy(nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);//����һ��nal ���ݵ�������
	nalu->forbidden_bit = nalu->buf[0] & 0x80;                     //1 bit  ����nal ͷ
	nalu->nal_reference_idc = nalu->buf[0] & 0x60;                 // 2 bit
	nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;                   // 5 bit
	free(Buf);
	return  4;//ͷ4�ֽ�
}

int GetFrameType(NALU_t * nal)
{
	bs_t s;
	int frame_type = 0; 
	unsigned char * OneFrameBuf_H264 = NULL ;
	if ((OneFrameBuf_H264 = (unsigned char *)calloc(nal->len + 4,sizeof(unsigned char))) == NULL)
	{
		printf("Error malloc OneFrameBuf_H264\n");
		return getchar();
	}
	if (nal->startcodeprefix_len == 3)
	{
		OneFrameBuf_H264[0] = 0x00;
		OneFrameBuf_H264[1] = 0x00;
		OneFrameBuf_H264[2] = 0x01;
		memcpy(OneFrameBuf_H264 + 3,nal->buf,nal->len);
	}
	else if (nal->startcodeprefix_len == 4)
	{
		OneFrameBuf_H264[0] = 0x00;
		OneFrameBuf_H264[1] = 0x00;
		OneFrameBuf_H264[2] = 0x00;
		OneFrameBuf_H264[3] = 0x01;
		memcpy(OneFrameBuf_H264 + 4,nal->buf,nal->len);
	}
	else
	{
		printf("H264��ȡ����\n");
	}

	bs_init( &s,OneFrameBuf_H264 + nal->startcodeprefix_len + 1  ,nal->len - 1 );


	if (nal->nal_unit_type == NAL_SLICE || nal->nal_unit_type ==  NAL_SLICE_IDR )
	{
		/* i_first_mb */
		bs_read_ue( &s );
		/* picture type */
		frame_type =  bs_read_ue( &s );
		switch(frame_type)
		{
		case 0: case 5: /* P */
			nal->Frametype = FRAME_P;
			break;
		case 1: case 6: /* B */
			nal->Frametype = FRAME_B;
			break;
		case 3: case 8: /* SP */
			nal->Frametype = FRAME_P;
			break;
		case 2: case 7: /* I */
			nal->Frametype = FRAME_I;
			break;
		case 4: case 9: /* SI */
			nal->Frametype = FRAME_I;
			break;
		}
	}
	else if (nal->nal_unit_type == NAL_SEI)
	{
		nal->Frametype = NAL_SEI;
	}
	else if(nal->nal_unit_type == NAL_SPS)
	{
		nal->Frametype = NAL_SPS;
	}
	else if(nal->nal_unit_type == NAL_PPS)
	{
		nal->Frametype = NAL_PPS;
	}
	if (OneFrameBuf_H264)
	{
		free(OneFrameBuf_H264);
		OneFrameBuf_H264 = NULL;
	}
	return 1;
}

int Read_One_H264_Frame(unsigned char * buf,unsigned int * videoframetype)
{
	NALU_t * n = NULL;
	unsigned int video_buf_size = 0;

	//����nal ��Դ
	n = AllocNALU(MAX_VIDEO_TAG_BUF_SIZE); 

	//��ȡһ֡����
	int startcodeprefix_size = GetAnnexbNALU(n);
	if (startcodeprefix_size == 0)
	{
		decode_video_done = 1;
		return 0;
	}
	//�ж�֡����
	GetFrameType(n);
	*videoframetype = n->Frametype;

	if (n->startcodeprefix_len == 3)
	{
		buf[0] = 0x00;
		buf[1] = 0x00;
		buf[2] = 0x01;
		memcpy(buf + 3,n->buf,n->len);
	}
	else if (n->startcodeprefix_len == 4)
	{
		buf[0] = 0x00;
		buf[1] = 0x00;
		buf[2] = 0x00;
		buf[3] = 0x01;
		memcpy(buf + 4,n->buf,n->len);
	}
	else
	{
		printf("H264��ȡ����\n");
		getchar();
	}

	video_buf_size = n->startcodeprefix_len + n->len;

	FreeNALU(n);                                                   //�ͷ�nal ��Դ 
	return video_buf_size;
}

int Read_One_H264_Frame(unsigned char * buf, unsigned int * videoframetype, void *pData, int iDataSize)
{
	NALU_t * n = NULL;
	unsigned int video_buf_size = 0;

	//����nal ��Դ
	n = AllocNALU(MAX_VIDEO_TAG_BUF_SIZE);

	//��ȡһ֡����
	int startcodeprefix_size = GetAnnexbNALU(n,pData,iDataSize);
	if (startcodeprefix_size == 0)
	{
		decode_video_done = 1;
		return 0;
	}
	//�ж�֡����
	GetFrameType(n);
	*videoframetype = n->Frametype;

	if (n->startcodeprefix_len == 3)
	{
		buf[0] = 0x00;
		buf[1] = 0x00;
		buf[2] = 0x01;
		memcpy(buf + 3, n->buf, n->len);
	}
	else if (n->startcodeprefix_len == 4)
	{
		buf[0] = 0x00;
		buf[1] = 0x00;
		buf[2] = 0x00;
		buf[3] = 0x01;
		memcpy(buf + 4, n->buf, n->len);
	}
	else
	{
		printf("H264��ȡ����\n");
		getchar();
	}

	video_buf_size = n->startcodeprefix_len + n->len;

	FreeNALU(n);                                                   //�ͷ�nal ��Դ 
	return video_buf_size;
}

int H2642PES(TsPes * tsh264pes, unsigned long Videopts, unsigned int * videoframetype)
{
	unsigned int h264pes_pos = 0;
	unsigned int OneFrameLen_H264 = 0;


	memset(tsh264pes->Es, 0, MAX_ONE_FRAME_SIZE);
	//��ȡ��һ֡����
	OneFrameLen_H264 = Read_One_H264_Frame(tsh264pes->Es,videoframetype);
	//������ȡ����

	h264pes_pos += OneFrameLen_H264;


	tsh264pes->packet_start_code_prefix = 0x000001;
	tsh264pes->stream_id = TS_H264_STREAM_ID;                               //E0~EF��ʾ����Ƶ��,C0~DF����Ƶ,H264-- E0
	tsh264pes->PES_packet_length = 0;                                      //һ֡���ݵĳ��� ������ PES��ͷ ,���8 �� ����Ӧ�ĳ���,��0 �����Զ�����
	tsh264pes->Pes_Packet_Length_Beyond = OneFrameLen_H264;

	if (OneFrameLen_H264 > 0xFFFF)                                          //���һ֡���ݵĴ�С��������
	{
		tsh264pes->PES_packet_length = 0x00;
		tsh264pes->Pes_Packet_Length_Beyond = OneFrameLen_H264;
		h264pes_pos += 16;
	}
	else
	{
		tsh264pes->PES_packet_length = 0x00;
		tsh264pes->Pes_Packet_Length_Beyond = OneFrameLen_H264;
		h264pes_pos += 14;
	}
	tsh264pes->marker_bit = 0x02;
	tsh264pes->PES_scrambling_control = 0x00;                               //��ѡ�ֶ� ���ڣ�������
	tsh264pes->PES_priority = 0x00;
	tsh264pes->data_alignment_indicator = 0x00;
	tsh264pes->copyright = 0x00;
	tsh264pes->original_or_copy = 0x00;
	tsh264pes->PTS_DTS_flags = 0x02;                                         //10'��PTS�ֶδ���,DTS������
	tsh264pes->ESCR_flag = 0x00;
	tsh264pes->ES_rate_flag = 0x00;
	tsh264pes->DSM_trick_mode_flag = 0x00;
	tsh264pes->additional_copy_info_flag = 0x00;
	tsh264pes->PES_CRC_flag = 0x00;
	tsh264pes->PES_extension_flag = 0x00;
	tsh264pes->PES_header_data_length = 0x05;                                //��������� ������PTS��ռ���ֽ���

	//�� 0 
	tsh264pes->tsptsdts.pts_32_30 = 0;
	tsh264pes->tsptsdts.pts_29_15 = 0;
	tsh264pes->tsptsdts.pts_14_0 = 0;

	tsh264pes->tsptsdts.reserved_1 = 0x0003;                                 //��д pts��Ϣ
	// Videopts����30bit��ʹ�������λ 
	if (Videopts > 0x7FFFFFFF)
	{
		tsh264pes->tsptsdts.pts_32_30 = (Videopts >> 30) & 0x07;
		tsh264pes->tsptsdts.marker_bit1 = 0x01;
	}
	else
	{
		tsh264pes->tsptsdts.marker_bit1 = 0;
	}
	// Videopts����15bit��ʹ�ø����λ���洢
	if (Videopts > 0x7FFF)
	{
		tsh264pes->tsptsdts.pts_29_15 = (Videopts >> 15) & 0x007FFF;
		tsh264pes->tsptsdts.marker_bit2 = 0x01;
	}
	else
	{
		tsh264pes->tsptsdts.marker_bit2 = 0;
	}
	//ʹ�����15λ
	tsh264pes->tsptsdts.pts_14_0 = Videopts & 0x007FFF;
	tsh264pes->tsptsdts.marker_bit3 = 0x01;

	return h264pes_pos;
}

int H2642PES(TsPes * tsh264pes,unsigned long Videopts,unsigned int * videoframetype,unsigned char*pData,int iDataSize)
{
	unsigned int h264pes_pos = 0;
	unsigned int OneFrameLen_H264 = 0;


	memset(tsh264pes->Es, 0, MAX_ONE_FRAME_SIZE);
    //��ȡ��һ֡����
	OneFrameLen_H264 = Read_One_H264_Frame(tsh264pes->Es,videoframetype,pData,iDataSize);
	//������ȡ����

	h264pes_pos += OneFrameLen_H264;


	tsh264pes->packet_start_code_prefix = 0x000001;
	tsh264pes->stream_id = TS_H264_STREAM_ID;                               //E0~EF��ʾ����Ƶ��,C0~DF����Ƶ,H264-- E0
	tsh264pes->PES_packet_length = 0 ;                                      //һ֡���ݵĳ��� ������ PES��ͷ ,���8 �� ����Ӧ�ĳ���,��0 �����Զ�����
	tsh264pes->Pes_Packet_Length_Beyond = OneFrameLen_H264;

	if (OneFrameLen_H264 > 0xFFFF)                                          //���һ֡���ݵĴ�С��������
	{
		tsh264pes->PES_packet_length = 0x00;
		tsh264pes->Pes_Packet_Length_Beyond = OneFrameLen_H264;
		h264pes_pos += 16;
	}
	else
	{
		tsh264pes->PES_packet_length = 0x00;
		tsh264pes->Pes_Packet_Length_Beyond = OneFrameLen_H264;
		h264pes_pos += 14;
	}
	tsh264pes->marker_bit = 0x02;
	tsh264pes->PES_scrambling_control = 0x00;                               //��ѡ�ֶ� ���ڣ�������
	tsh264pes->PES_priority = 0x00;
	tsh264pes->data_alignment_indicator = 0x00;
	tsh264pes->copyright = 0x00;
	tsh264pes->original_or_copy = 0x00;
	tsh264pes->PTS_DTS_flags = 0x02;                                         //10'��PTS�ֶδ���,DTS������
	tsh264pes->ESCR_flag = 0x00;
	tsh264pes->ES_rate_flag = 0x00;
	tsh264pes->DSM_trick_mode_flag = 0x00;
	tsh264pes->additional_copy_info_flag = 0x00;
	tsh264pes->PES_CRC_flag = 0x00;
	tsh264pes->PES_extension_flag = 0x00;
	tsh264pes->PES_header_data_length = 0x05;                                //��������� ������PTS��ռ���ֽ���

	//�� 0 
	tsh264pes->tsptsdts.pts_32_30  = 0;
	tsh264pes->tsptsdts.pts_29_15 = 0;
	tsh264pes->tsptsdts.pts_14_0 = 0;

	tsh264pes->tsptsdts.reserved_1 = 0x0003;                                 //��д pts��Ϣ
	// Videopts����30bit��ʹ�������λ 
	if(Videopts > 0x7FFFFFFF)
	{
		tsh264pes->tsptsdts.pts_32_30 = (Videopts >> 30) & 0x07;                 
		tsh264pes->tsptsdts.marker_bit1 = 0x01;
	}
	else 
	{
		tsh264pes->tsptsdts.marker_bit1 = 0;
	}
	// Videopts����15bit��ʹ�ø����λ���洢
	if(Videopts > 0x7FFF)
	{
		tsh264pes->tsptsdts.pts_29_15 = (Videopts >> 15) & 0x007FFF ;
		tsh264pes->tsptsdts.marker_bit2 = 0x01;
	}
	else
	{
		tsh264pes->tsptsdts.marker_bit2 = 0;
	}
	//ʹ�����15λ
	tsh264pes->tsptsdts.pts_14_0 = Videopts & 0x007FFF;
	tsh264pes->tsptsdts.marker_bit3 = 0x01;

	return h264pes_pos;
}

