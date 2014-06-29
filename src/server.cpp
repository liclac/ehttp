#include <iostream>
#include <ehttp/server.h>

using namespace ehttp;
using namespace asio;
using namespace asio::ip;

#define kReadBufferSize 1024



/// \private
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

asio::error_code server::listen(const std::string &address, const uint16_t &port)
{
	return this->listen(tcp::endpoint(address::from_string(address), port));
}

asio::error_code server::listen(const uint16_t &port)
{
	return this->listen(tcp::endpoint(tcp::v4(), port));
}

void server::run()
{
	p->service.run();
}

void server::stop()
{
	p->service.stop();
}

void server::poll()
{
	p->service.poll();
}

void server::accept()
{
	std::shared_ptr<server::connection> connection = std::make_shared<server::connection>(this, p->service);
	
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



/// \private
struct server::connection::impl
{
	// Prevent autodeletion while in use
	std::shared_ptr<server::connection> retain_self;
	
	server *server;
	
	io_service &service;
	tcp::socket socket;
	
	std::vector<char> read_buffer;
	
	impl(io_service &service):
		service(service), socket(service)
	{}
};



server::connection::connection(server *server, io_service &service):
	p(new impl(service))
{
	p->server = server;
	p->read_buffer.resize(kReadBufferSize);
}

server::connection::~connection()
{
	delete p;
}

tcp::socket& server::connection::socket() { return p->socket; }

void server::connection::connected()
{
	p->retain_self = shared_from_this();
	
	if(p->server->on_connected)
		p->server->on_connected(shared_from_this());
	
	this->read_chunk();
}

void server::connection::disconnect()
{
	if(p->socket.is_open())
	{
		try {
			p->socket.shutdown(tcp::socket::shutdown_both);
			p->socket.close();
		} catch (std::system_error err) {
			// If this happens, that means we've already disconnected
		}
		
		if(p->server->on_disconnected)
			p->server->on_disconnected(shared_from_this());
	}
	p->retain_self.reset();
}

void server::connection::read_chunk()
{
	p->socket.async_read_some(asio::buffer(p->read_buffer, kReadBufferSize),
		[&](const asio::error_code &error, std::size_t bytes_transferred)
	{
		if(!error)
		{
			if(p->server->on_data)
				p->server->on_data(shared_from_this(), &p->read_buffer[0], bytes_transferred);
			
			this->read_chunk();
		}
		else
		{
			if(p->server->on_error)
				p->server->on_error(error);
			if(p->server->on_disconnected)
				p->server->on_disconnected(shared_from_this());
		}
	});
}
