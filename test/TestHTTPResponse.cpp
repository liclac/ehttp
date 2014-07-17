#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <iostream>
#include <stdexcept>
#include <ehttp/HTTPResponse.h>

using namespace ehttp;

TEST_CASE("Test callback counts")
{
	std::shared_ptr<HTTPResponse> res = std::make_shared<HTTPResponse>();
	
	/*
	 * Every time onData or onEnd is called, increment a counter. If anything
	 * isn't working as expected, one of these numbers should be unexpected as
	 * well.
	 * 
	 * The end count should always be 1, or something is very wrong.
	 * 
	 * For a non-chunked response, the data count is also always 1.
	 * 
	 * For a chunked response, the data count should be equal to the number of
	 * calls to write() or endChunk(), plus one for the header section
	 * (written by a call to makeChunked() or before the first endChunk()),
	 * plus one for the terminating chunk (written by end()).
	 */
	int onDataCount = 0;
	res->onData = [&](std::shared_ptr<HTTPResponse> res, std::vector<char>) {
		++onDataCount;
	};
	
	int onEndCount = 0;
	res->onEnd = [&](std::shared_ptr<HTTPResponse> res) {
		++onEndCount;
	};
	
	/*
	 * Verifies that the onDataCount is what it's expected to be, and that
	 * onEndCount is 1 (since if it's not, you forgot a verifyAndReset(),
	 * or something isn't working properly), then reset the counters.
	 */
	auto verifyAndReset = [&](int expected_data_count) {
		CHECK(onDataCount == expected_data_count);
		REQUIRE(onEndCount == 1);
		
		onDataCount = 0;
		onEndCount = 0;
	};
	
	
	
	SECTION("Send a normal response")
	{
		res->begin()
			->header("Content-Type", "text/plain")
			->write("Lorem ipsum dolor sit amet")
			->end();
		
		verifyAndReset(1);
	}
	
	SECTION("Send a normal response in two writes")
	{
		res->begin()
			->header("Content-Type", "text/plain")
			->write("Lorem ipsum ")
			->write("dolor sit amet")
			->end();
		
		verifyAndReset(1);
	}
	
	SECTION("Send a response with makeChunked()")
	{
		res->begin()
			->header("Content-Type", "text/plain")
			->makeChunked()
			->write("Lorem ipsum ")
			->write("dolor sit amet")
			->end();
		
		verifyAndReset(1 + 2 + 1);
	}
	
	SECTION("Send a response with beginChunk() and write()")
	{
		res->begin()
			->header("Content-Type", "text/plain")
			->beginChunk()
				->write("Lorem ")
				->write("ipsum ")
				->endChunk()
			->beginChunk()
				->write("dolor ")
				->endChunk()
			->write("sit amet")
			->end();
		
		verifyAndReset(1 + 3 + 1);
	}
}

TEST_CASE("Test exceptions")
{
	std::shared_ptr<HTTPResponse> res = std::make_shared<HTTPResponse>();
	
	SECTION("Attempt end() with no callbacks")
	{
		res->begin();
		REQUIRE_THROWS_AS(res->end(), std::runtime_error);
	}
	
	SECTION("Attempt makeChunked() with no callbacks")
	{
		res->begin();
		REQUIRE_THROWS_AS(res->makeChunked(), std::runtime_error);
		REQUIRE_THROWS_AS(res->end(), std::runtime_error);
	}
	
	
	
	// Just set a callback that discards incoming data
	res->onData = [=](std::shared_ptr<HTTPResponse>, std::vector<char>) {};
	
	
	
	SECTION("Attempt to write to an ended response")
	{
		res->begin()->end();
		
		REQUIRE_THROWS_AS(res->write("Lorem ipsum dolor sit amet"), std::logic_error);
	}
	
	SECTION("Attempt to make an ended response chunked")
	{
		res->begin()->end();
		REQUIRE_THROWS_AS(res->makeChunked(), std::logic_error);
	}
	
	SECTION("Attempt to make chunks on an ended response")
	{
		res->begin()->end();
		
		REQUIRE_THROWS_AS(res->beginChunk(), std::logic_error);
	}
	
	SECTION("Attempt to write a chunk to an ended response")
	{
		res->begin();
		auto chk = res->beginChunk();
		res->end();
		
		chk->write("Lorem ipsum dolor sit amet");
		REQUIRE_THROWS_AS(chk->endChunk(), std::logic_error);
	}
}
