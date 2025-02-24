#include <iostream>
#include <cstdlib>
#include <cstring>

//Winsock API for Windows
#include <winsock2.h>
#include <ws2tcpip.h>

#include <sstream>
#include <vector>

#define BUFFER_SIZE 4096

#pragma comment(lib, "Ws2_32.lib") //Add Ws2_#2.lib during the linking process

/*
The server accepts a port number as a command line argument.
It creates a TCP socket, binds to the provided port on all network interfaces, and then listens for incoming connections.
When a client connects, it reads the message and returns the same message back, and closes the connection.
Then, the server loops back to wait for another client.
*/

//Reads a line from a socket, up to '\n'
std::string recvLine(SOCKET sock) {
    std::string line;
    char c;
    int ret;
    while ((ret = recv(sock, &c, 1, 0)) > 0) {
        if (c == '\n') {
            break;
        }
        line.push_back(c);
    }
    return line;
}

//Tokenizes a string by whitespace
std::vector<std::string> tokenize(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> tokens;
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

int main (int argc, char* argv[]){
    //Ensures the program runs with exactly one command-line argument, the port number
    if (argc != 2){
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }
    int port = std::atoi(argv[1]);

    //Initializes Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup Failed: " << result << std::endl;
        return 1;
    }

    //Creates a TCP Socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    //Server Address Structure
    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    //Binds the Socket to the Port
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind Failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    //Listens for Incoming Connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen Failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Echo Server listenting on Port " << port << std::endl;

    //Accept & Handle Client Connections
    while (true) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept Failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        //Displays Client's IP Address & Port
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        std::cout << "Connection From " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;

        //Receives Data from Client
        char buffer[BUFFER_SIZE];
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            std::cout << "Received: " << std::string(buffer, bytesReceived) << std::endl;
            send(clientSocket, buffer, bytesReceived, 0); //Echoes Message Back to the Client
        }
        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}