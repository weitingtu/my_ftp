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

#define PORT 12345

char* _getFileName( char* buff )
{
    char* file_name = get_data_dir_path();
    file_name = realloc( file_name, strlen( file_name ) + strlen( buff + sizeof( Message ) ) );
    strcat( file_name, buff + sizeof( Message ) );

    return file_name;
}

int _getRequest( int sd, char* file_name )
{
    int file_exist = access( file_name, F_OK | R_OK );

    if ( file_exist != 0 )
    {
        if ( access( file_name, F_OK ) != 0 )
        {
            printf( "file %s doesn't exist\n", file_name );
        }
        else
        {
            printf( "file %s doesn't have read permission\n", file_name );
        }
    }

    {
        Message reply_cmd;
        char* cmd_buff = createGetReplyCmd( &reply_cmd, file_exist );
        if ( sendCmdMsg( sd, cmd_buff, reply_cmd.length ) == 1 )
        {
            exit( 1 );
        }
        free( cmd_buff );
    }

    if ( file_exist != 0 )
    {
        return 0;
    }

    {
        Message reply_cmd;
        char* cmd_buff = createFileDataCmd( &reply_cmd, file_name );
        if ( sendCmdMsg( sd, cmd_buff, reply_cmd.length ) == 1 )
        {
            exit( 1 );
        }
        free( cmd_buff );
    }

    return 0;
}

int _putRequest( int sd, char* file_name )
{
    {
        Message reply_cmd;
        char* cmd_buff = createPutReplyCmd( &reply_cmd );
        if ( sendCmdMsg( sd, cmd_buff, reply_cmd.length ) == 1 )
        {
            exit( 1 );
        }
        free( cmd_buff );
    }

    char* buff = NULL;
    int len = 0;
    Message cmd;
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
    return 0;
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
        {
            int res = recvMsg( sd, ( char* )len, sizeof( int ) );
            if ( 1 == res )
            {
                fprintf( stderr, "error receiving, exit!\n" );
                break;
            }
            else if ( 2 == res )
            {
                printf( "stop recving, exit!\n" );
                break;
            }
        }
        char* buff = ( char* )calloc( sizeof( char ), *len + 1 );
        {
            int res = recvMsg( sd, buff, *len );
            if ( 1 == res )
            {
                fprintf( stderr, "error receiving, exit!\n" );
                break;
            }
            else if ( 2 == res )
            {
                printf( "stop recving, exit!\n" );
                break;
            }
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
                processListReplyCmd( buff );
                break;
            case GET_REQUEST:
            {
                char* file_name = _getFileName( buff );
                _getRequest( sd, file_name );
                free( file_name );
            }
            break;
            case GET_REPLY_EXIST:
                break;
            case PUT_REQUEST:
            {
                char* file_name = _getFileName( buff );
                _putRequest( sd, file_name );
                free( file_name );
            }
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
        free( buff );
    }
    free( len );
}

int main( int argc, char** argv )
{
    if ( 2 != argc )
    {
        printf( "USAGE: %s PORT_NUMBER\n", argv[0] );
        exit( 0 );
    }

    int port = atoi( argv[1] );
    if ( 0 == port )
    {
        fprintf( stderr, "opt error: wrong port number %s\n", argv[1] );
        exit( 0 );
    }

    int sd = socket( AF_INET, SOCK_STREAM, 0 );
    long val;
    if ( setsockopt( sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof( long ) ) == -1 )
    {
        printf( "setsockopt: %s (Errno:%d)\n", strerror( errno ), errno );
        exit( 0 );
    }
    int client_sd;
    struct sockaddr_in server_addr;
    memset( &server_addr, 0, sizeof( server_addr ) );
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl( INADDR_ANY );
    server_addr.sin_port = htons( port );
    if ( bind( sd, ( struct sockaddr* )&server_addr, sizeof( server_addr ) ) < 0 )
    {
        printf( "bind error: %s (Errno:%d)\n", strerror( errno ), errno );
        exit( 0 );
    }
    if ( listen( sd, 3 ) < 0 )
    {
        printf( "listen error: %s (Errno:%d)\n", strerror( errno ), errno );
        exit( 0 );
    }
    struct sockaddr_in client_addr;
    int addr_len = sizeof( client_addr );
    while ( client_sd = accept( sd, ( struct sockaddr* )&client_addr, &addr_len ) )
    {
        if ( client_sd  < 0 )
        {
            printf( "accept erro: %s (Errno:%d)\n", strerror( errno ), errno );
            exit( 0 );
        }
        else
        {
            printf( "receive connection from %s\n", inet_ntoa( client_addr.sin_addr ) );
        }

        pthread_t worker;
        pthread_create( &worker, NULL, pthread_prog, &client_sd );
    }
    close( sd );
    return 0;
}
