# Developer Diary - Chapter 1

With guidance from: https://www.codersblock.org/blog/multiplayer-fps-part-1

## Introduction

### Overview

We'll be creating a multiplayer game, meaning that we're focusing hard on the **network model**.

Networking involves replicating the game state between players while also allowing them to affect it.

We'll be using a client-server architecture, where one central process serves all other clients.

## Networking

### Trust

We have a few options over how much we 'trust' the client.

We can either fully trust the client, validate the client, or do everything server-side.

To avoid cheating, we'll be doing full server-side validation.

### Interpolation

We also have a few options over synchronising the server and client.

We need some form of interpolation, otherwise we'll use too much bandwidth.

Interpolation allows us to take a 'snapshots' of game state and smooth between them.

**Snapshot Interpolation** takes regular snapshots of visual state at a fast rate. This is pretty awesome, but it does require at least 2 states to interpolate between, which introduces some latency.

**State Synchronisation** takes 'bigger' snapshots of full game state, and runs the simulation on both sides. However, it means that the network code is dependent on the game code, and requires more bandwidth.

**Deterministic Lockstep** allows clients to send their input to each other, and then progress the simulation as one. However, even slight latency or packet loss will wreak havoc, so it's only really suitable for LAN.

### Sockets

For machines to communicate with each other, they send packets of data.

A data packet travels from one machine's socket to another machine's socket.

Networking is quirky, and can result in missing packets and duplicate packets.

### Internet Protocols

TCP is a protocol that enforces packets to not be missing or duplicate.

If one packet is dropped, TCP requires it to be re-sent and received before proceeding.

Therefore, we need to stick to UDP, and deal with packet loss and packet duplication.

## Coding

### Getting Started

We'll start off by creating the server code, which is the entry point for clients.

We'll be using `<winsock2.h>`, which is a sockets library for Windows.

### Creating a Socket

We need to first call `WSAStartup` before using Winsock. From the MSDN docs:

```cpp
int WSAStartup(
  _In_  WORD      wVersionRequested,
  _Out_ LPWSADATA lpWSAData
);
```

We'll incorporate this with some error handling, because we're good boys.

```cpp
std::cout << "Initialising socket..." << std::endl;
WORD winsock_version = 0x202;
WSADATA winsock_data;
if(WSAStartup(winsock_version, &winsock_data))
{
    std::cout << "WSAStartup failed: " << WSAGetLastError() << std::endl;
    return 1;
}
```

The docs also say that we need to call `WSACleanup` when we're done.

```cpp
std::cout << "Cleaning up Winsock..." << std::endl;
WSACleanup();
```

Now to create our socket.

The docs say to use the `socket` function.

```cpp
SOCKET WSAAPI socket(
  _In_ int af,			// Address family.
  _In_ int type,		// Socket type (SOCK_DGRAM for UDP).
  _In_ int protocol		// Internet protocol (IPPROTO_UDP).
);
```

We'll similarly implement this with error handling.

```cpp
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
```

### Receiving a Packet

A packet is not only sent to a machine, but to a particular port.

Therefore, we need to bind the socket to a particular port.

```cpp
int bind(
  _In_ SOCKET                s,
  _In_ const struct sockaddr *name,
  _In_ int                   namelen
);
```

After defining `PORT` globally, we have our implementation:

```cpp
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
```

To receive packets, we use the `recvfrom` function.

```cpp
int recvfrom(
_In_        SOCKET          s,
_Out_       char            *buf,
_In_        int             len,
_In_        int             flags,
_Out_       struct sockaddr *from,
_Inout_opt_ int             *fromlen
);
```

One important thing to note is the buffer, which will store data.

The buffer can't be smaller than the maximum size of a packet, which we'll keep to 1KB. This keeps the bandwidth pretty low, allowing us to distribute them faster and to more players.

Let's quickly knock that into a header file.

```cpp
const uint16_t PORT = 8192;
const uint32_t SOCKET_BUFFER_SIZE = 1024;
```

You'll also notice that it stores from whom the packet came from. Neat!

Right now, we'll just wait for one packet and then exit.

```cpp
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
```

### Sending a Packet

We need to start work on the client.

We do need to initialise `WSAStartup` and create our socket, just like our server.

```cpp
std::cout << "Initialising socket..." << std::endl;
WORD winsock_version = 0x202;
WSADATA winsock_data;
if (WSAStartup(winsock_version, &winsock_data)) {
    std::cout << "WSAStartup failed: " << WSAGetLastError() << std::endl;
    return 1;
}

std::cout << "Creating socket..." << std::endl;
int address_family = AF_INET;
int type = SOCK_DGRAM;
int protocol = IPPROTO_UDP;
SOCKET sock = socket(address_family, type, protocol);
if (sock == INVALID_SOCKET) {
    std::cout << "Socket failed: " << WSAGetLastError() << std::endl;
    return 1;
}

// ...

// ...

// ...

std::cout << "Cleaning up Winsock..." << std::endl;
WSACleanup();

return 0;
```

The `// ...` marks are where we'll send a packet.

We'll be making use of the `sendto` function.

```cpp
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
if(sendto(sock, message, strlen(message), flags, (SOCKADDR*)&server_address, sizeof(server_address)) == SOCKET_ERROR)
{
    printf("sendto failed: %d", WSAGetLastError());
    return 1;
}
std::cout << "Packet sent!" << std::endl;
```

And, well, that's pretty much it!

By building the files and running the executable twice, we can see what happens when we send a message with the client.

![The Client and Server in Action!](https://raw.githubusercontent.com/LloydTao/first-person-shooter/master/diary/01-Sockets/01-001-Preview.png)
