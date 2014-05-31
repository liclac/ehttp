#include <string>
#include <iostream>
#include <ehttp/server.h>

int main(int argc, const char **argv)
{
	ehttp::server srv;
	srv.on_data = [](void *data, std::size_t size) {
		std::cout << std::string(static_cast<char*>(data), size) << std::endl;
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
