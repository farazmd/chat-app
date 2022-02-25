#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
// #include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>


void getIP(void) {

    int _socket;
    struct sockaddr_in _address,my_addr;
    char myIP[16];
    socklen_t sin_size;
    _address.sin_family = AF_INET;
    _address.sin_port = htons(53);
    _address.sin_addr.s_addr = inet_addr("8.8.8.8");

    if ((_socket = socket(AF_INET,SOCK_DGRAM,0)) == -1) {
        printf("UDP socket error");
    }

    bind(_socket,( struct sockaddr*)&_address, sizeof(_address));

    connect(_socket, ( struct sockaddr*)&_address, sizeof(_address));
    sin_size = sizeof my_addr;
    bzero(&my_addr, sizeof(my_addr));
    getsockname(_socket, ( struct sockaddr*)&_address,&sin_size);

    inet_ntop(AF_INET, &_address.sin_addr, myIP, sizeof(myIP));
    printf("IP %s\n",myIP);

    close(_socket); 
}

void getPort(struct sockaddr_in *socket_addr) {
    int myPort;
    myPort = ntohs(socket_addr->sin_port);
    printf("Local port : %u\n", myPort);
}

void getAuthor(void) {
    ("I, %s, have read and understood the course academic integrity policy.\n", "mohamma9");
}
void listClients(struct sockaddr_in* socket_list[]);