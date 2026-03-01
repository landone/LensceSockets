#pragma once

#include <vector>

#include "LensceSocket.h"

using BYTE = unsigned char;

namespace Lensce {
	namespace TLS {

		class Socket {
		public:

			Socket(std::string ip, int port = 443);
			~Socket();

			bool send(const std::vector<BYTE>& data);
			bool receive(std::vector<BYTE>& buffer, bool append=false);

		private:

			Lensce::Socket baseSocket;
			void* sslObject = nullptr;
			void* sslContext = nullptr;

		};

	}
}