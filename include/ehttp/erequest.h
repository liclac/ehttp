#ifndef EHTTP_REQUEST_H
#define EHTTP_REQUEST_H

#include <string>
#include <map>
#include <vector>
#include "util.h"

namespace ehttp
{
	/**
	 * Represents an HTTP Request.
	 * 
	 * Instances of this class are typically created by ehttp::parser, but you
	 * can also create them manually if you have a different means of parsing
	 * a request, or for testing purposes.
	 * 
	 * It's a dumb container, change values at will.
	 */
	class erequest
	{
	public:
		/// Case-insensitive string-to-string map
		typedef std::map<std::string,std::string,util::ci_less> header_map;
		
		/// Constructor
		erequest(std::string method = "", std::string url = "", header_map headers = header_map{}, std::vector<char> body = {}, bool upgrade = false);
		virtual ~erequest();
		
		/// Request Method (also known as Verb); ex: GET, POST, DELETE, ...
		std::string method;
		/// Unparsed Request URL (tip: use the \ref eurl class)
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
