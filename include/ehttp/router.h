#ifndef EHTTP_ROUTER_H
#define EHTTP_ROUTER_H

#include <cstdint>
#include <memory>

namespace ehttp
{
	class request;
	class response;
	
	/**
	 * Path-based Router for HTTP Requests and handlers.
	 * 
	 * @todo Make an "abstract router" kind of class, that doesn't have an
	 * opinion on how things are routed, then make path routers able to mount
	 * other routers where it makes sense.\n
	 * That'll let higher-level code more easily implement other routing styles
	 * without having to squeeze it into a global dictionary of paths (think
	 * MVC routing), and let you mount other routing styles onto each other
	 * where it makes sense.
	 * 
	 * @todo Placeholders in routes, probably Flask-style <key>/<path:key>
	 * expressions. C++11 offers regular expressions, use that.
	 */
	class router
	{
	public:
		/// Constructor
		router();
		virtual ~router();
		
		/// Signature for a handler function
		typedef std::function<bool(std::shared_ptr<request> req, std::shared_ptr<response> res)> handler_func;
		
		/**
		 * Registers a handler function for an endpoint.
		 * 
		 * Only one handler can be registered for any one endpoint (defined as
		 * a unique pair of a method and route), and a handler can be
		 * unregistered if you so desire by registering `std::nullptr` (or `0`)
		 * for an endpoint.
		 * 
		 * @param method The method to respond to (eg. GET, POST, ...)
		 * @param route The path to handle (eg. /, /about/, ...)
		 * @param handler The handler function to call
		 */
		virtual void on(std::string method, std::string route, handler_func handler);
		
		/**
		 * Registers a handler for a status code, for displaying error pages.
		 * 
		 * These handlers are called when response::end() is called, but only if:
		 * 
		 * * There is a handler for the status code of the ending response
		 * * The response has no body
		 * * The response is not chunked
		 * 
		 * In other words, this gives you a way to provide default contents for
		 * empty responses based on their status. The intention is that you
		 * should be able to display an error page by returning an empty
		 * response with a status set, and setting a handler for that status.
		 * 
		 * Note that the response handed to a status handler will be the same
		 * as the one returned from the route handler; this means that calling
		 * `res->begin(0)` will reuse the status code and reason.\n
		 * Doing this is good practice, as it means you won't accidentally
		 * return the wrong status code or clobber a custom reason phrase.
		 * 
		 * As with on(), only one handler can be registered for a status, and
		 * registering `std::nullptr` or `0` for a status will remove a
		 * registered custom handler.
		 */
		virtual void on_error(uint16_t code, handler_func handler);
		
		/**
		 * Attempts to route a request.
		 * 
		 * It will attempt to find a handler matching the request's endpoint,
		 * or fire an empty response using #fallback_code to trigger a status
		 * handler if none is found.
		 * 
		 * The response is prepared so that it will call a status handler when
		 * appropriate (see on_error()), and #on_response_end and
		 * #on_response_chunk otherwise.
		 */
		virtual bool route(std::shared_ptr<request> req);
		
		/// Fallback code if no handler is found (defaults to 404)
		uint16_t fallback_code;
		
		/// Callback for response::end()
		std::function<void(std::shared_ptr<response> res)> on_response_end;
		/// Callback for response::chunk::end()
		std::function<void(std::shared_ptr<response> res, std::shared_ptr<chunk> chunk)> on_response_chunk;
		
	private:
		struct impl;
		impl *p;
	};
}

#endif
