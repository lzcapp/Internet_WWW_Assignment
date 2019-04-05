// Client.cpp : Defines the entry point for the console application.
//
// 1. open the *.c in the Visual C++, then "rebuild all".
// 2. click "yes" to create a project workspace.
// 3. You need to -add the library 'ws2_32.lib' to your project 
//    (Project -> Properties -> Linker -> Input -> Additional Dependencies) 
// 4. recompile the source.


#include "stdafx.h"
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define DEFAULT_PORT	5019


int main(int argc, char **argv){
	
	char szBuff[100];
	int msg_len;
	//int addr_len;
	struct sockaddr_in server_addr;
	struct hostent *hp;
	SOCKET connect_sock;
	WSADATA wsaData;

	char			*server_name = "localhost";
	unsigned short	port = DEFAULT_PORT;
	unsigned int	addr;

	if (argc != 3){
		printf("echoscln [server name] [port number]\n");
		return -1;
	}
	else{
		server_name = argv[1];
		port = atoi(argv[2]);
	}

	if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR){
		// stderr: standard error are printed to the screen.
		fprintf(stderr, "WSAStartup failed with error %d\n", WSAGetLastError());
		//WSACleanup function terminates use of the Windows Sockets DLL. 
		WSACleanup();
		return -1;
	}

	if (isalpha(server_name[0]))
		hp = gethostbyname(server_name);
	else{
		addr = inet_addr(server_name);
		hp = gethostbyaddr((char*)&addr, 4, AF_INET);
	}

	if (hp==NULL)
	{
		fprintf(stderr, "Cannot resolve address: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	//copy the resolved information into the sockaddr_in structure
	memset(&server_addr, 0, sizeof(server_addr));
	memcpy(&(server_addr.sin_addr), hp->h_addr, hp->h_length);
	server_addr.sin_family = hp->h_addrtype;
	server_addr.sin_port = htons(port);

	
	connect_sock = socket(AF_INET,SOCK_STREAM, 0);	//TCp socket


	if (connect_sock == INVALID_SOCKET){
		fprintf(stderr, "socket() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	printf("Client connecting to: %s\n", hp->h_name);

	if (connect(connect_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) 
		== SOCKET_ERROR){
		fprintf(stderr, "connect() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}


	printf("input character string:\n");
	gets(szBuff);
	
	msg_len = send(connect_sock, szBuff, sizeof(szBuff), 0);
	
	if (msg_len == SOCKET_ERROR){
		fprintf(stderr, "send() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	if (msg_len == 0){
		printf("server closed connection\n");
		closesocket(connect_sock);
		WSACleanup();
		return -1;
	}

	msg_len = recv(connect_sock, szBuff, sizeof(szBuff), 0);
	
	if (msg_len == SOCKET_ERROR){
		fprintf(stderr, "send() failed with error %d\n", WSAGetLastError());
		closesocket(connect_sock);
		WSACleanup();
		return -1;
	}

	if (msg_len == 0){
		printf("server closed connection\n");
		closesocket(connect_sock);
		WSACleanup();
		return -1;
	}

	printf("Echo from the server %s.\n", szBuff);

	closesocket(connect_sock);
	WSACleanup();
}
