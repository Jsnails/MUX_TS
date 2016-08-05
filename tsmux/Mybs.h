#pragma once

#include "Information.h"

//读取字节结构体
typedef struct Tag_bs_t
{
	unsigned char *p_start;	               //缓冲区首地址(这个开始是最低地址)
	unsigned char *p;			           //缓冲区当前的读写指针 当前字节的地址，这个会不断的++，每次++，进入一个新的字节
	unsigned char *p_end;		           //缓冲区尾地址		//typedef unsigned char   uint8_t;
	int     i_left;				           // p所指字节当前还有多少 “位” 可读写 count number of available(可用的)位 
}bs_t;		


/*
函数名称：
函数功能：初始化结构体
参    数：
返 回 值：无返回值,void类型
思    路：
资    料：
		  
*/
void bs_init( bs_t *s, void *p_data, int i_data );

/*
该函数的作用是：从s中读出i_count位，并将其做为uint32_t类型返回
思路:
	若i_count>0且s流并未结束，则开始或继续读取码流；
	若s当前字节中剩余位数大于等于要读取的位数i_count，则直接读取；
	若s当前字节中剩余位数小于要读取的位数i_count，则读取剩余位，进入s下一字节继续读取。
补充:
	写入s时，i_left表示s当前字节还没被写入的位，若一个新的字节，则i_left=8；
	读取s时，i_left表示s当前字节还没被读取的位，若一个新的字节，则i_left＝8。
	注意两者的区别和联系。

	00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 0000000
	-------- -----000 00000000 ...
	写入s时：i_left = 3
	读取s时：i_left = 5

我思：
	字节流提前放在了结构体bs_s的对象bs_t里了，可能字节流不会一次性读取/分析完，而是根据需要，每次都读取几比特
	bs_s里，有专门的字段用来记录历史读取的结果,每次读取，都会在上次的读取位置上进行
	比如，100字节的流，经过若干次读取，当前位置处于中间一个字节处，前3个比特已经读取过了，此次要读取2比特

	00001001
	000 01 001 (已读过的 本次要读的 以后要读的 )
	i_count = 2	(计划去读2比特)
	i_left  = 5	(还有5比特未读，在本字节中)
	i_shr = s->i_left - i_count = 5 - 2 = 3
	*s->p >> i_shr，就把本次要读的比特移到了字节最右边(未读，但本次不需要的给移到了字节外，抛掉了)
	00000001
	i_mask[i_count] 即i_mask[2] 即0x03:00000011
	( *s->p >> i_shr )&i_mask[i_count]; 即00000001 & 00000011 也就是00000001 按位与 00000011
	结果是：00000001
	i_result |= ( *s->p >> i_shr )&i_mask[i_count];即i_result |=00000001 也就是 i_result =i_result | 00000001 = 00000000 00000000 00000000 00000000 | 00000001 =00000000 00000000 00000000 00000001
	i_result =
	return( i_result ); 返回的i_result是4字节长度的，是unsigned类型 sizeof(unsigned)=4
*/
int bs_read( bs_t *s, int i_count );

/*
函数名称：
函数功能：从s中读出1位，并将其做为uint32_t类型返回。
函数参数：
返 回 值：
思    路：若s流并未结束，则读取一位
资    料：
		毕厚杰：第145页，u(n)/u(v)，读进连续的若干比特，并将它们解释为“无符号整数”
		return i_result;	//unsigned int
*/
int bs_read1( bs_t *s );

/*
函数名称：
函数功能：从s中解码并读出一个语法元素值
参    数：
返 回 值：
思    路：
		从s的当前位读取并计数，直至读取到1为止；
		while( bs_read1( s ) == 0 && s->p < s->p_end && i < 32 )这个循环用i记录了s当前位置到1为止的0的个数，并丢弃读到的第一个1；
		返回2^i-1+bs_read(s,i)。
		例：当s字节中存放的是0001010时，1前有3个0，所以i＝3；
		返回的是：2^i-1+bs_read(s,i)即：8－1＋010＝9
资    料：
		毕厚杰：第145页，ue(v)；无符号指数Golomb熵编码
		x264中bs.h文件部分函数解读 http://wmnmtm.blog.163.com/blog/static/382457142011724101824726/
		无符号整数指数哥伦布码编码 http://wmnmtm.blog.163.com/blog/static/38245714201172623027946/
*/
int bs_read_ue( bs_t *s );

