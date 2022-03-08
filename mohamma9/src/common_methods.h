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

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void initClientList(struct sockaddr_storage *clientList)
{
    struct sockaddr_storage cp = {0};
    for (int i = 0; i < 30; i++)
        memcpy(&clientList[i], &cp, sizeof(struct sockaddr_storage));
}

void addClient(struct sockaddr_storage *client, struct sockaddr_storage *clientList)
{
    int i;
    char myIP[16];
    struct sockaddr_in _address;
    _address = *(struct sockaddr_in *)client;
    struct sockaddr_storage cp = {0};
    // printf("Adding Client\n");
    for (i = 0; i < 30; i++)
    {
        if ((memcmp(&clientList[i], &cp, sizeof(struct sockaddr_storage))) == 0)
        {
            memcpy(&clientList[i], &client, sizeof(struct sockaddr_storage));
            break;
        }
    }
    // printf("Client Added\n");
}

void removeClient(struct sockaddr_storage *client, struct sockaddr_storage *clientList)
{
    struct sockaddr_storage cp = {0};
    for (int i = 0; i < 30; i++)
    {
        if ((memcmp(&clientList[i], &client, sizeof(struct sockaddr_storage))) == 0)
        {
            memcpy(&clientList[i], &cp, sizeof(struct sockaddr_storage));
            // free(client);
            break;
        }
    }
}

void getIP(void)
{

    int _socket;
    struct sockaddr_in _address, my_addr;
    char myIP[16];
    socklen_t sin_size;
    _address.sin_family = AF_INET;
    _address.sin_port = htons(53);
    _address.sin_addr.s_addr = inet_addr("8.8.8.8");

    if ((_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        printf("UDP socket error");
    }

    bind(_socket, (struct sockaddr *)&_address, sizeof(_address));

    connect(_socket, (struct sockaddr *)&_address, sizeof(_address));
    sin_size = sizeof my_addr;
    bzero(&my_addr, sizeof(my_addr));
    getsockname(_socket, (struct sockaddr *)&_address, &sin_size);

    inet_ntop(AF_INET, &_address.sin_addr, myIP, sizeof(myIP));
    printf("IP: %s\n", myIP);

    close(_socket);
}

void getPort(struct sockaddr_in *socket_addr)
{
    int myPort;
    myPort = ntohs(socket_addr->sin_port);
    printf("PORT: %u\n", myPort);
}

void getAuthor(void)
{
    printf("I, %s, have read and understood the course academic integrity policy.\n", "mohamma9");
}
void listClients(struct sockaddr_storage *clientList)
{
    struct sockaddr_storage cp = {0};
    int count = 0;
    for (int i = 0; i < 30; i++)
    {
        char ip[16];
        char storeIP[16];
        if (!((memcmp(&clientList[i], &cp, sizeof(struct sockaddr_storage))) == 0))
        {
            // printf("ClientList\n");
            inet_ntop(AF_INET, &((struct sockaddr_in *)&clientList[i])->sin_addr, storeIP, sizeof(storeIP));
            strcpy(ip, storeIP);
            printf("Client %d: IP -> %s, ", (i + 1), ip);
            getPort((struct sockaddr_in *)&clientList[i]);
            count++;
        }
    }
    if (count == 0)
    {
        printf("No clients connected\n");
    }
}

void trim_newline(char *text)
{
    int len = strlen(text) - 1;
    if (text[len] == '\n')
    {
        text[len] = '\0';
    }
}