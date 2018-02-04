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
#include "myftp.h"

#define IPADDR "127.0.0.1"
#define PORT 12345

int recvMsg( int sd, char* buff, int len )
{
    int recvLen = 0;
    while ( recvLen != len )
    {
        int rLen = recv( sd, buff + recvLen, len - recvLen, 0 );
        if ( rLen <= 0 )
        {
            fprintf( stderr, "error recving msg\n" );
            return 1;
        }
        recvLen += rLen;
    }
    return 0;
}

int sendMsg( int sd, char* buff, int len )
{
    int recvLen = 0;
    while ( recvLen != len )
    {
        int rLen = send( sd, buff + recvLen, len - recvLen, 0 );
        if ( rLen <= 0 )
        {
            fprintf( stderr, "error sending msg\n" );
            return 1;
        }
        recvLen += rLen;
    }
    return 0;
}

int sendCmdMsg( int sd, char* buff, int len )
{
    int* msg_len = ( int* )calloc( sizeof( int ), 1 );
    *msg_len = len;

    if ( sendMsg( sd, ( char* )msg_len, sizeof( int ) ) == 1 )
    {
        fprintf( stderr, "send error, exit\n" );
        exit( 0 );
    }
    if ( sendMsg( sd, buff, *msg_len ) == 1 )
    {
        fprintf( stderr, "send error, exit\n" );
        exit( 0 );
    }
    free( msg_len );
    return 0;
}

int sendCmd( int sd )
{
    Message cmd;
    char* buff = createListRequestCmd( &cmd );

    sendCmdMsg( sd, buff, cmd.length );

    free( buff );
}

/*
 * Worker thread performing receiving and outputing messages
 */
void* pthread_prog( void* sDescriptor )
{
    int sd = *( int* )sDescriptor;
    int* len = ( int* )calloc( sizeof( int ), 1 );
    while ( 1 )
    {
        if ( recvMsg( sd, ( char* )len, sizeof( int ) ) == 1 )
        {
            fprintf( stderr, "error receiving, exit!\n" );
            exit( 0 );
        }
        char* buff = ( char* )calloc( sizeof( char ), *len + 1 );
        if ( recvMsg( sd, buff, *len ) == 1 )
        {
            fprintf( stderr, "error receiving, exit!\n" );
            exit( 0 );
        }
        Message cmd;
        if ( parseCmd( buff, *len, &cmd ) == 0 )
        {
            switch ( cmd.type )
            {
            case LIST_REQUEST:
            {
                Message reply_cmd;
                char* cmd_buff = createListReplyCmd( &reply_cmd );
                if ( sendCmdMsg( sd, cmd_buff, reply_cmd.length ) == 1 )
                {
                    exit( 1 );
                }
                free( cmd_buff );
            }
            break;
            case LIST_REPLY:
                processListReplyCmd( buff, *len, &cmd );
                break;
            case GET_REQUEST:
                break;
            case GET_REPLY:
                break;
            case PUT_REQUEST:
                break;
            case PUT_REPLY:
                break;
            case FILE_DATA:
                break;
            default:
                break;
            }
        }
        else
        {
            printf( "recv'd msg: %s\n", buff );
        }
    }
    free( len );
}

int main( int argc, char** argv )
{
    if ( 4 != argc && 5 != argc )
    {
        printf( "USAGE: %s <server ip addr> <server port> <list|get|put> <file>\n", argv[0] );
        exit( 0 );
    }

    int port = atoi( argv[2] );
    if ( 0 == port )
    {
        fprintf( stderr, "opt error: wrong port number %s\n", argv[2] );
        exit( 0 );
    }

    if ( 0 != strcmp( argv[3], "list" )
            && 0 != strcmp( argv[3], "get" )
            && 0 != strcmp( argv[3], "put" ) )
    {
        fprintf( stderr, "opt error: wrong command %s\n", argv[3] );
        exit( 0 );
    }

    if ( ( 0 == strcmp( argv[3], "get" ) || 0 == strcmp( argv[3], "put" ) ) && argc != 5 )
    {
        fprintf( stderr, "opt error: missing %s file\n", argv[3] );
        exit( 0 );
    }

    int sd = socket( AF_INET, SOCK_STREAM, 0 );
    struct sockaddr_in server_addr;
    memset( &server_addr, 0, sizeof( server_addr ) );
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr( argv[1] );
    server_addr.sin_port = htons( port );
    if ( connect( sd, ( struct sockaddr* )&server_addr, sizeof( server_addr ) ) < 0 )
    {
        fprintf( stderr, "connection error: %s (Errno:%d)\n", strerror( errno ), errno );
        exit( 0 );
    }
    {
//    pthread_t worker;
//        pthread_create( &worker, NULL, pthread_prog, &sd );
//        char buff[100];
//        while ( 1 )
//        {
//            memset( buff, 0, 100 );
//            scanf( "%s", buff );
//            int* msgLen = ( int* )calloc( sizeof( int ), 1 );
//            *msgLen = strlen( buff );
//            if ( sendMsg( sd, ( char* )msgLen, sizeof( int ) ) == 1 )
//            {
//                fprintf( stderr, "send error, exit\n" );
//                exit( 0 );
//            }
//            if ( sendMsg( sd, buff, *msgLen ) == 1 )
//            {
//                fprintf( stderr, "send error, exit\n" );
//                exit( 0 );
//            }
//            if ( sendCmd( sd ) == 1 )
//            {
//                fprintf( stderr, "send error, exit\n" );
//                exit( 0 );
//            }
//            free( msgLen );
//        }
    }

    if ( 0 == strcmp( argv[3], "list" ) )
    {
        Message cmd;
        char* buff = createListRequestCmd( &cmd );
        if ( sendCmdMsg( sd, buff, cmd.length ) == 1 )
        {
            exit( 1 );
        }
        free( buff );
    }
    else
    {
        fprintf( stderr, "command error: command %s is not supported\n", argv[3] );
        exit( 0 );
    }

    int* msg_len = ( int* )calloc( sizeof( int ), 1 );
    if ( recvMsg( sd, ( char* )msg_len, sizeof( int ) ) == 1 )
    {
        fprintf( stderr, "error receiving, exit!\n" );
        exit( 0 );
    }
    char* buff = ( char* )calloc( sizeof( char ), *msg_len + 1 );
    if ( recvMsg( sd, buff, *msg_len ) == 1 )
    {
        fprintf( stderr, "error receiving, exit!\n" );
        exit( 0 );
    }

    Message cmd;
    if ( parseCmd( buff, *msg_len, &cmd ) != 0 )
    {
        printf( "recv'd msg: %s\n", buff );
    }
    else
    {
        switch ( cmd.type )
        {
        case LIST_REQUEST:
            break;
        case LIST_REPLY:
            processListReplyCmd( buff, *msg_len, &cmd );
            break;
        case GET_REQUEST:
            break;
        case GET_REPLY:
            break;
        case PUT_REQUEST:
            break;
        case PUT_REPLY:
            break;
        case FILE_DATA:
            break;
        default:
            break;
        }
    }
    free( msg_len );

    return 0;
}

