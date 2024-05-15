#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cjson/cJSON.h"
#include "cjson/cJSON.c"

#if defined _WIN32

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <ws2tcpip.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#elif defined __linux__

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#endif

#define MAX_BUFFER_SIZE 1024

#define DEFAULT_PORT 8000


/*

cd DNS_proxy_cerver
clang DNS_proxy_cerver_2.c

./a
./aa

./a.out
./aa.out

*/

typedef struct {
	char IP_ADDRESS[100];
	char ERR_MESS[100];
	int PORT;
} ServerInfo;

int ClientIpChecker(char* ClientIP);
ServerInfo DownloadUppstreamServerData();

int main() {	
	char ReceiveBuf[MAX_BUFFER_SIZE];
	char ReceiveBufUpstream[MAX_BUFFER_SIZE];
	char UPSTREAM_SERVER_IP_ADDR[100];
	char ERR_MESS[100];
	char ClientIP[100];

	// Get UPSTREAM Server information
	ServerInfo ServInformation = DownloadUppstreamServerData();
	int UPSTREAN_SERVER_PORT = ServInformation.PORT;
	strcpy(UPSTREAM_SERVER_IP_ADDR, ServInformation.IP_ADDRESS);
	strcpy(ERR_MESS, ServInformation.ERR_MESS);

	printf("UPSTREAM SERVER: %s, %s, %i\n", UPSTREAM_SERVER_IP_ADDR, ERR_MESS, UPSTREAN_SERVER_PORT);

#if defined _WIN32
	WSADATA wsaData;
	SOCKET ServerSocket = INVALID_SOCKET;
	SOCKET SocketForUpstreamServer = INVALID_SOCKET;	
	SOCKADDR_IN ReceiverAddr;
	SOCKADDR_IN ReceiverAddrCli;
	SOCKADDR_IN SenderAddr;

	// Initialize Winsock version 2.2
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("Server: WSAStartup failed with error: %i\n", WSAGetLastError());
		return EXIT_FAILURE;
	}
	printf("Server: The Winsock DLL status is %s.\n", wsaData.szSystemStatus);

#else
	int ServerSocket = 0;
	int SocketForUpstreamServer = 0;
	struct sockaddr_in ReceiverAddr;
	struct sockaddr_in ReceiverAddrCli;
	struct sockaddr_in SenderAddr;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#endif

	int SenderAddrSize = sizeof(SenderAddr);
	int ByteReceived = 0;
	int BytesSent = 0;
	int ButesSentToUpstreamServer = 0;

	// -------------------------- SERVER PART ------------------------------- //
	// Create ServerSocket
	ServerSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (ServerSocket < 0 || ServerSocket == INVALID_SOCKET) {
		printf("Creating ProxyServer Server socket failed.\n");

#ifdef _WIN32
		WSACleanup();
#endif

		return EXIT_FAILURE;
	}
	printf("Socket ProxyServer Server created successfully");

	// Create Address structure to bind it ot ServerSocket
	ReceiverAddr.sin_family = AF_INET;
	ReceiverAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	ReceiverAddr.sin_port = htons(DEFAULT_PORT);

	// Bind address to ServerSocket
