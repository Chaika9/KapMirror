#include <iostream>
#include <string.h>
#include <thread>

#include "KapMirror/Runtime/TcpListener.hpp"
#include "KapMirror/Runtime/TcpClient.hpp"
#include "KapMirror/Runtime/NetworkWriter.hpp"
#include "KapMirror/Runtime/NetworkReader.hpp"
#include "KapMirror/Runtime/NetworkServer.hpp"

void launchServer() {
    std::cout << "[Server] > Launch Test" << std::endl;
    KapMirror::NetworkServer server;
    server.start(25565);

    std::cout << "[Server] > Wait" << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(6000));

    server.close();

    //while (true);
}

void launchClient() {
    std::cout << "[Client] > Launch Test" << std::endl;
    auto address = KapMirror::Address::createAddress("127.0.0.1", 25565);
    KapMirror::TcpClient client(address);
    client.connect();

    std::cout << "[Client} > Connected" << std::endl;

    std::string msg = "Tobiichi Origami";
    KapMirror::NetworkWriter writer;
    writer.writeString(msg);

    KapMirror::ArraySegment<byte> message = writer;
    client.send(message);
    std::cout << "[Client] > Sent Message: Size=" << message.getSize() << std::endl;

    while (true);
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
