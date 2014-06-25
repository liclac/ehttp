#ifndef EHTTP_PARSER_H
#define EHTTP_PARSER_H

#include <functional>
#include <memory>

namespace ehttp
{
	class request;
	
	/**
	 * Parser for HTTP Requests.
	 * 
	 * You should keep an instance of this class per connection, and feed it
	 * any data you receive.
	 */
	class parser
	{
	public:
		parser();
		virtual ~parser();
		
		/// The return value of parse_chunk().
		enum status {
			/// There was an error parsing the data; it's most likely invalid
			error = -1,
			/// No problem, but we haven't got all the data yet, keep feeding
			keep_going = 0,
			/// Success! We got a request (retreive with request())
			got_request = 1
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
		parser::status parse_chunk(void *data, std::size_t length);
		
		/**
		 * Retreive the parsed request.
		 * 
		 * This will be 0 at all times except between a parse_chunk() that
		 * returned #got_request, and the next call to parse_chunk(). Thus,
		 * when a request is returned, you should store it elsewhere, since it
		 * will be cleared the next time data is received from the client.
		 */
		std::shared_ptr<request> request();
		
	private:
		struct impl;
		impl *p;
	};
}

#endif
