#pragma once

#include <unordered_map>
#include <string>
#include <list>

#include "JSON.h"
#include "LensceTLS.h"

namespace Lensce {
	namespace HTTP {

		enum REST {
			GET,
			POST,
			PUT,
			DELETE,
			PATCH,
			MAX_REST
		};

		class Response {
		public:
			std::unordered_map<std::string, std::list<std::string>> headers;
			std::string content;
		};

		Response read(const std::string& rawResponse);

		Response receive(TLS::Socket& socket);

		std::string create(REST type, const std::string& domain, const std::string& path, const JSON& headers, const JSON& content);
		std::string create(REST type, const std::string& domain, const std::string& path, const JSON& headers, const std::string& body);
		std::string create(REST type, const std::string& domain, const std::string& path, const JSON& headers);

	}
}