#include "../LensceServer.h"
#include "../LensceClient.h"

#include <iostream>
#include <string>

static LensceServer server;

void onConnect(int client) {

	printf("Client%d honk connected\n", client);
	std::string output = "Client";
	output.append(std::to_string(client));
	output.append(" connected");
	server.sendTCPAll(output.c_str(), (int)output.length());

}

void onDisconnect(int client) {

	printf("Client%d disconnected\n", client);
	std::string output = "Client";
	output.append(std::to_string(client));
	output.append(" disconnected");
	server.sendTCPAll(output.c_str(), (int)output.length());

}

void dataReceived(char* data, int len) {

	char* buffer = new char[len + 1];
	for (int i = 0; i < len; i++) {
		buffer[i] = data[i];
	}
	buffer[len] = '\0';
	printf("%s\n", buffer);

}

void dataReceivedServer(int client, char* data, int len) {

	std::string prefix = "Client";
	prefix.append(std::to_string(client));
	prefix.append(": ");
	int totalLen = (int)prefix.length() + len;
	char* buffer = new char[totalLen + 1];
	for (int i = 0; i < totalLen; i++) {
		if (i < prefix.length()) {
			buffer[i] = prefix.at(i);
		}
		else {
			buffer[i] = data[i - prefix.length()];
		}
	}
	buffer[totalLen] = '\0';
	printf("%s\n", buffer);
	std::vector<int> clients = server.getClients();
	for (int i = 0; i < clients.size(); i++) {
		int clientID = clients[i];
		if (clientID == client) {
			continue;
		}
		server.sendTCP(clientID, buffer, totalLen);
	}

}

int main() {

	LensceSocket::printErrors(true);
	std::string input;
	std::getline(std::cin, input);
	int port = 5000;
	int maxClients = 10;
	if (input.compare("s") == 0) {

		printf("Server Mode\n");
		server.connectCallback(onConnect);
		server.disconnectCallback(onDisconnect);
		server.receiveTCPCallback(dataReceivedServer);
		server.start(port, maxClients, maxClients);
		while (server.isRunning()) {

			std::getline(std::cin, input);
			if (input.compare("exit") == 0) {
				server.stop();
				break;
			}

			std::string output = "Server: ";
			output.append(input);
			server.sendTCPAll(output.c_str(), (int)output.length());

		}

	}
	else {

		printf("Client Mode\n");
		LensceClient client;
		client.connect("192.168.1.136", port);
		client.receiveTCPCallback(dataReceived);

		while (client.isConnected()) {
			std::getline(std::cin, input);
			if (input.compare("exit") == 0) {
				client.disconnect();
				break;
			}
			client.sendTCP(input.c_str(), (int)input.length());
		}

	}

	return 0;

}