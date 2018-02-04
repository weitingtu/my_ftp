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

int recvCmdMsg( int sd, char** buff, int* len, Message* cmd )
{
    int* msg_len = ( int* )calloc( sizeof( int ), 1 );
    if ( recvMsg( sd, ( char* )msg_len, sizeof( int ) ) == 1 )
    {
        fprintf( stderr, "error receiving, exit!\n" );
        exit( 0 );
    }
    *buff = ( char* )calloc( sizeof( char ), *msg_len + 1 );
    if ( recvMsg( sd, *buff, *msg_len ) == 1 )
    {
        fprintf( stderr, "error receiving, exit!\n" );
        exit( 0 );
    }

    int res = parseCmd( *buff, *msg_len, cmd );
    free( msg_len );

    if ( res != 0 )
    {
        printf( "recv'd msg: %s\n", buff );
        return 1;
    }
    return 0;
}

int _listCmd( int sd )
{
    {
        Message cmd;
        char* buff = createListRequestCmd( &cmd );
        if ( sendCmdMsg( sd, buff, cmd.length ) == 1 )
        {
            exit( 1 );
        }
        free( buff );
    }

    char* buff = NULL;
    Message cmd;
    int len;
    recvCmdMsg( sd, &buff, &len, &cmd );

    if ( LIST_REPLY == cmd.type )
    {
        processListReplyCmd( buff );
    }
    else
    {
        printf( "wrong cmd\n" );
        printCmd( &cmd );
    }

    return 0;
}

int _getCmd( int sd, char* file_name )
{
    {
        Message cmd;
        char* buff = createGetRequestCmd( &cmd, file_name );
        if ( sendCmdMsg( sd, buff, cmd.length ) == 1 )
        {
            exit( 1 );
        }
        free( buff );
    }

    char* buff = NULL;
    Message cmd;
    int len = 0;
    recvCmdMsg( sd, &buff, &len, &cmd );
    free( buff );

    if ( GET_REPLY_NON_EXIST == cmd.type )
    {
        printf( "file %s doesn't exist\n", file_name );
        return 0;
    }
    else if ( GET_REPLY_EXIST != cmd.type )
    {
        printf( "wrong cmd\n" );
        printCmd( &cmd );
        return 1;
    }

    printf( "Receiving file %s\n", file_name );

    buff = NULL;
    len = 0;
    recvCmdMsg( sd, &buff, &len, &cmd );
    FILE* fp = fopen( file_name, "w" );
    if ( NULL == fp )
    {
        fprintf( stderr, "failed to open %f\n", file_name );
        free( buff );
        return 1;
    }
    fwrite( buff + sizeof( Message ), sizeof( char ), cmd.length - sizeof( Message ), fp );
    fclose( fp );
    free( buff );
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

    if ( 0 == strcmp( argv[3], "list" ) )
    {
        _listCmd( sd );
    }
    else if ( 0 == strcmp( argv[3], "get" ) )
    {
        _getCmd( sd, argv[4] );
    }
    else
    {
        fprintf( stderr, "command error: command %s is not supported\n", argv[3] );
        exit( 0 );
    }

    return 0;
}

