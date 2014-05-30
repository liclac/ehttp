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
		
		asio::error_code listen(const unsigned short &port);
		asio::error_code listen(const std::string &address, const unsigned short &port);
		asio::error_code listen(const tcp::endpoint &endpoint);
		
		void run();
		
	protected:
		virtual void accept();
		
		struct impl;
		impl *p;
	};
}

#endif
