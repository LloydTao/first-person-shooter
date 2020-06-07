#include <iostream>

#include "client.h"
#include "server.h"

int main() {
    std::cout << "Client [0] or server [1]: ";

    int input;
    std::cin >> input;

    if (input == 0) {
        client_main();
    } else {
        server_main();
    }

    std::cout << std::endl << "Exit: " << std::endl;
    std::cin >> input;
    return 0;
}
