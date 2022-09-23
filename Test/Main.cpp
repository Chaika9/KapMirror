#include "KapMirror/Runtime/Transports/Telepathy/TelepathyTransport.hpp"
#include "KapMirror/Runtime/NetworkWriter.hpp"
#include "KapMirror/Runtime/NetworkReader.hpp"
#include <iostream>
#include <string.h>

void launchServer() {
    std::cout << "[Server] > Launch Test" << std::endl;

    KapMirror::TelepathyTransport transport;

    transport.onServerConnected = [](KapMirror::Transport& t, int connectionId) {
        std::cout << "[Server] > Client connected: " << connectionId << std::endl;
        KapMirror::NetworkWriter writer;
        writer.writeString("Tobiichi Origami");
        t.serverSend(connectionId, writer.toArraySegment());
    };
    transport.onServerDisconnected = [](KapMirror::Transport& t, int connectionId) {
        std::cout << "[Server] > Client disconnected: " << connectionId << std::endl;
    };
    transport.onServerDataReceived = [](KapMirror::Transport& t, int connectionId, std::shared_ptr<KapMirror::ArraySegment<byte>> data) {
        std::cout << "[Server] > Client data received: " << connectionId << std::endl;
        KapMirror::NetworkReader reader(data);
        std::cout << "[Server] > Data received: Text=" << reader.readString() << std::endl;
    };

    transport.serverStart();

    // Ticks
    while (true) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        transport.serverEarlyUpdate();
    }
}

void launchClient() {
    std::cout << "[Client] > Launch Test" << std::endl;

    KapMirror::TelepathyTransport transport;

    transport.onClientConnected = [](KapMirror::Transport& t) {
        std::cout << "[Client] > Connected" << std::endl;
        KapMirror::NetworkWriter writer;
        writer.writeString("Setsuna");
        t.clientSend(writer.toArraySegment());
    };
    transport.onClientDisconnected = [](KapMirror::Transport& t) {
        std::cout << "[Client] > Disconnected" << std::endl;
    };
    transport.onClientDataReceived = [](KapMirror::Transport& t, std::shared_ptr<KapMirror::ArraySegment<byte>> data) {
        std::cout << "[Client] > Data received: Size=" << data->getSize() << std::endl;
        KapMirror::NetworkReader reader(data);
        std::cout << "[Client] > Data received: Text=" << reader.readString() << std::endl;
    };

    transport.clientConnect("127.0.0.1", 25565);

    // Ticks
    while (true) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        transport.clientEarlyUpdate();
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
