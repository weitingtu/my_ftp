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
        return 1;
    }
    if ( sendMsg( sd, buff, *msg_len ) == 1 )
    {
        fprintf( stderr, "send error, exit\n" );
        return 1;
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
        return 1;
    }
    *buff = ( char* )calloc( sizeof( char ), *msg_len + 1 );
    if ( recvMsg( sd, *buff, *msg_len ) == 1 )
    {
        fprintf( stderr, "error receiving, exit!\n" );
        return 1;
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

char* get_data_dir_path()
{
    char* path = getcwd( NULL, 0 );
    path = realloc( path, strlen( path ) + 6 );
    strcat( path, "/data/" );

    return path;
}

char* _get_data_dir_files()
{
    char* path = get_data_dir_path();
    DIR* ptr_dir = opendir( path );
    free( path );

    char* file_str = NULL;
    struct dirent prev_dir_entry;
    struct dirent* ptr_dir_entry = NULL;

    for ( ;; )
    {
        readdir_r( ptr_dir, &prev_dir_entry, &ptr_dir_entry );
        if ( !ptr_dir_entry )
        {
            break;
        }
        else
        {
            if ( ( strcmp( ptr_dir_entry->d_name, "." ) != 0 ) && ( strcmp( ptr_dir_entry->d_name, ".." ) != 0 ) ) // skip "." and ".." file listings
            {
                if ( NULL == file_str )
                {
                    file_str = strdup( ptr_dir_entry->d_name );
                }
                else
                {
                    file_str = realloc( file_str, strlen( file_str ) + strlen( ptr_dir_entry->d_name ) + 1 + 1 );
                    strcat( file_str, " " );
                    strcat( file_str, ptr_dir_entry->d_name );
                }
            }
        }
    }
    closedir( ptr_dir );
    return file_str;
}

void printCmd ( Message* cmd )
{
    printf( "protocal: " );
    for ( int i = 0; i < 5; ++i )
    {
        printf( "%c", cmd->protocol[i] );
    }
    printf( "\n" );
    printf( "type:     %x\n",  cmd->type );
    printf( "length:   %zu\n", cmd->length );
}

char* createListRequestCmd( Message* cmd )
{
    strcpy( cmd->protocol, "myftp" );
    cmd->type = LIST_REQUEST;
    cmd->length = sizeof( Message );

    char* buff = malloc( sizeof( char ) * cmd->length );
    memcpy( buff, cmd, cmd->length );
    return buff;
}

char* createListReplyCmd( Message* cmd )
{
    char* file_str = _get_data_dir_files();
    strcpy( cmd->protocol, "myftp" );
    cmd->type = LIST_REPLY;
    cmd->length = sizeof( Message ) + strlen( file_str );

    char* buff = malloc( sizeof( char ) * cmd->length );
    memcpy( buff, cmd, sizeof( Message ) );
    memcpy( buff + sizeof( Message ), file_str, strlen( file_str ) );
    return buff;
}

char* createGetRequestCmd( Message* cmd, char* file_name )
{
    strcpy( cmd->protocol, "myftp" );
    cmd->type = GET_REQUEST;
    cmd->length = sizeof( Message ) + strlen( file_name );

    char* buff = malloc( sizeof( char ) * cmd->length );
    memcpy( buff, cmd, sizeof( Message ) );
    memcpy( buff + sizeof( Message ), file_name, strlen( file_name ) );
    return buff;
}

char* createGetReplyCmd( Message* cmd, int file_exist )
{
    strcpy( cmd->protocol, "myftp" );
    cmd->type = file_exist == 0 ? GET_REPLY_EXIST : GET_REPLY_NON_EXIST;
    cmd->length = sizeof( Message );

    char* buff = malloc( sizeof( char ) * cmd->length );
    memcpy( buff, cmd, cmd->length );
    return buff;
}

char* createPutRequestCmd( Message* cmd, char* file_name )
{
    strcpy( cmd->protocol, "myftp" );
    cmd->type = PUT_REQUEST;
    cmd->length = sizeof( Message ) + strlen( file_name );

    char* buff = malloc( sizeof( char ) * cmd->length );
    memcpy( buff, cmd, sizeof( Message ) );
    memcpy( buff + sizeof( Message ), file_name, strlen( file_name ) );
    return buff;
}

char* createPutReplyCmd( Message* cmd )
{
    strcpy( cmd->protocol, "myftp" );
    cmd->type = PUT_REPLY;
    cmd->length = sizeof( Message );

    char* buff = malloc( sizeof( char ) * cmd->length );
    memcpy( buff, cmd, cmd->length );
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
    memcpy( buff, cmd, sizeof( Message ) );
    fread( buff + sizeof( Message ), f_size, 1, fp );

    fclose( fp );

    return buff;
}

void processListReplyCmd( char* buff )
{
    printf( "%s\n", buff + sizeof( Message ) );
}

int parseCmd ( char* buff, int len, Message* cmd )
{
    if ( len < sizeof( Message ) )
    {
        return 1;
    }
    memcpy( cmd, buff, sizeof( Message ) );
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