#ifdef _WIN32
	if (bind(ServerSocket, (SOCKADDR*)&ReceiverAddr, sizeof(ReceiverAddr)) == SOCKET_ERROR) {
#else
	if (bind(ServerSocket, (struct sockaddr*)&ReceiverAddr, sizeof(ReceiverAddr)) < 0) {
#endif
		printf("ProxyServer Server: Error! bind() failed!\n");

		// Close the socket
#ifdef _WIN32
		closesocket(ServerSocket);
		WSACleanup();
#else
		close(ServerSocket);
#endif
		// Exit with error
		return EXIT_FAILURE;
	}
	printf("ProxyServer Server: bind() is OK!\n");


	// -------------------------- CLIENT PART ------------------------------- //
	// Create ClientSocket
	SocketForUpstreamServer = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (SocketForUpstreamServer < 0 || SocketForUpstreamServer == INVALID_SOCKET) {
		printf("Creating ProxyServer Client socket failed.\n");

#ifdef _WIN32
		WSACleanup();
#endif

		return EXIT_FAILURE;
	}
	printf("Socket ProxyServer Client successfully");

	// Create Address structure to bind it to ClientSocket
	ReceiverAddrCli.sin_family = AF_INET;
	//ReceiverAddrCli.sin_addr.s_addr = inet_addr(UPSTREAM_SERVER_IP_ADDR);
#ifdef _WIN32
	inet_pton(AF_INET, UPSTREAM_SERVER_IP_ADDR, (PVOID)&ReceiverAddrCli.sin_addr.s_addr);
#else
	inet_pton(AF_INET, UPSTREAM_SERVER_IP_ADDR, (void*)&ReceiverAddrCli.sin_addr.s_addr);
#endif
	ReceiverAddrCli.sin_port = htons(UPSTREAN_SERVER_PORT);


	// ----------------------- GET DATA FROM CLIENT --------------------------- //
	printf("Start receiving...\n");
	while (1) {
		printf("------------------------------------------\n");
#ifdef _WIN32
		ByteReceived = recvfrom(ServerSocket, ReceiveBuf, sizeof(ReceiveBuf), 0, (SOCKADDR*)&SenderAddr, &SenderAddrSize);
#else
		ByteReceived = recvfrom(ServerSocket, ReceiveBuf, sizeof(ReceiveBuf), 0, (struct sockaddr*)&SenderAddr, &SenderAddrSize);
#endif
		printf("Bytes received: %i\n", ByteReceived);
		printf("MESSAGE RECEIVED: %s\n", ReceiveBuf);

		if (ByteReceived > 0) { 

			// Get Client IP address
			//char* ClientIP = inet_ntoa(SenderAddr.sin_addr);	
#ifdef _WIN32		
			inet_ntop(AF_INET, (PCSTR)&SenderAddr.sin_addr, ClientIP, sizeof(ClientIP));
#else
			inet_ntop(AF_INET, &SenderAddr.sin_addr, ClientIP, sizeof(ClientIP));
#endif
			printf("Client IP address: %s\n", ClientIP);

			if (!ClientIpChecker(ClientIP)) {
				printf("ERROR RESPONSE TO CLIENT: %s\n", ERR_MESS);

				// Error response to client
#ifdef _WIN32
				BytesSent = sendto(ServerSocket, ERR_MESS, sizeof(ERR_MESS), 0, (SOCKADDR*)&SenderAddr, sizeof(SenderAddr));
#else
				BytesSent = sendto(ServerSocket, ERR_MESS, sizeof(ERR_MESS), 0, (struct sockaddr*)&SenderAddr, sizeof(SenderAddr));
#endif
				printf("Bytes sent: %d\n", BytesSent);
			}
			else {
				// Send message to UPSTREAM Server
#ifdef _WIN32
				ButesSentToUpstreamServer = sendto(SocketForUpstreamServer, ReceiveBuf, ByteReceived, 0, (SOCKADDR*)&ReceiverAddrCli, sizeof(ReceiverAddrCli));
#else
				ButesSentToUpstreamServer = sendto(SocketForUpstreamServer, ReceiveBuf, ByteReceived, 0, (struct sockaddr*)&ReceiverAddrCli, sizeof(ReceiverAddrCli));
#endif
				printf("WE SENT TO UPSTREAMServer: %s, %i\n", ReceiveBuf, ByteReceived);

				// Receive response from UPSTREAM Server
				ByteReceived = recv(SocketForUpstreamServer, ReceiveBufUpstream, sizeof(ReceiveBufUpstream), 0);				

				// Send response from UPSTREAM Server to client
#ifdef _WIN32	
				if (ByteReceived > 0) {
					BytesSent = sendto(ServerSocket, ReceiveBufUpstream, ByteReceived, 0, (SOCKADDR*)&SenderAddr, sizeof(SenderAddr));
					printf("WE RECEIVED FROM UPSTREAMServer: %s, %i\n", ReceiveBufUpstream, ByteReceived);
				}
				else {
					printf("UPSTREAMServer does not response\n");
					BytesSent = sendto(ServerSocket, ERR_MESS, sizeof(ERR_MESS), 0, (SOCKADDR*)&SenderAddr, sizeof(SenderAddr));
				}
#else
				if (ByteReceived > 0) {
					BytesSent = sendto(ServerSocket, ReceiveBufUpstream, ByteReceived, 0, (struct sockaddr*)&SenderAddr, sizeof(SenderAddr));
					printf("WE RECEIVED FROM UPSTREAMServer: %s, %i\n", ReceiveBufUpstream, ByteReceived);
				}
				else {
					printf("UPSTREAMServer does not response\n");
					BytesSent = sendto(ServerSocket, ERR_MESS, sizeof(ERR_MESS), 0, (struct sockaddr*)&SenderAddr, sizeof(SenderAddr));
				}
#endif
				printf("Bytes sent: %d\n", BytesSent);
			}
			printf("\n");
		}
		else if (ByteReceived <= 0) { 

			//If the buffer is empty	
#ifdef _WIN32
			printf("Server: Connection closed with error code: %i\n", WSAGetLastError());
#else
			printf("Server: Connection closed\n");
#endif
		}
		else { 

			//If error
#ifdef _WIN32
			printf("Server: recvfrom() failed with error code: %i\n", WSAGetLastError());
#else
			printf("Server: recvfrom() failed\n");
#endif
		}
	}


	// ----------------------- FINISH RECEIVING DATA --------------------------- //
#if defined _WIN32
	if (closesocket(ServerSocket) != 0) {
		printf("ProxyServer Server: was not closed!\n");
	}
	printf("ProxyServer Server: closed successfully!\n");

	if (closesocket(SocketForUpstreamServer) != 0) {
		printf("ProxyServer Client: was not closed!\n");
	}
	printf("ProxyServer Client: closed successfully!\n");

	if (WSACleanup() != 0) {
		printf("WSACleanup() failed! Error code: %i\n", WSAGetLastError());
	}
	printf("WSACleanup() is OK\n");

#else
	close(ServerSocket);
	close(SocketForUpstreamServer);
#endif

	printf("SUCCESS!");
	return EXIT_SUCCESS;
}


