#ifndef EHTTP_URL_H
#define EHTTP_URL_H

#include <string>

namespace ehttp
{
	class url
	{
	public:
		url(std::string string);
		virtual ~url();
		
		unsigned short port;
		std::string protocol, host, path, query, fragment;
		
		std::string str() const;
	};
}

#endif
