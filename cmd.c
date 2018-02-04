#include "cmd.h"
#include <dirent.h>

static char* _get_data_dir_path()
{
    char* path = getcwd( NULL, 0 );
    path = realloc( path, strlen( path ) + 5 );
    strcat( path, "/data" );

    return path;
}

char* _get_data_dir_files()
{
    char* path = _get_data_dir_path();
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

void processListReplyCmd( char* buff, int len, Message* cmd )
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
