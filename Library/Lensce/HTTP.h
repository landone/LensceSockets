#pragma once

#include <unordered_map>
#include <string>
#include <list>

#include "JSON.h"
#include "TLS.h"

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
			JSON headers;
			JSON cookies;
			std::string content;
			std::string toString() const;
		};

		Response read(const std::string& rawResponse);

		Response receive(TLS::Socket& socket);

		std::string create(REST type, const std::string& domain, const std::string& path, const JSON& headers, const JSON& cookies, const JSON& content);
		std::string create(REST type, const std::string& domain, const std::string& path, const JSON& headers, const JSON& cookies, const std::string& body);
		std::string create(REST type, const std::string& domain, const std::string& path, const JSON& headers, const JSON& cookies);

		std::string urlEncode(const std::string& value);

	}
}