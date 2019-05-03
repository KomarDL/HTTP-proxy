#include "Global.h"
#include <ws2tcpip.h>
#include <stdio.h>
#include "Get.h"

char* GetHostName(char **BuffWithHostName)
{
	(*BuffWithHostName) += strlen(HTTP_HOST_SIGNATURE);
	int HostNameLen = abs((int)((*BuffWithHostName) - strstr((*BuffWithHostName), HTTP_END_OF_LINE))) + 1;
	char *HostName = (char*)calloc(HostNameLen, sizeof(char));
	memcpy_s(HostName, HostNameLen, (*BuffWithHostName), HostNameLen - 1);
	return HostName;
}

int GetHostInfo(char HostName[], PSOCKADDR_IN* Info)
{
	int Res;
	struct addrinfo *IPList = NULL;
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	Res = getaddrinfo(HostName, HTTP_DEFAULT_PORT_STRING, &hints, &IPList);
	if (Res != 0)
	{
		printf("getaddrinfo failed with error: %d\n", Res);
		getchar();
		return 1;
	}

	memcpy((*Info), IPList->ai_addr, IPList->ai_addrlen);

	/*char str[16];
	while (IPList != NULL)
	{
		PSOCKADDR_IN addr = (PSOCKADDR_IN)IPList->ai_addr;
		inet_ntop(AF_INET, &(addr->sin_addr.s_addr), (PSTR)(str), 16);
		printf_s("%s\t%d\n", str, addr->sin_port);
		IPList = IPList->ai_next;
	}*/

	return 0;
}

void GetAndPrintFirstLine(char **BuffWithURL, char **EndOfURL)
{
	int len = abs((*BuffWithURL) - (*EndOfURL));
	char *URL = (char*)calloc(len + 1, sizeof(char));
	memcpy_s(URL, len, (*BuffWithURL), len);
	printf_s("%s\n", URL);
	free(URL);
}