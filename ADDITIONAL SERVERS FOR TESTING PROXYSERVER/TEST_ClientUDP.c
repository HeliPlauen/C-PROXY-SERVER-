#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdbool.h>

#if defined _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#endif

#define SERVER_PORT 8000
#define MAX_BUFFER_SIZE 1024
#define SERVER_ADDR "127.0.0.1"

/*

cd DNS_proxy_cerver
clang TEST_ClientUDP.c

./a
./ab

./a.out
./ab.out

*/

int main(int argc, char** argv) {

#if defined _WIN32

    WSADATA wsaData;
    SOCKET ReceiverSocket;

#else

    int ReceiverSocket = 0;

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#endif

    struct sockaddr_in ReceiverAddr;
    int RecvAddrSize = sizeof(ReceiverAddr);
    char RecvBuff[MAX_BUFFER_SIZE];    
    int TotalByteSent;
    int ByteReceived;
    char message[] = "Hello Oleg!!!";

#ifdef _WIN32
    // Initialize Winsock version 2.2
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Client: WSAStartup failed with error %i\n", WSAGetLastError());
        return EXIT_FAILURE;
    }
    printf("Client: The Winsock DLL status is %s.\n", wsaData.szSystemStatus);
#endif

    // Create a new socket to receive datagrams on.
    ReceiverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (ReceiverSocket == INVALID_SOCKET) {        

#ifdef _WIN32
        printf("Client: Error at socket(): %i\n", WSAGetLastError());
        WSACleanup();
        closesocket(ReceiverSocket);
#else
        printf("Client: Error at socket()\n");
        close(ReceiverSocket);
#endif

        // Exit with error
        return EXIT_FAILURE;
    }
    printf("Client: socket() is OK!\n");

    // Set up a SOCKADDR_IN structure that will be used for socket
    ReceiverAddr.sin_family = AF_INET;
    ReceiverAddr.sin_port = htons(SERVER_PORT);
    ReceiverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // Send data packages to the receiver(Server).
    printf("Client: Sending data...\n");
    printf("WE SENT: %s, %i\n", message, sizeof(message));
    TotalByteSent = sendto(ReceiverSocket, message, sizeof(message), 0, (struct sockaddr*)&ReceiverAddr, sizeof(ReceiverAddr));
    printf("Total number of bytes were sent: %i\n", TotalByteSent);
    if (TotalByteSent <= 0) {
        printf("Data was not sent!!!\n");
    }
    else {
        // Receive response
        printf("Waiting for response...\n");
        ByteReceived = recv(ReceiverSocket, RecvBuff, sizeof(RecvBuff), 0);
        if (ByteReceived > 0) {
            printf("Bytes received: %d\n", ByteReceived);
            printf("RESPONSE: %s\n", RecvBuff);
        }
        else if (ByteReceived == 0) {
            printf("Connection closed\n");
        }
        else {
            printf("recv failed\n");
        }
    }
 
#ifdef _WIN32
    // When your application is finished receiving datagrams close the socket.
    if (closesocket(ReceiverSocket) != 0) {
        printf("Client: closesocket() failed! Error code: %i\n", WSAGetLastError());
    }
    printf("Server: closesocket() is OK\n");

    // When your application is finished call WSACleanup.
    if (WSACleanup() != 0) {
        printf("Client: WSACleanup() failed! Error code: %i\n", WSAGetLastError());
    }
    printf("Client: WSACleanup() is OK\n");
#else
    close(ReceiverSocket);
    printf("Server: close() is OK\n");
#endif

    // Back to the system
    return EXIT_SUCCESS;
}