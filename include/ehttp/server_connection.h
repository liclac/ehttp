#ifndef EHTTP_SERVER_CONNECTION_H
#define EHTTP_SERVER_CONNECTION_H

#include <memory>
#include <vector>
#include <asio.hpp>

namespace ehttp
{
	using namespace asio;
	using namespace asio::ip;
	
	class server;
	class server_connection : public std::enable_shared_from_this<server_connection>
	{
	public:
		server_connection(server *server, io_service &service);
		virtual ~server_connection();
		
		tcp::socket& socket();
		
		void connected();
		void disconnect();
		
	protected:
		void read_chunk();
		
		struct impl;
		impl *p;
	};
}

#endif
