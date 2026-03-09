#include "HTTP.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>

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

			std::string jsonToCookies(JSON json) {
				std::string result;
				auto cookieMap = json.getMap();
				for (const auto& pair : cookieMap) {
					const std::string& key = pair.first;
					const std::string& value = pair.second.value();
					result += "Cookie: " + key + "=" + value + "\r\n";
				}
				return result;
			}

			std::vector<std::pair<std::string, std::string>> parseCookie(const std::string& cookie) {

				std::vector<std::pair<std::string, std::string>> result;
				size_t pos = 0;
				size_t cookieEnd = 0;
				while (true) {
					cookieEnd = cookie.find(';', pos);
					if (cookieEnd == std::string::npos) {
						cookieEnd = cookie.size();
					}
					std::string cookiePart = cookie.substr(pos, cookieEnd - pos);
					size_t equalsPos = cookiePart.find('=');
					if (equalsPos != std::string::npos) {
						std::string name = cookiePart.substr(0, equalsPos);
						std::string value = cookiePart.substr(equalsPos + 1);
						result.emplace_back(std::move(name), std::move(value));
					}
					pos = cookieEnd + 1;
					if (cookieEnd >= cookie.size() - 1) {
						break;
					}
				}
				return result;

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

			void handleHeader(const std::string& headerLine, Response& response) {

				auto header = readHeader(headerLine);
				if (header.first.empty()) {
					return;
				}

				if (header.first == "Set-Cookie" || header.first == "Cookie") {
					auto cookies = parseCookie(header.second);
					for (auto& cookie : cookies) {
						response.cookies[cookie.first] = cookie.second;
					}
				}
				else {
					response.headers[header.first] = header.second;
				}

			}

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
				handleHeader(line, result);

			}

			if (!hasContent) {
				std::string lastLine = rawResponse.substr(offset);
				handleHeader(lastLine, result);
			}
			else {
				result.content = rawResponse.substr(offset);
			}

			return result;

		}

		std::string create(REST type, const std::string& domain, const std::string& path, const JSON& headers, const JSON& cookies) {

			std::string result;
			std::string lineBreak = "\r\n";

			result += restMethodToString(type) + " " + path + " HTTP/1.1" + lineBreak;
			result += "Host: " + domain + lineBreak;
			result += jsonToHeaders(headers);
			result += jsonToCookies(cookies);

			return result;

		}

		std::string create(REST type, const std::string& domain, const std::string& path, const JSON& headers, const JSON& cookies, const std::string& body) {

			std::string result = create(type, domain, path, headers, cookies);
			std::string lineBreak = "\r\n";

			if (!body.empty()) {
				result += "Content-Length: " + std::to_string(body.size()) + lineBreak + lineBreak + body;
			}

			return result;

		}

		std::string create(REST type, const std::string& domain, const std::string& path, const JSON& headers, const JSON& cookies, const JSON& content) {

			return create(type, domain, path, headers, cookies, content.toString());

		}

		Response receive(TLS::Socket& socket) {
			std::vector<BYTE> buffer;
			std::string result;
			while (socket.receive(buffer, true)) {}
			result = std::string(buffer.begin(), buffer.end());
			return read(result);
		}

		std::string urlEncode(const std::string& value) {
			std::ostringstream escaped;
			escaped.fill('0');
			escaped << std::hex;

			for (auto i = value.begin(); i != value.end(); ++i) {
				std::string::value_type c = (*i);

				// Keep alphanumeric and other safe characters
				if (std::isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
					escaped << c;
					continue;
				}

				// Any other characters are percent-encoded
				escaped << std::uppercase << '%' << std::setw(2) << int((unsigned char)c);
			}

			return escaped.str();
		}

		std::string Response::toString() const {

			std::string result;
			for (const auto& header : headers.getMap()) {
					result += header.first + ": " + header.second.value() + "\r\n";
			}
			for (const auto& cookie : cookies.getMap()) {
				result += "Set-Cookie: " + cookie.first + "=" + cookie.second.value() + "\r\n";
			}
			if (!content.empty()) {
				result += "\r\n";
				std::string asJSON = JSON(content).toString(true);
				if (asJSON.empty()) {
					result += content;
				}
				else {
					result += asJSON;
				}
			}
			return result;

		}

	}
}