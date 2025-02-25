#include <iostream>
#include <cstdlib>
#include <cstring>

//Winsock API for Windows
#include <winsock2.h>
#include <ws2tcpip.h>

#include <sstream>
#include <vector>
#include <chrono>

#define BUFFER_SIZE 4096

#pragma comment(lib, "Ws2_32.lib") //Add Ws2_#2.lib during the linking process

/*
The client accepts a host anme and a port number as command line arguments.
It creates a TCP socket, resolves the hostname, and connects to the server.
Then, the client prompts the user to enter a message to the server.
The message is sent to the server, it waits for the echod message, prints it, and exits.
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

//Creates a payload string of the specified length
std::string createPayload(int len) {
    return std::string(len, 'X');
}

int main (int argc, char* argv[]) {
    //Ensures the program runs with exactly two arguments, the host name and port number
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " <hostname> <port> <m-type> <msg size> <probes>\n";
        return 1;
    }

    //Initializes Arguments
    const char* hostname = argv[1];
    const char* portStr = argv[2];
    std::string mType = argv[3]; // "rtt" or "tput"
    int msgSize = std::atoi(argv[4]);
    int probes = std::atoi(argv[5]);
    int serverDelay = 0;

    //Initializes Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup Failed\n";
        return 1;
    }
    
    //Creates a TCP Socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << "\n";
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
        std::cerr << "getaddrinfo Failed\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    //Connects to the Server
    if (connect(sock, serverInfo -> ai_addr, (int)serverInfo -> ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "Connect Failed: " << WSAGetLastError() << "\n";
        freeaddrinfo(serverInfo);
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(serverInfo);
    std::cout << "Connected to " << hostname << " on port " << portStr << "\n";
    
    //Connection Setup Phase (CSP)
    std::ostringstream cspStream;
    cspStream << "s " << mType << " " << msgSize << " " << probes << " " << serverDelay << "\n"; //Expected Tokens: s <m-type> <msg size> <probes> <server delay>\n
    std::string cspMsg = cspStream.str();
    send(sock, cspMsg.c_str(), (int)cspMsg.size(), 0);
    std::cout << "Sent CSP: " << cspMsg;

    //Waits for Server Acknowledgment
    std::string ack = recvLine(sock);
    std::cout << "Received From Server: " << ack << "\n";
    if (ack.find("200 OK") == std::string::npos) {
        std::cerr << "Server did not acknowledge setup. Terminating.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    //Measurement Phase (MP)
    std::string payload = createPayload(msgSize);
    for (int seq =  1; seq <= probes; seq++) {
        std::ostringstream mpStream; //Expected Tokens: m <PAYLOAD> <SEQUENCE>\n
        mpStream << "m " << payload << " " << seq << "\n";
        std::string mpMsg = mpStream.str();

        //Records time before sending message
        auto start = std::chrono::high_resolution_clock::now();

        send(sock, mpMsg.c_str(), (int)mpMsg.size(), 0);
        std::cout << "Sent MP Probe " << seq << "\n";

        //Receives Echo
        std::string echoMsg = recvLine(sock);

        //Records time after sending message
        auto end = std::chrono::high_resolution_clock::now();

        //Computes RTT in Microseconds
        auto rtt = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << "Probe " << seq << " RTT: " << rtt << " microseconds\n";
    }

    //Connection Termination Phase (CTP)
    std::string termMsg = "t\n";
    send(sock, termMsg.c_str(), (int)termMsg.size(), 0);
    std::cout << "Sent Termination Message.\n";
    std::string termAck = recvLine(sock);
    std::cout << "Received Termination Acknowledgment: " << termAck << "\n";

    closesocket(sock);
    WSACleanup();
    return 0;
}
