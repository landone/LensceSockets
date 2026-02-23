#pragma once
#include <string>

namespace Lensce {

	void cleanup();

	class Socket {
	public:

		Socket(bool isTCP = true);
		~Socket();

		bool connect(const std::string& address, int port);
		bool send(const char* data, int size);
		int receive(char* buffer, int size, unsigned long* address = nullptr, unsigned short* port = nullptr);

		bool listen(int port, int backlog);
		bool listen(int port);

		bool accept(Socket& connection);

		void close();

	private:

		unsigned long long socketHandle;
		char* socketAddressPtr = nullptr;
		bool isTCP = true;

		bool bind(int port);

	};

}