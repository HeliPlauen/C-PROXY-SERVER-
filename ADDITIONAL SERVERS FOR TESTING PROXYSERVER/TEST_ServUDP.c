#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
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


#define DEFAULT_PORT 8888
#define MAX_BUFFER_SIZE 1024


/*

cd DNS_proxy_cerver
clang TEST_ServUDP.c

./a
./ac

./a.out
./ac.out

*/

int main(int argc, char** argv) {

#if defined _WIN32

    WSADATA wsaData;
    SOCKET ReceivingSocket = INVALID_SOCKET;

#else
    int ReceivingSocket = 0;

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
    char ReceiveBuf[MAX_BUFFER_SIZE];
    struct sockaddr_in SenderAddr;
    int SenderAddrSize = sizeof(SenderAddr);
    int ByteReceived = 0;
    int ByteSent = 0;
    char ResponseMessage[] = "Hello Murka! Hello Murka! Hello Murka!";

#ifdef _WIN32
    // Initialize Winsock version 2.2
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("UPSTREAMServer: WSAStartup failed with error: %i\n", WSAGetLastError());
        return EXIT_FAILURE;
    }
    printf("UPSTREAMServer: The Winsock DLL status is: %s.\n", wsaData.szSystemStatus);
#endif

    // Create a new UPSTREAM socket to receive datagrams on.
    ReceivingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (ReceivingSocket == INVALID_SOCKET || ReceivingSocket < 0) {
        printf("UPSTREAMServer: Error at socket()\n");

#ifdef _WIN32
        // Clean up
        WSACleanup();
#endif

        // Exit with error
        return EXIT_FAILURE;
    }
    printf("UPSTREAMServer: socket() is OK!\n");

    // Set up a SOCKADDR_IN structure that will be bind to UPSTREAM socket
    ReceiverAddr.sin_family = AF_INET;
    ReceiverAddr.sin_port = htons(DEFAULT_PORT);
    ReceiverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Associate the address information with the socket using bind.
    if (bind(ReceivingSocket, (struct sockaddr*)&ReceiverAddr, sizeof(ReceiverAddr)) == SOCKET_ERROR) {
        printf("UPSTREAMServer: Error! bind() failed!\n");

#ifdef _WIN32
        closesocket(ReceivingSocket);
        WSACleanup();
#else
        close(ReceivingSocket);
#endif

        // and exit with error
        return EXIT_FAILURE;
    }
    printf("UPSTREAMServer: bind() is OK!\n");

    // At this point you can receive datagrams on your bound socket.
    printf("UPSTREAMServer: start receiving...\n");
    while (true) { 
        printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$4\n");
        
        // Server is receiving data until you will close it.(You can replace while(1) with a condition to stop receiving.)
        ByteReceived = recvfrom(ReceivingSocket, ReceiveBuf, sizeof(ReceiveBuf), 0, (struct sockaddr*)&SenderAddr, &SenderAddrSize);

        if (ByteReceived > 0) { 
            printf("UPSTREAMServer: Total Bytes received: %d\n", ByteReceived);
            printf("UPSTREAMServer: MESSAGE RECEIVED: %s\n", ReceiveBuf);
            printf("\n");
        }
        else if (ByteReceived <= 0) { 
            printf("UPSTREAMServer: Connection closed\n");
        }
        else {
            printf("UPSTREAMServer: recvfrom() failed\n");
        }

        // Send response to client
        printf("SEND RESPONSE: %s, %i\n", ResponseMessage, sizeof(ResponseMessage));
        ByteSent = sendto(ReceivingSocket, ResponseMessage, sizeof(ResponseMessage), 0, (struct sockaddr*)&SenderAddr, sizeof(SenderAddr));
        if (ByteSent == 0) {
            printf("send failed\n");

#ifdef _WIN32
            closesocket(ReceivingSocket);
            WSACleanup();
#else
            close(ReceivingSocket);
#endif

            return EXIT_FAILURE;
        }
        printf("Bytes sent: %d\n", ByteSent);
    }

#ifdef _WIN32
    // When your application is finished receiving datagrams close the socket.
    if (closesocket(ReceivingSocket) != 0) {
        printf("UPSTREAMServer: closesocket() failed! Error code: %i\n", WSAGetLastError());
    }
    printf("UPSTREAMServer: closesocket() is OK\n");

    // When your application is finished call WSACleanup.
    if (WSACleanup() != 0) {
        printf("UPSTREAMServer: WSACleanup() failed! Error code: %i\n", WSAGetLastError());
    }
    printf("UPSTREAMServer: WSACleanup() is OK\n");
#else
    close(ReceivingSocket);
    printf("UPSTREAMServer: close() is OK\n");
#endif

    // Back to the system
    return EXIT_SUCCESS;
}