#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

//Winsock API for Windows
#include <winsock2.h>
#include <ws2tcpip.h>

#define BUFFER_SIZE 1024

#pragma comment(lib, "Ws2_32.lib") //Add Ws2_#2.lib during the linking process

int main (int argc, char* argv[]) {
    //Ensures the program runs with exactly two arguments, the host name and port number
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <hostname> <port>" << std::endl;
        return 1;
    }
    const char* hostname = argv[1];
    const char* portStr = argv[2];

    //Initializes Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup Failed: " << result << std::endl;
        return 1;
    }
    
    //Creates a TCP Socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    //Resolves the Server Address and Port
    addrinfo hints, *serverInfo = nullptr;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    result = getaddrinfo(hostname, portStr, &hints, &serverInfo);
    if (result != 0) {
        std::cerr << "getaddrinfo Failed: " << result << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    //Connects to the Server
    if (connect(sock, serverInfo -> ai_addr, (int)serverInfo -> ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "Connect Failed: " << WSAGetLastError() << std::endl;
        freeaddrinfo(serverInfo);
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(serverInfo);

    std::cout << "Connected to " << hostname << " on port " << portStr << std::endl;
    std::cout << "Enter a Message";
    std::string message;
    std::getline(std::cin, message);

    //Sends Message to Server
    int sentBytes = send (sock, message.c_str(), (int)message.size(), 0);
    if (sentBytes == SOCKET_ERROR) {
        std::cerr << "Send Failed: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    //Receives Echoed Message from Server
    char buffer[BUFFER_SIZE];
    int receivedBytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (receivedBytes > 0) {
        buffer[receivedBytes] = '\0'; //Null Terminate Received Data
        std::cout << "Echo from Server: " << buffer << std::endl;
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
