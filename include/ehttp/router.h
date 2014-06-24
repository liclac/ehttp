#ifndef EHTTP_ROUTER_H
#define EHTTP_ROUTER_H

#include <cstdint>
#include <memory>

namespace ehttp
{
	class request;
	class response;
	class router
	{
	public:
		router();
		virtual ~router();
		
		typedef std::function<uint16_t(std::shared_ptr<request> req, std::shared_ptr<response> res)> handler_func;
		
		virtual void on(std::string route, handler_func handler);
		virtual void on(std::string method, std::string route, handler_func handler);
		virtual void on_error(uint16_t code, handler_func handler);
		
		virtual void route(std::shared_ptr<request> req, std::shared_ptr<response> res);
		
	protected:
		struct impl;
		impl *p;
	};
}

#endif
