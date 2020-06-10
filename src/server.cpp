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

    /// Receive packets.
    std::cout << "Waiting for packet..." << std::endl;
    char buffer[SOCKET_BUFFER_SIZE];
    float_t position_x = 0;
    float_t position_y = 0;
    float_t position_z = 0;
    float_t rotation_x = 0; // roll:    will usually be 0.
    float_t rotation_y = 0; // yaw:     look left and right.
    float_t rotation_z = 0; // pitch:   look up and down.
    bool active = true;
    while(active) {
        int flags = 0;
        SOCKADDR_IN from;
        int from_size = sizeof(from);

        // Get the packet.
        int bytes_received = recvfrom(sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR * ) & from, &from_size);
        if (bytes_received == SOCKET_ERROR) {
            std::cout << "recvfrom returned SOCKET_ERROR, WSAGetLastError(): " << WSAGetLastError() << std::endl;
        } else {
            buffer[bytes_received] = 0;
            std::cout << "Input from "
                      << (int) from.sin_addr.S_un.S_un_b.s_b1 << "."
                      << (int) from.sin_addr.S_un.S_un_b.s_b2 << "."
                      << (int) from.sin_addr.S_un.S_un_b.s_b3 << "."
                      << (int) from.sin_addr.S_un.S_un_b.s_b4 << ":"
                      << (int) from.sin_port << " - "
                      << buffer << std::endl;

            // Process Input
            char input = buffer[0];
            switch(input)
            {
                case 'w': ++position_y; break;
                case 'a': --position_x; break;
                case 's': --position_y; break;
                case 'd': ++position_x; break;

                case '[': ++rotation_y; break;
                case ']': --rotation_y; break;

                default:
                    std::cout << "Redundant input: " << input << std::endl;
                    break;
            }

            // Generate game state packet.
            int32_t write_index = 0;

            memcpy(&buffer[write_index], &position_x, sizeof(position_x));
            write_index += sizeof(position_x);
            memcpy(&buffer[write_index], &position_y, sizeof(position_y));
            write_index += sizeof(position_y);
            memcpy(&buffer[write_index], &position_z, sizeof(position_z));
            write_index += sizeof(position_z);
            memcpy(&buffer[write_index], &rotation_x, sizeof(rotation_x));
            write_index += sizeof(rotation_x);
            memcpy(&buffer[write_index], &rotation_y, sizeof(rotation_y));
            write_index += sizeof(rotation_y);
            memcpy(&buffer[write_index], &rotation_z, sizeof(rotation_z));
            write_index += sizeof(rotation_z);

            // Send state packet to client.
            int buffer_length = sizeof(position_x) + sizeof(position_y) + sizeof(position_z)
                    + sizeof(rotation_x) + sizeof(rotation_y) + sizeof(rotation_z);
            flags = 0;
            SOCKADDR* to = (SOCKADDR*)&from;
            int to_length = sizeof(from);
            if (sendto(sock, buffer, buffer_length, flags, to, to_length) == SOCKET_ERROR)
            {
                std::cout << "Sendto failed: " << WSAGetLastError() << std::endl;
                return 1;
            }
        }
    }

    /// Clean up socket.
    std::cout << "Cleaning up Winsock..." << std::endl;
    WSACleanup();

    return 0;
}
