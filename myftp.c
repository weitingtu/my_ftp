#include "myftp.h"
#include <dirent.h>

int recvMsg( int sd, char* buff, int len )
{
    int recvLen = 0;
    while ( recvLen != len )
    {
        int rLen = recv( sd, buff + recvLen, len - recvLen, 0 );
        if ( rLen < 0 )
        {
            fprintf( stderr, "error recving msg\n" );
            return 1;
        }
        if ( rLen == 0 )
        {
            printf( "stop recving msg\n" );
            return 2;
        }
        recvLen += rLen;
    }
//    int i = 0;
//    for ( i = 0; i < len - 1; i += 2 )
//    {
//        *( uint16_t* )( buff + i ) = ntohs( *( uint16_t* )( buff + i ) );
//    }
    return 0;
}

int sendMsg( int sd, char* buff, int len )
{
//    int i = 0;
//    for ( i = 0; i < len - 1; i += 2 )
//    {
//        *( uint16_t* )( buff + i ) = htons( *( uint16_t* )( buff + i ) );
//    }
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
    *( uint32_t* )( msg_len ) = htonl( *( uint32_t* )( msg_len ) );

    if ( sendMsg( sd, ( char* )msg_len, sizeof( int ) ) == 1 )
    {
        fprintf( stderr, "send error, exit\n" );
        free( msg_len );
        return 1;
    }
    free( msg_len );
    if ( sendMsg( sd, buff, len ) == 1 )
    {
        fprintf( stderr, "send error, exit\n" );
        return 1;
    }
    return 0;
}

int recvCmdMsg( int sd, char** buff, int* len, Message* cmd )
{
    int* msg_len = ( int* )calloc( sizeof( int ), 1 );

    int res = recvMsg( sd, ( char* )msg_len, sizeof( int ) );
    if ( 1 == res )
    {
        fprintf( stderr, "error receiving, exit!\n" );
        return res;
    }
    else if ( 2 == res )
    {
        printf( "stop recving, exit!\n" );
        return res;
    }
    *( uint32_t* )( msg_len ) = ntohl( *( uint32_t* )( msg_len ) );
    *len = *msg_len;

    *buff = ( char* )calloc( sizeof( char ), *msg_len + 1 );
    res = recvMsg( sd, *buff, *msg_len );
    if ( 1 == res )
    {
        fprintf( stderr, "error receiving, exit!\n" );
        free( msg_len );
        free( *buff );
        *buff = NULL;
        return res;
    }
    else if ( 2 == res )
    {
        printf( "stop recving, exit!\n" );
        free( msg_len );
        free( *buff );
        *buff = NULL;
        return res;
    }

    res = parseCmd( *buff, *msg_len, cmd );
    free( msg_len );

    if ( res != 0 )
    {
        printf( "recv'd msg: %s\n", buff );
        free( *buff );
        *buff = NULL;
        return 3;
    }
    return 0;
}

void printCmd ( Message* cmd )
{
    printf( "protocal: " );
    int i = 0;
    for ( i = 0; i < 5; ++i )
    {
        printf( "%c", cmd->protocol[i] );
    }
    printf( "\n" );
    printf( "type:     %x\n", cmd->type );
    printf( "length:   %u\n", cmd->length );
}

Message _createSendCmd( Message* cmd )
{
    Message s = *cmd;
    s.length = htonl( s.length );
    return s;
}

char* createListRequestCmd( Message* cmd )
{
    strcpy( cmd->protocol, "myftp" );
    cmd->type = LIST_REQUEST;
    cmd->length = sizeof( Message );

    char* buff = malloc( sizeof( char ) * cmd->length );
    Message send_cmd = _createSendCmd( cmd );
    memcpy( buff, &send_cmd, cmd->length );
    return buff;
}

