#include <string>
#include <stdexcept>
#include <ehttp/HTTPParser.h>
#include <ehttp/HTTPRequest.h>
#include "../vendor/http-parser/http_parser.h"

using namespace ehttp;



int ehttp_parser_on_message_begin(http_parser *psr);
int ehttp_parser_on_url(http_parser *psr, const char *data, size_t length);
int ehttp_parser_on_header_field(http_parser *psr, const char *data, size_t length);
int ehttp_parser_on_header_value(http_parser *psr, const char *data, size_t length);
int ehttp_parser_on_headers_complete(http_parser *psr);
int ehttp_parser_on_body(http_parser *psr, const char *data, size_t length);
int ehttp_parser_on_message_complete(http_parser *psr);

static const http_parser_settings ehttp_parser_parser_settings = {
	ehttp_parser_on_message_begin,
	ehttp_parser_on_url,
	0, // on_status is never called for requests
	ehttp_parser_on_header_field,
	ehttp_parser_on_header_value,
	ehttp_parser_on_headers_complete,
	ehttp_parser_on_body,
	ehttp_parser_on_message_complete
};

namespace ehttp
{
	/// \private
	struct HTTPParser_parser_ctx
	{
		std::shared_ptr<HTTPRequest> req;
		bool done;
		
		std::string tmp_header_field, tmp_header_value;
		bool was_reading_header_value;
		
		HTTPParser *psr;
	};
}

void ehttp_parser_push_header(http_parser *psr);



/// \private
struct HTTPParser::impl
{
	http_parser *psr;
};

HTTPParser::HTTPParser():
	p(new impl)
{
	p->psr = new http_parser;
	http_parser_init(p->psr, HTTP_REQUEST);
	
	HTTPParser_parser_ctx *ctx = new HTTPParser_parser_ctx;
	ctx->done = false;
	ctx->was_reading_header_value = false;
	ctx->psr = this;
	p->psr->data = ctx;
}

HTTPParser::~HTTPParser()
{
	delete static_cast<HTTPParser_parser_ctx*>(p->psr->data);
	delete p->psr;
	delete p;
}

HTTPParser::Status HTTPParser::parseChunk(const char *data, std::size_t length)
{
	HTTPParser_parser_ctx *ctx = static_cast<HTTPParser_parser_ctx*>(p->psr->data);
	
	if(ctx->done)
	{
		ctx->done = false;
		ctx->req = 0;
	}
	
	std::size_t nparsed = http_parser_execute(p->psr, &ehttp_parser_parser_settings, data, length);
	if(p->psr->upgrade)
	{
		ctx->req->upgrade = true;
		return GotRequest;
	}
	else if(nparsed != length)
		return Error;
	
	return (ctx->done ? GotRequest : KeepGoing);
}

std::shared_ptr<HTTPRequest> HTTPParser::req()
{
	HTTPParser_parser_ctx *ctx = static_cast<HTTPParser_parser_ctx*>(p->psr->data);
	return ctx->req;
}



int ehttp_parser_on_message_begin(http_parser *psr)
{
	HTTPParser_parser_ctx *ctx = static_cast<HTTPParser_parser_ctx*>(psr->data);
	ctx->req = std::make_shared<HTTPRequest>();
	return 0;
}

int ehttp_parser_on_url(http_parser *psr, const char *data, size_t length)
{
	HTTPParser_parser_ctx *ctx = static_cast<HTTPParser_parser_ctx*>(psr->data);
	ctx->req->url += std::string(data, length);
	return 0;
}

int ehttp_parser_on_header_field(http_parser *psr, const char *data, size_t length)
{
	HTTPParser_parser_ctx *ctx = static_cast<HTTPParser_parser_ctx*>(psr->data);
	if(ctx->was_reading_header_value) ehttp_parser_push_header(psr);
	ctx->tmp_header_field += std::string(data, length);
	return 0;
}

int ehttp_parser_on_header_value(http_parser *psr, const char *data, size_t length)
{
	HTTPParser_parser_ctx *ctx = static_cast<HTTPParser_parser_ctx*>(psr->data);
	ctx->was_reading_header_value = true;
	ctx->tmp_header_value += std::string(data, length);
	return 0;
}

int ehttp_parser_on_headers_complete(http_parser *psr)
{
	//HTTPParser_parser_ctx *ctx = static_cast<HTTPParser_parser_ctx*>(psr->data);
	ehttp_parser_push_header(psr);
	return 0;
}

int ehttp_parser_on_body(http_parser *psr, const char *data, size_t length)
{
	HTTPParser_parser_ctx *ctx = static_cast<HTTPParser_parser_ctx*>(psr->data);
	ctx->req->body.reserve(ctx->req->body.size() + length);
	ctx->req->body.insert(ctx->req->body.end(), data, data + length);
	return 0;
}

int ehttp_parser_on_message_complete(http_parser *psr)
{
	HTTPParser_parser_ctx *ctx = static_cast<HTTPParser_parser_ctx*>(psr->data);
	ctx->req->method = http_method_str(static_cast<http_method>(psr->method));
	ctx->done = true;
	return 0;
}



void ehttp_parser_push_header(http_parser *psr)
{
	HTTPParser_parser_ctx *ctx = static_cast<HTTPParser_parser_ctx*>(psr->data);
	ctx->req->headers.insert(std::make_pair(ctx->tmp_header_field, ctx->tmp_header_value));
	ctx->tmp_header_field.clear();
	ctx->tmp_header_value.clear();
	ctx->was_reading_header_value = false;
}
