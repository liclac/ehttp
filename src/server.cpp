#include <iostream>
#include <ehttp/server.h>
#include <ehttp/server_connection.h>

using namespace ehttp;
using namespace asio;
using namespace asio::ip;



struct server::impl
{
	io_service service;
	
	io_service::work *work;
	tcp::acceptor acceptor;
	
	std::deque<std::thread> worker_threads;
	
	impl():
		acceptor(service)
	{}
};



server::server(unsigned int workers):
	p(new impl)
{
	p->work = new io_service::work(p->service);
	
	for(unsigned int i = 0; i < workers; i++)
		p->worker_threads.emplace_back([&]{ p->service.run(); });
}

server::~server()
{
	delete p->work;
	p->acceptor.close();
	p->service.stop();
	for(auto it = p->worker_threads.begin(); it != p->worker_threads.end(); it++)
		it->join();
	
	delete p;
}

asio::error_code server::listen(const uint16_t &port)
{
	return this->listen(tcp::endpoint(tcp::v4(), port));
}

asio::error_code server::listen(const std::string &address, const uint16_t &port)
{
	return this->listen(tcp::endpoint(address::from_string(address), port));
}

asio::error_code server::listen(const tcp::endpoint &endpoint)
{
	asio::error_code error;
	
	p->acceptor.open(endpoint.protocol(), error);
	if(error) return error;
	
	p->acceptor.bind(endpoint, error);
	if(error) return error;
	
	p->acceptor.listen(asio::socket_base::max_connections, error);
	if(error) return error;
	
	this->accept();
	return error;
}

void server::run()
{
	p->service.run();
}

void server::accept()
{
	std::shared_ptr<server_connection> connection = std::make_shared<server_connection>(this, p->service);
	
	p->acceptor.async_accept(connection->socket(), [=](const asio::error_code &error)
	{
		if(!error)
		{
			connection->connected();
			this->accept();
		}
		else if(on_error)
			on_error(error);
	});
}
