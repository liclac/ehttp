#include <sstream>
#include <bitset>
#include <ehttp/url.h>
#include "../vendor/http-parser/http_parser.h"

using namespace ehttp;

#define parser_data_extract(_idx, _var) \
	if(set_fields[_idx]) \
		_var = std::string(cstr + data.field_data[_idx].off, data.field_data[_idx].len)

url::url(std::string string):
	port(0)
{
	const char *cstr = string.c_str();
	http_parser_url data = {0};
	if(http_parser_parse_url(cstr, strlen(cstr), 0, &data) == 0)
	{
		port = data.port;
		
		std::bitset<16> set_fields(data.field_set);
		parser_data_extract(UF_SCHEMA, protocol);
		parser_data_extract(UF_HOST, host);
		parser_data_extract(UF_PATH, path);
		parser_data_extract(UF_QUERY, query);
		parser_data_extract(UF_FRAGMENT, fragment);
	}
}

url::~url()
{
	
}

std::string url::str() const
{
	std::stringstream ss;
	
	if(!protocol.empty()) ss << protocol << "://";
	else if(!host.empty()) ss << "http://";
	if(!host.empty()) ss << host;
	if(!path.empty()) ss << path;
	if(!path.empty()) ss << "?" << query;
	if(!path.empty()) ss << "#" << fragment;
	
	return ss.str();
}
