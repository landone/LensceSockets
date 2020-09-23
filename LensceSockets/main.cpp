#include "../LensceServer.h"
#include "../LensceClient.h"

#include <iostream>
#include <string>

void onConnect(int client) {

	printf("Client%d connected\n", client);

}

void onDisconnect(int client) {

	printf("Client%d disconnected\n", client);

}

int main() {

	LensceSocket::printErrors(true);
	std::string input;
	std::getline(std::cin, input);
	int port = 5000;
	int maxClients = 10;
	if (input.compare("s") == 0) {

		printf("Server Mode\n");
		LensceServer server;
		server.connectCallback(onConnect);
		server.disconnectCallback(onDisconnect);
		server.start(port, maxClients, maxClients);
		while (server.isRunning()) {

			std::getline(std::cin, input);
			if (input.compare("exit") == 0) {
				server.stop();
			}

		}

	}
	else {

		printf("Client Mode\n");
		LensceClient client;
		client.connect("192.168.1.127", port);

		std::getline(std::cin, input);
		client.disconnect();

	}

	return 0;

}