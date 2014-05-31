#include <string>
#include <ehttp/parser.h>
#include <ehttp/request.h>
#include "../vendor/http-parser/http_parser.h"

using namespace ehttp;



int ehttp_parser_on_message_begin(http_parser *parser);
int ehttp_parser_on_url(http_parser *parser, const char *data, size_t length);
int ehttp_parser_on_header_field(http_parser *parser, const char *data, size_t length);
int ehttp_parser_on_header_value(http_parser *parser, const char *data, size_t length);
int ehttp_parser_on_headers_complete(http_parser *parser);
int ehttp_parser_on_body(http_parser *parser, const char *data, size_t length);
int ehttp_parser_on_message_complete(http_parser *parser);

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

struct ehttp_parser_parser_ctx
{
	request *request;
	
	std::string tmp_header_field, tmp_header_value;
	bool was_reading_header_value;
	
	parser *parser;
};

void ehttp_parser_push_header(http_parser *parser);
void ehttp_parser_emit_request(http_parser *parser);



struct parser::impl
{
	http_parser *parser;
};

parser::parser():
	p(new impl)
{
	p->parser = new http_parser;
	http_parser_init(p->parser, HTTP_REQUEST);
	
	ehttp_parser_parser_ctx *ctx = new ehttp_parser_parser_ctx;
	ctx->parser = this;
	ctx->was_reading_header_value = false;
	p->parser->data = ctx;
}

parser::~parser()
{
	delete static_cast<ehttp_parser_parser_ctx*>(p->parser->data);
	delete p->parser;
	delete p;
}

void parser::parse_chunk(void *data, std::size_t length)
{
	ehttp_parser_parser_ctx *ctx = static_cast<ehttp_parser_parser_ctx*>(p->parser->data);
	
	std::size_t nparsed = http_parser_execute(p->parser, &ehttp_parser_parser_settings, static_cast<char*>(data), length);
	if(p->parser->upgrade)
	{
		ctx->request->upgrade = true;
		ehttp_parser_emit_request(p->parser);
	}
	else if(nparsed != length)
	{
		delete ctx->request;
		if(on_error) on_error();
	}
}



int ehttp_parser_on_message_begin(http_parser *parser)
{
	ehttp_parser_parser_ctx *ctx = static_cast<ehttp_parser_parser_ctx*>(parser->data);
	ctx->request = new request;
	return 0;
}

int ehttp_parser_on_url(http_parser *parser, const char *data, size_t length)
{
	ehttp_parser_parser_ctx *ctx = static_cast<ehttp_parser_parser_ctx*>(parser->data);
	ctx->request->url += std::string(data, length);
	return 0;
}

int ehttp_parser_on_header_field(http_parser *parser, const char *data, size_t length)
{
	ehttp_parser_parser_ctx *ctx = static_cast<ehttp_parser_parser_ctx*>(parser->data);
	if(ctx->was_reading_header_value) ehttp_parser_push_header(parser);
	ctx->tmp_header_field += std::string(data, length);
	return 0;
}

int ehttp_parser_on_header_value(http_parser *parser, const char *data, size_t length)
{
	ehttp_parser_parser_ctx *ctx = static_cast<ehttp_parser_parser_ctx*>(parser->data);
	ctx->was_reading_header_value = true;
	ctx->tmp_header_value += std::string(data, length);
	return 0;
}

int ehttp_parser_on_headers_complete(http_parser *parser)
{
	//ehttp_parser_parser_ctx *ctx = static_cast<ehttp_parser_parser_ctx*>(parser->data);
	ehttp_parser_push_header(parser);
	return 0;
}

int ehttp_parser_on_body(http_parser *parser, const char *data, size_t length)
{
	ehttp_parser_parser_ctx *ctx = static_cast<ehttp_parser_parser_ctx*>(parser->data);
	ctx->request->body.reserve(ctx->request->body.size() + length);
	ctx->request->body.insert(ctx->request->body.end(), data, data + length);
	return 0;
}

int ehttp_parser_on_message_complete(http_parser *parser)
{
	//ehttp_parser_parser_ctx *ctx = static_cast<ehttp_parser_parser_ctx*>(parser->data);
	ehttp_parser_emit_request(parser);
	return 0;
}



void ehttp_parser_push_header(http_parser *parser)
{
	ehttp_parser_parser_ctx *ctx = static_cast<ehttp_parser_parser_ctx*>(parser->data);
	ctx->request->headers.insert(std::make_pair(ctx->tmp_header_field, ctx->tmp_header_value));
	ctx->tmp_header_field.clear();
	ctx->tmp_header_value.clear();
	ctx->was_reading_header_value = false;
}

void ehttp_parser_emit_request(http_parser *parser)
{
	ehttp_parser_parser_ctx *ctx = static_cast<ehttp_parser_parser_ctx*>(parser->data);
	ctx->parser->on_request(ctx->request);
	ctx->request = 0;
}
