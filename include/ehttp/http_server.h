#ifndef EHTTP_HTTP_SERVER_H
#define EHTTP_HTTP_SERVER_H

#include <functional>
#include "eserver.h"
#include "eparser.h"
#include "erequest.h"
#include "eresponse.h"
#include "erouter.h"

namespace ehttp
{
	/**
	 * A convenience class that combines \ref eserver and \ref eparser.
	 * 
	 * If your application does not already have an HTTP or TCP server set up,
	 * or simply does not have any special needs that would necessitate a
	 * custom implementation, you can easily drop this in to create a server
	 * that handles the most common use case out of the box: serving HTTP
	 * requests on a given port.
	 * 
	 * This also serves as an example of a custom server implementation, and as
	 * such it's more extensively documented than the rest of the library.
	 */
	class http_server : public eserver
	{
	public:
		using eserver::eserver;
		
		/**
		 * Optional router for handling requests.
		 */
		std::shared_ptr<erouter> rtr;
		
		/**
		 * Callback for when a new request is received.
		 * 
		 * @param connection The connection that received the request
		 * @param req The received request
		 * @param res A response object that writes back to the connection
		 */
		std::function<void(std::shared_ptr<eserver::connection> connection, std::shared_ptr<erequest> req, std::shared_ptr<eresponse> res)> on_request;
		
	protected:
		/// Overridable emitter for #on_request
		virtual void event_request(std::shared_ptr<eserver::connection> connection, std::shared_ptr<erequest> req, std::shared_ptr<eresponse> res) {
			if(on_request) on_request(connection, req, res);
		}
		
		virtual void event_connected(std::shared_ptr<eserver::connection> connection) override;
		virtual void event_disconnected(std::shared_ptr<eserver::connection> connection) override;
		virtual void event_data(std::shared_ptr<eserver::connection> connection, const char *data, std::size_t size) override;
		virtual void event_error(asio::error_code error) override;
		
		/// Struct for keeping context data
		struct context {
			eparser psr;	///< A parser for the connection
		};
		/// Map between connections and contexts
		std::map<std::shared_ptr<connection>, context> contexts;
	};
}

#endif
