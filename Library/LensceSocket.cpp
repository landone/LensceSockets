#include "LensceSocket.h"

#include <iostream>
#include <regex>

#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

namespace {
	
	bool isInitialized = false;
	std::regex ipPattern(R"(^(([0-9]{1,3}\.){3}([0-9]{1,3}))$)");

	void initializeWinSock() {

		if (isInitialized) return;

		isInitialized = true;
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			std::cerr << "WSAStartup failed: " << iResult << std::endl;
		}

	}

}

namespace Lensce {

	void cleanup() {
		WSACleanup();
		isInitialized = false;
	}
	
	Socket::Socket(bool isTCP) {
		
		this->isTCP = isTCP;
		initializeWinSock();
		if (isTCP) {
			socketHandle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		}
		else {
			socketHandle = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		}
		if (socketHandle == INVALID_SOCKET) {
			std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
			return;
		}

	}

	Socket::~Socket() {

		if (socketAddressPtr) {
			delete reinterpret_cast<sockaddr_in*>(socketAddressPtr);
			socketAddressPtr = nullptr;
		}
		close();

	}

	bool Socket::send(const char* data, int size) {

		int bytesSent;
		if (isTCP) {
			bytesSent = ::send(static_cast<SOCKET>(socketHandle), data, size, 0);
		}
		else {
			if (socketAddressPtr == nullptr) {
				std::cerr << "UDP socket address not set. Call connect() first." << std::endl;
				return false;
			}
			bytesSent = sendto(static_cast<SOCKET>(socketHandle), data, size, 0, 
				reinterpret_cast<sockaddr*>(socketAddressPtr), sizeof(sockaddr_in));
		}

		if (bytesSent == SOCKET_ERROR) {
			std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
			return false;
		}

		return true;

	}

	int Socket::receive(char* buffer, int size, unsigned long* address, unsigned short* port) {

		int bytesReceived;
		if (isTCP) {
			bytesReceived = ::recv(static_cast<SOCKET>(socketHandle), buffer, size, 0);
		}
		else {
			sockaddr_in fromAddress{};
			int fromAddressSize = sizeof(fromAddress);
			bytesReceived = recvfrom(static_cast<SOCKET>(socketHandle), buffer, size, 0, (sockaddr*)&fromAddress, &fromAddressSize);
			if (address) {
				*address = fromAddress.sin_addr.S_un.S_addr;
			}
			if (port) {
				*port = fromAddress.sin_port;
			}
		}
		if (bytesReceived == SOCKET_ERROR) {
			std::cerr << "Receive failed with error: " << WSAGetLastError() << std::endl;
			return -1;
		}

		return bytesReceived;

	}

	bool Socket::connect(const std::string& address, int port) {

		bool isDomain = !std::regex_match(address, ipPattern);

		sockaddr_in serverAddress{};
		serverAddress.sin_family = AF_INET;

		if (isDomain) {
			struct addrinfo hints {};
			struct addrinfo* result = nullptr;
			hints.ai_family = AF_INET;
			hints.ai_socktype = isTCP ? SOCK_STREAM : SOCK_DGRAM;
			if (getaddrinfo(address.c_str(), NULL, &hints, &result) != 0) {
				std::cerr << "Failed to resolve domain name: " << address << std::endl;
				return false;
			}
			serverAddress.sin_addr = ((sockaddr_in*)result->ai_addr)->sin_addr;
			freeaddrinfo(result);
		}
		else if (inet_pton(AF_INET, address.c_str(), &serverAddress.sin_addr) <= 0) {
			std::cerr << "Invalid address / Address not supported: " << address << std::endl;
			return false;
		}

		serverAddress.sin_port = htons(static_cast<u_short>(port));

		if (!isTCP) {

			if (socketAddressPtr) {
				delete reinterpret_cast<sockaddr_in*>(socketAddressPtr);
				socketAddressPtr = nullptr;
			}

			auto addrPtr = new sockaddr_in;
			*addrPtr = serverAddress;
			socketAddressPtr = reinterpret_cast<char*>(addrPtr);
			return true;

		}

		int result = ::connect(static_cast<SOCKET>(socketHandle), reinterpret_cast<sockaddr*>(&serverAddress), sizeof(serverAddress));
		if (result == SOCKET_ERROR) {
			std::cerr << "Unable to connect to server: " << WSAGetLastError() << std::endl;
			return false;
		}

		return true;

	}

	bool Socket::accept(Socket& newSocket) {

		if (!isTCP) {
			return false;
		}

		sockaddr_in clientAddress{};
		int clientAddressSize = sizeof(clientAddress);
		SOCKET clientSocket = ::accept(static_cast<SOCKET>(socketHandle), reinterpret_cast<sockaddr*>(&clientAddress), &clientAddressSize);
		if (clientSocket == INVALID_SOCKET) {
			std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
			return false;
		}

		newSocket.socketHandle = static_cast<unsigned long long>(clientSocket);
		return true;

	}

	bool Socket::listen(int port) {
		return listen(port, SOMAXCONN);
	}

	bool Socket::listen(int port, int backlog) {

		if (!bind(port)) {
			return false;
		}

		if (!isTCP) {
			return false;
		}
		
		int result = ::listen(static_cast<SOCKET>(socketHandle), backlog);
		if (result == SOCKET_ERROR) {
			std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
			return false;
		}

		return true;
	}

	bool Socket::bind(int port) {

		sockaddr_in address{};
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(static_cast<u_short>(port));

		int result = ::bind(static_cast<SOCKET>(socketHandle), reinterpret_cast<sockaddr*>(&address), sizeof(address));
		if (result == SOCKET_ERROR) {
			std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
			return false;
		}

		return true;

	}

	void Socket::close() {
		if (socketHandle != INVALID_SOCKET) {
			::closesocket(static_cast<SOCKET>(socketHandle));
			socketHandle = INVALID_SOCKET;
		}
	}

}