#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
// #include <sys/time.h>
#include <sys/select.h>


void start_server() {

    int server_socket, total_clients = 30, client_connections[30], client_socket, max_socket_descriptors,
    addrlen, sd, valread;
    fd_set read_descriptors;
    char buf[1000],s[20];
    struct sockaddr_in server_address;
    struct sockaddr_storage client_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;

    for (int i = 0; i < total_clients; i++)  
    {  
        client_connections[i] = 0;  
    }

    if((server_socket = socket(AF_INET,SOCK_STREAM,0)) == -1){
        printf("Server: Error");
        exit(EXIT_FAILURE);
    }

    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
    listen(server_socket, 5);

    addrlen = sizeof(server_address);  
    puts("Waiting for connections ...");

    while(1) {
        FD_ZERO(&read_descriptors);  
     
        //add master socket to set 
        FD_SET(server_socket, &read_descriptors); 
        FD_SET(STDIN_FILENO, &read_descriptors); 
        max_socket_descriptors = server_socket; 

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

            for (int i = 0; i < total_clients; i++)  
            {  
                //if position is empty 
                if( client_connections[i] == 0 )  
                {  
                    client_connections[i] = client_socket;  
                    printf("Adding to list of sockets as %d\n" , i);  
                         
                    break;  
                }  
            }
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
            read(STDIN_FILENO, msg, sizeof(msg));
            printf(msg);
        }
        for (int i = 0; i < total_clients; i++)  
        {  
            sd = client_connections[i];  
                
            if (FD_ISSET( sd , &read_descriptors))  
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
                    client_connections[i] = 0;  
                }  
                    
                //Echo back the message that came in 
                else 
                {  
                    //set the string terminating NULL byte on the end 
                    //of the data read 
                    buf[valread] = '\0';  
                    // send(sd , buf , strlen(buf) , 0 );
                    printf(buf);  
                }  
            }  
        }  
    }
}