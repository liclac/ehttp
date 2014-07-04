#include <ehttp/http_server.h>

// Used by the example on_end below
/*
#include <ehttp/url.h>
#include <iostream>
#include <ctime>
*/

using namespace ehttp;

void http_server::event_connected(std::shared_ptr<server::connection> connection)
{
	// If you want to do some setup when the connection is established (such
	// as if you used pointers for context objects), do it here.
	
	server::event_connected(connection);
}

void http_server::event_disconnected(std::shared_ptr<server::connection> connection)
{
	// Delete the context data for the disconnected connection
	contexts.erase(connection);
	
	server::event_disconnected(connection);
}

void http_server::event_data(std::shared_ptr<server::connection> connection, const char *data, std::size_t size)
{
	// std::map's operator[] implicitly creates an object if it doesn't exist,
	// and then returns a reference to it. Thus, no need to do it manually!
	context &ctx = contexts[connection];
	
	// Parse until we get a request; note: we need one parser per connection!
	if(ctx.psr.parse_chunk(data, size) == parser::got_request)
	{
		std::shared_ptr<request> req = ctx.psr.req();
		std::shared_ptr<response> res = std::make_shared<response>(req);
		
		// Just set up the response to feed written data back to the connection
		res->on_data = [=](std::shared_ptr<response> res, std::vector<char> data) {
			connection->write(data);
		};
		
		// If you want to log responses, on_end is the place to do it!
		/*
		res->on_end = [=](std::shared_ptr<response> res) {
			char timestamp[128] = {0};
			
			// The if() is because it's possible for strftime() to fail,
			// leaving timestamp in an undefined, potentially dangerous state.
			std::time_t t = std::time(nullptr);
			if(!std::strftime(timestamp, sizeof(timestamp), "%H:%M:%S", std::localtime(&t)))
				strcpy(timestamp, "00:00:00");
			
			std::cout << "[" << timestamp << "] " << res->code << " " << url(req->url).path << std::endl;
		};
		*/
		
		// Let the router attempt to handle it first, if it exists
		if(rtr) rtr->route(req, res);
		
		// Then fire the request event afterwards
		event_request(connection, req, res);
	}
	
	server::event_data(connection, data, size);
}

void http_server::event_error(asio::error_code error)
{
	// If you want to handle errors
	
	server::event_error(error);
}
