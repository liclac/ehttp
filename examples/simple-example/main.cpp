#include <string>
#include <iostream>
#include <ehttp/server.h>
#include <ehttp/parser.h>
#include <ehttp/request.h>
#include <ehttp/response.h>

using namespace ehttp;

int main(int argc, const char **argv)
{
	server srv;
	parser parser;
	
	
	
	srv.on_data = [&](std::shared_ptr<server::connection> connection, void *data, std::size_t size) {
		//std::cout << std::string(static_cast<char*>(data), size) << std::endl;
		if(parser.parse_chunk(data, size) == parser::got_request)
		{
			std::shared_ptr<request> req = parser.request();
			std::cout << "Got a request for " << req->url << std::endl;
			
			auto res = std::make_shared<response>(req);
			res->on_head = [](std::shared_ptr<response> res, std::vector<char> data) {
				std::cout << "--> on_head" << std::endl;
				std::cout << std::string(data.begin(), data.end()) << std::endl;
			};
			res->on_body = [](std::shared_ptr<response> res, std::vector<char> data) {
				std::cout << "--> on_body" << std::endl;
				std::cout << std::string(data.begin(), data.end()) << std::endl;
			};
			res->on_chunk = [](std::shared_ptr<response> res, std::shared_ptr<response::chunk> chunk, std::vector<char> data) {
				std::cout << "--> on_chunk" << std::endl;
				std::cout << std::string(data.begin(), data.end()) << std::endl;
			};
			res->on_end = [](std::shared_ptr<response> res) {
				std::cout << "--> on_end" << std::endl;
			};
			
			try
			{
				res->begin(200)
					->header("Content-Type", "text/plain")
					//->make_chunked()
					->write("Lorem ipsum dolor sit amet")
					/*->begin_chunk()
						->write("Lorem ipsum dolor sit amet")
						->end()*/
					->end();
			}
			catch(std::exception *e)
			{
				std::cerr << "EXCEPTION: " << e->what() << std::endl;
				throw e;
			}
		}
	};
	srv.on_error = [](asio::error_code error) {
		std::cerr << "Error: " << error.message() << std::endl;
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
