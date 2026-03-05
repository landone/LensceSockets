#include "JSON.h"

#include <string>
#include <stack>

namespace Lensce {

	std::string JSON::toString(bool isFormatted) const {

		return toStringHelper(isFormatted, 0);

	}

	std::string JSON::toStringHelper(bool isFormatted, int indentLevel) const {

		if (children.empty()) {
			return "";
		}

		std::string lineBreak = isFormatted ? "\n" : "";
		std::string padding = isFormatted ? std::string((indentLevel + 1) * 4, ' ') : "";
		std::string lesserPadding = isFormatted ? std::string((indentLevel) * 4, ' ') : "";
		std::string result = "{" + lineBreak;

		for (const auto& pair : children) {
			const std::string& key = pair.first;
			const JSON& child = pair.second;

			result += padding + "\"" + key + "\":" + (isFormatted ? " " : "");

			if (child.children.empty()) {
				result += "\"" + child.value() + "\"";
			}
			else {
				result += child.toStringHelper(isFormatted, indentLevel + 1);
			}

			result += "," + lineBreak;

		}
		if (!children.empty()) {
			result.pop_back();
			if (isFormatted) {
				result.pop_back();
				result += lineBreak;
			}
		}
		result += lesserPadding + "}";
		return result;

	}

	JSON::JSON(const std::string& rawJSON) {
		
		std::stack<JSON*> jsonStack;
		
		auto strPos = rawJSON.find('{');
		if (strPos == std::string::npos) {
			return;
		}

		jsonStack.push(this);
		size_t pos = strPos + 1;
		std::string currentKey;
		size_t quoteStart = -1;
		while (!jsonStack.empty() && strPos < rawJSON.size()) {
			switch (rawJSON[pos]) {
				case '{': {
					if (quoteStart != -1) {
						break;
					}
					if (currentKey.empty()) {
						clear();
						return;
					}
					JSON& top = *jsonStack.top();
					jsonStack.push(&top[currentKey]);
					currentKey.clear();
					break;
				}
				case '}': {
					if (quoteStart != -1) {
						break;
					}
					if (!currentKey.empty()) {
						currentKey.clear();
					}
					jsonStack.pop();
					break;
				}
				case ',': {
					if (quoteStart != -1) {
						break;
					}
					if (!currentKey.empty()) {
						size_t start = rawJSON.find_last_of(':', pos) + 1;
						JSON& top = *jsonStack.top();
						top[currentKey] = rawJSON.substr(start, pos - start);
						currentKey.clear();
					}
					break;
				}
				case '\"': {
					//Allow escaped quotes
					if (rawJSON[pos - 1] == '\\') {
						break;
					}
					if (quoteStart == -1) {
						quoteStart = pos + 1;
					}
					else {
						std::string str = rawJSON.substr(quoteStart, pos - quoteStart);
						size_t escapedQuote = 0;
						while ((escapedQuote = str.find("\"", escapedQuote)) != std::string::npos) {
							str.erase(escapedQuote - 1, 1);
						}
						if (currentKey.empty()) {
							currentKey = str;
						}
						else {
							JSON& top = *jsonStack.top();
							top[currentKey] = str;
							currentKey.clear();
						}
						quoteStart = -1;
					}
					break;
				}
			}
			pos++;
		}

		// If the stack isn't empty, it means we had mismatched braces, so we clear the JSON object to indicate an error.
		if (!jsonStack.empty()) {
			clear();
		}

	}

	void JSON::clear() {
		children.clear();
		value_.clear();
	}

}