#include "KapMirror/Runtime/Transports/Telepathy/Server.hpp"
#include "KapMirror/Runtime/Transports/Telepathy/Client.hpp"
#include <iostream>
#include <string.h>

#include "KapMirror/Runtime/ArraySegment.hpp"
#include "KapMirror/Runtime/NetworkWriter.hpp"
#include "KapMirror/Runtime/NetworkReader.hpp"

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
    std::string msg = "Tobiichi Origami";
    KapMirror::NetworkWriter writer;
    writer.writeString(msg);
    writer.write((int)39);
    KapMirror::ArraySegment<char> segment = writer;
    client.send(segment);

    while (client.connected()) {
        std::cout << "Client: waiting for packet" << std::endl;
        try {
            KapMirror::ArraySegment<char> data = client.receive(1024);
            KapMirror::NetworkReader reader(data);
            std::string msg = reader.readString();
            std::cout << "Client: received message=" << msg << std::endl;
        } catch (std::exception& e) {
            std::cout << "Client: server disconnected" << std::endl;
            break;
        }
    }
}

// void test() {
//     std::string msg("Tobiichi Origami");
//     KapMirror::NetworkWriter writer;
//     writer.writeString(msg);

//     KapMirror::ArraySegment<char> segment = writer;

//     KapMirror::NetworkReader reader(segment);
//     std::string msg2 = reader.readString();
//     std::cout << "msg=" << msg << std::endl;
// }

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