char* createListReplyCmd( Message* cmd, char* file_str )
{
    strcpy( cmd->protocol, "myftp" );
    cmd->type = LIST_REPLY;
    cmd->length = sizeof( Message ) + strlen( file_str ) + 1;

    char* buff = malloc( sizeof( char ) * cmd->length );
    Message send_cmd = _createSendCmd( cmd );
    memcpy( buff, &send_cmd, sizeof( Message ) );
    memcpy( buff + sizeof( Message ), file_str, strlen( file_str ) + 1 );
    return buff;
}

char* createGetRequestCmd( Message* cmd, char* file_name )
{
    strcpy( cmd->protocol, "myftp" );
    cmd->type = GET_REQUEST;
    cmd->length = sizeof( Message ) + strlen( file_name ) + 1;

    char* buff = malloc( sizeof( char ) * cmd->length );
    Message send_cmd = _createSendCmd( cmd );
    memcpy( buff, &send_cmd, sizeof( Message ) );
    memcpy( buff + sizeof( Message ), file_name, strlen( file_name ) + 1 );
    return buff;
}

char* createGetReplyCmd( Message* cmd, int file_exist )
{
    strcpy( cmd->protocol, "myftp" );
    cmd->type = file_exist == 0 ? GET_REPLY_EXIST : GET_REPLY_NON_EXIST;
    cmd->length = sizeof( Message );

    char* buff = malloc( sizeof( char ) * cmd->length );
    Message send_cmd = _createSendCmd( cmd );
    memcpy( buff, &send_cmd, cmd->length );
    return buff;
}

char* createPutRequestCmd( Message* cmd, char* file_name )
{
    strcpy( cmd->protocol, "myftp" );
    cmd->type = PUT_REQUEST;
    cmd->length = sizeof( Message ) + strlen( file_name ) + 1;

    char* buff = malloc( sizeof( char ) * cmd->length );
    Message send_cmd = _createSendCmd( cmd );
    memcpy( buff, &send_cmd, sizeof( Message ) );
    memcpy( buff + sizeof( Message ), file_name, strlen( file_name ) + 1 );
    return buff;
}

char* createPutReplyCmd( Message* cmd )
{
    strcpy( cmd->protocol, "myftp" );
    cmd->type = PUT_REPLY;
    cmd->length = sizeof( Message );

    char* buff = malloc( sizeof( char ) * cmd->length );
    Message send_cmd = _createSendCmd( cmd );
    memcpy( buff, &send_cmd, cmd->length );
    return buff;
}

char* createFileDataCmd( Message* cmd, char* file_name )
{
    strcpy( cmd->protocol, "myftp" );
    cmd->type = FILE_DATA;
    cmd->length = sizeof( Message );

    FILE* fp = fopen( file_name, "r" );
    if ( NULL == fp )
    {
        fprintf( stderr, "failed to open %f\n", file_name );

        char* buff = malloc( sizeof( char ) * cmd->length );
        memcpy( buff, cmd, sizeof( Message ) );
        return buff;
    }

    fseek( fp, 0, SEEK_END );
    long f_size = ftell( fp );
    fseek( fp, 0, SEEK_SET );

    cmd->length = cmd->length + f_size;
    char* buff = malloc( sizeof( char ) * cmd->length + 1 );
    Message send_cmd = _createSendCmd( cmd );
    memcpy( buff, &send_cmd, sizeof( Message ) );
    fread( buff + sizeof( Message ), f_size, 1, fp );

    fclose( fp );

    return buff;
}

int parseCmd ( char* buff, int len, Message* cmd )
{
    if ( len < sizeof( Message ) )
    {
        return 1;
    }
    memcpy( cmd, buff, sizeof( Message ) );
    cmd->length = ntohl( cmd->length );
    printCmd( cmd );
    if ( cmd->protocol[0] != 'm'
            || cmd->protocol[1] != 'y'
            || cmd->protocol[2] != 'f'
            || cmd->protocol[3] != 't'
            || cmd->protocol[4] != 'p'
       )
    {
        return 1;
    }
    return 0;
}
