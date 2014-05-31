#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <iostream>
#include <ehttp/url.h>

TEST_CASE("url/complete_url", "Constructing a complete URL")
{
	std::string str("http://example.com/path/to/something?q1=abc&q2=xyz#fragment");
	ehttp::url url(str);
	REQUIRE(url.str() == str);
}
