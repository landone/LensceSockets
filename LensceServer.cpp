#include "LensceServer.h"

#include <iostream>
#ifdef LENSCE_LINUX
#include <poll.h>
#else
#include <WS2tcpip.h>
#endif

void LensceClientLoop(LensceServer& server, int client) {

	LensceSocket& soc = server.thrClients[client].soc;
	bool& kicked = server.thrClients[client].kicked;
	#ifdef LENSCE_LINUX
		struct pollfd fdArray = { 0 };
		fdArray.fd = soc.getTCPSocket();
		fdArray.events = POLLRDNORM;
		fdArray.revents = 0;

		while (server.isRunning() && soc.isConnected() && !kicked) {
			
			if (poll(&fdArray, 1, 1) > 0) {
				char buf[server.MAX_READ];
				int read = 0;
				if (soc.RecvTCP(buf, server.MAX_READ, read) && read > 0) {
					if (server.receiveTCPCbk) {
						server.receiveTCPCbk(client, buf, read);
					}
				}
				else {
					break;
				}
			}

		}
	#else
		WSAPOLLFD fdArray = { 0 };
		fdArray.fd = soc.getTCPSocket();
		fdArray.events = POLLRDNORM;
		fdArray.revents = 0;

		while (server.isRunning() && soc.isConnected() && !kicked) {
			
			if (WSAPoll(&fdArray, 1, 1) > 0) {
				char buf[server.MAX_READ];
				int read = 0;
				if (soc.RecvTCP(buf, server.MAX_READ, read) && read > 0) {
					if (server.receiveTCPCbk) {
						server.receiveTCPCbk(client, buf, read);
					}
				}
				else {
					break;
				}
			}

		}
	#endif

	if (server.disconnectCbk) {
		server.disconnectCbk(client);
	}
	soc.Disconnect();

	server.clientMtx.lock();
	server.clientCount--;
	server.clientMtx.unlock();

}

void LensceServerTCPLoop(LensceServer& server) {

	LensceSocket& soc = server.soc;

	while (soc.isConnected()) {

		LensceSocket client = soc.AcceptTCP();
		if (client.isConnected()) {
			if (server.addClient(client) == -1) {
				client.Disconnect();
			}
		}

	}

}

void LensceServerUDPLoop(LensceServer& server) {

	LensceSocket& soc = server.soc;
	#ifdef LENSCE_LINUX
		struct pollfd fdArray = { 0 };
		fdArray.fd = soc.getUDPSocket();
		fdArray.events = POLLRDNORM;
		fdArray.revents = 0;
		char buf[server.MAX_READ];

		while (soc.isConnected()) {
			
			if (poll(&fdArray, 1, 1) > 0) {
				int read = 0;
				sockaddr addr = soc.RecvUDP(buf, server.MAX_READ, read);
				if ((server.receiveUDPCbk) && (read > 0)) {
					struct sockaddr_in adin = *((struct sockaddr_in*)&addr);
					int client = -1;

					for (int i = 0; i < server.maxClients; ++i) {

						if (!server.thrClients[i].soc.isConnected()) {
							continue;
						}

						struct sockaddr_in clAddr = server.thrClients[i].soc.getAddr();
						if ((clAddr.sin_addr.s_addr == adin.sin_addr.s_addr)
							&& (clAddr.sin_port == adin.sin_port)) {
							client = i;
						}

					}

					if (client > -1) {
						server.receiveUDPCbk(client, buf, read);
					}

				}
			}
		}
	#else
		WSAPOLLFD fdArray = { 0 };
		fdArray.fd = soc.getUDPSocket();
		fdArray.events = POLLRDNORM;
		fdArray.revents = 0;
		char buf[server.MAX_READ];

		while (soc.isConnected()) {
			
			if (WSAPoll(&fdArray, 1, 1) > 0) {
				int read = 0;
				sockaddr addr = soc.RecvUDP(buf, server.MAX_READ, read);
				if ((server.receiveUDPCbk) && (read > 0)) {
					SOCKADDR_IN adin = *((SOCKADDR_IN*)&addr);
					int client = -1;

					for (int i = 0; i < server.maxClients; ++i) {

						if (!server.thrClients[i].soc.isConnected()) {
							continue;
						}

						SOCKADDR_IN clAddr = server.thrClients[i].soc.getAddr();
						if ((clAddr.sin_addr.S_un.S_addr == adin.sin_addr.S_un.S_addr)
							&& (clAddr.sin_port == adin.sin_port)) {
							client = i;
						}

					}

					if (client > -1) {
						server.receiveUDPCbk(client, buf, read);
					}

				}
			}
		}
	#endif
}

