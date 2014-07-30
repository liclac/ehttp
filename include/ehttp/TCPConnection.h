#ifndef EHTTP_TCPCONNECTION_H
#define EHTTP_TCPCONNECTION_H

#include <asio.hpp>

#include <vector>
#include <functional>
#include <memory>

namespace ehttp
{
	using namespace asio;
	using namespace asio::ip;
	
	/**
	 * Represents a connection for a TCPServer or a TCPClient.
	 * 
	 * Note that a connection will retain a shared_ptr to itself, thus
	 * preventing it from getting deleted until it has disconnected.\n
	 * When you are done with a connection, you should thus always call
	 * disconnect() on it to prevent it from just laying around.
	 */
	class TCPConnection : public std::enable_shared_from_this<TCPConnection>
	{
	public:
		/**
		 * Constructor.
		 * @param service The ASIO service
		 */
		TCPConnection(io_service &service);
		virtual ~TCPConnection();
		
		/// Returns the connection's ASIO socket
		tcp::socket& socket();
		
		/**
		 * Writes a chunk of data to the stream.
		 * The data is copied into an internal write buffer to ensure that it
		 * stays valid.
		 */
		void write(std::vector<char> data, std::function<void(const asio::error_code &error, std::size_t bytes_transferred)> callback = nullptr);
		
		/**
		 * Disconnects from the client, and allows the connection to be deleted
		 * if there are no more references to it.
		 */
		void disconnect();
		
		
		
		/// Arbitrary, user-assigned pointer, to be used for context data
		std::shared_ptr<void> userdata;
		
		
		
		/**
		 * Callback for when a connection is established.
		 */
		std::function<void()> onConnected;
		
		/**
		 * Callback for when a connection receives data.
		 * 
		 * @param data Pointer to the response data; this is only guaranteed to be valid until this callback returns
		 * @param size Size of the response data
		 */
		std::function<void(const char *data, std::size_t size)> onData;
		
		/**
		 * Callback for when a connection is disconnected.
		 * 
		 * This is called regardless of if the connection was cleanly closed or
		 * if it was closed in response to an error (after #onError), so you
		 * may do any cleanup here regardless.
		 */
		std::function<void()> onDisconnected;
		
		/**
		 * Callback for when there's a problem.
		 * 
		 * Note that this is not called when there is an error that's expected
		 * and handled by the server or connection, such as EOFs.
		 * 
		 * @param error The ASIO Error code
		 */
		std::function<void(asio::error_code error)> onError;
		
		
		
		/**
		 * \private
		 * Called when the socket is connected.
		 * 
		 * For internal use only.
		 */
		void connected();
		
		/**
		 * \private
		 * Called in a loop to read data form the stream.
		 * 
		 * For internal use only.
		 */
		void readChunk();
		
		/**
		 * \private
		 * Called from a write() to write the next thing in the write queue.
		 * 
		 * For internal use only.
		 */
		void writeNext();
		
	protected:
		/// Overridable emitter for #onConnected
		virtual void eventConnected() { if(onConnected) onConnected(); }
		
		/// Overridable emitter for #onData
		virtual void eventData(const char *data, std::size_t size) { if(onData) onData(data, size); }
		
		/// Overridable emitter for #onDisconnected
		virtual void eventDisconnected() { if(onDisconnected) onDisconnected(); }
		
		/// Overridable emitter for #onError
		virtual void eventError(asio::error_code error) { if(onError) onError(error); }
		
	private:
		struct impl;
		impl *p;
	};
}

#endif
