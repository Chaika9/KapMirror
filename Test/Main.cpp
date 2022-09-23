#include "KapMirror/Runtime/NetworkServer.hpp"
#include "KapMirror/Runtime/NetworkClient.hpp"
#include "KapMirror/Runtime/NetworkWriter.hpp"
#include <iostream>
#include <string.h>

void launchServer() {
    std::cout << "[Server] > Launch Test" << std::endl;
    KapMirror::NetworkServer server;
    server.start(25565);

    std::cout << "[Server] > Waiting for client" << std::endl;
    while (true) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        server.tick(100);
    }
}

void launchClient() {
    std::cout << "[Client] > Launch Test" << std::endl;
    KapMirror::NetworkClient client;
    client.connect("127.0.0.1", 25565);

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    KapMirror::NetworkWriter writer;
    writer.writeString("Tobiichi Origami");
    std::shared_ptr<KapMirror::ArraySegment<byte>> message = writer.toArraySegment();
    client.send(message);

    std::cout << "[Client] > Waiting for server" << std::endl;
    while (client.connected() || client.connecting()) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        client.tick(100);
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
