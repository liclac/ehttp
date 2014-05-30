#include "server.h"
#include "_config.h"
#include "server_connection.h"

using namespace ehttp;
using namespace asio;
using namespace asio::ip;

server::server():
	acceptor(service)
{
	work = new io_service::work(service);
	
	for(unsigned int i = 0; i < kServerWorkerThreads; i++)
		worker_threads.emplace_back([&]{ service.run(); });
}

server::~server()
{
	delete work;
	service.stop();
	for(auto it = worker_threads.begin(); it != worker_threads.end(); it++)
		it->join();
}

bool server::listen(const unsigned short &port)
{
	return this->listen(tcp::endpoint(tcp::v6(), port));
}

bool server::listen(const std::string &address, const unsigned short &port)
{
	return this->listen(tcp::endpoint(address::from_string(address), port));
}

bool server::listen(const tcp::endpoint &endpoint)
{
	acceptor.open(endpoint.protocol());
	
	asio::error_code ec;
	acceptor.bind(endpoint, ec);
	
	if(!ec)
	{
		this->accept();
		return true;
	}
	else return false;
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
		else delete connection;
	});
}
