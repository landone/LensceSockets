#pragma once

#include <string>

#include <LensceTLS.h>
#include <LensceHTTP.h>
#include <JSON.h>

class SteamBot {
public:

	SteamBot(const std::string& username, const std::string& password);

private:

	Lensce::TLS::Socket socket;
	Lensce::JSON headers;
	std::string username;
	
	Lensce::HTTP::Response sendRequest(const std::string& path, const std::string& body);

};