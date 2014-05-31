#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <iostream>
#include <ehttp/url.h>

TEST_CASE("url/complete_url", "Complete URL")
{
	std::string str("http://example.com/path/to/something?q1=abc&q2=xyz#fragment");
	ehttp::url url(str);
	REQUIRE(url.str() == str);
}

TEST_CASE("url/no_suffixes", "URL with no query or fragment")
{
	std::string str("http://example.com/path/to/something");
	ehttp::url url(str);
	REQUIRE(url.str() == str);
}

TEST_CASE("url/absolute_path", "URL with only an absolute path")
{
	std::string str("/path/to/something");
	ehttp::url url(str);
	REQUIRE(url.str() == str);
}

TEST_CASE("url/relative_path", "URL with only an relative path")
{
	std::string str("path/to/something");
	ehttp::url url(str);
	REQUIRE(url.str() == str);
}
