#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

//Winsock API for Windows
#include <winsock2.h>
#include <ws2tcpip.h>

#define BUFFER_SIZE 1024

#pragma comment(lib, "Ws2_32.lib") //Add Ws2_#2.lib during the linking process

/*
The server accepts a port number as a command line argument.
It creates a TCP socket, binds to the provided port on all network interfaces, and then listens for incoming connections.
When a client connects, it reads the message and returns the same message back, and closes the connection.
Then, the server loops back to wait for another client.
*/

int main (int argc, char* argv[]){
    //Ensures the program runs with exactly one command-line argument, the port number
    if (argc != 2){
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }
    int port = std::atoi(argv[1]);
}