#pragma once

#include <unordered_map>
#include <string>
#include <list>

namespace Lensce {
	namespace HTTP {

		class Response {
		public:
			std::unordered_map<std::string, std::list<std::string>> headers;
			std::string content;
		};

		Response read(const std::string& rawResponse);

	}
}