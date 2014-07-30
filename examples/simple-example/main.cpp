#include <string>
#include <iostream>
#include <ehttp/HTTPServer.h>
#include <ehttp/HTTPRequest.h>
#include <ehttp/HTTPRequestParser.h>
#include <ehttp/HTTPResponse.h>
#include <ehttp/HTTPResponseFactory.h>

using namespace ehttp;

int main(int argc, const char **argv)
{
	// Make a server, and a map of parsers corresponding to them
	HTTPServer srv;
	HTTPResponseFactory<HTTPServer::Connection*> resf;
	std::map<std::shared_ptr<HTTPServer::Connection>, std::shared_ptr<HTTPRequestParser>> parsers;
	
	
	
	resf.onDataFunc = [=](HTTPServer::Connection *connection) {
		return [=](std::shared_ptr<HTTPResponse> res, std::vector<char> data) {
			// Log data being written, before just feeding it to the connection
			std::cout << "--> onData" << std::endl;
			std::cout << std::string(data.begin(), data.end()) << std::endl;
			connection->write(data);
		};
	};
	resf.onEnd = [=](std::shared_ptr<HTTPResponse> res) {
		std::cout << "--> onEnd" << std::endl;
	};
	
	
	
	// Create and destroy parsers in the connection and disconnection handlers
	srv.onConnected = [&](std::shared_ptr<HTTPServer::Connection> connection) {
		std::cout << "> Connected!" << std::endl;
		parsers[connection] = std::make_shared<HTTPRequestParser>();
	};
	srv.onDisconnected = [&](std::shared_ptr<HTTPServer::Connection> connection) {
		std::cout << "> Disconnected!" << std::endl;
		parsers.erase(connection);
	};
	
	
	
	// Handle data received on connections
	srv.onData = [&](std::shared_ptr<HTTPServer::Connection> connection, const char *data, std::size_t size) {
		// If you want to log the raw received data:
		//std::cout << std::string(static_cast<char*>(data), size) << std::endl;
		
		// Grab the connection's parser and feed it the received data
		std::shared_ptr<HTTPRequestParser> parser = parsers[connection];
		if(parser->parseChunk(data, size) == HTTPRequestParser::GotRequest)
		{
			auto req = parser->req();
			auto res = resf.res(req, &*connection);
			
			std::cout << "Got a request for " << req->url << std::endl;
			
			// Begin the response; status codes other than 200 can be given as
			// `begin(404)`, custom reasons as `begin(500, "Man Overboard")`.
			res->begin()
				
				// Write a header; this should be done before writing anything,
				// as attempting to change a header after calling end() or even
				// write() on a chunked connection will throw an exception.
				// The Date and Content-Length headers are autogenerated.
				->header("Content-Type", "text/plain")
				
				// Uncomment to make the response chunked, and cause write() to
				// write chunks rather than append to the body. This will also
				// set the Transfer-Encoding header, and write out any already
				// written body data as the first chunk.
				//->make_chunked()
				
				// Write some data!
				->write("Lorem ipsum do")
				->write("lor sit amet")
				
				// You can also explicitly create chunks; writing one out with
				// end_chunk() will automatically make the response chunked if
				// it's not already, by calling make_chunked() on it.
				/*
				->begin_chunk()
					->write("Lorem ipsum dolor sit amet")
					->end_chunk()
				*/
				
				// End the response and send it to the browser, or write a
				// terminating chunk for a chunked connection. Attempting to
				// modify anything after calling end() will throw an exception.
				->end();
		}
	};
	
	// Optionally log any technical errors that occur (network issues, etc)
	srv.onError = [](asio::error_code error) {
		std::cerr << "Error: " << error.message() << std::endl;
	};
	
	
	
	// Attempt to bind to (listen on) a port. Note that this may fail!
	// See the documentation for server::listen() for more details.
	asio::error_code error = srv.listen(8080);
	if(!error)
	{
		// If it succeeds, print something happy and run until terminated
		std::cout << "Listening!" << std::endl;
		srv.run();
	}
	else
	{
		// Report an error and exit nonzero (failure) if binding fails
		std::cout << "Couldn't listen: " << error.message() << std::endl;
		return 1;
	}
	
	return 0;
}
