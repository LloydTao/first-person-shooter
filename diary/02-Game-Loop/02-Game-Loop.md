# Developer Diary - Chapter 2

With guidance from: https://www.codersblock.org/blog/multiplayer-fps-part-2

## Introduction

### Overview

So far, we've set up sockets so that a client and server can establish a connection and communicate.

At the moment, a single packet is sent from the client to the server, and then both processes end.

We're first going to set up a loop so that packets can keep being sent.

We will then process inputs, as currently we just print the received packet.

## The Server

### Server Loop

At the moment, the server's job is to receive packets.

It currently uses `recvfrom` in order to receive a packet.

Our goal is to incorporate `recvfrom` into a loop, which is simple.

```cpp
bool active = true;
while(active) {

    ...

    int bytes_received = recvfrom( ... );
    
    ...

}
```

This will allow our server to keep receiving messages from different clients.

![Receiving Multiple Packets](https://raw.githubusercontent.com/LloydTao/first-person-shooter/master/diary/02-Game-Loop/02-001-Many-Packets.png)

Firstly' we're going to implement the player state.

In the context of the game, this means we have a position (x,y,z) and viewangles (x,y,z) for each player.

We'll start off with just 1 player. I'm going to define their state outside of the game loop for now.

```cpp
float_t position_x = 0;
float_t position_y = 0;
float_t position_z = 0;
float_t rotation_x = 0;
float_t rotation_y = 0;
float_t rotation_z = 0;

bool active = true;
while(active) { ...
```

### Processing Inputs

We'll need to append some code to where we currently handle inputs.

```cpp
if (bytes_received == SOCKET_ERROR) {

    ...

} else {
    buffer[bytes_received] = 0;
    
    std::cout << << "Message from " << ...

    // Input processing code here.
}
```

In our game, the player will use `WASD` in order to move around.

I'm also letting them use `[` and `]` to rotate around the vertical axis (yaw).

```cpp
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
```

### Sending Game State

Now that we've edited the game state, we need to send the current game state to the player.

To do this, we're going to write each state variable to an array.

The `memcpy` function will write the second argument to the first.

*Since C++ is epic, we need to keep track of our pointer based on variable size.*

```cpp
int32_t write_index = 0;

memcpy(&buffer[write_index], &position_x, sizeof(position_x));
write_index += sizeof(position_x);
memcpy(&buffer[write_index], &position_y, sizeof(position_y));
write_index += sizeof(position_y);
...
memcpy(&buffer[write_index], &rotation_z, sizeof(rotation_z));
write_index += sizeof(rotation_z);
```

The game state is now in the buffer, and we'll send it using `sendto`.

*Again, there's some C++ boilerplate to calculate `buffer_length`.*

```cpp
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
```

## The Client

### Client Loop

Currently, our client's job is to send packets.

It currently uses `sendto` in order to send a packet.

Our goal is to incorporate `sendto` into a loop, which is also simple.

```cpp
bool active = true;
while(active) {

    ...

    if( sendto( ... )
    
    ...

}
```

We can now send multiple inputs, and additionally, we can see how the server reacts.

![Sending Multiple Packets](https://raw.githubusercontent.com/LloydTao/first-person-shooter/master/diary/02-Game-Loop/02-002-Inputs.png)

### Receiving Game State

After sending input, the client needs to wait to receive a game state packet.

This will be a code snippet similar to the `recvfrom` on the server-side.

```cpp
flags = 0;
SOCKADDR_IN from;
int from_size = sizeof(from);
int bytes_received = recvfrom( sock, buffer, SOCKET_BUFFER_SIZE, flags, (SOCKADDR*)&from, &from_size );
if (bytes_received == SOCKET_ERROR)
{
    std::cout << "recvfrom returned SOCKET_ERROR, WSAGetLastError(): " << WSAGetLastError() << std::endl;
    break;
}
```

Once the client receives the packet, it displays the result and then continues the loop.

This code will be very similar to the receiving code on the server-side.

One distinction to note is that the first and second arguments have been switched around.

```cpp
// Process the packet.
int32_t read_index = 0;
memcpy(&position_x, &buffer[read_index], sizeof(position_x));
read_index += sizeof(position_x);
memcpy(&position_y, &buffer[read_index], sizeof(position_y));
read_index += sizeof(position_y);
...
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
```

Let's give it a whizz and see what happens.

![Final Results of Game Loop](https://raw.githubusercontent.com/LloydTao/first-person-shooter/master/diary/02-Game-Loop/02-003-Full-Loop.png)

Perfect!
