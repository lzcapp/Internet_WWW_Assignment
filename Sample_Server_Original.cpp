// Server.cpp : Defines the entry point for the console application.
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
	int addr_len;
	struct sockaddr_in local, client_addr;

	SOCKET sock, msg_sock;
	WSADATA wsaData;

	if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR){
		// stderr: standard error are printed to the screen.
		fprintf(stderr, "WSAStartup failed with error %d\n", WSAGetLastError());
		//WSACleanup function terminates use of the Windows Sockets DLL. 
		WSACleanup();
		return -1;
	}
	// Fill in the address structure
	local.sin_family		= AF_INET;
	local.sin_addr.s_addr	= INADDR_ANY;
	local.sin_port		= htons(DEFAULT_PORT);

	sock = socket(AF_INET,SOCK_STREAM, 0);	//TCp socket


	if (sock == INVALID_SOCKET){
		fprintf(stderr, "socket() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	if (bind(sock, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR){
		fprintf(stderr, "bind() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}


	//waiting the connection
	if (listen(sock, 5) == SOCKET_ERROR){
		fprintf(stderr, "listen() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	
	printf("Waiting the connection........\n");

	addr_len = sizeof(client_addr);
	msg_sock = accept(sock, (struct sockaddr*)&client_addr, &addr_len);
	if (msg_sock == INVALID_SOCKET){
		fprintf(stderr, "accept() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	printf("accepted connection from %s, port %d\n",
		inet_ntoa(client_addr.sin_addr),
		htons(client_addr.sin_port));

	msg_len = recv(msg_sock, szBuff, sizeof(szBuff), 0);

	if (msg_len == SOCKET_ERROR){
		fprintf(stderr, "recv() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	if (msg_len == 0){
		printf("Client closed connection\n");
		closesocket(msg_sock);
		return -1;
	}

	printf("Bytes Received: %d, message: %s from %s\n", msg_len, szBuff, inet_ntoa(client_addr.sin_addr));
	
	msg_len = send(msg_sock, szBuff, sizeof(szBuff), 0);
	if (msg_len == 0){
		printf("Client closed connection\n");
		closesocket(msg_sock);
		return -1;
	}

	closesocket(msg_sock);
	WSACleanup();
}
		
