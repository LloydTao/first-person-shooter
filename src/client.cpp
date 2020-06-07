#include <iostream>
#include <winsock2.h>

#include "config.h"

int client_main()
{
    /// Start socket service.
    std::cout << "Initialising socket..." << std::endl;
    WORD winsock_version = 0x202;
    WSADATA winsock_data;
    if (WSAStartup(winsock_version, &winsock_data)) {
        std::cout << "WSAStartup failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    /// Create socket.
    std::cout << "Creating socket..." << std::endl;
    int address_family = AF_INET;
    int type = SOCK_DGRAM;
    int protocol = IPPROTO_UDP;
    SOCKET sock = socket(address_family, type, protocol);
    if (sock == INVALID_SOCKET) {
        std::cout << "Socket failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    /// Send packet to server.
    std::cout << "Preparing to send packet..." << std::endl;
    SOCKADDR_IN server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    // Prepare message.
    char message[SOCKET_BUFFER_SIZE];
    std::cout << "Enter message: ";
    std::cin >> message;

    // Send packet with message.
    int flags = 0;
    if( sendto(sock, message, strlen(message), flags, (SOCKADDR*)&server_address, sizeof(server_address)) == SOCKET_ERROR)
    {
        printf("sendto failed: %d", WSAGetLastError());
        return 1;
    }
    std::cout << "Packet sent!" << std::endl;

    /// Clean up socket.
    std::cout << "Cleaning up Winsock..." << std::endl;
    WSACleanup();

    return 0;
}
