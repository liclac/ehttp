#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <iostream>
#include <ehttp/response.h>

using namespace ehttp;

TEST_CASE("Test callback counts")
{
	std::shared_ptr<response> res = std::make_shared<response>();
	
	/*
	 * Every time on_data or on_end is called, increment a counter. If anything
	 * isn't working as expected, one of these numbers should be unexpected as
	 * well.
	 * 
	 * The end count should always be 1, or something is very wrong.
	 * 
	 * For a non-chunked response, the data count is also always 1.
	 * 
	 * For a chunked response, the data count should be equal to the number of
	 * calls to write() or end_chunk(), plus one for the header section
	 * (written by a call to make_chunked() or before the first end_chunk()),
	 * plus one for the terminating chunk (written by end()).
	 */
	int on_data_count = 0;
	res->on_data = [&](std::shared_ptr<response> res, std::vector<char>) {
		++on_data_count;
	};
	
	int on_end_count = 0;
	res->on_end = [&](std::shared_ptr<response> res) {
		++on_end_count;
	};
	
	/*
	 * Verifies that the on_data_count is what it's expected to be, and that
	 * on_end_count is 1 (since if it's not, you forgot a verify_and_reset(),
	 * or something isn't working properly), then reset the counters.
	 */
	auto verify_and_reset = [&](int expected_data_count) {
		CHECK(on_data_count == expected_data_count);
		REQUIRE(on_end_count == 1);
		
		on_data_count = 0;
		on_end_count = 0;
	};
	
	
	
	SECTION("Send a normal response")
	{
		res->begin()
			->header("Content-Type", "text/plain")
			->write("Lorem ipsum dolor sit amet")
			->end();
		
		verify_and_reset(1);
	}
	
	SECTION("Send a normal response in two writes")
	{
		res->begin()
			->header("Content-Type", "text/plain")
			->write("Lorem ipsum ")
			->write("dolor sit amet")
			->end();
		
		verify_and_reset(1);
	}
	
	SECTION("Send a response with make_chunked()")
	{
		res->begin()
			->header("Content-Type", "text/plain")
			->make_chunked()
			->write("Lorem ipsum ")
			->write("dolor sit amet")
			->end();
		
		verify_and_reset(1 + 2 + 1);
	}
	
	SECTION("Send a response with begin_chunk() and write()")
	{
		res->begin()
			->header("Content-Type", "text/plain")
			->begin_chunk()
				->write("Lorem ")
				->write("ipsum ")
				->end_chunk()
			->begin_chunk()
				->write("dolor ")
				->end_chunk()
			->write("sit amet")
			->end();
		
		verify_and_reset(1 + 3 + 1);
	}
}
