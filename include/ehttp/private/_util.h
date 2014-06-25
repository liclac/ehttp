#ifndef EHTTP_UTIL
#define EHTTP_UTIL

#include <functional>
#include <string>
#include <cstring>
#include "_shiv.h"

namespace ehttp
{
	// Based on an answer to http://stackoverflow.com/questions/1801892
	/// \private
	struct ci_less : public std::binary_function<std::string, std::string, bool> {
		bool operator()(const std::string &lhs, const std::string &rhs) const {
			return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
		}
	};
}

#endif
