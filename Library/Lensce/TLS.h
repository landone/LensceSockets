#pragma once

#include <vector>

#include "Socket.h"

using BYTE = unsigned char;

namespace Lensce {
	namespace TLS {

		class Socket {
		public:

			Socket(std::string ip, int port = 443);
			~Socket();

			bool send(const std::vector<BYTE>& data);
			bool send(const std::string& data);
			bool receive(std::vector<BYTE>& buffer, bool append=false);

		private:

			Lensce::Socket baseSocket;
			void* sslObject = nullptr;
			void* sslContext = nullptr;

		};

		class PrivateKey {
		public:

			PrivateKey(const std::string& modulusHex, const std::string& exponentHex);
			~PrivateKey();

			std::vector<BYTE> encrypt(const std::vector<BYTE>& data);

		private:

			void* privateKeyContext = nullptr;

		};

		std::string base64Encode(const std::vector<BYTE>& data);

	}
}