#include <iostream>
#include <ehttp/server_connection.h>
#include <ehttp/server.h>

using namespace ehttp;
using namespace asio;
using namespace asio::ip;

#define kReadBufferSize 1024

server_connection::server_connection(server *srv, io_service &service):
	socket(service),
	service(service), srv(srv),
	read_buffer(kReadBufferSize)
{
	
}

server_connection::~server_connection()
{
	
}

void server_connection::connected()
{
	this->read_chunk();
}

void server_connection::disconnect()
{
	if(socket.is_open())
	{
		try {
			socket.shutdown(tcp::socket::shutdown_both);
			socket.close();
		} catch (std::system_error err) {
			// If this happens, that means we've already disconnected
		}
	}
}

void server_connection::read_chunk()
{
	socket.async_read_some(asio::buffer(read_buffer, kReadBufferSize),
		[&](const asio::error_code &error, std::size_t bytes_transferred)
	{
		if(!error)
		{
			std::string str(&read_buffer[0], bytes_transferred);
			std::cout << str;
		}
		else
		{
			std::cerr << "Read Error: " << error.message() << std::endl;
		}
	});
}
