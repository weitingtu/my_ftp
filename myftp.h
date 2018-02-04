#ifndef _MYFTP_H_
#define _MYFTP_H_

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

typedef enum Cmd_e
{
    LIST_REQUEST = 0xA1,
    LIST_REPLY   = 0xA2,
    GET_REQUEST  = 0xB1,
    GET_REPLY    = 0xB2,
    PUT_REQUEST  = 0xC1,
    PUT_REPLY    = 0xC2,
    FILE_DATA    = 0xFF
} Cmd;

struct message_s
{
    unsigned char protocol[5]; /* protocol string (5 bytes) */
    unsigned char type;        /* type (1 byte) */
    unsigned int length;       /* length (header + payload) (4 bytes) */
} __attribute__ ( ( packed ) );

typedef struct message_s Message;

void printCmd ( Message* cmd );
char* createListRequestCmd( Message* cmd );
char* createListReplyCmd( Message* cmd );
void processListReplyCmd( char* buff, int len, Message* cmd );
int parseCmd ( char* buff, int len, Message* cmd );

#endif
