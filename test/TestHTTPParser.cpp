#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <iostream>
#include <ehttp/HTTPParser.h>

using namespace ehttp;

const char *valid_GET =
	"GET /path?q1=abc&q2=123 HTTP/1.1\r\n"
	"Host: example.com\r\n"
	"\r\n";

const char *valid_POST =
	"POST /something/ HTTP/1.1\r\n"
	"Host: example.net\r\n"
	"Content-Length: 26\r\n"
	"\r\n"
	"Lorem ipsum dolor sit amet";

const char *junk =
	"Lorem ipsum dolor sit amet, consectetur adipiscing elit.\r\n"
	"Aenean lobortis eros et augue mattis, et rutrum tellus pulvinar.\r\n"
	"Fusce gravida tincidunt felis non cursus.\r\n"
	"Vestibulum semper nunc id varius facilisis.\r\n"
	"Fusce faucibus convallis sodales.\r\n"
	"Donec mauris purus, aliquet sed malesuada ac, malesuada non ipsum.\r\n"
	"Cras commodo justo quis ante accumsan, nec cursus magna consectetur.\r\n"
	"Integer pretium porta gravida. Ut eget egestas arcu.\r\n";



TEST_CASE("Parsing GET requests")
{
	HTTPParser p;
	
	SECTION("Parsing a valid request")
	{
		REQUIRE(p.parseChunk(valid_GET, strlen(valid_GET)) == HTTPParser::GotRequest);
		
		std::shared_ptr<HTTPRequest> req = p.req();
		CHECK(req->method == "GET");
		CHECK(req->url == "/path?q1=abc&q2=123");
		CHECK(req->headers["Host"] == "example.com");
	}
	
	SECTION("Parsing some more requests in a sequence")
	{
		REQUIRE(p.parseChunk(valid_GET, strlen(valid_GET)) == HTTPParser::GotRequest);
		REQUIRE(p.parseChunk(valid_GET, strlen(valid_GET)) == HTTPParser::GotRequest);
	}
}

TEST_CASE("Parsing POST requests")
{
	HTTPParser p;
	
	SECTION("Parsing a valid request")
	{
		REQUIRE(p.parseChunk(valid_POST, strlen(valid_POST)) == HTTPParser::GotRequest);
		
		std::shared_ptr<HTTPRequest> req = p.req();
		CHECK(req->method == "POST");
		CHECK(req->url == "/something/");
		CHECK(req->headers["Host"] == "example.net");
		CHECK(std::string(req->body.begin(), req->body.end()) == "Lorem ipsum dolor sit amet");
	}
	
	SECTION("Parsing some more requests in a sequence")
	{
		REQUIRE(p.parseChunk(valid_POST, strlen(valid_POST)) == HTTPParser::GotRequest);
		REQUIRE(p.parseChunk(valid_POST, strlen(valid_POST)) == HTTPParser::GotRequest);
	}
}

TEST_CASE("Parsing invalid requests")
{
	HTTPParser p;
	
	SECTION("Parsing some junk data")
	{
		REQUIRE(p.parseChunk(junk, strlen(junk)) == HTTPParser::Error);
	}
	
	SECTION("Parsing some valid requests afterwards")
	{
		REQUIRE(p.parseChunk(valid_GET, strlen(valid_GET)) == HTTPParser::GotRequest);
		REQUIRE(p.parseChunk(valid_POST, strlen(valid_POST)) == HTTPParser::GotRequest);
	}
}
