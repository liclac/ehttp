#ifndef EHTTP_HTTPROUTER_H
#define EHTTP_HTTPROUTER_H

#include "HTTPRequest.h"
#include "HTTPResponse.h"

#include <cstdint>
#include <memory>

namespace ehttp
{
	/**
	 * Path-based Router for HTTP Requests and handlers.
	 * 
	 * Note that the response callbacks are not called directly; the router
	 * installs its own callbacks in responses that then call yours if
	 * appropriate.
	 * 
	 * @todo Rails-style placeholders in routes.
	 * 
	 * @todo A `mountpoint` class that can be mounted onto arbitrary points in
	 * the tree, handling any request to antyhing under it.
	 */
	class HTTPRouter
	{
	public:
		/// Constructor
		HTTPRouter();
		virtual ~HTTPRouter();
		
		/// Signature for a handler function
		typedef std::function<void(std::shared_ptr<HTTPRequest> req, std::shared_ptr<HTTPResponse> res)> handler_func;
		
		/**
		 * Registers a handler function for an endpoint.
		 * 
		 * Only one handler can be registered for any one endpoint (defined as
		 * a unique combination of a method and path), and a handler can be
		 * unregistered if you so desire by registering `std::nullptr` (or `0`)
		 * for an endpoint.
		 * 
		 * @param method The method to respond to (eg. GET, POST, ...)
		 * @param path The path to handle (eg. /, /about/, ...)
		 * @param handler The handler function to call
		 */
		virtual void on(std::string method, std::string path, handler_func handler);
		
		/**
		 * Registers a handler for a status code, for displaying error pages.
		 * 
		 * If an empty, non-chunked response is returned from a handler called
		 * by route(), and a status handler is registered for the returned
		 * status code (or #fallback_code if no handler that calls
		 * HTTPResponse::begin() is found), that status handler will be called
		 * with the same request and response as was passed to the original
		 * handler.
		 * 
		 * This is intended to be used to let you fire standard error pages
		 * with something along the lines of:
		 * 
		 *     res->begin(403)
		 *         ->end()
		 */
		virtual void onError(uint16_t code, handler_func handler);
		
		/**
		 * Attempts to route a request.
		 * 
		 * It will attempt to find a handler matching the request's endpoint,
		 * or fire an empty response using #fallback_code to trigger a status
		 * handler if none is found.
		 */
		virtual void route(std::shared_ptr<HTTPRequest> req, std::shared_ptr<HTTPResponse> res);
		
		
		
		/// Fallback code if no handler is found (defaults to 404)
		uint16_t fallback_code;
		
	protected:
		/**
		 * Wrap a response's handlers.
		 * 
		 * This registers handlers that call status handlers where appropriate,
		 * then the original handlers.
		 * 
		 * Note that the wrapper handlers will in some cases set the response's
		 * handlers several times from within themselves for performance, and
		 * once a response has been wrapped, you should never rely on the value
		 * of the HTTPResponse::onData and HTTPResponse::onEnd variables.
		 * 
		 * @see onError
		 */
		void wrapResponseHandlers(std::shared_ptr<HTTPRequest> req, std::shared_ptr<HTTPResponse> res);
		
	private:
		struct impl;
		impl *p;
	};
}

#endif
