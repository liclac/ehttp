#ifndef EHTTP_TCPCLIENT_H
#define EHTTP_TCPCLIENT_H

#include "TCPConnection.h"
#include <asio.hpp>

#include <string>
#include <memory>

namespace ehttp
{
	class TCPClient
	{
	public:
		TCPClient(unsigned int workers = 0);
		virtual ~TCPClient();
		
		std::shared_ptr<TCPConnection> connect(std::string host);
		
	private:
		struct impl;
		impl *p;
	};
}

#endif
