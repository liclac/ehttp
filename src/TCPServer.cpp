#include <ehttp/TCPServer.h>

#include <iostream>

using namespace ehttp;
using namespace asio;
using namespace asio::ip;

#define kReadBufferSize 1024



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
	std::shared_ptr<TCPServer::Connection> connection = std::make_shared<TCPServer::Connection>(this, p->service);
	
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



/// \private
struct TCPServer::Connection::impl
{
	// Prevent autodeletion while in use
	std::shared_ptr<TCPServer::Connection> retain_self;
	
	TCPServer *srv;
	
	io_service &service;
	tcp::socket socket;
	
	std::vector<char> read_buffer;
	std::deque<std::vector<char>> write_queue;
	
	impl(io_service &service):
		service(service), socket(service)
	{}
};



TCPServer::Connection::Connection(TCPServer *srv, io_service &service):
	p(new impl(service))
{
	p->srv = srv;
	p->read_buffer.resize(kReadBufferSize);
}

TCPServer::Connection::~Connection()
{
	delete p;
}

tcp::socket& TCPServer::Connection::socket() { return p->socket; }

void TCPServer::Connection::write(std::vector<char> data, std::function<void(const asio::error_code &error, std::size_t bytes_transferred)> callback)
{
	p->write_queue.push_back(data);
	if(p->write_queue.size() == 1)
		this->writeNext();
}

void TCPServer::Connection::disconnect()
{
	if(p->socket.is_open())
	{
		try {
			p->socket.shutdown(tcp::socket::shutdown_both);
			p->socket.close();
		} catch (std::system_error err) {
			// If this happens, that means we've already disconnected
		}
		
		p->srv->eventDisconnected(shared_from_this());
	}
	p->retain_self.reset();
}

void TCPServer::Connection::connected()
{
	p->retain_self = shared_from_this();
	p->srv->eventConnected(shared_from_this());
	this->readChunk();
}

void TCPServer::Connection::readChunk()
{
	p->socket.async_read_some(asio::buffer(p->read_buffer, kReadBufferSize),
		[&](const asio::error_code &error, std::size_t bytes_transferred)
	{
		if(!error)
		{
			p->srv->eventData(shared_from_this(), &p->read_buffer[0], bytes_transferred);
			this->readChunk();
		}
		else
		{
			if(error != asio::error::eof && error != asio::error::connection_reset)
				p->srv->eventError(error);
			p->srv->eventDisconnected(shared_from_this());
		}
	});
}

void TCPServer::Connection::writeNext()
{
	std::vector<char> &data = p->write_queue[0];
	asio::async_write(p->socket, asio::buffer(data), [=](const asio::error_code &error, std::size_t bytes_transferred) {
		p->write_queue.pop_front();
		if(p->write_queue.size() > 0)
			this->writeNext();
	});
}
