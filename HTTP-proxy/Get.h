#pragma once

#include <WinSock2.h>

char* GetHostName(char **BuffWithHostName);
int GetHostInfo(char HostName[], PSOCKADDR_IN* Info);
void GetAndPrintFirstLine(char **BuffWithURL, char **EndOfURL);
