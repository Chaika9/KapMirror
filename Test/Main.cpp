#include <iostream>
#include "KapMirror/Runtime/Transports/Telepathy/Server.hpp"

int main() {
    std::cout << "> Test" << std::endl;

    KapMirror::Transports::Server server;
    server.listen(25565);
    return 0;
}
