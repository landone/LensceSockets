#include <iostream>

#include <LensceSocket.h>
#include <LensceHTTP.h>
#include <LensceTLS.h>

int main() {

	std::string ip = "steamcommunity.com";
	//char buffer[1024 * 10];

	std::string request = 
		"GET / HTTP/1.1\r\n"
		"Host: steamcommunity.com\r\n"
		"Accept: */*"
		"Accept-Language: en-US,en;q=0.9\r\n"
		"Connection: close\r\n"
		"\r\n";

	std::cout << "Connecting to " << ip << "..." << std::endl << std::endl;

	{
		Lensce::TLS::Socket socket(ip);
		std::vector<BYTE> data(request.begin(), request.end());
		socket.send(data);
		while (socket.receive(data)) {
			for (BYTE byte : data) {
				std::cout << static_cast<char>(byte);
			}
		}
		std::cout << std::endl << std::endl;
	}

	system("pause");
	Lensce::cleanup();
	return 0;
}