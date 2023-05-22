#pragma once

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <string>

#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>

#include <string.h>
#endif

#ifdef _WIN32
#define CRLF_LEN 2
#endif

#ifdef __linux__
#define CRLF_LEN 2

#define SOCKET int
#define SOCKADDR sockaddr
#define closesocket close

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))
#define ZeroMemory RtlZeroMemory
#endif

#define CRLF '\r\n'
#define CRLF_STR "\r\n"

#define SEND_BLOCK_SIZE 4096

#define COMPRESS_CHUNK_SIZE 16384