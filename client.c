#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define IPADDR "127.0.0.1"
#define PORT 12345

int main( int argc, char** argv )
{
    /* if(connect(sd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0){ */
    /*     printf("connection error: %s (Errno:%d)\n",strerror(errno),errno); */
    /*     exit(0); */
    /* } */
    int sd = socket( AF_INET, SOCK_DGRAM, 0 );
    struct sockaddr_in server_addr;
    memset( &server_addr, 0, sizeof( server_addr ) );
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr( IPADDR );
    server_addr.sin_port = htons( PORT );
    socklen_t addrLen = sizeof( server_addr );

    char recvBuff[100];
    char* buff = "hello";
    struct sockaddr_in client_addr;

    for ( int i = 0; i < 5; i++ )
    {
        int len;
        if ( ( len = sendto( sd, buff, strlen( buff ), 0,
                             ( struct sockaddr* )&server_addr, addrLen ) ) <= 0 )
        {
            printf( "Send Error: %s (Errno:%d)\n", strerror( errno ), errno );
            exit( 0 );
        }
        /** Even directly use the following "if" is okay. Knowing IP addr just
         * decides who sends data first*/
        //    if ((len = recvfrom(sd, recvBuff, sizeof(recvBuff), 0, NULL, NULL)) <=
        //    0) {
        if ( ( len = recvfrom( sd, recvBuff, sizeof( recvBuff ), 0,
                               ( struct sockaddr* )&client_addr, &addrLen ) ) <= 0 )
        {
            printf( "recv Error: %s (Errno:%d)\n", strerror( errno ), errno );
        }
        else
        {
            recvBuff[len] = '\0';
            printf( "recved response from server: %s\n", recvBuff );
        }
        sleep( 3 );
    }
    close( sd );
    return 0;
}

