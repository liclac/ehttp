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
		server();
		virtual ~server();
		
		virtual bool listen(const unsigned short &port);
		virtual bool listen(const std::string &address, const unsigned short &port);
		virtual bool listen(const tcp::endpoint &endpoint);
		
	protected:
		virtual void accept();
		
		io_service service;
		io_service::work *work;
		tcp::acceptor acceptor;
		
		std::deque<std::thread> worker_threads;
	};
}

#endif
