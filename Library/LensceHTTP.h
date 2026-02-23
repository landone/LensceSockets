#pragma once

#include <unordered_map>
#include <string>

namespace Lensce {
	namespace HTTP {

		class Response {
		public:
			std::unordered_map<std::string, std::list<std::string>> headers;
		};

		Response read(const std::string& rawResponse);

	}
}