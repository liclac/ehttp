#include <ehttp/request.h>

using namespace ehttp;

request::request(std::string method, std::string url, header_map headers, std::vector<char> body, bool upgrade):
	method(method), url(url), headers(headers), body(body), upgrade(upgrade)
{
	
}

request::~request()
{
	
}
