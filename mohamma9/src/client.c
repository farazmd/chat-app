#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "common_methods.h"

#include <arpa/inet.h>

#define PORT "9002" // the port client will be connecting to 

#define MAXDATASIZE 1024 // max number of bytes we can get at once 

struct sockaddr_in * client;
int clientSock, databytes,serverSock,temp,max_descriptors;
struct sockaddr_storage clientList[30];
int loggedIn = 0;
struct timeval tv;
fd_set read_descriptors;
struct sockaddr_in client_address,sock_address;
struct addrinfo *server,hints;

void login(char* data) {
    char serverIP[16];
    char *ip,*port;
    memset(&hints, 0, sizeof hints);
    ip = strsep(&data," ");
    port = data;
    trim_newline(ip);
    trim_newline(port);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    clientSock = socket(AF_INET,SOCK_STREAM,0);
    struct timeval tv;

    tv.tv_sec = 3;  /* 30 Secs Timeout */

    setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));
    // setsockopt(clientSock,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int));
    if(clientSock < 0){
        perror("Client: socket");
    }

    if ((temp = getaddrinfo(ip, port, &hints, &server)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(temp));
        // return 1;
    }
    if(connect(clientSock, server->ai_addr, server->ai_addrlen)<0){
        perror("Client: server connect\n");
        printf("Error\n");
    }
    fflush(stdout);
    // inet_ntop(AF_INET, &server->ai_addr, serverIP, sizeof(serverIP));
    printf("Connected to server\n");
    loggedIn = 1;
}

void logout() {
    close(clientSock);
    loggedIn = 0;
    printf("Logged out from server\n");
}

void exitChat() {
    if(loggedIn)
        logout();
    exit(0);
}

// void send() {
    
// }

void parse_user_input(char *s) {
    char * token;
    token = strsep(&s," ");
    trim_newline(token);

    if (strcmp(token,"IP")==0){
        getIP();
    }
    else if(strcmp(token,"PORT")==0){
        getPort(client);
    }
    else if (strcmp(token,"AUTHOR")==0){
        getAuthor();
    }
    else if (strcmp(token,"LIST")==0){
        if(loggedIn)
            printf("Lists connected users\n");
        else
            printf("Login to the server.\n");
        // listClients();
    }
    else if (strcmp(token,"LOGIN")==0){
        login(s);
    }
    else if (strcmp(token,"LOGOUT")==0){
        if(loggedIn)
            logout();
        else
            printf("You are not connected to any server\n");
    }
    else if (strcmp(token,"EXIT")==0){
        exitChat();
    }
    else{
        printf("Invalid Command\n");
    }
}

void execute_command(char *command, char* data){
    printf("Inside execute_command");
    if (strcmp(command,"IP")==0){
        getIP();
    }
    else if(strcmp(command,"PORT")==0){
        getPort(client);
    }
    else {
        printf("Not a command");
    }
}

int main() {
    char buf[1024];
    int true = 1;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = INADDR_ANY;
    client = &client_address;

    initClientList(clientList);

    // setsockopt(clientSock, SOL_SOCKET, SO_RCVLOWAT,&opt,sizeof(opt));
    // setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    // fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    // connect(clientSock, server->ai_addr, server->ai_addrlen);

    while(1) {
        FD_ZERO(&read_descriptors);
        FD_SET(STDIN_FILENO, &read_descriptors);
        if(loggedIn){
            FD_SET(clientSock, &read_descriptors);
            max_descriptors = clientSock;
        }
        else {
            max_descriptors = STDIN_FILENO;
        }
        fflush (stdin);
        printf(">");
        select(max_descriptors + 1,&read_descriptors, NULL, NULL, NULL);

        if(FD_ISSET(STDIN_FILENO,&read_descriptors)) {
            char msg[1024];
            memset(msg, 0, sizeof(msg));
            fgets (msg, 1024, stdin);
            trim_newline(msg);
            parse_user_input(msg);
        }
        else if(FD_ISSET(clientSock,&read_descriptors)){
            printf("Connected\n");
            int index = 0;
            // Bytes read by the socket in one go
            ssize_t bytesRead;
            while(1){
                printf("Before\n");
                bytesRead = read(clientSock, buf + index, MAXDATASIZE);
                printf("%u\n",bytesRead);
                if(bytesRead < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)){
                    perror("recv");
                    break;
                }
                else if(bytesRead <= 0){
                    printf("Received all the data\n");
                    buf[index+1] = '\0';
                    printf("%u",buf);
                    break;
                }
                else {
                    printf("Here\n");
                    index += bytesRead;
                    // printf("%u\n",index);
                }
            }
            // databytes = recv(clientSock, buf, MAXDATASIZE-1,MSG_PEEK);
            // printf("%d\n",databytes);
            // if(databytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)){

            //     perror("recv");
            //     continue;
            // }

            // buf[databytes] = '\0';

            // printf("client: received '%s'\n",buf);
        }
        else {
            continue;
        }
    }
}

// int main(int argc, char *argv[])
// {
//     int sockfd, numbytes;  
//     char buf[MAXDATASIZE];
//     struct addrinfo hints, *servinfo, *p;
//     int rv;
//     char s[INET6_ADDRSTRLEN];

//     if (argc != 2) {
//         fprintf(stderr,"usage: client hostname\n");
//         exit(1);
//     }

//     memset(&hints, 0, sizeof hints);
//     hints.ai_family = AF_UNSPEC;
//     hints.ai_socktype = SOCK_STREAM;

//     if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
//         fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
//         return 1;
//     }

//     // loop through all the results and connect to the first we can
//     for(p = servinfo; p != NULL; p = p->ai_next) {
//         if ((sockfd = socket(p->ai_family, p->ai_socktype,
//                 p->ai_protocol)) == -1) {
//             perror("client: socket");
//             continue;
//         }

//         if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
//             close(sockfd);
//             perror("client: connect");
//             continue;
//         }

//         break;
//     }

//     if (p == NULL) {
//         fprintf(stderr, "client: failed to connect\n");
//         return 2;
//     }

//     inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
//             s, sizeof s);
//     printf("client: connecting to %s\n", s);

//     freeaddrinfo(servinfo); // all done with this structure

//     // if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
//     //     perror("recv");
//     //     exit(1);
//     // }

//     // buf[numbytes] = '\0';

//     // printf("client: received '%s'\n",buf);

//     // close(sockfd);

//     send(sockfd,"Hello world!",13,0);

//     close(sockfd);

//     get_IP();
//     get_port((struct sockaddr_in *)&p);

//     return 0;
// }