#ifndef EHTTP_SERVER_H
#define EHTTP_SERVER_H

#include <cstdint>
#include <string>
#include <deque>
#include <thread>
#include <functional>
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
		
		asio::error_code listen(const uint16_t &port);
		asio::error_code listen(const std::string &address, const uint16_t &port);
		asio::error_code listen(const tcp::endpoint &endpoint);
		
		void run();
		
		std::function<void(void *data, std::size_t size)> on_data;
		std::function<void(asio::error_code error)> on_error;
		
	protected:
		virtual void accept();
		
		struct impl;
		impl *p;
	};
}

#endif
