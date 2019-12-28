#include "LensceClient.h"

#include <iostream>

static void LensceClientTCPThread(LensceClient& client) {

	SOCKET tcpSoc = client.soc.getTCPSocket();
	WSAPOLLFD fdArray = { 0 };
	fdArray.fd = tcpSoc;
	fdArray.events = POLLRDNORM;
	fdArray.revents = 0;

	while (client.soc.isConnected()) {

		if (WSAPoll(&fdArray, 1, 1) > 0) {
			char data[LensceClient::MAX_READ];
			int read = 0;
			if ((client.soc.RecvTCP(data, LensceClient::MAX_READ, read)) && (read > 0)) {
				if (client.receiveTCPCbk) {
					client.receiveTCPCbk(data, read);
				}
			}
			else {
				break;
			}
		}

	}
	
	if (client.disconnectCbk) {
		client.disconnectCbk();
	}
	
	client.disconnect();

}

static void LensceClientUDPThread(LensceClient& client) {

	SOCKET udpSoc = client.soc.getUDPSocket();
	WSAPOLLFD fdArray = { 0 };
	fdArray.fd = udpSoc;
	fdArray.events = POLLRDNORM;
	fdArray.revents = 0;
	
	while (client.soc.isConnected()) {

		if (WSAPoll(&fdArray, 1, 1) > 0) {
			char data[LensceClient::MAX_READ];
			int read = 0;
			sockaddr addr = client.soc.RecvUDP(data, LensceClient::MAX_READ, read);
			if ((read > 0) && (client.receiveUDPCbk)) {

				SOCKADDR_IN adin = *((SOCKADDR_IN*)&addr);
				SOCKADDR_IN server = client.soc.getAddr();

				if ((server.sin_addr.S_un.S_addr == adin.sin_addr.S_un.S_addr)
					&& (server.sin_port == adin.sin_port)) {
					client.receiveUDPCbk(data, read);
				}

			}
		}
	}
}

LensceClient::LensceClient() {

	/* Do nothing */

}

LensceClient::~LensceClient() {

	disconnect();

}

bool LensceClient::connect(std::string ip, int port) {

	disconnect();
	soc.init(ip.c_str(), port);
	soc.Bind();
	if (soc.Connect()) {
		threads[0] = std::thread(LensceClientTCPThread, std::ref(*this));
		threads[1] = std::thread(LensceClientUDPThread, std::ref(*this));
		connected = true;
		return true;
	}

	return false;

}

void LensceClient::disconnect() {

	discMtx.lock();

	soc.Disconnect();
	soc.CloseUDP();
	for (int i = 0; i < 2; ++i) {
		if (threads[i].joinable()) {
			/* Only join threads which are not self */
			if (std::this_thread::get_id() != threads[i].get_id()) {
				threads[i].join();
			}
		}
	}
	discMtx.unlock();

	connected = false;

}

bool LensceClient::sendTCP(const char* buf, int len) {

	return soc.SendTCP(buf, len);

}

bool LensceClient::sendUDP(const char* buf, int len) {

	return soc.SendUDP(buf, len);

}

void LensceClient::receiveTCPCallback(void(*f)(char*, int)) {

	receiveTCPCbk = f;

}

void LensceClient::receiveUDPCallback(void(*f)(char*, int)) {

	receiveUDPCbk = f;

}

void LensceClient::disconnectCallback(void(*f)()) {

	disconnectCbk = f;

}