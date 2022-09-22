#include <iostream>
#include <string.h>
#include <thread>

#include "KapMirror/Runtime/Transports/Telepathy/TcpListener.hpp"
#include "KapMirror/Runtime/Transports/Telepathy/TcpClient.hpp"
#include "KapMirror/Runtime/NetworkWriter.hpp"
#include "KapMirror/Runtime/NetworkReader.hpp"
#include "KapMirror/Runtime/Transports/Telepathy/Server.hpp"

void launchServer() {
    std::cout << "[Server] > Launch Test" << std::endl;
    KapMirror::Transports::Server server;
    server.start(25565);

    std::cout << "[Server] > Wait" << std::endl;

    while (true);

    // std::cout << "[Server] > Launch Test" << std::endl;
    // auto address = KapMirror::Transports::Address::createAddress(25565);
    // KapMirror::Transports::TcpListener listener(address);
    // listener.start();

    // std::cout << "[Server] > Waiting for client" << std::endl;

    // while (true) {
    //     if (!listener.isReadable()) {
    //         std::this_thread::sleep_for(std::chrono::milliseconds(1));
    //         continue;
    //     }

    //     try {
    //         auto client = listener.acceptTcpClient();
    //         std::cout << "[Server] > Client connected" << std::endl;

    //         while (client->connected()) {
    //             try {
    //                 auto message = client->receive(1024);
    //                 std::cout << "[Server] > Received Message: Size=" << message.getSize() << std::endl;

    //                 KapMirror::NetworkReader reader(message);
    //                 std::string msg = reader.readString();
    //                 std::cout << "[Server] > Message: " << msg << std::endl;
    //             } catch (KapMirror::Transports::SocketException& e) {
    //                 std::cout << "[Server] > Client disconnected" << std::endl;
    //                 break;
    //             }
    //         }
    //     } catch (KapMirror::Transports::SocketException& e) {
    //         std::cout << "[Server] [Error] > " << e.what() << std::endl;
    //     }
    // }
}

void launchClient() {
    std::cout << "[Client] > Launch Test" << std::endl;
    auto address = KapMirror::Transports::Address::createAddress("127.0.0.1", 25565);
    KapMirror::Transports::TcpClient client(address);
    client.connect();

    std::cout << "[Client} > Connected" << std::endl;

    // std::string msg = "Tobiichi Origami";
    // KapMirror::NetworkWriter writer;
    // writer.writeString(msg);

    // KapMirror::ArraySegment<byte> message = writer;
    // client.send(message);

    // std::cout << "[Client] > Sent Message: Size=" << message.getSize() << std::endl;

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
