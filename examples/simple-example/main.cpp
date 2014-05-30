#include <ehttp/server.h>
#include <iostream>

int main(int argc, const char **argv)
{
	ehttp::server srv;
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
