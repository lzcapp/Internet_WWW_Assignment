///////////////////////////////////////////////////////////////////////////////////////
// Client.cpp                                                                        //
//    Defines the entry point for the console application.                           //
// 1. open the *.c in the Visual C++, then "rebuild all".                            //
// 2. click "yes" to create a project workspace.                                     //
// 3. You need to -add the library 'ws2_32.lib' to your project                      //
//    (Project -> Properties -> Linker -> Input -> Additional Dependencies)          //
// 4. recompile the source.                                                          //
///////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_PORT 5019

int main(int argc, char **argv)
{
	char szBuff[100];
	int msg_len;
	//int addr_len;
	struct sockaddr_in server_addr;
	struct hostent *hp;
	SOCKET connect_sock;
	WSADATA wsaData;

	char *server_name = "localhost";
	unsigned short port = DEFAULT_PORT;
	unsigned int addr;
	while (1)
	{
		if (argc != 3)
		{
			printf("Client Usage: echoscln [server name] [port number]\n\n");
			printf("Please Run in CMD: client 127.0.0.1 5019\n\n");
			return -1;
		}
		else
		{
			server_name = argv[1];
			port = atoi(argv[2]);
		}

		if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR)
		{
			// stderr: standard error are printed to the screen.
			fprintf(stderr, "WSAStartup Failed with ERROR %d\n", WSAGetLastError());
			// WSACleanup function terminates use of the Windows Sockets DLL.
			WSACleanup();
			return -1;
		}

		if (isalpha(server_name[0]))
			hp = gethostbyname(server_name);
		else
		{
			addr = inet_addr(server_name);
			hp = gethostbyaddr((char *)&addr, 4, AF_INET);
		}

		if (hp == NULL)
		{
			fprintf(stderr, "Cannot Resolve Address: %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}

		// copy the resolved information into the sockaddr_in structure
		memset(&server_addr, 0, sizeof(server_addr));
		memcpy(&(server_addr.sin_addr), hp->h_addr, hp->h_length);
		server_addr.sin_family = hp->h_addrtype;
		server_addr.sin_port = htons(port);

		// TCP socket
		connect_sock = socket(AF_INET, SOCK_STREAM, 0);

		if (connect_sock == INVALID_SOCKET)
		{
			fprintf(stderr, "socket() Failed with ERROR %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}

		printf("\nEstablishing Connecting to Sever: %s\n", hp->h_name);

		if (connect(connect_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
		{
			fprintf(stderr, "connect() Failed with ERROR %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}

		///// Program Output Starts /////
		char modle[3];

		///// Welcome Interface /////
		printf("\n");
		printf("     ∧  ∧＿__    ╭─────────────────────────╮  \n");
		printf("  ／(*ﾟーﾟ) ／＼ ＜　Online Phone Book System ┃  \n");
		printf(" ／|￣∪∪￣|＼／ ╰━━━━━━━━━━━━━━━━━━━━━━━━━━━╯  \n");
		printf("   |        |／  \n");
		printf("    ￣￣￣￣      \n");
		printf("\n");
		printf("Connected Successfully!\n");
		printf("\n");
		printf("   |￣￣￣￣￣￣|   \n");
		printf("   |  Welcome!  |   \n");
		printf("   |＿＿＿＿＿＿|     \n");
		printf("         ||           \n");
		printf(" 　∧ ∧ ||              \n");
		printf("  ( ﾟдﾟ)||              \n");
		printf("  /　づ  Φ               \n");
		printf("\n");
		///// Welcome Interface Ends /////

		// Module Selection
		printf("Choose the Function: [Q]uery [A]dd [D]elete [E]xit: ");
		gets_s(modle);

		// Send the message
		msg_len = send(connect_sock, modle, sizeof(modle), 0);

		if (msg_len == SOCKET_ERROR)
		{
			fprintf(stderr, "send() Failed with ERROR %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}

		if (msg_len == 0)
		{
			printf("Server Closed the Connection\n");
			closesocket(connect_sock);
			WSACleanup();
			return -1;
		}

		///// Modules /////
		///// Query Module /////
		printf("\n");
		if (strcmp(modle, "Q") == 0 || strcmp(modle, "q") == 0)
		{
			printf("Please Input the Name: ");
			gets_s(szBuff);
			printf("\n");

			msg_len = send(connect_sock, szBuff, sizeof(szBuff), 0);

			if (msg_len == SOCKET_ERROR)
			{
				fprintf(stderr, "send() Failed with ERROR %d\n", WSAGetLastError());
				WSACleanup();
				return -1;
			}

			if (msg_len == 0)
			{
				printf("Server Closed the Connection\n");
				closesocket(connect_sock);
				WSACleanup();
				return -1;
			}

			msg_len = recv(connect_sock, szBuff, sizeof(szBuff), 0);

			if (msg_len == SOCKET_ERROR)
			{
				fprintf(stderr, "send() Failed with ERROR %d\n", WSAGetLastError());
				closesocket(connect_sock);
				WSACleanup();
				return -1;
			}

			if (msg_len == 0)
			{
				printf("Server Closed the Connection\n");
				closesocket(connect_sock);
				WSACleanup();
				return -1;
			}

			if (strcmp(szBuff, "NOTFOUND") == 0) {
				printf("\nResult from the Server:\n");
				printf("[ERROR] The Name is not Found in the Data.\n");
			} else {
				printf("\nResult from the Server:\n");
				printf("Name\t\tPhone No.\tStudnet ID\t\n");
				printf("%s\n", szBuff);
			}
		}
		///// Query Module Ends /////

		///// Add Module /////
		else if (strcmp(modle, "A") == 0 || strcmp(modle, "a") == 0)
		{

			// send the name to the sever
			printf("\nInput the Name of Person to Add: ");
			gets_s(szBuff);
			//printf("\n");

			msg_len = send(connect_sock, szBuff, sizeof(szBuff), 0);

			if (msg_len == SOCKET_ERROR)
			{
				fprintf(stderr, "send() Failed with ERROR %d\n", WSAGetLastError());
				WSACleanup();
				return -1;
			}

			if (msg_len == 0)
			{
				printf("Server Closed the Connection\n");
				closesocket(connect_sock);
				WSACleanup();
				return -1;
			}

			///////////send the phone number to the sever////////
			printf("Please Input the Phone No.: ");
			gets_s(szBuff);
			//printf("\n");

			msg_len = send(connect_sock, szBuff, sizeof(szBuff), 0);

			if (msg_len == SOCKET_ERROR)
			{
				fprintf(stderr, "send() Failed with ERROR %d\n", WSAGetLastError());
				WSACleanup();
				return -1;
			}

			if (msg_len == 0)
			{
				printf("Server Closed the Connection\n");
				closesocket(connect_sock);
				WSACleanup();
				return -1;
			}

			//////////////////////////////////

			///////////send the Fax to the sever////////
			printf("Please Input the Student ID: ");
			gets_s(szBuff);
			printf("\n");

			msg_len = send(connect_sock, szBuff, sizeof(szBuff), 0);

			if (msg_len == SOCKET_ERROR)
			{
				fprintf(stderr, "send() Failed with ERROR %d\n", WSAGetLastError());
				WSACleanup();
				return -1;
			}

			if (msg_len == 0)
			{
				printf("Server Closed the Connection\n");
				closesocket(connect_sock);
				WSACleanup();
				return -1;
			}
		}
		///// Add Module Ends /////

		///// Delete Module /////
		else if (strcmp(modle, "D") == 0 || strcmp(modle, "d") == 0)
		{
			char a[10];
			printf("\nWhose Information to Delete? ");
			gets_s(a);
			printf("\n");

			msg_len = send(connect_sock, a, sizeof(a), 0);

			if (msg_len == SOCKET_ERROR)
			{
				fprintf(stderr, "send() Failed with ERROR %d\n", WSAGetLastError());
				WSACleanup();
				return -1;
			}

			if (msg_len == 0)
			{
				printf("Server Closed the Connection\n");
				closesocket(connect_sock);
				WSACleanup();
				return -1;
			}
		}
		///// Delete Module Ends /////

		///// Exit Module /////
		else if (strcmp(modle, "E") == 0 || strcmp(modle, "e") == 0)
		{
			printf("Client Exiting...\n");
			break;
		}
		///// Exit Module Ends /////

		closesocket(connect_sock);
		WSACleanup();
	}
}
