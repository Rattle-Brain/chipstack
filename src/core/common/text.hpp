#pragma once

#include <string>
#include <vector>

inline std::string join(
		const std::vector<std::string>& items, const std::string& separator
) {
	std::string joined;
	for (const std::string& item : items) {
		if (!joined.empty()) joined += separator;
		joined += item;
	}
	return joined;
}