LensceServer::LensceServer() {

	

}

LensceServer::~LensceServer() {

	stop();

}

bool LensceServer::start(int port, int maxClients, int backlog) {

	if (isRunning()) {
		return true;
	}

	soc.Disconnect();
	soc.CloseUDP();

	this->maxClients = maxClients;
	this->backlog = backlog;

	if (!soc.init(NULL, port)) {
		printError("initializing");
		return false;
	}

	if (thrClients) {
		delete[] thrClients;
		thrClients = nullptr;
	}
	thrClients = new Client[maxClients];
	
	if (!(soc.Bind() && soc.ListenTCP(backlog))) {
		printError("starting");
		soc.Disconnect();
		return false;
	}

	thrServer[0] = std::thread(LensceServerTCPLoop, std::ref(*this));
	thrServer[1] = std::thread(LensceServerUDPLoop, std::ref(*this));

	return true;

}

void LensceServer::stop() {

	stopMtx.lock();

	soc.Disconnect();
	for (int i = 0; i < 2; i++) {
		if (thrServer[i].joinable()) {
			if (std::this_thread::get_id() != thrServer[i].get_id()) {
				thrServer[i].join();
			}
		}
	}

	for (int i = 0; i < maxClients; ++i) {
		if (thrClients[i].thr.joinable()) {
			if (std::this_thread::get_id() != thrClients[i].thr.get_id()) {
				thrClients[i].thr.join();
			}
		}
	}

	stopMtx.unlock();

}

int LensceServer::addClient(LensceSocket soc) {

	clientMtx.lock();

	if (clientCount >= maxClients) {
		clientMtx.unlock();
		return -1;
	}

	clientCount++;
	clientMtx.unlock();

	for (int i = 0; i < maxClients; ++i) {
		if (!thrClients[i].soc.isConnected()) {

			thrClients[i].soc = soc;

			if (thrClients[i].thr.joinable()) {
				thrClients[i].thr.join();
			}

			thrClients[i].kicked = false;

			if (connectCbk) {
				connectCbk(i);
			}

			thrClients[i].thr = std::thread(LensceClientLoop, std::ref(*this), i);
			return i;
		}
	}

	return -1;

}

bool LensceServer::sendTCP(int client, const char* data, int size) {

	LensceSocket& soc = thrClients[client].soc;

	if (!soc.isConnected()) {
		return false;
	}

	return soc.SendTCP(data, size);

}

void LensceServer::sendTCPAll(const char* data, int size) {

	for (int i = 0; i < maxClients; ++i) {

		if (thrClients[i].soc.isConnected()) {
			thrClients[i].soc.SendTCP(data, size);
		}

	}

}

bool LensceServer::sendUDP(int client, const char* data, int size) {

	LensceSocket& soc = thrClients[client].soc;

	if (!soc.isConnected()) {
		return false;
	}

	return soc.SendUDP(data, size);

}

void LensceServer::sendUDPAll(const char* data, int size) {

	for (int i = 0; i < maxClients; ++i) {

		if (thrClients[i].soc.isConnected()) {
			thrClients[i].soc.SendUDP(data, size);
		}

	}

}

void LensceServer::kick(int client) {

	clientMtx.lock();
	if (client < 0 || client >= clientCount) {
		clientMtx.unlock();
		return;
	}
	clientMtx.unlock();

	thrClients[client].kicked = true;

}

void LensceServer::receiveTCPCallback(void(*f)(int client, char[MAX_READ], int size))
{
	receiveTCPCbk = f;
}

void LensceServer::receiveUDPCallback(void(*f)(int client, char[MAX_READ], int size))
{
	receiveUDPCbk = f;
}

void LensceServer::connectCallback(void(*f)(int client)) {
	connectCbk = f;
}

void LensceServer::disconnectCallback(void(*f)(int client)) {
	disconnectCbk = f;
}

void LensceServer::printError(const char* error) {

	std::cerr << "Lensce_Server: Error " << error << std::endl;

}