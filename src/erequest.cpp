#include <ehttp/erequest.h>

using namespace ehttp;

erequest::erequest(std::string method, std::string url, header_map headers, std::vector<char> body, bool upgrade):
	method(method), url(url), headers(headers), body(body), upgrade(upgrade)
{
	
}

erequest::~erequest()
{
	
}
