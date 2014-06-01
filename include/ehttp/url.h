#ifndef EHTTP_URL_H
#define EHTTP_URL_H

#include <string>
#include <cstdint>

namespace ehttp
{
	class url
	{
	public:
		url(std::string string);
		virtual ~url();
		
		uint16_t port;
		std::string protocol, host, path, query, fragment;
		
		std::string str() const;
	};
}

#endif
