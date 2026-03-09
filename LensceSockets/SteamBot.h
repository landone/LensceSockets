#pragma once

#include <string>

#include <Lensce/TLS.h>
#include <Lensce/HTTP.h>
#include <Lensce/JSON.h>

class SteamBot {
public:

	SteamBot(const std::string& username, const std::string& password);

private:

	Lensce::TLS::Socket socket;
	Lensce::JSON headers;
	Lensce::JSON cookies;
	std::string username;
	
	Lensce::HTTP::Response sendRequest(const std::string& path, const std::string& body);

};