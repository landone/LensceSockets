#include "LensceSocket.h"
#include <errno.h>
#include <iostream>

#ifdef LENSCE_LINUX
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#define SOCKET_ERROR	-1
#else
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

bool LensceSocket::PRINT_ERRORS = false;
#ifndef LENSCE_LINUX
static bool wsa_uninit = true;
static const WORD WSA_VERSION = MAKEWORD(2, 2);
static WSADATA WSA_DATA = { 0 };
#endif

LensceSocket::LensceSocket(const char* ip, int port) {

	init(ip, port);

}

LensceSocket::LensceSocket(int port) : LensceSocket(nullptr, port) { }

LensceSocket::LensceSocket() { }

bool LensceSocket::init(const char* ip, int port) {

	#ifdef LENSCE_LINUX
		tcp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (tcp == -1){
			printError("creating TCP socket");
			return false;
		}
		udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (udp == -1){
			close(tcp);
			return false;
		}

		udpAddr.sin_family = AF_INET;
		udpAddr.sin_port = 0;
		udpAddr.sin_addr.s_addr = INADDR_ANY;

		if (!ip) {
			tcpAddr.sin_addr.s_addr = INADDR_ANY;
			udpAddr.sin_port = htons(port);
		}
		else {
			if (inet_pton(AF_INET, ip, &tcpAddr.sin_addr.s_addr) != 1) {
				printError("formatting address (InetPton)");
				return false;
			}
		}
	#else
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
	#endif

	tcpAddr.sin_family = AF_INET;
	tcpAddr.sin_port = htons(port);
	return true;

}

void LensceSocket::printErrors(bool toggle) {

	PRINT_ERRORS = toggle;

}

bool LensceSocket::Connect() {

	if (isConnected()) {
		return true;
	}

	connected = false;

	#ifdef LENSCE_LINUX
		if (connect(tcp, (sockaddr*)&tcpAddr, sizeof(tcpAddr)) == -1) {
				if (errno == ECONNREFUSED || errno == ENETUNREACH || errno == ETIMEDOUT) {
					return false;
				}
				printError("connecting to socket");
				return false;
		}
	#else
		if (connect(tcp, (SOCKADDR*) &tcpAddr, sizeof(tcpAddr)) == SOCKET_ERROR) {
			int error = WSAGetLastError();
			if (error == WSAECONNREFUSED || error == WSAENETUNREACH || error == WSAETIMEDOUT) {
				return false;
			}
			printError("connecting to socket");
			return false;
		}
	#endif

	connected = true;
	return true;

}

void LensceSocket::Disconnect() {
	#ifdef LENSCE_LINUX
	shutdown(tcp, SHUT_RDWR);
	close(tcp);
	#else
	shutdown(tcp, SD_BOTH);
	closesocket(tcp);
	#endif
	connected = false;
}

void LensceSocket::CloseUDP() {
	#ifdef LENSCE_LINUX
	shutdown(udp, SHUT_RDWR);
	close(udp);
	#else
	shutdown(udp, SD_BOTH);
	closesocket(_USE_DECLSPECS_FOR_SAL);
	#endif
}

bool LensceSocket::SendTCP(const char* buf, int len) {

	if (!isConnected()) {
		return false;
	}

	if (!buf) {
		printError("null-referenced buffer (Send)");
		return false;
	}

	#ifdef LENSCE_LINUX
	if (sendto(tcp, buf, len, 0, (sockaddr*)&tcpAddr, sizeof(sockaddr)) == -1) {
		printError("sending TCP data");
		return false;
	}
	#else
	if (sendto(tcp, buf, len, NULL, (SOCKADDR*)&tcpAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
		printError("sending TCP data");
		return false;
	}
	#endif

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

	bytes_read = recv(tcp, buf, len, 0);
	if (bytes_read == SOCKET_ERROR) {
		bytes_read = 0;
		return false;
	}

	return true;

}

bool LensceSocket::SendUDP(const char* buf, int len) {

	#ifdef LENSCE_LINUX
	if (sendto(udp, buf, len, 0, (sockaddr*)& tcpAddr, sizeof(sockaddr)) == SOCKET_ERROR) {
	#else
	if (sendto(udp, buf, len, 0, (SOCKADDR*)& tcpAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
	#endif
		printError("sending UDP data");
		return false;
	}

	return true;

}

sockaddr LensceSocket::RecvUDP(char* buf, int max_len, int& bytes_read) {

	sockaddr sender = { 0 };
	int fromlen = sizeof(sockaddr_in);
	#ifdef LENSCE_LINUX
	bytes_read = recvfrom(udp, buf, max_len, 0, &sender, (socklen_t*)&fromlen);
	#else
	bytes_read = recvfrom(udp, buf, max_len, 0, &sender, &fromlen);
	#endif
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

	#ifdef LENSCE_LINUX
		if (bind(udp, (sockaddr*)& udpAddr, sizeof(sockaddr)) == SOCKET_ERROR) {
			printError("binding udp socket");
			std::cout << "Error Number " << errno << std::endl;
			return false;
		}

		sockaddr newAddr = { 0 };
		int len = sizeof(sockaddr);
		if (getsockname(udp, &newAddr, (socklen_t*)&len) == -1) {
			printError("retrieving udp socket name");
			close(udp);
			return false;
		}

		if (newAddr.sa_family != AF_INET) {
			printError("unexpected sa_family (bind())");
			close(udp);
			return false;
		}

		udpAddr = *((sockaddr_in*) &newAddr);
		/* Bind TCP to same local port as UDP */
		if (bind(tcp, (sockaddr*) &udpAddr, sizeof(sockaddr)) == SOCKET_ERROR) {
			printError("binding tcp socket");
			close(udp);
			return false;
		}
	#else
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
	#endif

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
	#ifdef LENSCE_LINUX
		int clientAddrSize = sizeof(sockaddr_in);
		client.tcp = accept(tcp, (sockaddr*)& client.tcpAddr, (socklen_t*)&clientAddrSize);
		client.udp = udp;
		client.udpAddr = udpAddr;
		client.connected = (client.tcp != -1);
		if (!client.connected) {
			printError("accepting socket");
		}
	#else
		int clientAddrSize = sizeof(SOCKADDR_IN);
		client.tcp = accept(tcp, (SOCKADDR*)& client.tcpAddr, &clientAddrSize);
		client.udp = udp;
		client.udpAddr = udpAddr;
		client.connected = (client.tcp != INVALID_SOCKET);
		if (!client.connected) {
			printError("accepting socket");
		}
	#endif
	return client;

}

void LensceSocket::printError(const char* error) {

	if (PRINT_ERRORS) {
		std::cerr << "Lensce_Socket: Error(" << errno << ") " << error << std::endl;
	}

}

void LensceSocket::CleanUp() {

	#ifndef LENSCE_LINUX
	WSACleanup();
	#endif

}