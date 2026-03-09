#include "SteamBot.h"

#include <string>
#include <iostream>
#include <chrono>

namespace {

	const std::string steamDomain = "steamcommunity.com";

	std::string getUnixTimestamp() {
		auto now = std::chrono::system_clock::now();
		auto duration = now.time_since_epoch();
		auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

		return std::to_string(seconds);
	}

};

SteamBot::SteamBot(const std::string& username, const std::string& password) : username(username), socket(steamDomain) {
	
	headers["Content-Type"] = "application/x-www-form-urlencoded";
	headers["Accept-Language"] = "en-US,en;q=0.9";
	headers["Accept"] = "text/javascript, text/html, application/xml, text/xml, */*";
	headers["Referer"] = "https://steamcommunity.com/login/home/?goto=";
	headers["User-Agent"] = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36";
	headers["Connection"] = "close";

	auto response = sendRequest("/login/getrsakey/", "username=" + username);
	cookies = response.cookies;
	Lensce::JSON jsonResponse(response.content);

	Lensce::TLS::PrivateKey privateKey(jsonResponse["publickey_mod"].value(), jsonResponse["publickey_exp"].value());

	std::vector<BYTE> encryptedPass = privateKey.encrypt(std::vector<BYTE>(password.begin(), password.end()));

	std::string encryptedPassBase64 = Lensce::TLS::base64Encode(encryptedPass);

	std::string body = "username=" + username;
	body += "&password=" + Lensce::HTTP::urlEncode(encryptedPassBase64);
	body += "&rsatimestamp=" + jsonResponse["timestamp"].value();
	body += "&donotcache=" + getUnixTimestamp();
	body += "&remember_login=false";
	response = sendRequest("/login/dologin/", body);
	std::cout << response.toString() << std::endl;

}

Lensce::HTTP::Response SteamBot::sendRequest(const std::string& path, const std::string& body) {

	std::string request = Lensce::HTTP::create(Lensce::HTTP::REST::POST, steamDomain, path, headers, cookies, body);
	std::cout << request << std::endl;
	std::cout << "-----------------------------" << std::endl;
	socket.send(request);
	return Lensce::HTTP::receive(socket);

}