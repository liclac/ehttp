#include <ehttp/http_server.h>
#include <ehttp/parser.h>
#include <ehttp/response.h>
#include <ehttp/request.h>
#include <iostream>

using namespace ehttp;

void http_server::event_connected(std::shared_ptr<server::connection> connection)
{
	server::event_connected(connection);
}

void http_server::event_disconnected(std::shared_ptr<server::connection> connection)
{
	contexts.erase(connection);
	server::event_disconnected(connection);
}

void http_server::event_data(std::shared_ptr<server::connection> connection, void *data, std::size_t size)
{
	context &ctx = contexts[connection];
	if(ctx.psr.parse_chunk(data, size) == parser::got_request)
	{
		std::shared_ptr<request> req = ctx.psr.request();
		std::shared_ptr<response> res = std::make_shared<response>(req);
		
		res->on_head = [=](std::shared_ptr<response> res, std::vector<char> data) {
			connection->write(data);
		};
		res->on_body = [=](std::shared_ptr<response> res, std::vector<char> data) {
			connection->write(data);
		};
		res->on_chunk = [=](std::shared_ptr<response> res, std::shared_ptr<response::chunk> chunk, std::vector<char> data) {
			connection->write(data);
		};
		/*
		res->on_end = [=](std::shared_ptr<response> res) {
			// Just left here to point out that it exists
		};
		*/
		
		event_request(connection, req, res);
	}
	server::event_data(connection, data, size);
}

void http_server::event_error(asio::error_code error)
{
	server::event_error(error);
}
