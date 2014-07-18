#ifndef EHTTP_HTTPREQUEST_H
#define EHTTP_HTTPREQUEST_H

#include <string>
#include <map>
#include <vector>
#include "util.h"

namespace ehttp
{
	/**
	 * Represents an HTTP Request.
	 * 
	 * Instances of this class are typically created by HTTPRequestParser, but
	 * you can also create them manually if you have a different means of
	 * parsing a request, or for testing purposes.
	 * 
	 * It's a dumb container, change values at will.
	 */
	class HTTPRequest
	{
	public:
		/// Case-insensitive string-to-string map
		typedef std::map<std::string,std::string,util::ci_less> header_map;
		
		/// Constructor
		HTTPRequest(std::string method = "", std::string url = "", header_map headers = header_map{}, std::vector<char> body = {}, bool upgrade = false);
		virtual ~HTTPRequest();
		
		/// Request Method (also known as Verb); ex: GET, POST, DELETE, ...
		std::string method;
		/// Unparsed Request URL (tip: use the \ref URL class)
		std::string url;
		
		/// Request headers (case insensitive keys)
		header_map headers;
		/// Request body
		std::vector<char> body;
		
		/// Is the request an Upgrade request (eg. for WebSockets)?
		bool upgrade;
	};
}

#endif
