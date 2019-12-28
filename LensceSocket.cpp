#include "LensceSocket.h"

#include <WS2tcpip.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

static bool wsa_uninit = true;
static const WORD WSA_VERSION = MAKEWORD(2, 2);
static WSADATA WSA_DATA = { 0 };

LensceSocket::LensceSocket(const char* ip, int port) {

	init(ip, port);

}

LensceSocket::LensceSocket(int port) : LensceSocket(nullptr, port) { }

LensceSocket::LensceSocket() { }

bool LensceSocket::init(const char* ip, int port) {

	if (wsa_uninit) {
		wsa_uninit = false;
		if (WSAStartup(WSA_VERSION, &WSA_DATA) != 0) {
			printError("initializing WSA");
			return false;
		}
	}

	tcp = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, NULL);
	if (tcp == INVALID_SOCKET) {
		printError("creating TCP socket");
		return false;
	}
	udp = WSASocketW(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, NULL, NULL);
	if (udp == INVALID_SOCKET) {
		closesocket(tcp);
		printError("creating UDP socket");
		return false;
	}

	udpAddr.sin_family = AF_INET;
	udpAddr.sin_port = 0;
	udpAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	if (!ip) {
		tcpAddr.sin_addr.S_un.S_addr = INADDR_ANY;
		udpAddr.sin_port = htons(port);
	}
	else {
		if (InetPton(AF_INET, ip, &tcpAddr.sin_addr.S_un.S_addr) != 1) {
			printError("formatting address (InetPton)");
			return false;
		}
	}

	tcpAddr.sin_family = AF_INET;
	tcpAddr.sin_port = htons(port);
	return true;

}

bool LensceSocket::Connect() {

	if (isConnected()) {
		return true;
	}

	connected = false;

	if (connect(tcp, (SOCKADDR*) &tcpAddr, sizeof(tcpAddr)) == SOCKET_ERROR) {
		int error = WSAGetLastError();
		if (error == WSAECONNREFUSED || error == WSAENETUNREACH || error == WSAETIMEDOUT) {
			return false;
		}
		printError("connecting to socket");
		return false;
	}

	connected = true;
	return true;

}

void LensceSocket::Disconnect() {
	shutdown(tcp, SD_BOTH);
	closesocket(tcp);
	connected = false;
}

void LensceSocket::CloseUDP() {
	shutdown(udp, SD_BOTH);
	closesocket(udp);
}

bool LensceSocket::SendTCP(const char* buf, int len) {

	if (!isConnected()) {
		return false;
	}

	if (!buf) {
		printError("null-referenced buffer (Send)");
		return false;
	}

	if (sendto(tcp, buf, len, NULL, (SOCKADDR*)&tcpAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
		printError("sending TCP data");
		std::cout << WSAGetLastError() << std::endl;
		return false;
	}

	return true;

}

bool LensceSocket::RecvTCP(char* buf, int len, int& bytes_read) {

	bytes_read = 0;

	if (!isConnected()) {
		return false;
	}

	if (!buf) {
		printError("null-referenced buffer (Recv)");
		return false;
	}

	bytes_read = recv(tcp, buf, len, NULL);
	if (bytes_read == SOCKET_ERROR) {
		bytes_read = 0;
		return false;
	}

	return true;

}

bool LensceSocket::SendUDP(const char* buf, int len) {

	if (sendto(udp, buf, len, 0, (SOCKADDR*)& tcpAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
		printError("sending UDP data");
		return false;
	}

	return true;

}

sockaddr LensceSocket::RecvUDP(char* buf, int max_len, int& bytes_read) {

	sockaddr sender = { 0 };
	int fromlen = sizeof(sockaddr_in);
	bytes_read = recvfrom(udp, buf, max_len, 0, &sender, &fromlen);
	if (bytes_read == SOCKET_ERROR) {
		bytes_read = 0;
		printError("receiving UDP data");
	}

	return sender;

}

bool LensceSocket::Bind() {

	if (isConnected()) {
		return false;
	}

	if (bind(udp, (SOCKADDR*)& udpAddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		printError("binding udp socket");
		std::cout << WSAGetLastError() << std::endl;
		return false;
	}

	sockaddr newAddr = { 0 };
	int len = sizeof(sockaddr);
	if (getsockname(udp, &newAddr, &len) == -1) {
		printError("retrieving udp socket name");
		closesocket(udp);
		return false;
	}

	if (newAddr.sa_family != AF_INET) {
		printError("unexpected sa_family (bind())");
		closesocket(udp);
		return false;
	}

	udpAddr = *((SOCKADDR_IN*) &newAddr);
	/* Bind TCP to same local port as UDP */
	if (bind(tcp, (SOCKADDR*) &udpAddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		printError("binding tcp socket");
		closesocket(udp);
		return false;
	}

	return true;

}

bool LensceSocket::ListenTCP(const int backlog) {

	if (listen(tcp, backlog) == SOCKET_ERROR) {
		printError("listening to socket");
		return false;
	}

	connected = true;

	return true;

}

LensceSocket LensceSocket::AcceptTCP() {

	LensceSocket client;
	client.tcp = this->tcp;
	int clientAddrSize = sizeof(SOCKADDR_IN);
	client.tcp = accept(tcp, (SOCKADDR*)& client.tcpAddr, &clientAddrSize);
	client.udp = udp;
	client.udpAddr = udpAddr;
	client.connected = (client.tcp != INVALID_SOCKET);
	if (!client.connected) {
		printError("accepting socket");
	}

	return client;

}

void LensceSocket::printError(const char* error) {

	std::cerr << "Lensce_Socket: Error " << error << std::endl;

}

void LensceSocket::CleanUp() {

	WSACleanup();

}