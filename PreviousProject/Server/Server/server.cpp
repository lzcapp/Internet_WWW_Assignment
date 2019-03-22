///////////////////////////////////////////////////////////////////////////////////////
// Server.cpp                                                                        //
//    create a console application, and include the sources in the project           //
// 1. open the *.c in the Visual C++, then "rebuild all".                            //
// 2. click "yes" to create a project workspace.                                     //
// 3. You need to -add the library 'ws2_32.lib' to your project.                     //
//    (Project -> Properties -> Linker -> Input -> Additional Dependencies)          //
// 4. recompile the source.                                                          //
///////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_PORT 5019

struct Client
{
	char name[20];
	char mobile[20];
	char fax[20];
	int nameCount;
	int mobileCount;
	int faxCount;
};

DWORD WINAPI CommFunction(LPVOID lpParameter)
{
	int msg_sock = *((int *)lpParameter);
	int msg_len;
	while (1)
	{
		char szBuff[100];
		int msg_len;
		int addr_len;
		Client client[100];

		SOCKET sock;
		WSADATA wsaData;

		///// revice the module choice from client /////
		char modle[3];
		msg_len = recv(msg_sock, modle, sizeof(modle), 0);

		if (msg_len == SOCKET_ERROR)
		{
			fprintf(stderr, "recv() Failed with ERROR %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}
		if (msg_len == 0)
		{
			printf("Client Closed the Connection\n");
			closesocket(msg_sock);
			return -1;
		}

		printf("[Client Input] Module: %s (Bytes Received: %d)\n\n", modle, msg_len);
		///// receive module end /////

		FILE *fp;
		fp = fopen("abc.txt", "a+");

		///// query module /////
		if (strcmp(modle, "Q") == 0 || strcmp(modle, "q") == 0)
		{
			printf("[Query Module]\n");
			msg_len = recv(msg_sock, szBuff, sizeof(szBuff), 0);
			if (msg_len == SOCKET_ERROR)
			{
				fprintf(stderr, "recv() Failed with ERROR %d\n", WSAGetLastError());
				WSACleanup();
				return -1;
			}

			if (msg_len == 0)
			{
				printf("Client Closed the Connection\n");
				closesocket(msg_sock);
				return -1;
			}

			printf("[Client Input] Message: %s (Bytes Received: %d)\n\n", szBuff, msg_len);

			int row = 0, k = 1;
			while (fscanf(fp, "%s", &client[row].name) != EOF)
			{
				fscanf(fp, "%s", &client[row].mobile);
				fscanf(fp, "%s", &client[row].fax);
				client[row].nameCount = strlen(client[row].name);
				client[row].mobileCount = strlen(client[row].mobile);
				client[row].faxCount = strlen(client[row].fax);
				row++;
			}
			//printf("%d\n", row);
			for (k = 0; k < row; k++)
			{
				if (strcmp(szBuff, client[k].name) != 0)
				{
					if (strcmp(szBuff, client[k].mobile) != 0)
					{
						//Update//
						if (strcmp(szBuff, client[k].fax) != 0)
						{
							continue;
						}
					}
				}
				break;
			}
			if (k < row) {
				strcpy(szBuff, strcat(client[k].name, "\t"));
				strcat(szBuff, strcat(client[k].mobile, "\t"));
				strcat(szBuff, strcat(client[k].fax, "\t"));
				printf("Send Message to Client: %s\n", szBuff);
				msg_len = send(msg_sock, szBuff, sizeof(szBuff), 0);
			} else {
				msg_len = send(msg_sock, "NOTFOUND", sizeof("NOTFOUND"), 0);
			}
			
			if (msg_len == 0)
			{
				printf("Client Closed the Connection\n");
				closesocket(msg_sock);
				return -1;
			}
		}
		///// query moule end /////

		///// add module /////
		else if (strcmp(modle, "A") == 0 || strcmp(modle, "a") == 0)
		{
			printf("[Add Module]\n");
			// recvive the name from the client
			char name[100];
			msg_len = recv(msg_sock, szBuff, sizeof(szBuff), 0);
			if (msg_len == SOCKET_ERROR)
			{
				fprintf(stderr, "recv() Failed with ERROR %d\n", WSAGetLastError());
				WSACleanup();
				return -1;
			}

			if (msg_len == 0)
			{
				printf("Client Closed the Connection\n");
				closesocket(msg_sock);
				return -1;
			}
			strcpy(name, szBuff);
			printf("[Client Input] Message: %s (Bytes Received: %d)\n\n", szBuff, msg_len);

			// recvive the phone number from the client
			char phone[100];
			msg_len = recv(msg_sock, szBuff, sizeof(szBuff), 0);
			if (msg_len == SOCKET_ERROR)
			{
				fprintf(stderr, "recv() Failed with ERROR %d\n", WSAGetLastError());
				WSACleanup();
				return -1;
			}

			if (msg_len == 0)
			{
				printf("Client Closed the Connection\n");
				closesocket(msg_sock);
				return -1;
			}
			strcpy(phone, szBuff);
			
			printf("[Client Input] Message: %s (Bytes Received: %d)\n\n", szBuff, msg_len);

			// recvive the Fax number from the client
			char fax[100];
			msg_len = recv(msg_sock, szBuff, sizeof(szBuff), 0);
			if (msg_len == SOCKET_ERROR)
			{
				fprintf(stderr, "recv() Failed with ERROR %d\n", WSAGetLastError());
				WSACleanup();
				return -1;
			}

			if (msg_len == 0)
			{
				printf("Client Closed the Connection\n");
				closesocket(msg_sock);
				return -1;
			}
			strcpy(fax, szBuff);

			printf("[Client Input] Message: %s (Bytes Received: %d)\n\n", szBuff, msg_len);

			fprintf(fp, "\n");

			fprintf(fp, "%s %s %s", name, phone, fax);

			msg_len = send(msg_sock, szBuff, sizeof(szBuff), 0);
			if (msg_len == 0)
			{
				printf("Client Closed the Connection\n");
				closesocket(msg_sock);
				return -1;
			}
			//Update//
			fclose(fp);
		}
		///// add module end /////

		///// delete module /////
		else if (strcmp(modle, "D") == 0 || strcmp(modle, "d") == 0)
		{
			printf("[Delete Module]\n");
			char a[10];
			msg_len = recv(msg_sock, a, sizeof(a), 0);
			if (msg_len == SOCKET_ERROR)
			{
				fprintf(stderr, "recv() Failed with ERROR %d\n", WSAGetLastError());
				WSACleanup();
				return -1;
			}

			if (msg_len == 0)
			{
				printf("Client Closed the Connection\n");
				closesocket(msg_sock);
				return -1;
			}

			printf("%s", a);
			FILE *fp2;
			fp2 = fopen("abc.txt", "r");

			int row = 0, k;
			while (fscanf(fp2, "%s", &client[row].name) != EOF)
			{
				fscanf(fp2, "%s", &client[row].mobile);
				fscanf(fp2, "%s", &client[row].fax);
				client[row].nameCount = strlen(client[row].name);
				client[row].mobileCount = strlen(client[row].mobile);
				client[row].faxCount = strlen(client[row].fax);
				row++;
			}

			for (k = 0; k < row; k++)
			{
				if (strcmp(a, client[k].name) != 0)
				{
					if (strcmp(a, client[k].mobile) != 0)
					{
						if (strcmp(a, client[k].name) != 0)
						{
							continue;
						}
					}
				}
				break;
			}

			char buf[4096];
			FILE *fp1 = fopen("abc.txt", "r");

			FILE *fpt = fopen("temp.txt", "w");
			int i = 0;
			while (!feof(fp1))
			{
				i++;
				if (i == k + 1)
				{
					fgets(buf, sizeof(buf), fp1);
				}
				else
				{
					fgets(buf, sizeof(buf), fp1);
					fprintf(fpt, "%s", buf);
				}
			}
			fclose(fp1);
			fclose(fpt);

			fpt = fopen("temp.txt", "r");

			fp1 = fopen("abc.txt", "w+");
			fclose(fp1);

			fp1 = fopen("abc.txt", "a");
			while (!feof(fpt))
			{
				fgets(buf, sizeof(buf), fpt);
				fprintf(fp1, "%s", buf);
			}

			fclose(fp1);
			fclose(fpt);

			system("ERASE temp.txt");

			printf("%d\n", k);
		}
		///// module delete ends /////

		///// module exit /////
		else if (strcmp(modle, "E") == 0 || strcmp(modle, "e") == 0)
		{
			printf("[Exit Module]\n");
			printf("This Connection Ends at Client's Request\n");
			return 0;
		}
		///// module exit ends /////
	}
}

int main(int argc, char **argv)
{
	while (1)
	{
		char szBuff[100];
		int msg_len;
		int addr_len;
		struct sockaddr_in local, client_addr;
		Client client[100];

		SOCKET sock, msg_sock;
		WSADATA wsaData;

		if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR)
		{
			// stderr: standard error are printed to the screen.
			fprintf(stderr, "WSAStartup Failed with ERROR %d\n", WSAGetLastError());
			// WSACleanup function terminates use of the Windows Sockets DLL.
			WSACleanup();
			return -1;
		}
		// Fill in the address structure
		local.sin_family = AF_INET;
		local.sin_addr.s_addr = INADDR_ANY;
		local.sin_port = htons(DEFAULT_PORT);

		sock = socket(AF_INET, SOCK_STREAM, 0); //TCp socket

		if (sock == INVALID_SOCKET)
		{
			fprintf(stderr, "socket() Failed with ERROR %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}

		if (bind(sock, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
		{
			fprintf(stderr, "bind() Failed with ERROR %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}

		// waiting for the connections
		if (listen(sock, 5) == SOCKET_ERROR)
		{
			fprintf(stderr, "listen() Failed with ERROR %d\n", WSAGetLastError());
			WSACleanup();
			return -1;
		}

		printf("\n");
		printf("     ∧  ∧＿__    ╭─────────────────────────╮  \n");
		printf("  ／(*ﾟーﾟ) ／＼ ＜　Online Phone Book System ┃  \n");
		printf(" ／|￣∪∪￣|＼／ ╰━━━━━━━━━━━━━━━━━━━━━━━━━━━╯  \n");
		printf("   |        |／  \n");
		printf("    ￣￣￣￣      \n");
		printf("\n");

		printf("Waiting for the Connections......\n\n");

		//////////////////////////////////////after connection/////////////////////////////
		while (1)
		{
			addr_len = sizeof(client_addr);
			msg_sock = accept(sock, (struct sockaddr *)&client_addr, &addr_len);
			if (msg_sock == INVALID_SOCKET)
			{
				fprintf(stderr, "accept() Failed with ERROR %d\n", WSAGetLastError());
				WSACleanup();
				
				// return -1;

				system("C:\\Users\\rainy\\Programming\\Networking_Project\\run_server.cmd");
				system("C:\\Users\\rainy\\Programming\\Networking_Project\\run_client.cmd");
				return -1;
			}

			printf("Accepted Connection from Client: %s, on Port %d\n\n",
				   inet_ntoa(client_addr.sin_addr),
				   htons(client_addr.sin_port));
			CreateThread(NULL, 0, CommFunction, (SOCKET *)&msg_sock, 0, NULL);
		}
		closesocket(msg_sock);
		WSACleanup();

		return 0;
	}
}
