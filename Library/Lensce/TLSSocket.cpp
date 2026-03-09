#include "TLS.h"

#include <iostream>

#include <openssl/ssl.h>
#include <openssl/err.h>

namespace Lensce {
	namespace TLS
	{

		static const int MAX_READ_SIZE = 4096;

		Socket::Socket(std::string ip, int port) {
			sslContext = SSL_CTX_new(TLS_client_method());
			if (!sslContext) {
				std::cerr << "Failed to create SSL context: " << ERR_get_error() << std::endl;
				return;
			}
			baseSocket.connect(ip, port);
			sslObject = SSL_new(static_cast<SSL_CTX*>(sslContext));
			if (!sslObject) {
				std::cerr << "Failed to create SSL object: " << ERR_get_error() << std::endl;
				return;
			}
			SSL_set_tlsext_host_name(static_cast<SSL*>(sslObject), ip.c_str());
			SSL_set_fd(static_cast<SSL*>(sslObject), static_cast<int>(baseSocket.getHandle()));
			if (SSL_connect(static_cast<SSL*>(sslObject)) <= 0) {
				std::cerr << "Failed to establish SSL connection: " << ERR_get_error() << std::endl;
				return;
			}
		}

		Socket::~Socket() {
			if (sslObject) {
				SSL_shutdown(static_cast<SSL*>(sslObject));
				SSL_free(static_cast<SSL*>(sslObject));
				sslObject = nullptr;
			}
			if (sslContext) {
				SSL_CTX_free(static_cast<SSL_CTX*>(sslContext));
				sslContext = nullptr;
			}
			baseSocket.close();
		}

		bool Socket::send(const std::vector<BYTE>& data) {
			if (!sslObject) {
				std::cerr << "SSL object not initialized." << std::endl;
				return false;
			}
			int bytesSent = SSL_write(static_cast<SSL*>(sslObject), data.data(), static_cast<int>(data.size()));
			if (bytesSent <= 0) {
				std::cerr << "Failed to send data over SSL: " << ERR_get_error() << std::endl;
				return false;
			}
			return true;
		}

		bool Socket::send(const std::string& data) {
			if (!sslObject) {
				std::cerr << "SSL object not initialized." << std::endl;
				return false;
			}
			int bytesSent = SSL_write(static_cast<SSL*>(sslObject), data.data(), static_cast<int>(data.size()));
			if (bytesSent <= 0) {
				std::cerr << "Failed to send data over SSL: " << ERR_get_error() << std::endl;
				return false;
			}
			return true;
		}

		bool Socket::receive(std::vector<BYTE>& buffer, bool append) {
			if (!sslObject) {
				std::cerr << "SSL object not initialized." << std::endl;
				return false;
			}
			std::vector<BYTE> newData;
			newData.resize(MAX_READ_SIZE);
			int bytesRead = SSL_read(static_cast<SSL*>(sslObject), newData.data(), MAX_READ_SIZE);
			if (bytesRead <= 0) {
				if (bytesRead < 0) {
					std::cerr << "Failed to read data over SSL: " << ERR_get_error() << std::endl;
				}
				return false;
			}
			newData.resize(bytesRead);

			if (append) {
				buffer.insert(buffer.end(), newData.begin(), newData.end());
			}
			else {
				buffer = std::move(newData);
			}

			return true;
		}

	}
}