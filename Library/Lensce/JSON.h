#pragma once

#include <unordered_map>
#include <string>

namespace Lensce {
	class JSON {
	public:

		JSON() {}

		JSON(const std::string& rawJSON);

		JSON& operator[](const std::string& key) {
			return children[key];
		}

		std::string operator=(const std::string& newValue) {
			value_ = newValue;
			return value_;
		}

		const std::string& value() const {
			return value_;
		}

		std::string toString(bool isFormatted=false) const;

		const auto& getMap() const { return children; }

	private:

		std::unordered_map<std::string, JSON> children;
		std::string value_;

		std::string toStringHelper(bool isFormatted, int indentLevel) const;
		void clear();
	};
}