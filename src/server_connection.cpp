#include <iostream>
#include <ehttp/server_connection.h>
#include <ehttp/server.h>

using namespace ehttp;
using namespace asio;
using namespace asio::ip;

server_connection::server_connection(server *srv, io_service &service):
	socket(service),
	service(service), srv(srv)
{
	
}

server_connection::~server_connection()
{
	
}

void server_connection::connected()
{
	std::cout << "Connected";
	this->read_chunk();
}

void server_connection::read_chunk()
{
	
}
