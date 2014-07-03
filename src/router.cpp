#include <map>
#include <regex>
#include <ehttp/router.h>
#include <ehttp/url.h>
#include <iostream>

using namespace ehttp;

/// \private
struct router::impl
{
	// Route handlers are indexed as <method, <path, handler> >
	std::map<std::string,std::map<std::string,handler_func>> handlers;
	std::map<uint16_t,handler_func> status_handlers;
	
	// Map between responses and their data buffers
	std::map<std::shared_ptr<response>,std::vector<char>> buffers;
};

router::router():
	fallback_code(404),
	p(new impl)
{
	
}

router::~router()
{
	
}

void router::on(std::string method, std::string path, handler_func handler)
{
	p->handlers[method][path] = handler;
}

void router::on_error(uint16_t code, handler_func handler)
{
	p->status_handlers[code] = handler;
}

void router::route(std::shared_ptr<request> req, std::shared_ptr<response> res)
{
	// Throw exceptions for responses without required handlers
	if(!res->on_data)
		throw std::runtime_error("router::route() response lacks on_data");
	
	// Set callbacks that call status handlers where appropriate
	auto old_on_data = res->on_data;
	auto old_on_end = res->on_end;
	wrap_response_handlers(req, res);
	
	// Extract only the path component; note that the HTTP specs say requests
	// may contain anything from only the path to a full URL.
	std::string path = url(req->url).path;
	
	/*
	 * Routes are indexed as "handlers[method][path] = handler".
	 * We can't retreive them with [], that would create an empty std::function
	 * for every request we receive that doesn't have a handler.
	 */
	bool handler_found = false;
	auto method_it = p->handlers.find(req->method);
	if(method_it != p->handlers.end())
	{
		auto route_it = method_it->second.find(path);
		if(route_it != method_it->second.end() && route_it->second)
		{
			handler_found = true;
			route_it->second(req, res);
		}
	}
	
	// If no handler is found, attempt to use the handler for #fallback_code
	if(!handler_found)
	{
		res->code = this->fallback_code;
		
		auto status_handler_it = p->status_handlers.find(res->code);
		if(status_handler_it != p->status_handlers.end() && status_handler_it->second)
		{
			// Don't use our newly installed on_data and on_end, as that could
			// create an infinite loop of handler calls in some circumstances.
			res->on_data = old_on_data;
			res->on_end = old_on_end;
			
			status_handler_it->second(req, res);
		}
	}
}

void router::wrap_response_handlers(std::shared_ptr<request> req, std::shared_ptr<response> res)
{
	auto old_on_data = res->on_data;
	auto old_on_end = res->on_end;
	
	/*
	 * The first time data is received from a response, check it and swap
	 * its on_data and on_end handlers out for a more appropriate one.
	 */
	res->on_data = [=](std::shared_ptr<response> res, std::vector<char> data)
	{
		/*
		 * If the response is not chunked, and there's a handler for its code,
		 * buffer the response because we may want to fire a status handler
		 * instead of actually sending the data it presents.
		 */
		if(!res->is_chunked() && p->status_handlers.find(res->code) != p->status_handlers.end())
		{
			// on_data should buffer data rather than send it to the client
			res->on_data = [=](std::shared_ptr<response> res, std::vector<char> data) {
				// std::map::operator[] will create this if it doesn't exist
				std::vector<char> &buffer = p->buffers[res];
				buffer.insert(buffer.end(), data.begin(), data.end());
			};
			
			/*
			 * on_end should again evaluate if a handler should be fired, fire
			 * it if appropriate, otherwise just send the client the buffer.
			 */
			res->on_end = [=](std::shared_ptr<response> res) {
				bool status_handler_fired = false;
				
				// Only fire handlers for empty, non-chunked responses
				if(!res->is_chunked() && res->body.size() == 0)
				{
					auto status_handler_it = p->status_handlers.find(res->code);
					if(status_handler_it != p->status_handlers.end() && status_handler_it->second)
					{
						// Use the original on_data and on_end to prevent loops
						res->on_data = old_on_data;
						res->on_end = old_on_end;
						status_handler_fired = true;
						
						status_handler_it->second(req, res);
					}
				}
				
				// If a handler hasn't been fired, fire the original handlers
				if(!status_handler_fired)
				{
					old_on_data(res, p->buffers[res]);
					if(old_on_end) old_on_end(res);
				}
				
				// Remember to delete the buffer when we're done with it!
				p->buffers.erase(res);
			};
		}
		// Otherwise, substitute this with the original handlers
		else
		{
			res->on_data = old_on_data;
			res->on_end = old_on_end;
		}
		
		// Either way, call whatever function is now in on_data
		res->on_data(res, data);
	};
}
