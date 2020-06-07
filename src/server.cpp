#include <iostream>
#include <winsock2.h>

#include "config.h"

int server_main() {
    /// Start socket service.
    std::cout << "Initialising socket..." << std::endl;
    WORD winsock_version = 0x202;
    WSADATA winsock_data;
    if(WSAStartup(winsock_version, &winsock_data))
    {
        std::cout << "WSAStartup failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    /// Create socket.
    std::cout << "Creating socket..." << std::endl;
    int address_family = AF_INET;
    int type = SOCK_DGRAM;
    int protocol = IPPROTO_UDP;
    SOCKET sock = socket(address_family, type, protocol);
    if(sock == INVALID_SOCKET)
    {
        std::cout << "Socket failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    /// Bind socket to port.
    std::cout << "Binding socket to port " << PORT << "..." << std::endl;
    SOCKADDR_IN local_address;
    local_address.sin_family = AF_INET;
    local_address.sin_port = htons(PORT);
    local_address.sin_addr.s_addr = INADDR_ANY;
    if(bind(sock, (SOCKADDR*)&local_address, sizeof(local_address)) == SOCKET_ERROR)
    {
        std::cout << "Bind failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    /// Receive packet.
    std::cout << "Waiting for packet..." << std::endl;
    char buffer[SOCKET_BUFFER_SIZE];
    int flags = 0;
    SOCKADDR_IN from;
    int from_size = sizeof(from);

    // Get the packet.
    int bytes_received = recvfrom(sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size);
    if (bytes_received == SOCKET_ERROR)
    {
        std::cout << "recvfrom returned SOCKET_ERROR, WSAGetLastError(): " << WSAGetLastError() << std::endl;
    }
    else
    {
        buffer[bytes_received] = 0;
        std::cout << "Message from "
                  << (int)from.sin_addr.S_un.S_un_b.s_b1 << "."
                  << (int)from.sin_addr.S_un.S_un_b.s_b2 << "."
                  << (int)from.sin_addr.S_un.S_un_b.s_b3 << "."
                  << (int)from.sin_addr.S_un.S_un_b.s_b4 << ":"
                  << (int)from.sin_port  << " - "
                  << buffer << std::endl;
    }

    /// Clean up socket.
    std::cout << "Cleaning up Winsock..." << std::endl;
    WSACleanup();

    return 0;
}
