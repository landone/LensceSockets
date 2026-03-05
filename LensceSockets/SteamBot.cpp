#include "SteamBot.h"

#include <string>
#include <iostream>

namespace {

	const std::string steamDomain = "steamcommunity.com";

};

SteamBot::SteamBot(const std::string& username, const std::string& password) : username(username), socket(steamDomain) {

	headers["Content-Type"] = "application/x-www-form-urlencoded";
	headers["Accept-Language"] = "en-US,en;q=0.9";
	headers["Connection"] = "close";

	auto response = sendRequest("/login/getrsakey/", "username=" + username);
	Lensce::JSON jsonResponse(response.content);
	std::cout << jsonResponse.toString() << std::endl;

}

Lensce::HTTP::Response SteamBot::sendRequest(const std::string& path, const std::string& body) {

	std::string request = Lensce::HTTP::create(Lensce::HTTP::REST::POST, steamDomain, path, headers, body);
	socket.send(request);
	return Lensce::HTTP::receive(socket);

}