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

}
