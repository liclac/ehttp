#include <ehttp/http_server.h>
#include <iostream>

using namespace ehttp;

int main(int argc, char **argv)
{
	http_server srv;
	srv.on_request = [=](std::shared_ptr<server::connection>, std::shared_ptr<request> req, std::shared_ptr<response> res) {
		res->begin()
			->header("Content-Type", "text/plain")
			->write("Lorem ipsum dolor sit amet")
			->end();
	};
	srv.on_error = [=](asio::error_code error) {
		std::cout << "Error: " << error << std::endl;
	};
	
	asio::error_code error = srv.listen(8080);
	if(!error)
	{
		std::cout << "Listening!" << std::endl;
		srv.run();
	}
	else
	{
		std::cout << "Couldn't listen: " << error.message() << std::endl;
	}
	
	return 0;
}
