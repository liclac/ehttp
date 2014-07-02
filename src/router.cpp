#include <map>
#include <regex>
#include <ehttp/router.h>
#include <iostream>

using namespace ehttp;

/// \private
struct router::impl
{
	// Indexed as <method, <route, handler> >
	std::map<std::string,std::map<std::string,handler_func>> handlers;
	std::map<uint16_t,handler_func> status_handlers;
};

router::router():
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
	std::cout << "Route a request to " << req->url << std::endl;
}