// Check Client IP
int ClientIpChecker(char* ClientIP) {
	FILE* fp = NULL;
	char buffer[1024];
	size_t buffLen;
	cJSON* ptr = NULL;
	cJSON* json = NULL;
	cJSON* BlackList = NULL;

	fp = fopen("data.json", "r");
	if (fp == NULL) {
		printf("Error: Unable to open the file.\n");
		return 1;
	}

	// read the file contents into a string 	
	buffLen = fread(buffer, 1, sizeof(buffer), fp);
	fclose(fp);

	// parse the JSON data 
	json = cJSON_Parse(buffer);
	if (json == NULL) {
		const char* error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL) {
			printf("Error: %s\n", error_ptr);
		}
		cJSON_Delete(json);
		exit(EXIT_FAILURE);
	}

	BlackList = cJSON_GetObjectItemCaseSensitive(json, "BLACK_LIST");
	if (cJSON_IsArray(BlackList)) {  
		ptr = NULL;
		for (ptr = BlackList->child; ptr != NULL; ptr = ptr->next) {
			printf("%s\n", ptr->valuestring);

			if (!strcmp(ClientIP, ptr->valuestring)) {
				printf("CLIENT IP IS IN THE BLACK LIST!!!\n");

				// delete the JSON object 
				cJSON_Delete(json);
				return 0;
			}

		}
	}

	// delete the JSON object 
	cJSON_Delete(json);

	return 1;
}


ServerInfo DownloadUppstreamServerData() {
	FILE* fp = NULL;
	ServerInfo SE;
	SE.PORT = 0;
	char buffer[1024];
	size_t buffLen;
	cJSON* json = NULL;
	cJSON* UpstreamServerIP = NULL;
	cJSON* ErrMess = NULL;
	cJSON* UpstreamServerPORT = NULL;

	fp = fopen("data.json", "r");
	if (fp == NULL) {
		printf("Error: Unable to open the file.\n");
		exit(EXIT_FAILURE);
	}

	// read the file contents into a string 	
	buffLen = fread(buffer, 1, sizeof(buffer), fp);
	fclose(fp);

	// parse the JSON data 
	json = cJSON_Parse(buffer);
	if (json == NULL) {
		const char* error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL) {
			printf("Error: %s\n", error_ptr);
		}
		cJSON_Delete(json);
		exit(EXIT_FAILURE);
	}

	UpstreamServerIP = cJSON_GetObjectItemCaseSensitive(json, "UPSTREAM_SERVER_IP");
	if (!cJSON_IsString(UpstreamServerIP) || (UpstreamServerIP->valuestring == NULL)) {
		printf("SERVER IP ERROR!\n");
		exit(EXIT_FAILURE);
	}
	printf("SERVER IP: %s\n", UpstreamServerIP->valuestring);
	strcpy(SE.IP_ADDRESS, UpstreamServerIP->valuestring);

	ErrMess = cJSON_GetObjectItemCaseSensitive(json, "RESPONSES");
	if (!cJSON_IsString(ErrMess) || (ErrMess->valuestring == NULL)) {
		printf("ERR_MESS ERROR!\n");
		exit(EXIT_FAILURE);
	}
	printf("ERROR NESSAGE: %s\n", ErrMess->valuestring);
	strcpy(SE.ERR_MESS, ErrMess->valuestring);

	UpstreamServerPORT = cJSON_GetObjectItemCaseSensitive(json, "UPSTREAM_SERVER_PORT");
	if (!cJSON_IsNumber(UpstreamServerPORT)) {
		printf("SERVER PORT ERROR\n");
		exit(EXIT_FAILURE);
	}
	printf("SERVER PORT: %i\n", UpstreamServerPORT->valueint);
	SE.PORT = UpstreamServerPORT->valueint;

	// delete the JSON object 
	cJSON_Delete(json);

	return SE;
}