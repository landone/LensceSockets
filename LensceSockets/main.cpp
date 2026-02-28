#include <iostream>

#include <LensceSocket.h>
#include <LensceHTTP.h>
#include <LensceTLS.h>
#include <JSON.h>

int main() {

	std::string rawJson =
	"{"
		"\"username\": \"test\","
		"\"password\" : \"spooky\","
		"\"object\" : {"
			"\"value1\": \"test2\","
			"\"value2\" : \"test3\""
		"}"
	"}";

	Lensce::JSON json(rawJson);

	std::cout << json.toString(true) << std::endl;

	/*std::string ip = "steamcommunity.com";

	std::string request =
		"GET / HTTP/1.1\r\n"
		"Host: steamcommunity.com\r\n"
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
		while (socket.receive(data)) {
			for (BYTE byte : data) {
				std::cout << static_cast<char>(byte);
			}
		}
		std::cout << std::endl << std::endl;
	}*/

	system("pause");
	Lensce::cleanup();
	return 0;
}