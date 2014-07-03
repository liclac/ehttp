#include <map>
#include <regex>
#include <ehttp/router.h>
#include <ehttp/url.h>
#include <iostream>

using namespace ehttp;

/// \private
struct router::impl
{
	// Route handlers are indexed as <method, <route, handler> >
	std::map<std::string,std::map<std::string,handler_func>> handlers;
	std::map<uint16_t,handler_func> status_handlers;
};

router::router():
	fallback_code(404),
	p(new impl)
{
	
}

router::~router()
{
	
}

void router::on(std::string method, std::string route, handler_func handler)
{
	p->handlers[method][route] = handler;
}

void router::on_error(uint16_t code, handler_func handler)
{
	p->status_handlers[code] = handler;
}

void router::route(std::shared_ptr<request> req, std::shared_ptr<response> res)
{
	// Default to the fallback code; a found, valid handler will overwrite this
	res->code = fallback_code;
	
	/* 
	 * Extract only the path component; note that the HTTP specs say requests
	 * may contain anything from only the path to a full URL.
	 */
	std::string path = url(req->url).path;
	
	/* 
	 * Routes are indexed as "handlers[method][route] = handler".
	 * We can't retreive them with [], that would create an empty std::function
	 * for every request we receive that doesn't have a handler.
	 */
	auto method_it = p->handlers.find(req->method);
	if(method_it != p->handlers.end())
	{
		auto route_it = method_it->second.find(path);
		if(route_it != method_it->second.end() && route_it->second)
			route_it->second(req, res);
	}
	
	/* 
	 * Check for a status callback, but only for empty, non-chunked responses.
	 * This will let us fire standard status pages by firing an empty response
	 * with the desired code to let a status handler react.
	 */
	if(res->body.size() == 0 && !res->is_chunked())
	{
		auto status_handler_it = p->status_handlers.find(res->code);
		if(status_handler_it != p->status_handlers.end() && status_handler_it->second)
			status_handler_it->second(req, res);
	}
}
