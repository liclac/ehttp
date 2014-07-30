#include <ehttp/TCPServer.h>

#include <iostream>

using namespace ehttp;
using namespace asio;
using namespace asio::ip;



/// \private
struct TCPServer::impl
{
	io_service service;
	
	io_service::work *work;
	tcp::acceptor acceptor;
	
	std::deque<std::thread> worker_threads;
	
	impl():
		acceptor(service)
	{}
};



TCPServer::TCPServer(unsigned int workers):
	p(new impl)
{
	p->work = new io_service::work(p->service);
	
	for(unsigned int i = 0; i < workers; i++)
		p->worker_threads.emplace_back([&]{ p->service.run(); });
}

TCPServer::~TCPServer()
{
	delete p->work;
	p->acceptor.close();
	p->service.stop();
	for(auto it = p->worker_threads.begin(); it != p->worker_threads.end(); it++)
		it->join();
	
	delete p;
}

asio::error_code TCPServer::listen(const tcp::endpoint &endpoint)
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

asio::error_code TCPServer::listen(const std::string &address, const uint16_t &port)
{
	return this->listen(tcp::endpoint(address::from_string(address), port));
}

asio::error_code TCPServer::listen(const uint16_t &port)
{
	return this->listen(tcp::endpoint(tcp::v4(), port));
}

void TCPServer::run()
{
	p->service.run();
}

void TCPServer::stop()
{
	p->service.stop();
}

void TCPServer::poll()
{
	p->service.poll();
}

void TCPServer::accept()
{
	auto connection = std::make_shared<TCPConnection>(p->service);
	connection->onConnected = [&]{ this->eventConnected(connection); };
	connection->onData = [&](const char *data, std::size_t size){ this->eventData(connection, data, size); };
	connection->onDisconnected = [&]{ this->eventDisconnected(connection); };
	connection->onError = [&](asio::error_code error){ this->eventError(error); };
	
	p->acceptor.async_accept(connection->socket(), [=](const asio::error_code &error)
	{
		if(!error)
		{
			connection->connected();
			this->accept();
		}
		else eventError(error);
	});
}
