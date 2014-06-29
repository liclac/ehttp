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
	
	/**
	 * A simple ASIO-based TCP server.
	 * 
	 * You can use this to easily listen for incoming connections, but you are
	 * in no way required to - if you have another TCP server that works better
	 * for you, feel free to use that.
	 */
	class server
	{
	public:
		class connection;
		
		/**
		 * Constructor.
		 * @param workers The number of worker threads to create
		 */
		server(unsigned int workers = 0);
		virtual ~server();
		
		/**
		 * Listens on an endpoint.
		 * @param endpoint The ASIO endpoint to listen on
		 */
		asio::error_code listen(const tcp::endpoint &endpoint);
		
		/**
		 * Listens on an address and port.
		 * @overload
		 * @param address The address (0.0.0.0, 127.0.0.1, ...) to listen on
		 * @param port The port to listen on
		 */
		asio::error_code listen(const std::string &address, const uint16_t &port);
		
		/**
		 * Listens on a port on all addresses.
		 * @overload
		 * @param port The port to listen on
		 */
		asio::error_code listen(const uint16_t &port);
		
		/**
		 * Runs the server on the current thread.
		 * This function will not return until the server is stopped, either by
		 * calling stop() or in response to a termination request (SIGTERM).
		 */
		void run();
		
		/**
		 * Stops a running server.
		 * This will return immediately, but run() may take a moment before it
		 * returns, as it will wait until all currently queued events have been
		 * processed.
		 */
		void stop();
		
		/**
		 * Runs one turn of the server loop.
		 * If you have an existing run loop in your application, you can call
		 * this function as a part of it (or just once in a while, really), to
		 * neatly integrate ehttp into your application without having to give
		 * it its own thread.
		 */
		void poll();
		
		
		
		/**
		 * Callback for when a connection is established.
		 * 
		 * @param connection The newly established connection
		 */
		std::function<void(std::shared_ptr<server::connection> connection)> on_connected;
		
		/**
		 * Callback for when a connection receives data.
		 * 
		 * @param connection The connection that received the data
		 * @param data Pointer to the response data; this is only guaranteed to be valid until this callback returns
		 * @param size Size of the response data
		 */
		std::function<void(std::shared_ptr<server::connection> connection, void *data, std::size_t size)> on_data;
		
		/**
		 * Callback for when a connection is disconnected.
		 * 
		 * This is called regardless of if the connection was cleanly closed or
		 * if it was closed in response to an error (after #on_error), so you
		 * may do any cleanup here regardless.
		 * 
		 * @param connection The newly disconnected connection
		 */
		std::function<void(std::shared_ptr<server::connection> connection)> on_disconnected;
		
		/**
		 * Callback for when there's a problem.
		 * 
		 * Note that this is not called when there is an error that's expected
		 * and handled by the server or connection, such as EOFs.
		 * 
		 * @param error The ASIO Error code
		 */
		std::function<void(asio::error_code error)> on_error;
		
	protected:
		/// Gets the server ready to accept a new connection
		virtual void accept();
		
	private:
		struct impl;
		impl *p;
	};
	
	
	
	/**
	 * Represents a connection for \ref server.
	 * 
	 * Note that a connection will retain a shared_ptr to itself, thus
	 * preventing it from getting deleted until it has disconnected.\n
	 * When you are done with a connection, you should thus always call
	 * disconnect() on it to prevent it from just laying around.
	 * 
	 * @todo Make it automatically disconnect when the socket is closed
	 * 
	 * @todo Make connected() and read_chunk() protected somehow
	 */
	class server::connection : public std::enable_shared_from_this<connection>
	{
	public:
		/**
		 * Constructor.
		 * @param server The parent server
		 * @param service The ASIO service
		 */
		connection(server *server, io_service &service);
		virtual ~connection();
		
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
