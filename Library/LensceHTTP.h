#pragma once

#include <unordered_map>
#include <string>
#include <list>

#include "JSON.h"

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

		std::string create(REST type, const std::string& domain, const std::string& path, JSON headers, JSON content = JSON());

	}
}