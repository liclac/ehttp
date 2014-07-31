#include <ehttp/TCPClient.h>

#include <deque>
#include <thread>

using namespace ehttp;
using namespace asio;
using namespace asio::ip;



/// \private
struct TCPClient::impl
{
	io_service service;
	io_service::work *work;
	tcp::resolver resolver;
	
	std::deque<std::thread> worker_threads;
	
	impl(): resolver(service)
	{}
};



TCPClient::TCPClient(unsigned int workers):
	p(new impl)
{
	
}

TCPClient::~TCPClient()
{
	
}

std::shared_ptr<TCPConnection> TCPClient::connect(std::string host)
{
	return nullptr;
}
