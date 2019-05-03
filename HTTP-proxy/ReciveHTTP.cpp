#include "Global.h"
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include "ReciveHTTP.h"
#include "Get.h"

int ReciveAndSendHTTPRequest(SOCKET &ClientSock, SOCKET &HostSock, char* &HTTPBuff, char* &HostName)
{
	int Res, RecvRes;
	bool HostFounded = false, EndOfData = false;
	timeval Delay = { 1, 0 };
	fd_set ForRead = { 1, ClientSock };

	do
	{
		Res = select(0, &ForRead, NULL, NULL, &Delay);
		if (Res == SOCKET_ERROR)
		{
			printf("select function failed with error = %d\n", WSAGetLastError());
			getchar();
			return 1;
		}

		if (Res == 0)
		{
			EndOfData = true;
		}
		else
		{
			RecvRes = recv(ClientSock, HTTPBuff, HTTP_BUFFER_SIZE - 1, 0);
			if (!RecvRes)
			{
				if (HostName != NULL)
				{
					free(HostName);
				}
				getchar();
				return 1;
			}

			if (!HostFounded)
			{
				char *Tmp = NULL;
				Tmp = strstr(HTTPBuff, HTTP_HOST_SIGNATURE);
				if (Tmp != NULL)
				{
					HostName = GetHostName(&Tmp);
					Tmp = strstr(HTTPBuff, HTTP_END_OF_LINE);
					if (Tmp != NULL)
					{
						GetAndPrintFirstLine(&HTTPBuff, &Tmp);
					}
					else
					{
						printf_s("URL not founded\n");
					}

					printf_s("Host name: %s\n", HostName);
					HostFounded = true;

					PSOCKADDR_IN Info = (PSOCKADDR_IN)malloc(sizeof(SOCKADDR_IN));
					Res = GetHostInfo(HostName, &Info);
					if (Res)
					{
						free(HostName);
						free(Info);
						return 1;
					}
					HostSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
					if (HostSock == INVALID_SOCKET)
					{
						printf("socket function failed with error = %d\n", WSAGetLastError());
						free(Info);
						free(HostName);
						getchar();
						return 1;
					}

					Res = connect(HostSock, (PSOCKADDR)Info, sizeof(SOCKADDR_IN));
					if (Res == SOCKET_ERROR)
					{
						printf("connect function failed with error = %d\n", WSAGetLastError());
						shutdown(HostSock, SD_BOTH);
						closesocket(HostSock);
						free(HostName);
						free(Info);
						getchar();
						return 1;
					}
					free(Info);
				}
				else
				{
					printf_s("Host name not founded\n");
					getchar();
					return 1;
				}
				ForRead.fd_count = 1;
				ForRead.fd_array[0] = ClientSock;
			}

			send(HostSock, HTTPBuff, RecvRes, 0);
			if (Res == SOCKET_ERROR)
			{
				printf("sendto function failed with error = %d\n", WSAGetLastError());
				free(HTTPBuff);
				shutdown(HostSock, SD_BOTH);
				closesocket(HostSock);
				free(HostName);
				getchar();
				return 1;
			}
		}
	} while (!EndOfData);
	return 0;
}

int ReciveAndSendHTTPResponse(SOCKET &ClientSock, SOCKET &HostSock, char* &HTTPBuff)
{
	int Res, RecvRes;
	bool EndOfData = false, AnswPrinted = false;
	timeval Delay = { 1, 0 };
	fd_set ForRead = { 1, HostSock };

	do
	{
		Res = select(0, &ForRead, NULL, NULL, &Delay);
		if (Res == SOCKET_ERROR)
		{
			printf("select function failed with error = %d\n", WSAGetLastError());
			getchar();
			return 1;
		}

		if (Res == 0)
		{
			EndOfData = true;
		}
		else
		{
			RecvRes = recv(HostSock, HTTPBuff, HTTP_BUFFER_SIZE - 1, 0);
			if (!AnswPrinted)
			{
				AnswPrinted = true;
				char *Tmp = NULL;
				Tmp = strstr(HTTPBuff, HTTP_END_OF_LINE);
				if (Tmp != NULL)
				{
					GetAndPrintFirstLine(&HTTPBuff, &Tmp);
				}
				else
				{
					printf_s("No answer\n\n");
				}
			}
			
			send(ClientSock, HTTPBuff, RecvRes, 0);
			if (Res == SOCKET_ERROR)
			{
				printf("sendto function failed with error = %d\n", WSAGetLastError());
				getchar();
				return 1;
			}

			ForRead.fd_count = 1;
			ForRead.fd_array[0] = HostSock;
		}
	} while (!EndOfData);
	return 0;
}

void ReciveHTTP(SOCKET ClientSock)
{
	int Res;
	SOCKET HostSock;
	char *HTTPBuff = (char*)malloc(HTTP_BUFFER_SIZE);
	char *HostName = NULL;
	ZeroMemory(HTTPBuff, HTTP_BUFFER_SIZE);
	Res = ReciveAndSendHTTPRequest(ClientSock, HostSock, HTTPBuff, HostName);
	if (Res)
	{
		free(HTTPBuff);
		shutdown(ClientSock, SD_BOTH);
		closesocket(ClientSock);

		shutdown(HostSock, SD_BOTH);
		closesocket(HostSock);
		printf_s("\n\n\n");
		return;
	}

	Res = ReciveAndSendHTTPResponse(ClientSock, HostSock, HTTPBuff);
	if (Res)
	{
		free(HTTPBuff);
		shutdown(ClientSock, SD_BOTH);
		closesocket(ClientSock);

		shutdown(HostSock, SD_BOTH);
		closesocket(HostSock);
		printf_s("\n\n\n");
		return;
	}	
	shutdown(ClientSock, SD_BOTH);
	closesocket(ClientSock);
	shutdown(HostSock, SD_BOTH);
	closesocket(HostSock);

	free(HTTPBuff);

	printf_s("\n\n\n");
}