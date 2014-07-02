#include <ehttp/http_server.h>
#include <ehttp/router.h>
#include <iostream>

using namespace ehttp;

int main(int argc, char **argv)
{
	// Create a server and a router
	http_server srv;
	srv.router = std::make_shared<router>();
	
	srv.router->on("GET", "/", [](std::shared_ptr<request> req, std::shared_ptr<response> res) {
		res->begin()
			->header("Content-Type", "text/plain")
			->write("Lorem ipsum dolor sit amet")
			->end();
	});
	
	// Handle server errors (network issues, etc)
	srv.on_error = [=](asio::error_code error) {
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
