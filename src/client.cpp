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

    // Prepare messages.
    char buffer[SOCKET_BUFFER_SIZE];
    float_t position_x = 0;
    float_t position_y = 0;
    float_t position_z = 0;
    float_t rotation_x = 0; // roll:    will usually be 0.
    float_t rotation_y = 0; // pitch:   look left and right.
    float_t rotation_z = 0; // yaw:     look up and down.
    bool active = true;
    while(active) {
        scanf_s( "\n%c", &buffer[0], 1 );


        // Send packet with message.
        int flags = 0;
        if( sendto(sock, buffer, 1, flags, (SOCKADDR*)&server_address, sizeof(server_address)) == SOCKET_ERROR)
        {
            printf("sendto failed: %d", WSAGetLastError());
            return 1;
        }

        // Wait for reply.
        flags = 0;
        SOCKADDR_IN from;
        int from_size = sizeof(from);
        int bytes_received = recvfrom( sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size );
        if (bytes_received == SOCKET_ERROR)
        {
            std::cout << "recvfrom returned SOCKET_ERROR, WSAGetLastError(): " << WSAGetLastError() << std::endl;
            break;
        }

        // Process the packet.
        int32_t read_index = 0;

        memcpy(&position_x, &buffer[read_index], sizeof(position_x));
        read_index += sizeof(position_x);
        memcpy(&position_y, &buffer[read_index], sizeof(position_y));
        read_index += sizeof(position_y);
        memcpy(&position_z, &buffer[read_index], sizeof(position_z));
        read_index += sizeof(position_z);
        memcpy(&rotation_x, &buffer[read_index], sizeof(rotation_x));
        read_index += sizeof(rotation_x);
        memcpy(&rotation_y, &buffer[read_index], sizeof(rotation_y));
        read_index += sizeof(rotation_y);
        memcpy(&rotation_z, &buffer[read_index], sizeof(rotation_z));
        read_index += sizeof(rotation_z);

        std::cout << "Position: "
                << "x: " << position_x << ", "
                << "y: " << position_y << ", "
                << "z: " << position_z << ", "
                << "roll: " << rotation_x << ", "
                << "pitch: " << rotation_y << ", "
                << "yaw: " << rotation_z << ", "
                << std::endl;
    }

    /// Clean up socket.
    std::cout << "Cleaning up Winsock..." << std::endl;
    WSACleanup();

    return 0;
}
