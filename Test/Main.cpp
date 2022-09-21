#include "KapMirror/Runtime/Transports/Telepathy/Server.hpp"
#include "KapMirror/Runtime/Transports/Telepathy/Client.hpp"
#include <iostream>
#include <string.h>

#include "KapMirror/Runtime/NetworkWriter.hpp"

void launchServer() {
    std::cout << "> Test Server" << std::endl;
    KapMirror::Transports::Server server;
    server.start(25565);
}

void launchClient() {
    std::cout << "> Test Client" << std::endl;
    KapMirror::Transports::Client client;
    client.connect("127.0.0.1", 25565);
}

int main(int argc, char** argv) {
    // KapMirror::NetworkWriter writer;
    // writer.writeString("Tobiichi Origami");

    // std::cout << "Size: " << writer.size() << std::endl;

    // char *data = writer.toArray();
    // std::cout << "Data: " << (data + 2) << std::endl;

    // delete[] data;

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " [server|client]" << std::endl;
        return 1;
    }

    if (strcmp(argv[1], "server") == 0) {
        launchServer();
    } else if (strcmp(argv[1], "client") == 0) {
        launchClient();
    } else {
        std::cout << "Usage: " << argv[0] << " [server|client]" << std::endl;
        return 1;
    }
    return 0;
}
