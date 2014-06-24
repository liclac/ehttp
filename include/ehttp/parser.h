#ifndef EHTTP_PARSER_H
#define EHTTP_PARSER_H

#include <functional>
#include <memory>

namespace ehttp
{
	class request;
	class parser
	{
	public:
		parser();
		virtual ~parser();
		
		enum status {
			error = -1,
			keep_going = 0,
			got_request = 1
		};
		
		parser::status parse_chunk(void *data, std::size_t length);
		std::shared_ptr<request> request();
		
	protected:
		struct impl;
		impl *p;
	};
}

#endif
