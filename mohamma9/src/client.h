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
#include "../include/logger.h"
#include "common_methods.h"

#include <arpa/inet.h>

//#define PORT "9002" // the port client will be connecting to

#define MAXDATASIZE 1024 // max number of bytes we can get at once

struct sockaddr_in *client;
int clientSock, databytes, serverSock, temp, max_descriptors;
int clientListFD[30];
struct clientData listOfClients[30] = {0};
int loggedIn = 0;
int msg_count = 0;
struct timeval tv;
fd_set read_descriptors;
struct sockaddr_in client_address, sock_address;
struct addrinfo *server, hints;

void Broadcast(char *msg);

int login(char *data)
{
    char serverIP[16];
    char *ip, *port;
    memset(&hints, 0, sizeof hints);
    ip = strsep(&data, " ");
    port = data;
    trim_newline(ip);
    trim_newline(port);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    clientSock = socket(AF_INET, SOCK_STREAM, 0);
    struct timeval tv = {0, 500};
    ;
    int yes = 1;

    // tv.tv_sec = 1
    // tv.tv;  /* 3 Secs Timeout */
    setsockopt(clientSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval));
    if (clientSock < 0)
    {
        perror("Client: socket");
    }

    if ((temp = getaddrinfo(ip, port, &hints, &server)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(temp));
        close(clientSock);
        return 1;
    }
    bind(clientSock, (struct sockaddr *)&client_address, sizeof(client_address));
    if ((temp = connect(clientSock, server->ai_addr, server->ai_addrlen)) < 0)
    {
        // perror("Client: server connect\n");
        // printf("Error\n");
        fprintf(stderr, "Server Connect: %s\n", gai_strerror(temp));
        close(clientSock);
        return 1;
    }
    initClientList(clientListFD);
    fflush(stdout);
    // inet_ntop(AF_INET, &server->ai_addr, serverIP, sizeof(serverIP));
    printf("Connected to server\n");
    loggedIn = 1;
    return 0;
}

void logout()
{
    close(clientSock);
    // free(&clientList);
    loggedIn = 0;
    msg_count = 0;
    printf("Logged out from server\n");
}

void exitChat()
{
    if (loggedIn)
        logout();
    exit(0);
}

void refreshClients(char *msg)
{
    // unsigned char data[sizeof(msg)+1];
    // trim_newline(msg);
    // strcpy(data,msg);
    // memcpy(data+strlen(msg)," ",sizeof(" "));
    send(clientSock, msg, sizeof(msg), 0);
}

void handleRevieveData(char *msg)
{
    char *token;
    token = strsep(&msg, "-");
    trim_newline(token);

    printf("msg from: %s\n[msg]:%s\n", token, msg);
}

void parse_client_user_input(char *s)
{
    char *token;
    token = strsep(&s, " ");
    trim_newline(token);

    if (strcmp(token, "IP") == 0)
    {
        cse4589_print_and_log("[%s:SUCCESS]\n", "IP");
        getIP();
        cse4589_print_and_log("[%s:END]\n", "IP");
    }
    else if (strcmp(token, "PORT") == 0)
    {
        cse4589_print_and_log("[%s:SUCCESS]\n", "PORT");
        getPort(client);
        cse4589_print_and_log("[%s:END]\n", "IP");
    }
    else if (strcmp(token, "AUTHOR") == 0)
    {
        cse4589_print_and_log("[%s:SUCCESS]\n", "AUTHOR");
        getAuthor();
        cse4589_print_and_log("[%s:END]\n", "IP");
    }
    else if (strcmp(token, "LIST") == 0)
    {
        if (loggedIn)
        {
            cse4589_print_and_log("[%s:SUCCESS]\n", "LIST");
            listClientsForClient(listOfClients);
            cse4589_print_and_log("[%s:END]\n", "LIST");
        }
        // listClients(clientList);
        else
        {
            cse4589_print_and_log("[%s:ERROR]\n", "LIST");
            // printf("Login to the server.\n");
            cse4589_print_and_log("[%s:END]\n", "LIST");
        }
        // listClients();
    }
    else if (strcmp(token, "REFRESH") == 0)
    {
        if (loggedIn)
        {
            cse4589_print_and_log("[%s:SUCCESS]\n", "REFRESH");
            refreshClients(token);
            cse4589_print_and_log("[%s:END]\n", "REFRESH");
        }
        else
        {
            cse4589_print_and_log("[%s:ERROR]\n", "REFRESH");
            printf("Login to the server.\n");
            cse4589_print_and_log("[%s:END]\n", "REFRESH");
        }
    }
    else if (strcmp(token, "SEND") == 0)
    {
        sendMessage(&clientSock, s);
    }
    else if (strcmp(token, "BROADCAST") == 0)
    {
        Broadcast(s);
    }
    else if (strcmp(token, "LOGIN") == 0)
    {
        int i = login(s);
        if (i == 0)
        {
            cse4589_print_and_log("[%s:SUCCESS]\n", "LOGIN");
            cse4589_print_and_log("[%s:END]\n", "LOGIN");
        }
        else if (i == 1)
        {
            cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
            cse4589_print_and_log("[%s:END]\n", "LOGIN");
        }
    }
    else if (strcmp(token, "LOGOUT") == 0)
    {
        if (loggedIn)
        {
            cse4589_print_and_log("[%s:SUCCESS]\n", "LOGOUT");
            logout();
            cse4589_print_and_log("[%s:END]\n", "LOGOUT");
        }
        else
        {
            cse4589_print_and_log("[%s:ERROR]\n", "LOGOUT");
            cse4589_print_and_log("[%s:END]\n", "LOGOUT");
            // printf("You are not connected to any server\n");
        }
    }
    else if (strcmp(token, "EXIT") == 0)
    {
        cse4589_print_and_log("[%s:SUCCESS]\n", "EXIT");
        exitChat();
        cse4589_print_and_log("[%s:END]\n", "EXIT");
    }
    else
    {
        printf("Invalid Command\n");
    }
}

