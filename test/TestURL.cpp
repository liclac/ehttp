#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <iostream>
#include <ehttp/URL.h>

using namespace ehttp;

TEST_CASE("Complete URL")
{
	std::string str("http://example.com/path/to/something?q1=abc&q2=xyz#fragment");
	URL url(str);
	REQUIRE(url.str() == str);
}

TEST_CASE("URL with no query or fragment")
{
	std::string str("http://example.com/path/to/something");
	URL url(str);
	REQUIRE(url.str() == str);
}

TEST_CASE("URL with only an absolute path")
{
	std::string str("/path/to/something");
	URL url(str);
	REQUIRE(url.str() == str);
}

TEST_CASE("URL with only an relative path")
{
	std::string str("path/to/something");
	URL url(str);
	REQUIRE(url.str() == str);
}
