#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
// #include <sys/time.h>
#include "common_methods.h"
#include <sys/select.h>

int server_socket, total_clients = 30,client_connections[30], client_socket, max_socket_descriptors,
    addrlen, sd, valread;
struct sockaddr_storage clientList[30];
fd_set read_descriptors;
char buf[1024];
struct sockaddr_in server_address;
char clientData[30][sizeof(struct sockaddr_in)];

struct clientStats {
    char ip[20];
    int port;
    int message_sent_count;
    int message_receive_count;
    // char message_queue[][1024];
};

struct clientStats clientStatsList[30] = {0};

void updateClientStats(int *client,int sent,int received){
    struct sockaddr_in addr,clientInfo;
    socklen_t addr_len = sizeof(addr);
    char ip[20];
    getpeername(*client,(struct sockaddr *)&addr,&addr_len);
    memcpy(&clientInfo,&addr,addr_len);
    memcpy(ip,inet_ntoa(clientInfo.sin_addr),sizeof(ip));
    ip[20] = '\0';
    int clientFound = 0;
    for(int i=0; i<30;i++){
        if(strcmp(ip,clientStatsList[i].ip)==0){
            // printf("Found Client\n");
            if(sent == 1){
                clientStatsList[i].message_sent_count +=1;
            }
            if(received == 1){
                clientStatsList[i].message_receive_count +=1;
            }
            clientFound = 1;
        }
    }
    if(clientFound == 0){
        // puts("Did not find client");
        for(int i=0; i<30;i++){
            if(strlen(clientStatsList[i].ip)==0){
                // puts("Adding client here");
                strcpy(clientStatsList[i].ip,ip);
                clientStatsList[i].port = ntohs(clientInfo.sin_port);
                clientStatsList[i].message_sent_count = 0;
                clientStatsList[i].message_receive_count = 0;
                break;
            }
        }
    }
}

void showStats(){
    for(int i=0; i<30;i++){
        struct sockaddr_in addr,clientInfo;
        socklen_t addr_len = sizeof(addr);
        char ip[20];
        if(client_connections[i]!=0){
            getpeername(client_connections[i],(struct sockaddr *)&addr,&addr_len);
            memcpy(&clientInfo,&addr,addr_len);
            memcpy(ip,inet_ntoa(clientInfo.sin_addr),sizeof(ip));
            if(strcmp(ip,clientStatsList[i].ip)==0){
                printf("Client IP: %s, Messages Sent: %d, Messages Received: %d\n",clientStatsList[i].ip,
                clientStatsList[i].message_sent_count,clientStatsList[i].message_receive_count);
            }
        }
    }
}

void parse_user_input(char *s) {
    char * token;
    token = strsep(&s," ");
    trim_newline(token);

    if (strcmp(token,"IP")==0){
        getIP();
    }
    else if(strcmp(token,"PORT")==0){
        getPort(&server_address);
    }
    else if (strcmp(token,"AUTHOR")==0){
        getAuthor();
    }
    else if (strcmp(token,"LIST")==0){
        listClients(client_connections);
    }
    else if (strcmp(token,"STATISTICS")==0){
        showStats();
    }
    else{
        printf("Invalid Command\n");
    }
}

void handleRefresh(int *client){
    sendClientList(client,client_connections);
}

void handleSendData(int *client,char *msg){
    char * token;
    token = strsep(&msg,"-");
    trim_newline(token);
    trim_newline(msg);
    updateClientStats(client,1,0);
    for(int i=0;i<30;i++){
        struct sockaddr_in addr,clientInfo;
        socklen_t addr_len = sizeof(addr);
        char ip[20];
        if(client_connections[i]!=0){
            getpeername(client_connections[i],(struct sockaddr *)&addr,&addr_len);
            memcpy(&clientInfo,&addr,addr_len);
            memcpy(ip,inet_ntoa(clientInfo.sin_addr),sizeof(ip));
            ip[20] = '\0';
            if(strcmp(ip,token)==0){
                struct sockaddr_in senderAddr,senderInfo;
                socklen_t senderAddr_len = sizeof(senderAddr);
                char senderIp[20];
                unsigned char * messageData = msg;
                getpeername(*client,(struct sockaddr *)&senderAddr,&senderAddr_len);
                memcpy(&senderInfo,&senderAddr,senderAddr_len);
                memcpy(senderIp,inet_ntoa(senderInfo.sin_addr),sizeof(ip));
                senderIp[20] = '\0';
                unsigned char * separator = (char *)" ";
                unsigned char data[sizeof(senderIp) + sizeof(separator) + sizeof(msg)];
                // printf("%d\n",sizeof(data));
                strcpy(data,senderIp);
                // printf("%s,%d\n",data,strlen(data));
                memcpy(data + strlen(senderIp) , separator,sizeof(separator)+ sizeof(senderIp));
                // printf("%s,%d\n",data,strlen(data));
                memcpy(data + strlen(senderIp) + strlen(separator), messageData,sizeof(separator)+ sizeof(senderIp)+ sizeof(messageData));
                // printf("%s,%d\n",data,strlen(data));
                // printf("%s,%d\n",messageData,strlen(messageData));
                sendMessage(&client_connections[i],data);
                updateClientStats(&client_connections[i],0,1);
            }
        }
    }
}

