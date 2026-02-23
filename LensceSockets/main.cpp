#include <iostream>

#include <LensceSocket.h>
#include <LensceHTTP.h>

int main() {

	std::string ip = "google.com";
	char buffer[1024 * 10];

	std::string request = 
		"GET / HTTP/1.1\r\n"
		"Host: www.google.com\r\n"
		"Accept: */*"
		"Accept-Language: en-US,en;q=0.9\r\n"
		"Connection: close\r\n"
		"\r\n";

	std::cout << "Connecting to " << ip << "..." << std::endl << std::endl;

	Lensce::Socket socket;
	socket.connect(ip, 80);

	socket.send(request.c_str(), request.size());

	char* pos = buffer;
	int bytesReceived = 0;
	while ((bytesReceived = socket.receive(pos, sizeof(buffer))) > 0) {
		pos += bytesReceived;
	}

	std::cout << buffer << std::endl;

	socket.close();

	auto response = Lensce::HTTP::read(buffer);

	std::cout << buffer << std::endl;
	

	system("pause");
	Lensce::cleanup();
	return 0;
}