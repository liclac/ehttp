#include <iostream>
#include <ehttp/server.h>
#include <ehttp/server_connection.h>

using namespace ehttp;
using namespace asio;
using namespace asio::ip;

server::server(unsigned int workers):
	acceptor(service)
{
	work = new io_service::work(service);
	
	for(unsigned int i = 0; i < workers; i++)
		worker_threads.emplace_back([&]{ service.run(); });
}

server::~server()
{
	delete work;
	acceptor.close();
	service.stop();
	for(auto it = worker_threads.begin(); it != worker_threads.end(); it++)
		it->join();
}

asio::error_code server::listen(const unsigned short &port)
{
	return this->listen(tcp::endpoint(tcp::v4(), port));
}

asio::error_code server::listen(const std::string &address, const unsigned short &port)
{
	return this->listen(tcp::endpoint(address::from_string(address), port));
}

asio::error_code server::listen(const tcp::endpoint &endpoint)
{
	asio::error_code error;
	
	acceptor.open(endpoint.protocol(), error);
	if(error) return error;
	
	acceptor.bind(endpoint, error);
	if(error) return error;
	
	acceptor.listen(asio::socket_base::max_connections, error);
	if(error) return error;
	
	this->accept();
	return error;
}

void server::run()
{
	service.run();
}

void server::accept()
{
	server_connection *connection = new server_connection(this, service);
	
	acceptor.async_accept(connection->socket, [=](const asio::error_code &error)
	{
		if(!error)
		{
			connection->connected();
			this->accept();
		}
		else
		{
			delete connection;
			std::cerr << "Couldn't accept: " << error.message() << std::endl;
		}
	});
}
