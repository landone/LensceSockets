#include <iostream>

#include <LensceSocket.h>
#include <LensceHTTP.h>
#include <LensceTLS.h>
#include <JSON.h>

int main() {

	std::string domain = "steamcommunity.com";

	Lensce::JSON headers;
	headers["Content-Type"] = "application/x-www-form-urlencoded";
	headers["Accept-Language"] = "en-US,en;q=0.9";
	headers["Connection"] = "close";

	std::string body = "username=username";
	std::string rsaKeyReq = Lensce::HTTP::create(Lensce::HTTP::REST::POST, domain, "/login/getrsakey/", headers, body);

	std::cout << "Connecting to " << domain << "..." << std::endl << std::endl;

	 {
		Lensce::TLS::Socket socket(domain);
		std::vector<BYTE> data(rsaKeyReq.begin(), rsaKeyReq.end());
		socket.send(data);
		data.clear();
		std::string result;
		while (socket.receive(data, true)) {}
		result = std::string(data.begin(), data.end());

		auto response = Lensce::HTTP::read(result);
		Lensce::JSON jsonResponse(response.content);
		std::cout << jsonResponse["publickey_mod"].value() << std::endl;
	}

	system("pause");
	Lensce::cleanup();
	return 0;
}