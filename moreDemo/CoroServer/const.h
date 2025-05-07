#pragma once

#define MAX_LENGTH      2 * 1024
#define HEAD_TOTAL_LEN  4
#define HEAD_ID_LEN     2
#define HEAD_DATA_LEN   2
#define MAX_RECV_QUEUE  10000
#define MAX_SEND_QUEUE  1000    //发送队列

//消息ID
enum MSG_IDS
{
	MSG_HELLO_WORD = 1001
};