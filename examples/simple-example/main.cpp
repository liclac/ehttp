#include <string>
#include <iostream>
#include <ehttp/server.h>
#include <ehttp/server_connection.h>
#include <ehttp/parser.h>
#include <ehttp/request.h>
#include <ehttp/response.h>

int main(int argc, const char **argv)
{
	ehttp::server srv;
	ehttp::parser parser;
	
	
	
	srv.on_data = [&](std::shared_ptr<ehttp::server_connection> connection, void *data, std::size_t size) {
		//std::cout << std::string(static_cast<char*>(data), size) << std::endl;
		parser.parse_chunk(data, size);
	};
	srv.on_error = [](asio::error_code error) {
		std::cerr << "Error: " << error.message() << std::endl;
	};
	
	
	
	parser.on_request = [](std::shared_ptr<ehttp::request> req) {
		std::cout << "Got a request for " << req->url << std::endl;
		
		auto res = std::make_shared<ehttp::response>(req);
		res->on_end = [](std::shared_ptr<ehttp::response> res) {
			std::vector<char> http = res->to_http();
			//std::cout << std::string(http.begin(), http.end()) << std::endl;
		};
		res->on_chunk = [](std::shared_ptr<ehttp::response> res, std::shared_ptr<ehttp::response::chunk> chunk) {
			std::vector<char> http = chunk->to_http();
			std::cout << std::string(http.begin(), http.end()) << std::endl;
		};
		
		try
		{
			res->begin(200);
			res->header("Content-Type", "text/plain");
			res->write("Lorem ipsum dolor sit amet");
			res->end();
		}
		catch(std::exception *e)
		{
			std::cerr << "EXCEPTION: " << e->what() << std::endl;
			throw e;
		}
	};
	parser.on_error = []() {
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
