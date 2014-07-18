#ifndef EHTTP_HTTPREQUESTPARSER_H
#define EHTTP_HTTPREQUESTPARSER_H

#include <functional>
#include <memory>
#include "HTTPRequest.h"

namespace ehttp
{
	/**
	 * Parser for HTTP Requests.
	 * 
	 * You should keep an instance of this class per connection, and feed it
	 * any data you receive.
	 */
	class HTTPRequestParser
	{
	public:
		HTTPRequestParser();
		virtual ~HTTPRequestParser();
		
		/// The return value of parseChunk().
		enum Status {
			/// There was an error parsing the data; it's most likely invalid
			Error = -1,
			/// No problem, but we haven't got all the data yet, keep feeding
			KeepGoing = 0,
			/// Success! We got a request (retreive with req())
			GotRequest = 1
		};
		
		/**
		 * Parse a received chunk.
		 * 
		 * You should call this for every piece of data you receive, and check
		 * the return value to find out when you've got data for a whole
		 * request.
		 * 
		 * @see request()
		 */
		HTTPRequestParser::Status parseChunk(const char *data, std::size_t length);
		
		/**
		 * Retreive the parsed request.
		 * 
		 * This will be 0 at all times except between a parseChunk() that
		 * returned #GotRequest, and the next call to parseChunk(). Thus,
		 * when a request is returned, you should store it elsewhere, since it
		 * will be cleared the next time data is received from the client.
		 */
		std::shared_ptr<HTTPRequest> req();
		
	private:
		struct impl;
		impl *p;
	};	
}

#endif
