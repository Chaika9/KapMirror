#include "KapMirror/Runtime/Transports/Telepathy/Server.hpp"
#include "KapMirror/Runtime/Transports/Telepathy/Client.hpp"
#include <iostream>
#include <string.h>

#include "KapMirror/Runtime/ArraySegment.hpp"
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

    std::cout << "Client: connected" << std::endl;

    std::cout << "Client: send packet" << std::endl;
    std::string data = "Tobiichi Origami";
    KapMirror::ArraySegment<char> segment((char *)data.c_str(), data.length());
    client.send(segment);

    while (client.connected()) {
        std::cout << "Client: waiting for packet" << std::endl;
        try {
            KapMirror::ArraySegment<char> data = client.receive(1024);
            std::string msg(data.toArray(), data.getSize());
            std::cout << "Client: received message=" << msg << std::endl;
        } catch (std::exception& e) {
            std::cout << "Client: server disconnected" << std::endl;
            break;
        }
    }
}

int main(int argc, char** argv) {
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
