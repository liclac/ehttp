#ifndef EHTTP_HTTP_SERVER_H
#define EHTTP_HTTP_SERVER_H

#include <functional>
#include "server.h"
#include "parser.h"
#include "request.h"
#include "response.h"

namespace ehttp
{
	/**
	 * A convenience class that combines \ref server and \ref parser.
	 * 
	 * If your application does not already have an HTTP or TCP server set up,
	 * or simply does not have any special needs that would necessitate a
	 * custom implementation, you can easily drop this in to create a server
	 * that handles the most common use case out of the box: serving HTTP
	 * requests on a given port.
	 * 
	 * This also serves as an example of a custom server implementation.
	 */
	class http_server : public server
	{
	public:
		using server::server;
		
		std::function<void(std::shared_ptr<server::connection> connection, std::shared_ptr<request> req, std::shared_ptr<response> res)> on_request;
		
	protected:
		/// Overridable emitter for #on_request
		virtual void event_request(std::shared_ptr<server::connection> connection, std::shared_ptr<request> req, std::shared_ptr<response> res) {
			if(on_request) on_request(connection, req, res);
		}
		
		virtual void event_connected(std::shared_ptr<server::connection> connection) override;
		virtual void event_disconnected(std::shared_ptr<server::connection> connection) override;
		virtual void event_data(std::shared_ptr<server::connection> connection, void *data, std::size_t size) override;
		virtual void event_error(asio::error_code error) override;
		
		struct context {
			parser psr;
		};
		std::map<std::shared_ptr<connection>, context> contexts;
	};
}

#endif
