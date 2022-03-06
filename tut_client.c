#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

int main()
{
    // create a socket
    int network_socket = socket(AF_INET, SOCK_STREAM, 0);
    // specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9001);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int connection_status = connect(network_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    printf("The network socket is %d \n", network_socket);
    // printf("The server address is %s \n", server_address);
    printf("the connection status is %d \n", connection_status);
    if (connection_status == -1)
    {
        printf("Connection is not successful. Debug for more");
    }
    // reveive data from the server
    char server_response[256];
    recv(network_socket, &server_response, sizeof(server_response), 0);

    // printing out the server's response
    printf("The response from the server is %s \n", server_response);

    // Finally close the socket
    close(network_socket);
    return 0;
}