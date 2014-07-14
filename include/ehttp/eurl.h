#ifndef EHTTP_URL_H
#define EHTTP_URL_H

#include <string>
#include <cstdint>

namespace ehttp
{
	/**
	 * Represents a URL.
	 */
	class eurl
	{
	public:
		/**
		 * Constructor.
		 * 
		 * Parses the given string into components.\n
		 * Invalid URLs will be assumed to consist only of a path.
		 */
		eurl(std::string string);
		virtual ~eurl();
		
		uint16_t port;				///< The port (eg. 80)
		std::string protocol;		///< The protocol (eg. http)
		std::string host;			///< The host (eg. google.com)
		std::string path;			///< The path (eg. /path/to/something)
		std::string query;			///< The query (eg. ?q=abc123&p=...)
		std::string fragment;		///< The fragment (eg. \#top)
		
		/**
		 * Formats the different components into a full URL.
		 * This is a rather cheap operation, but it still consists of some
		 * copying every time, so you may not want to call this over and over
		 * in a tight loop.
		 */
		std::string str() const;
	};
}

#endif
