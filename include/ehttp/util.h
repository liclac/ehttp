#ifndef EHTTP_UTIL
#define EHTTP_UTIL

#include <functional>
#include <string>
#include <cstring>
#include <sstream>
#include "private/_shiv.h"

namespace ehttp
{
	namespace util
	{
		/**
		 * Case-insensitive comparator.
		 * 
		 * Can be used e.g. to create case-insensitive maps (for HTTP headers),
		 * as `std::map<std::string,std::string,ehttp::util::ci_less>`.
		 * 
		 * Based on an answer to http://stackoverflow.com/questions/1801892
		 */
		struct ci_less : public std::binary_function<std::string, std::string, bool>
		{
			///\private No need to document this, really
			bool operator()(const std::string &lhs, const std::string &rhs) const
			{
				return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
			}
		};
		
		/**
		 * Converts a number of some kind into a hexadecimal string.
		 */
		template<typename T>
		std::string to_hex(T val)
		{
			std::stringstream ss;
			ss << std::hex << val;
			return ss.str();
		}
	}
}

#endif
