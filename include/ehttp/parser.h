#ifndef EHTTP_PARSER_H
#define EHTTP_PARSER_H

#include <functional>

namespace ehttp
{
	class request;
	class parser
	{
	public:
		parser();
		virtual ~parser();
		
		void parse_chunk(void *data, std::size_t length);
		
		std::function<void(ehttp::request *req)> on_request;
		std::function<void()> on_error;
		
	protected:
		struct impl;
		impl *p;
	};
}

#endif