void parser_server_data(char *msg)
{
    char *token;
    // printf("Server data: %s\n",msg);
    token = strsep(&msg, " ");
    trim_newline(token);

    if (strcmp(token, "LIST") == 0)
    {
        receiveClientList(msg, listOfClients);
    }
    else if (strcmp(token, "SEND") == 0)
    {
        handleRevieveData(msg);
    }
}

void execute_command(char *command, char *data)
{
    printf("Inside execute_command");
    if (strcmp(command, "IP") == 0)
    {
        getIP();
    }
    else if (strcmp(command, "PORT") == 0)
    {
        getPort(client);
    }
    else
    {
        printf("Not a command");
    }
}

void start_client(int port)
{
    char buf[1024];
    int yes = 1;
    int msg_count = 0;
    tv.tv_sec = 0;
    tv.tv_usec = 500;
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = INADDR_ANY;
    client_address.sin_port = htons(port);
    client = &client_address;
    puts("Client started");
    // initClientList(clientList);

    // setsockopt(clientSock, SOL_SOCKET, SO_RCVLOWAT,&opt,sizeof(opt));
    // setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    // fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    // connect(clientSock, server->ai_addr, server->ai_addrlen);

    while (1)
    {
        FD_ZERO(&read_descriptors);
        FD_SET(STDIN_FILENO, &read_descriptors);
        if (loggedIn)
        {
            FD_SET(clientSock, &read_descriptors);
            max_descriptors = clientSock;
        }
        else
        {
            max_descriptors = STDIN_FILENO;
        }
        fflush(stdin);
        // puts(">");
        select(max_descriptors + 1, &read_descriptors, NULL, NULL, NULL);

        if (FD_ISSET(STDIN_FILENO, &read_descriptors))
        {
            char msg[1024];
            memset(msg, 0, sizeof(msg));
            fgets(msg, 1024, stdin);
            trim_newline(msg);
            parse_client_user_input(msg);
        }
        else if (FD_ISSET(clientSock, &read_descriptors))
        {
            setsockopt(clientSock, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval));
            int index = 0;
            // Bytes read by the socket in one go
            ssize_t bytesRead;
            while (1)
            {
                printf("Before\n");
                bytesRead = read(clientSock, buf + index, MAXDATASIZE);
                printf("%u\n", bytesRead);
                if (bytesRead <= 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
                {
                    // perror("recv");
                    if (sizeof(buf) != 0)
                    {
                        // printf("Received all the data\n");
                        buf[index + 1] = '\0';
                        // printf("Client Receive: %s\n",buf);
                        parser_server_data(buf);
                    }
                    // if( msg_count == 0){
                    //     // uint32_t data[30];
                    //     // printf("Copying to list");
                    //     // memcpy(data,buf,sizeof(data));
                    //     receiveClientList(buf,listOfClients);
                    // }
                    // printf("Done\n");
                    msg_count += 1;
                    break;
                }
                // else if(bytesRead <= 0){
                //     printf("Received all the data\n");
                //     buf[index+1] = '\0';
                //     printf("%u",sizeof(buf));
                //     break;
                // }
                else
                {
                    // printf("Here\n");

                    index = index + bytesRead;
                    // printf("%d\n",index);
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
        else
        {
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

void Broadcast(char *msg)
{
    // int sockfd;
    // struct sockaddr_in their_addr; // connector's address information
    // struct hostent *he;
    // int numbytes;
    // int broadcast = 1;
    // their_addr.sin_family = AF_INET;   // host byte order
    // their_addr.sin_port = htons(); // short, network byte order
    // their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    // memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

    trim_newline(msg);
    unsigned char *data = msg;
    unsigned char *prepend = (char *)"BROADCAST ";
    unsigned char dataToSend[sizeof(prepend) + sizeof(msg) + 5];

    strcpy(dataToSend, prepend);
    strcat(dataToSend, msg);
    // memcpy(dataToSend+strlen(prepend),ip,sizeof(ip));
    // printf("%s,%d\n",dataToSend,sizeof(ip)+strlen(prepend));
    // memcpy(dataToSend+strlen(prepend)+sizeof(ip),separator,sizeof(separator));
    // printf("%s,%d\n",dataToSend,sizeof(ip)+strlen(prepend));
    // printf("%s,%d\n",dataToSend,strlen(dataToSend));
    // memcpy(dataToSend+strlen(prepend)+ sizeof(ip) + strlen(separator), data, sizeof(data));
    printf("%s,%d\n", dataToSend, strlen(dataToSend));
    // printf("%s,%d\n",msg,strlen(msg));
    // handleBroadcast(client, dataToSend);
    send(clientSock, dataToSend, sizeof(dataToSend), 0);
}
