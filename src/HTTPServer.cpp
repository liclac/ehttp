#include <ehttp/HTTPServer.h>

// Used by the example on_end below
/*
#include <ehttp/eurl.h>
#include <iostream>
#include <ctime>
*/

using namespace ehttp;

void HTTPServer::eventConnected(std::shared_ptr<TCPServer::Connection> connection)
{
	// If you want to do some setup when the connection is established (such
	// as if you used pointers for context objects), do it here.
	
	TCPServer::eventConnected(connection);
}

void HTTPServer::eventDisconnected(std::shared_ptr<TCPServer::Connection> connection)
{
	// Delete the context data for the disconnected connection
	contexts.erase(connection);
	
	TCPServer::eventDisconnected(connection);
}

void HTTPServer::eventData(std::shared_ptr<TCPServer::Connection> connection, const char *data, std::size_t size)
{
	// std::map's operator[] implicitly creates an object if it doesn't exist,
	// and then returns a reference to it. Thus, no need to do it manually!
	context &ctx = contexts[connection];
	
	// Parse until we get a request; note: we need one parser per connection!
	if(ctx.psr.parseChunk(data, size) == HTTPRequestParser::GotRequest)
	{
		std::shared_ptr<HTTPRequest> req = ctx.psr.req();
		std::shared_ptr<HTTPResponse> res = std::make_shared<HTTPResponse>(req);
		
		// Just set up the response to feed written data back to the connection
		res->onData = [=](std::shared_ptr<HTTPResponse> res, std::vector<char> data) {
			connection->write(data);
		};
		
		// If you want to log responses, onEnd is the place to do it!
		/*
		res->onEnd = [=](std::shared_ptr<HTTPResponse> res) {
			char timestamp[128] = {0};
			
			// The if() is because it's possible for strftime() to fail,
			// leaving timestamp in an undefined, potentially dangerous state.
			std::time_t t = std::time(nullptr);
			if(!std::strftime(timestamp, sizeof(timestamp), "%H:%M:%S", std::localtime(&t)))
				strcpy(timestamp, "00:00:00");
			
			std::cout << "[" << timestamp << "] " << res->code << " " << eurl(req->url).path << std::endl;
		};
		*/
		
		// Let the router attempt to handle it first, if it exists
		if(rtr) rtr->route(req, res);
		
		// Then fire the request event afterwards
		eventRequest(connection, req, res);
	}
	
	TCPServer::eventData(connection, data, size);
}

void HTTPServer::eventError(asio::error_code error)
{
	// If you want to handle errors
	
	TCPServer::eventError(error);
}
