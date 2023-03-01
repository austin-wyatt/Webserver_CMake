#include <iostream>
#include "HttpRequest.h"
#include <thread>
#include <string_view>
#include <fstream>

using namespace std;

void HandleConnection(SOCKET socket);

int main()
{
#ifdef _WIN32
    //initialize winsock
    WSADATA wsaData;

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        wprintf(L"Error at WSAStartup()\n");
        WSACleanup();
        return 1;
    }
#endif 

    //Get ip and port from .env file
    string ip = "";
    string port = "";

    ifstream envFile = ifstream(".env");
    
    if (!envFile.is_open()) 
    {
        cout << "a .env file containing the ip (in the form ip=1.1.1.1) must be present." << endl;
        return 1;
    }

    string line;
    while(!envFile.eof())
    {
        getline(envFile, line);

        int delimPos = line.find('=');
        if (delimPos != -1) 
        {
            string token = line.substr(0, delimPos);
            string value = line.substr(delimPos + 1);

            if (token == "ip")
                ip = value;
            else if (token == "port")
                port = value;
        }
    }

    if (ip == "") 
    {
        cout << ".env file containing ip must be present." << endl;
        return 1;
    }
    if (port == "")
        port = "80";

    envFile.close();


    //start listening on the provided port
    SOCKET mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //Bind the socket
    sockaddr_in  addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(stoi(port));
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr.s_addr);
    int result = bind(mainSocket, (SOCKADDR*)&addr, sizeof(addr));

    if (result == SOCKET_ERROR)
    {
        cout << "Socket connection failed" << endl;
        closesocket(mainSocket);
        return 1;
    }

    cout << "Socket bound to port " << port << endl;


    listen(mainSocket, SOMAXCONN);

    while (mainSocket != INVALID_SOCKET) 
    {
        SOCKET connectedSocket = accept(mainSocket, NULL, NULL);

        if (connectedSocket != INVALID_SOCKET)
        {
            thread(HandleConnection, connectedSocket).detach();
        }
    }
}

void HandleConnection(SOCKET socket)
{
#ifdef DEBUG
    std::cout << "Socket opened: " << socket << std::endl;
#endif

    const int READ_BUFFER_LEN = 64 * 1024;
    char* readBuffer = new char[READ_BUFFER_LEN];
    ZeroMemory(readBuffer, READ_BUFFER_LEN);

    int bytesReceived;

    int byteOffset = 0;

    bool keepAlive = true;

    while (socket != INVALID_SOCKET && keepAlive)
    {
        bytesReceived = recv(socket, readBuffer, READ_BUFFER_LEN, 0);

        if (bytesReceived == 0 || bytesReceived == -1) 
        {
            break;
        }
        else 
        {
            //This is pretty flimsy and will break if a request is batched over multiple recv calls

            cout << "READ BUFFER" << endl;
            cout << "|||||||||||||||||||||||||" << endl;
            cout << readBuffer << endl;

            HttpRequest* request = new HttpRequest();

            request->BuildFromBuffer(readBuffer, READ_BUFFER_LEN);
            request->ClientSocket = socket;

#ifdef DEBUG
            request->Print();
#endif

            keepAlive = request->HandleRequest();

            delete request;
        }
    }

    closesocket(socket);

#ifdef DEBUG
    std::cout << "Socket closed: " << socket << std::endl;
#endif

    delete[] readBuffer;
}
