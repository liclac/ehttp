#ifndef EHTTP_SERVER_H
#define EHTTP_SERVER_H

#include <deque>
#include <thread>
#include <asio.hpp>

namespace ehttp
{
	using namespace asio;
	using namespace asio::ip;
	
	class server
	{
	public:
		server(unsigned int workers = 0);
		virtual ~server();
		
		virtual asio::error_code listen(const unsigned short &port);
		virtual asio::error_code listen(const std::string &address, const unsigned short &port);
		virtual asio::error_code listen(const tcp::endpoint &endpoint);
		
		virtual void run();
		
		io_service service;
		
	protected:
		virtual void accept();
		
		io_service::work *work;
		tcp::acceptor acceptor;
		
		std::deque<std::thread> worker_threads;
	};
}

#endif
