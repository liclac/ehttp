#include <ehttp/HTTPServer.h>
#include <ehttp/RequestRouter.h>
#include <iostream>

using namespace ehttp;

int main(int argc, char **argv)
{
	HTTPServer srv;
	srv.rtr = std::make_shared<RequestRouter>();
	
	srv.rtr->on("GET", "/", [=](std::shared_ptr<HTTPRequest> req, std::shared_ptr<HTTPResponse> res) {
		res->begin()
			->header("Content-Type", "text/plain")
			->write("Lorem ipsum dolor sit amet")
			->end();
	});
	srv.rtr->on("GET", "/unauthorized", [=](std::shared_ptr<HTTPRequest> req, std::shared_ptr<HTTPResponse> res) {
		res->begin(403)
			->end();
	});
	
	srv.rtr->onError(404, [=](std::shared_ptr<HTTPRequest> req, std::shared_ptr<HTTPResponse> res) {
		res->begin(0)
			->header("Content-Type", "text/plain")
			->write("Not Found")
			->end();
	});
	
	srv.rtr->onError(403, [=](std::shared_ptr<HTTPRequest> req, std::shared_ptr<HTTPResponse> res) {
		res->begin(0)
			->header("Content-Type", "text/plain")
			->write("You're not authorized to access this")
			->end();
	});
	
	// Handle server errors (network issues, etc)
	srv.onError = [=](asio::error_code error) {
		std::cout << "Error: " << error << std::endl;
	};
	
	// Attempt to bind to (listen on) a port.
	// Note that this may fail! See the documentation for more details.
	asio::error_code error = srv.listen(8080);
	if(!error)
	{
		std::cout << "Listening!" << std::endl;
		srv.run();
	}
	else
	{
		std::cout << "Couldn't listen: " << error.message() << std::endl;
		return 1;
	}
	
	return 0;
}