void parse_client_data(int *client,char *s){
    char * token;
    printf("%s\n",s);
    token = strsep(&s," ");
    trim_newline(token);
    if(strcmp(token,"REFRESH")==0){
        handleRefresh(client);
    }
    else if(strcmp(token,"SEND")==0){
        handleSendData(client,s);
    }
}

void start_server(int port) {

    struct sockaddr_storage client_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // for (int i = 0; i < total_clients; i++)  
    // {  
    //     client_connections[i] = 0;  
    // }

    if((server_socket = socket(AF_INET,SOCK_STREAM,0)) == -1){
        printf("Server: Error");
        exit(EXIT_FAILURE);
    }

    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
    listen(server_socket, 5);

    addrlen = sizeof(server_address);  
    puts("Waiting for connections ...");

    initClientList(client_connections);

    while(1) {
        FD_ZERO(&read_descriptors);  
     
        //add master socket to set 
        FD_SET(server_socket, &read_descriptors); 
        FD_SET(STDIN_FILENO, &read_descriptors); 
        max_socket_descriptors = server_socket; 
        fflush (stdin);
        puts(">");

        for ( int i = 0 ; i < total_clients ; i++)  
        {  
            //socket descriptor 
            sd = client_connections[i];  
                 
            //if valid socket descriptor then add to read list 
            if(sd > 0)  
                FD_SET( sd , &read_descriptors);  
                 
            //highest file descriptor number, need it for the select function 
            if(sd > max_socket_descriptors)  
                max_socket_descriptors = sd;  
        }


        select(max_socket_descriptors + 1, &read_descriptors, NULL, NULL, NULL);

        if (FD_ISSET(server_socket, &read_descriptors)) {

            if ((client_socket = accept(server_socket, 
                    (struct sockaddr *)&client_address, (socklen_t*)&addrlen))<0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            } 
            addClient(&client_socket,client_connections);
            updateClientStats(&client_socket,0,0);
            // for (int i = 0; i < total_clients; i++)  
            // {  
            //     //if position is empty 
            //     if( client_connections[i] == 0 )  
            //     {  
            //         client_connections[i] = client_socket;  
            //         printf("Adding to list of sockets as %d\n" , i);  
            //         unsigned char data[sizeof(clientList)];
            //         // data = (unsigned char)malloc(sizeof(clientList[0]));
            //         memcpy(data,clientList, sizeof(clientList));
            //         // printf("%u\n",data);
            //         // printf("hello\n");
            //         send(client_socket,data,sizeof(data),0);
            //         break;  
            //     }  
            // }
            sendClientList(&client_socket,client_connections);
            // unsigned char data[sizeof(client_connections)];
            // memcpy(data,client_connections, sizeof(client_connections));
            // send(client_socket,data,sizeof(data),0);
            // char buf[1024];
            // memset(buf, 0, sizeof(buf));
            // int lastBit;

            // lastBit = recv(server_socket, buf, sizeof(buf), 0);
            // if (lastBit > 0 && lastBit < 1024)
            // {
            //     buf[lastBit] = '\0';
            // }
            // else
            // {
            //     close(server_socket);
            // }
        }
        else if (FD_ISSET(STDIN_FILENO, &read_descriptors)) {
            char msg[1024];
            memset(msg, 0, sizeof(msg));
            fgets (msg, 1024, stdin);
            trim_newline(msg);
            parse_user_input(msg);
            
        }
        for (int i = 0; i < 30; i++)  
        {  
            sd = client_connections[i];  
                
            if (FD_ISSET( sd , &read_descriptors) && sd!=0)  
            {  
                //Check if it was for closing , and also read the 
                //incoming message 
                if ((valread = read( sd , buf, 1024)) == 0)  
                {  
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&client_address , \
                        (socklen_t*)&addrlen);  
                    // printf("Host disconnected , ip %s , port %d \n" , 
                    //     inet_ntoa(client_address.sin_addr) , ntohs(((struct sockaddr* )client_address).sin_port));  
                        
                    //Close the socket and mark as 0 in list for reuse 
                    close( sd );  
                    // client_connections[i] = 0;  
                    removeClient(&client_connections[i],client_connections);
                }  
                else 
                {   
                    //set the string terminating NULL byte on the end 
                    //of the data read 
                    buf[valread+1] = '\0';    
                    parse_client_data(&sd,buf);
                }  
            }  
        }  
    }
}