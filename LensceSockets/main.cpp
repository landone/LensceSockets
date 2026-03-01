#include <iostream>

#include <LensceSocket.h>
#include <LensceHTTP.h>
#include <LensceTLS.h>
#include <JSON.h>

int main() {

	std::string ip = "steamcommunity.com";

	std::string request =
		"GET / HTTP/1.1\r\n"
		"Host: steamcommunity.com\r\n"
		"Accept: */*\r\n"
		"Accept-Language: en-US,en;q=0.9\r\n"
		"Connection: close\r\n"
		"\r\n";

	std::string publicKeyRequest = 
		"POST /IAuthenticationService/GetPasswordRSAPublicKey/v1/ HTTP/1.1\r\n"
		"Host: api.steampowered.com\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Accept-Language: en-US,en;q=0.9\r\n"
		"Connection: close\r\n"
		"\r\n";

	std::cout << "Connecting to " << ip << "..." << std::endl << std::endl;

	 {
		Lensce::TLS::Socket socket(ip);
		std::vector<BYTE> data(request.begin(), request.end());
		socket.send(data);
		data.clear();
		std::string result;
		while (socket.receive(data, true)) {}
		result = std::string(data.begin(), data.end());

		auto response = Lensce::HTTP::read(result);
		for (const auto& pair : response.headers) {
			const std::string& header = pair.first;
			const std::list<std::string>& values = pair.second;
			for (const std::string& value : values) {
				std::cout << header << ": " << value << std::endl;
			}
		}
		//std::cout << response.content << std::endl;
	}

	system("pause");
	Lensce::cleanup();
	return 0;
}