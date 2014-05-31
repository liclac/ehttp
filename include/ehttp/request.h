#ifndef EHTTP_REQUEST_H
#define EHTTP_REQUEST_H

#include <string>
#include <map>
#include <vector>
#include "util.h"

namespace ehttp
{
	class request
	{
	public:
		request();
		virtual ~request();
		
		std::string method;
		std::string url;
		
		std::map<std::string,std::string,ci_less> headers;
		std::vector<char> body;
		
		bool upgrade;
	};
}

#endif
