#include <string>
#include <iostream>
#include <ehttp/server.h>
#include <ehttp/parser.h>
#include <ehttp/request.h>

int main(int argc, const char **argv)
{
	ehttp::server srv;
	ehttp::parser parser;
	
	
	
	srv.on_data = [&](void *data, std::size_t size) {
		//std::cout << std::string(static_cast<char*>(data), size) << std::endl;
		parser.parse_chunk(data, size);
	};
	srv.on_error = [&](asio::error_code error) {
		std::cerr << "Error: " << error.message() << std::endl;
	};
	
	
	
	parser.on_request = [&](ehttp::request *req) {
		std::cout << "Got a request for " << req->url << std::endl;
	};
	parser.on_error = [&]() {
		std::cerr << "Parser Error" << std::endl;
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
