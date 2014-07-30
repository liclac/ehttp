#include <ehttp/HTTPRouter.h>
#include <ehttp/URL.h>
#include <ehttp/util.h>

#include <map>
#include <set>
#include <iostream>

using namespace ehttp;

/// \private
struct HTTPRouter::impl
{
	/// \private Tree structure for routes
	struct route_node
	{
		handler_func handler;
		std::map<std::string,route_node> children;
	};
	
	// Root nodes; mapped by method
	std::map<std::string, route_node> methods;
	
	// Handlers
	std::map<uint16_t,handler_func> status_handlers;
	
	// Map between responses and their data buffers
	std::map<std::shared_ptr<HTTPResponse>,std::vector<char>> buffers;
};

HTTPRouter::HTTPRouter():
	fallback_code(404),
	p(new impl)
{
	
}

HTTPRouter::~HTTPRouter()
{
	
}

void HTTPRouter::on(std::string method, std::string path, handler_func handler)
{
	std::vector<std::string> components = util::split(path, '/');
	impl::route_node &root = p->methods[method];
	
	impl::route_node *node = &root;
	for(auto component : components)
		node = &node->children[component];
	
	node->handler = handler;
}

void HTTPRouter::onError(uint16_t code, handler_func handler)
{
	p->status_handlers[code] = handler;
}

void HTTPRouter::route(std::shared_ptr<HTTPRequest> req, std::shared_ptr<HTTPResponse> res)
{
	// Throw exceptions for responses without required handlers
	if(!res->onData)
		throw std::runtime_error("HTTPRouter::route() response lacks onData");
	
	// Set callbacks that call status handlers where appropriate
	auto old_onData = res->onData;
	auto old_onEnd = res->onEnd;
	wrapResponseHandlers(req, res);
	
	// Extract only the path components; note that the HTTP specs say requests
	// may contain anything from only the path to a full URL.
	std::string path = URL(req->url).path;
	std::vector<std::string> components = util::split(path, '/');
	
	// Look up a matching node in the route tree
	// We can't use operator[] here, since it'll implicitly create objects
	impl::route_node *node = nullptr;
	auto method_it = p->methods.find(req->method);
	if(method_it != p->methods.end())
	{
		// Start at the method's root node, if there is one
		node = &method_it->second;
		for(auto component : components)
		{
			// Try to find the next component in the chain and reassign it to
			// the current node; clear it and break if there is none
			auto it = node->children.find(component);
			if(it == node->children.end())
			{
				node = nullptr;
				break;
			}
			node = &it->second;
		}
	}
	
	// Use the node if it's been found, and it has a handler
	if(node && node->handler)
	{
		node->handler(req, res);
	}
	// If no handler is found, attempt to use the handler for #fallback_code
	else
	{
		res->code = this->fallback_code;
		
		auto status_handler_it = p->status_handlers.find(res->code);
		if(status_handler_it != p->status_handlers.end() && status_handler_it->second)
		{
			// Don't use our newly installed onData and onEnd, as that could
			// create an infinite loop of handler calls in some circumstances.
			res->onData = old_onData;
			res->onEnd = old_onEnd;
			
			status_handler_it->second(req, res);
		}
	}
}

void HTTPRouter::wrapResponseHandlers(std::shared_ptr<HTTPRequest> req, std::shared_ptr<HTTPResponse> res)
{
	auto old_onData = res->onData;
	auto old_onEnd = res->onEnd;
	
	/*
	 * The first time data is received from a response, check it and swap
	 * its onData and onEnd handlers out for a more appropriate one.
	 */
	res->onData = [=](std::shared_ptr<HTTPResponse> res, std::vector<char> data)
	{
		/*
		 * If the response is not chunked, and there's a handler for its code,
		 * buffer the response because we may want to fire a status handler
		 * instead of actually sending the data it presents.
		 */
		if(!res->isChunked() && p->status_handlers.find(res->code) != p->status_handlers.end())
		{
			// onData should buffer data rather than send it to the client
			res->onData = [=](std::shared_ptr<HTTPResponse> res, std::vector<char> data) {
				// std::map::operator[] will create this if it doesn't exist
				std::vector<char> &buffer = p->buffers[res];
				buffer.insert(buffer.end(), data.begin(), data.end());
			};
			
			/*
			 * onEnd should again evaluate if a handler should be fired, fire
			 * it if appropriate, otherwise just send the client the buffer.
			 */
			res->onEnd = [=](std::shared_ptr<HTTPResponse> res) {
				bool status_handler_fired = false;
				
				// Only fire handlers for empty, non-chunked responses
				if(!res->isChunked() && res->body.size() == 0)
				{
					auto status_handler_it = p->status_handlers.find(res->code);
					if(status_handler_it != p->status_handlers.end() && status_handler_it->second)
					{
						// Use the original onData and onEnd to prevent loops
						res->onData = old_onData;
						res->onEnd = old_onEnd;
						status_handler_fired = true;
						
						status_handler_it->second(req, res);
					}
				}
				
				// If a handler hasn't been fired, fire the original handlers
				if(!status_handler_fired)
				{
					old_onData(res, p->buffers[res]);
					if(old_onEnd) old_onEnd(res);
				}
				
				// Remember to delete the buffer when we're done with it!
				p->buffers.erase(res);
			};
		}
		// Otherwise, substitute this with the original handlers
		else
		{
			res->onData = old_onData;
			res->onEnd = old_onEnd;
		}
		
		// Either way, call whatever function is now in onData
		res->onData(res, data);
	};
}
