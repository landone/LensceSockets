#include "LensceHTTP.h"

#include <algorithm>
#include <iostream>

namespace Lensce {
	namespace HTTP {

		namespace {

			std::string restMethodToString(REST type) {
				switch (type) {
				case GET:
					return "GET";
				case POST:
					return "POST";
				case PUT:
					return "PUT";
				case DELETE:
					return "DELETE";
				case PATCH:
					return "PATCH";
				default:
					return "";
				}
			}

			std::string jsonToHeaders(JSON json) {

				std::string result;
				auto headerMap = json.getMap();
				for (const auto& pair : headerMap) {
					const std::string& key = pair.first;
					const std::string& value = pair.second.value();
					result += key + ": " + value + "\r\n";
				}
				return result;

			}

		}

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
			bool hasContent = false;
			std::string searchString = "\r\n";
			while ((lineEnd = rawResponse.find(searchString, offset)) != std::string::npos) {

				std::string line = rawResponse.substr(offset, lineEnd - offset);

				if (line.empty()) {
					hasContent = true;
					break;
				}

				offset = lineEnd + searchString.size();

				auto header = readHeader(line);
				if (header.first.empty()) {
					continue;
				}

				result.headers[header.first].push_back(header.second);

			}

			if (!hasContent) {
				std::string lastLine = rawResponse.substr(offset);
				auto header = readHeader(lastLine);
				if (!header.first.empty()) {
					result.headers[header.first].push_back(header.second);
				}
			}
			else {
				result.content = rawResponse.substr(offset);
			}

			return result;

		}

		std::string create(REST type, const std::string& domain, const std::string& path, const JSON& headers) {

			std::string result;
			std::string lineBreak = "\r\n";

			result += restMethodToString(type) + " " + path + " HTTP/1.1" + lineBreak;
			result += "Host: " + domain + lineBreak;
			result += jsonToHeaders(headers);

			return result;

		}

		std::string create(REST type, const std::string& domain, const std::string& path, const JSON& headers, const std::string& body) {

			std::string result = create(type, domain, path, headers);
			std::string lineBreak = "\r\n";

			if (!body.empty()) {
				result += "Content-Length: " + std::to_string(body.size()) + lineBreak + lineBreak + body;
			}

			return result;

		}

		std::string create(REST type, const std::string& domain, const std::string& path, const JSON& headers, const JSON& content) {

			return create(type, domain, path, headers, content.toString());

		}

		Response receive(TLS::Socket& socket) {
			std::vector<BYTE> buffer;
			std::string result;
			while (socket.receive(buffer, true)) {}
			result = std::string(buffer.begin(), buffer.end());
			return read(result);
		}

	}
}