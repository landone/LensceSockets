#include "LensceHTTP.h"

#include <algorithm>
#include <iostream>

namespace Lensce {
	namespace HTTP {

		std::pair<std::string, std::string> readHeader(const std::string& line) {

			size_t colonPos = line.find(':');
			if (colonPos == std::string::npos) {
				return {};
			}

			std::string header = line.substr(0, colonPos);
			header.erase(std::remove_if(header.begin(), header.end(), ::isspace), header.end());

			std::string value = line.substr(colonPos + 1);
			value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());

			return { std::move(header), std::move(value) };

		}

		Response read(const std::string& rawResponse) {

			Response result;
			size_t offset = 0;
			size_t lineEnd = 0;
			std::string searchString = "\r\n";
			while ((lineEnd = rawResponse.find(searchString, offset)) != std::string::npos) {

				std::string line = rawResponse.substr(offset, lineEnd - offset);
				offset = lineEnd + searchString.size();

				auto header = readHeader(line);
				if (header.first.empty()) {
					continue;
				}

				result.headers[header.first].push_back(std::move(header.second));

			}

			std::string lastLine = rawResponse.substr(offset);
			auto header = readHeader(lastLine);
			if (!header.first.empty()) {
				result.headers[header.first].push_back(std::move(header.second));
			}

			return result;

		}

	}
}