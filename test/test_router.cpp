#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <iostream>
#include <ehttp/erouter.h>

using namespace ehttp;

/*
 * There are so many macroes here, but seriously, the file would be miles long
 * and bloody impossible to debug if I had to type all this over and over...
 */

#define REGISTER_ERROR_HANDLER(_code, _var, _text) \
	bool _var = false; \
	rtr.on_error(_code, [&](std::shared_ptr<erequest> req, std::shared_ptr<eresponse> res) { \
		res->begin(0)->header("Content-Type", "text/plain")->write(_text)->end(); \
		_var = true; \
	})

#define REGISTER_HANDLER(_method, _path, _var, _code, _text) \
	bool _var = false; \
	rtr.on(_method, _path, [&](std::shared_ptr<erequest> req, std::shared_ptr<eresponse> res) { \
		res->begin(_code)->header("Content-Type", "text/plain")->write(_text)->end(); \
		_var = true; \
	})

#define ROUTE_REQUEST(_method, _path) \
	rtr.route(std::make_shared<erequest>(_method, _path), std::make_shared<eresponse>(nullptr, on_data, on_end))

#define REQUIRE_AND_RESET(_var) \
	CHECK(_var == true); \
	_var = false

#define TEST_REQUEST(_method, _path, _code, _var) \
	ROUTE_REQUEST(_method, _path); \
	REQUIRE_AND_RESET(_var); \
	CHECK(last_code == _code)

TEST_CASE("Test callbacks")
{
	erouter rtr;
	uint16_t last_code = 0;
	
	REGISTER_ERROR_HANDLER(403, forbidden_called, "Can't let you do that, Star Fox!");
	REGISTER_ERROR_HANDLER(404, not_found_called, "These aren't the routes you're looking for");
	
	REGISTER_HANDLER("GET", "/", root_called, 200, "Lorem ipsum");
	REGISTER_HANDLER("GET", "/path/", second_called, 200, "Dolor sit amet");
	REGISTER_HANDLER("GET", "/path/to/", third_called, 200, "Consectetur adipiscing");
	REGISTER_HANDLER("GET", "/path/to/something", fourth_called, 200, "Elit pellentesque");
	REGISTER_HANDLER("GET", "/error", error_called, 403, "");
	REGISTER_HANDLER("GET", "/nothing", nothing_called, 404, "");
	
	auto on_data = [&](std::shared_ptr<eresponse> res, std::vector<char> data) {};
	auto on_end = [&](std::shared_ptr<eresponse> res) {
		last_code = res->code;
	};
	
	SECTION("Test normal handlers")
	{
		TEST_REQUEST("GET", "/", 200, root_called);
		TEST_REQUEST("GET", "/path/", 200, second_called);
		TEST_REQUEST("GET", "/path/to/", 200, third_called);
		TEST_REQUEST("GET", "/path/to/something", 200, fourth_called);
	}
	
	SECTION("Test trailing slash mismatch")
	{
		TEST_REQUEST("GET", "/path", 200, second_called);
		TEST_REQUEST("GET", "/path/to", 200, third_called);
		TEST_REQUEST("GET", "/path/to/something/", 200, fourth_called);
	}
	
	SECTION("Test error handlers")
	{	
		TEST_REQUEST("GET", "/error", 403, error_called);
		REQUIRE_AND_RESET(forbidden_called);
		
		TEST_REQUEST("GET", "/nothing", 404, nothing_called);
		REQUIRE_AND_RESET(not_found_called);
	}
	
	SECTION("Test fallback handler")
	{
		TEST_REQUEST("GET", "/asdfghjkl", 404, not_found_called);
	}
}
