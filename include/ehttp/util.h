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
		inline std::string to_hex(T val)
		{
			std::stringstream ss;
			ss << std::hex << val;
			return ss.str();
		}
		
		/**
		 * Returns the current time as an HTTP-formatted date string.
		 * 
		 * The result will be formatted like "Fri, 31 Dec 1999 23:59:59 GMT".
		 */
		inline std::string http_date()
		{
			// Temporarily force an english locale, otherwise the textual parts
			// of the result would be translated to the current system language
			auto old_locale = std::locale::global(std::locale("en_US.UTF-8"));
			
			char timestamp[128] = {0};
			std::time_t t = std::time(nullptr);
			if(!std::strftime(timestamp, sizeof(timestamp), "%a, %d %b %Y %H:%M:%S", std::gmtime(&t)))
				strcpy(timestamp, "00:00:00");
			
			std::locale::global(old_locale);
			
			return std::string(timestamp);
		}
	}
}

#endif
