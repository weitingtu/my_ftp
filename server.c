#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 12345

int main( int argc, char** argv )
{
    int sd = socket( AF_INET, SOCK_DGRAM, 0 );
    struct sockaddr_in server_addr;
    memset( &server_addr, 0, sizeof( server_addr ) );
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl( INADDR_ANY );
    server_addr.sin_port = htons( PORT );
    socklen_t addrLen = sizeof( server_addr );
    if ( bind( sd, ( struct sockaddr* )&server_addr, sizeof( server_addr ) ) < 0 )
    {
        printf( "bind error: %s (Errno:%d)\n", strerror( errno ), errno );
        exit( 0 );
    }
    struct sockaddr_in client_addr;
    for ( int i = 0; i < 5; i++ )
    {
        char buff[100];
        int len;
        char* ackBuff = "ack";
        printf( "Waiting heartbeat\n" );
        if ( ( len = recvfrom( sd, buff, sizeof( buff ), 0,
                               ( struct sockaddr* )&client_addr, &addrLen ) ) <= 0 )
        {
            printf( "receive error: %s (Errno:%d)\n", strerror( errno ), errno );
        }

        buff[len] = '\0';
        printf( "Received heartbeat: " );
        if ( strlen( buff ) != 0 )
        {
            printf( "%s\n", buff );
        }

        if ( ( len = sendto( sd, ackBuff, strlen( ackBuff ), 0,
                             ( struct sockaddr* )&client_addr, addrLen ) ) <= 0 )
        {
            printf( "Send Error: %s (Errno:%d)\n", strerror( errno ), errno );
            // exit(0);
        }
    }
    close( sd );
    return 0;
}

