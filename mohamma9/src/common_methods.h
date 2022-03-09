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
#include <netdb.h>

struct clientData {
    char ip[16];
    int port;
};

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void initClientList(int *clientList){
    struct sockaddr_storage cp = {0};
    for(int i=0;i<30;i++)
        clientList[i] = 0;
}

void addClient(int *client,int *clientList){
    int i;
    char myIP[16];
    struct sockaddr_in _address;
    _address = *(struct sockaddr_in *)client;
    struct sockaddr_storage cp = {0};
    // printf("Adding Client\n");
    for (i=0;i<30;i++){
        if ( clientList[i] == 0 ){
            printf("Adding to list of sockets as %d\n" , i);
            clientList[i] = *client;
            break;
        }
    }
    // printf("Client Added\n");
}

void removeClient(int *client,int *clientList){
    struct sockaddr_storage cp = {0};
    for (int i=0;i<30;i++){
        if (clientList[i] == *client){
            clientList[i] = 0;
            // free(client);
            break;
        }
    }
}


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
    printf("IP: %s\n",myIP);

    close(_socket); 
}

void getPort(struct sockaddr_in *socket_addr) {
    int myPort;
    myPort = ntohs(socket_addr->sin_port);
    printf("PORT: %u\n", myPort);
}

void getAuthor(void) {
    printf("I, %s, have read and understood the course academic integrity policy.\n", "mohamma9");
}
void listClients(int * clientList){
    struct sockaddr_storage cp = {0};
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int count = 0;
    for(int i=0;i<30;i++){
        if(clientList[i] !=0){
            getpeername(clientList[i],(struct sockaddr *)&addr, &addr_len);
            printf("Peer IP address: %s\n", inet_ntoa(addr.sin_addr));
            printf("Peer port      : %d\n", ntohs(addr.sin_port));
            count++;
        }
    }
    if(count == 0) {
        printf("No clients connected\n");
    }
}

void sendClientList(int *client,int *clientList){
    struct clientData data[30];
    char ip[16];
    unsigned char * char_data;

    for(int i=0;i<30;i++){
        // data[i] = clientList[i];
        struct sockaddr_in addr,clientInfo;
        socklen_t addr_len = sizeof(addr);
        if(clientList[i]!=0){
            getpeername(clientList[i],(struct sockaddr *)&addr,&addr_len);
            memcpy(&clientInfo,&addr,addr_len);
            memcpy(data[i].ip,inet_ntoa(clientInfo.sin_addr),sizeof(data[i].ip));
            data[i].ip[16] = '\0';
            data[i].port = clientInfo.sin_port;
        }
        else {
            memcpy(data[i].ip,"",sizeof(data[i].ip));
            data[i].ip[16] = '\0';
            data[i].port = 0;
        }
    }
    char_data = (char *)&data;
    send(*client,char_data, sizeof(data),0);
}


void receiveClientList(char *data,struct clientData *clientList){
    struct clientData receivedData[30];
    memcpy(receivedData,data,sizeof(receivedData));
    // printf()
    printf("Receiving list of clients\n");
    for(int i=0;i<30;i++){
        memcpy(clientList[i].ip,receivedData[i].ip,sizeof(clientList[i].ip));
        clientList[i].port = ntohs(receivedData[i].port);
    }
    // send(*client,data, sizeof(data),0);
    printf("Received list of clients\n");
}

void listClientsForClient(struct clientData *data){
    for(int  i=0;i<30;i++){
        // printf("%d\n",strlen(data[i].ip));
        if(strlen(data[i].ip)>0 && data[i].port!=0)
            printf("IP: %s PORT: %d\n",data[i].ip,data[i].port);
    }
}

void trim_newline (char *text)
{
    int len = strlen (text) - 1;
    if (text[len] == '\n')
      {
          text[len] = '\0';
      }
}