#include <iostream>
#include <ehttp/private/server_connection.h>
#include <ehttp/server.h>

using namespace ehttp;
using namespace asio;
using namespace asio::ip;

#define kReadBufferSize 1024



struct server_connection::impl
{	
	io_service &service;
	tcp::socket socket;
	
	std::vector<char> read_buffer;
	
	impl(io_service &service):
		service(service), socket(service),
		read_buffer(kReadBufferSize)
	{}
};



server_connection::server_connection(io_service &service):
	p(new impl(service))
{
	
}

server_connection::~server_connection()
{
	
}

tcp::socket& server_connection::socket() { return p->socket; }

void server_connection::connected()
{
	this->read_chunk();
}

void server_connection::disconnect()
{
	if(p->socket.is_open())
	{
		try {
			p->socket.shutdown(tcp::socket::shutdown_both);
			p->socket.close();
		} catch (std::system_error err) {
			// If this happens, that means we've already disconnected
		}
	}
}

void server_connection::read_chunk()
{
	p->socket.async_read_some(asio::buffer(p->read_buffer, kReadBufferSize),
		[&](const asio::error_code &error, std::size_t bytes_transferred)
	{
		if(!error)
		{
			std::string str(&p->read_buffer[0], bytes_transferred);
			std::cout << str;
		}
		else
		{
			std::cerr << "Read Error: " << error.message() << std::endl;
		}
	});
}
