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
	
	/**
	 * Represents a connection for \ref server.
	 * 
	 * Note that a connection will retain a shared_ptr to itself, thus
	 * preventing it from getting deleted until it has disconnected.\n
	 * When you are done with a connection, you should thus always call
	 * disconnect() on it to prevent it from just laying around.
	 * 
	 * @todo Move this inside \ref server
	 * 
	 * @todo Make it automatically disconnect when the socket is closed
	 * 
	 * @todo Make connected() and read_chunk() protected somehow
	 */
	class server_connection : public std::enable_shared_from_this<server_connection>
	{
	public:
		/**
		 * Constructor.
		 * @param server The parent server
		 * @param service The ASIO service
		 */
		server_connection(server *server, io_service &service);
		virtual ~server_connection();
		
		/// Returns the connection's ASIO socket
		tcp::socket& socket();
		
		/**
		 * Disconnects from the client, and allows the connection to be deleted
		 * if there are no more references to it.
		 */
		void disconnect();
		
		
		
		/**
		 * \private (Hide it from Doxygen's output)
		 * Called when the socket is connected. Only public because it is
		 * currently necessary due to an implementation detail in \ref server,
		 * which can change at any time.
		 */
		void connected();
		
		/**
		 * \private (Hide it from Doxygen's output)
		 * Called in a loop to read data form the stream. As with connected(),
		 * it being public is an implementation detail and subject to change.
		 */
		void read_chunk();
		
	private:
		struct impl;
		impl *p;
	};
}

#endif
