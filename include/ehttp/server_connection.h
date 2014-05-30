#ifndef EHTTP_SERVER_CONNECTION_H
#define EHTTP_SERVER_CONNECTION_H

#include <vector>
#include <asio.hpp>

namespace ehttp
{
	using namespace asio;
	using namespace asio::ip;
	
	class server;
	class server_connection
	{
	public:
		server_connection(server *srv, io_service &service);
		virtual ~server_connection();
		
		virtual void connected();
		virtual void disconnect();
		
		tcp::socket socket;
		
	protected:
		virtual void read_chunk();
		
		io_service &service;
		server *srv;
		
		std::vector<char> read_buffer;
	};
}

#endif
