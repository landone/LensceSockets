#include <iostream>

#include <LensceSocket.h>
#include <LensceHTTP.h>
#include <LensceTLS.h>
#include <JSON.h>

int main() {

	std::string domain = "steamcommunity.com";

	Lensce::JSON content;
	content["username"] = "username";

	Lensce::JSON headers;
	headers["Content-Type"] = "application/x-www-form-urlencoded";
	headers["Accept-Language"] = "en-US,en;q=0.9";
	headers["Connection"] = "close";

	std::string rsaKeyReq = Lensce::HTTP::create(Lensce::HTTP::REST::POST, domain, "/login/getrsakey/", headers, content);

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
		for (const auto& pair : response.headers) {
			const std::string& header = pair.first;
			const std::list<std::string>& values = pair.second;
			for (const std::string& value : values) {
				std::cout << header << ": " << value << std::endl;
			}
		}
		std::cout << response.content << std::endl;
	}

	system("pause");
	Lensce::cleanup();
	return 0;
}