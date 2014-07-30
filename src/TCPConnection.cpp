#include <ehttp/TCPConnection.h>

#include <deque>

using namespace ehttp;
using namespace asio;
using namespace asio::ip;

#define kReadBufferSize 1024

/// \private
struct TCPConnection::impl
{
	// Prevent autodeletion while in use
	std::shared_ptr<TCPConnection> retain_self;
	
	io_service &service;
	tcp::socket socket;
	
	std::vector<char> read_buffer;
	std::deque<std::vector<char>> write_queue;
	
	impl(io_service &service):
		service(service), socket(service)
	{}
};



TCPConnection::TCPConnection(io_service &service):
	p(new impl(service))
{
	p->read_buffer.resize(kReadBufferSize);
}

TCPConnection::~TCPConnection()
{
	delete p;
}

tcp::socket& TCPConnection::socket() { return p->socket; }

void TCPConnection::write(std::vector<char> data, std::function<void(const asio::error_code &error, std::size_t bytes_transferred)> callback)
{
	p->write_queue.push_back(data);
	if(p->write_queue.size() == 1)
		this->writeNext();
}

void TCPConnection::disconnect()
{
	if(p->socket.is_open())
	{
		try {
			p->socket.shutdown(tcp::socket::shutdown_both);
			p->socket.close();
		} catch (std::system_error err) {
			// If this happens, that means we've already disconnected
		}
		
		this->eventDisconnected();
	}
	p->retain_self.reset();
}

void TCPConnection::connected()
{
	p->retain_self = shared_from_this();
	this->eventConnected();
	this->readChunk();
}

void TCPConnection::readChunk()
{
	p->socket.async_read_some(asio::buffer(p->read_buffer, kReadBufferSize),
		[&](const asio::error_code &error, std::size_t bytes_transferred)
	{
		if(!error)
		{
			this->eventData(&p->read_buffer[0], bytes_transferred);
			this->readChunk();
		}
		else
		{
			if(error != asio::error::eof && error != asio::error::connection_reset)
				this->eventError(error);
			this->eventDisconnected();
		}
	});
}

void TCPConnection::writeNext()
{
	std::vector<char> &data = p->write_queue[0];
	asio::async_write(p->socket, asio::buffer(data), [=](const asio::error_code &error, std::size_t bytes_transferred) {
		p->write_queue.pop_front();
		if(p->write_queue.size() > 0)
			this->writeNext();
	});
}
