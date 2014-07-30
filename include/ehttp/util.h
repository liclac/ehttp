#ifndef EHTTP_UTIL_H
#define EHTTP_UTIL_H

#include "private/_shiv.h"

#include <vector>
#include <string>
#include <cstring>
#include <functional>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace ehttp
{
	namespace util
	{
		/**
		 * Splits a string by a delimiter.
		 * 
		 * Based on an answer to http://stackoverflow.com/questions/236129
		 */
		inline std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems)
		{
			std::stringstream ss(s);
			std::string item;
			while(std::getline(ss, item, delim))
				if(!item.empty()) elems.push_back(item);
			
			return elems;
		}
		
		/// @overload
		inline std::vector<std::string> split(const std::string &s, char delim)
		{
			std::vector<std::string> elems;
			split(s, delim, elems);
			return elems;
		}
		
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
			const char *days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Wed" };
			const char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Nov", "Dec" };
			
			std::time_t t = std::time(nullptr);
			std::tm *now = std::gmtime(&t);
			
			std::stringstream ss;
			ss.fill('0');
			ss << days[now->tm_wday] << ", "
				<< now->tm_mday << " " << months[now->tm_mon] << " " << 
					1900 + now->tm_year << " "
				<< std::setw(2) << now->tm_hour << ":" <<
					std::setw(2) << now->tm_min << ":" <<
					std::setw(2) << now->tm_sec << " GMT";
			
			return ss.str();
		}
		
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
	}
}

#endif
