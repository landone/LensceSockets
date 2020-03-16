#define IS_SERVER   false

#include "LensceServer.h"
#include "LensceClient.h"
#include <iostream>

void OnReceiveTCP(int client, char* msg, int len) {

    std::cout << "Client#" << client << ": " << msg << std::endl;

}

int main() {
    LensceSocket::printErrors(true);
    if (IS_SERVER){
        LensceServer server(39420, 10, 10);
        server.receiveTCPCallback(OnReceiveTCP);
        if (server.start()) {
            std::cout << "Server started\n";
        }
        while (server.isRunning()){
            /* Do nothing */
        }
    }
    else {
        LensceClient client;
        if (client.connect("127.0.0.1", 39420)) {
            client.sendTCP("U SMELL", 8);
        }
        else {
            std::cout << "Unable to connect." << std::endl;
        }
        client.disconnect();
    }

    return 0;

}