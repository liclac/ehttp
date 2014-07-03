#ifndef EHTTP_RESPONSE_H
#define EHTTP_RESPONSE_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include "request.h"
#include "util.h"

namespace ehttp
{
	/**
	 * Represents an HTTP Response.
	 * 
	 * Unlike \ref request, response is more of a generator than a simple
	 * container. The reason for this is obviously that our main order of
	 * business here is generating responses, rather than requests.
	 * 
	 * The API is designed to make it easy to chain calls. For example, a
	 * simple response can be generated with:
	 * 
	 *     res->begin()
	 *         ->header("Content-Type", "text/plain")
	 *         ->write("Lorem ipsum dolor sit amet")
	 *         ->end()
	 * 
	 * To make a chunked response, either use begin_chunk(), chunk::write() and
	 * chunk::end_chunk(), or call make_chunked(), which will cause all
	 * sequential write() calls to write chunks.
	 * 
	 * You don't have to care whether a response is chunked or not at any given
	 * time:\n
	 * Non-chunked calls will automatically write chunks if the response is
	 * chunked, otherwise add to a buffer.\n
	 * Chunked calls will make the response chunked if it's not, immediately
	 * writing a chunk consisting of any data written using non-chunked calls.
	 */
	class response : public std::enable_shared_from_this<response>
	{
	public:
		class chunk;
		
		/**
		 * Constructor
		 * @param req The request we're responding to, for context
		 */
		response(std::shared_ptr<request> req = 0);
		virtual ~response();
		
		/**
		 * Begins a response with the given status code, and optionally a
		 * custom reason string. If a custom reason is not given, the standard
		 * reason phrase associated with the response code ("OK" for 200, "Not
		 * Found" for 404, "Internal Server Error" for 500, etc.) is used.
		 * 
		 * Calling `begin()` will clear the response if there is data in it
		 * already; you should only use this if you know it's safe, and be sure
		 * to discard any buffers.
		 * 
		 * A far more useful thing to do is to call `begin(0)`, which will
		 * clear only the contents, while keeping the status code and reason
		 * phrase. This is intended for use with status handlers, as is done in
		 * \ref router.
		 */
		std::shared_ptr<response> begin(uint16_t code = 200, std::string custom_reason = "");
		
		/** 
		 * Sets a header, overwriting any previous value.
		 * 
		 * @throws std::logic_error if end() has already been called.
		 */
		std::shared_ptr<response> header(std::string name, std::string value);
		
		/**
		 * Appends some data to the response body.
		 * 
		 * If the response is not chunked, the data is appended to the #body
		 * buffer. Otherwise, a chunk is written containing the data.
		 * 
		 * @throws std::logic_error if the body has already been written, and
		 * the response is not chunked.
		 */
		std::shared_ptr<response> write(const std::vector<char> &data);
		/// @overload
		std::shared_ptr<response> write(const std::string &data);
		
		/**
		 * Finalizes the response and calls #on_end if present.
		 * 
		 * @throws std::runtime_error if #on_data is NULL.
		 */
		void end();
		
		/**
		 * Makes the response chunked.
		 * 
		 * This is automatically called from chunk::end_chunk(), and sets a
		 * flag that will make any subsequent calls to write() send chunks
		 * rather than append to the #body buffer.
		 * 
		 * If there is data in #body, it's written out as a chunk first.
		 * 
		 * @throws std::runtime_error if #on_data is NULL.
		 */
		std::shared_ptr<response> make_chunked();
		
		/**
		 * Begins a chunk.
		 * 
		 * Use chunk::end_chunk() to end it and write it out.
		 * 
		 * There is no reference counting for this or anything - if you realize
		 * after you've begun a chunk that you don't actually need to send it,
		 * just don't call chunk::end_chunk() on it.\n
		 * Chunks keep a std::shared_ptr to their parent response, which means
		 * that the response will not be destroyed until all of its chunks are.
		 * 
		 * @throws std::logic_error if the response has already ended
		 */
		std::shared_ptr<chunk> begin_chunk();
		
		/**
		 * Is the current response chunked?
		 */
		bool is_chunked() const;
		
		
		
		/**
		 * Returns the response formatted according to the HTTP specification.
		 * For chunked responses, only the header will be formatted - chunks
		 * are responsible for formatting themselves.
		 * 
		 * @param headers_only Only include headers, not the body.
		 */
		std::vector<char> to_http(bool headers_only = false);
		
		
		
		/// The request we're responding to, for context
		std::shared_ptr<request> req;
		
		/// HTTP Status Code (eg. 200, 404, 500, ...)
		uint16_t code;
		/// Reason phrase for the status
		std::string reason;
		/// Response headers
		std::map<std::string,std::string,util::ci_less> headers;
		/// Response body for non-chunked transfers
		std::vector<char> body;
		
		
		
		/**
		 * Called whenever there is data to be written.
		 * 
		 * @param res The response
		 * @param chunk The associated chunk, a null pointer if there is none
		 * @param data HTTP-formatted data, ready to be written to a stream
		 */
		std::function<void(std::shared_ptr<response> res, std::vector<char> data)> on_data;
		
		/**
		 * Called when a response has ended.
		 * 
		 * @see end()
		 * 
		 * @param res The response
		 */
		std::function<void(std::shared_ptr<response> res)> on_end;
		
	protected:
		/// Overridable emitter for #on_data
		virtual void event_data(std::shared_ptr<response> res, std::vector<char> data) {
			if(on_data) on_data(res, data);
		}
		
		/// Overridable emitter for #on_end
		virtual void event_end(std::shared_ptr<response> res) {
			if(on_end) on_end(res);
		}
		
	private:
		struct impl;
		impl *p;
	};
	
	
	
	/**
	 * Represents a chunk in a \ref response.
	 */
	class response::chunk : public std::enable_shared_from_this<response::chunk>
	{
	public:
		/**
		 * Constructor, typically not called directly.
		 * Instead, you should use response::begin_chunk() to create chunks.
		 * This is exposed mainly for unit testing purposes.
		 * @param res The response the chunk is part of
		 */
		chunk(std::shared_ptr<response> res);
		virtual ~chunk();
		
		/**
		 * Appends data to the chunk body.
		 * 
		 * @throws std::logic_error if end() has already been called.
		 */
		std::shared_ptr<response::chunk> write(const std::vector<char> &data);
		/// @overload
		std::shared_ptr<response::chunk> write(const std::string &data);
		
		/**
		 * Ends the chunk.
		 * 
		 * Attempts to end an empty chunk will be ignored, as empty chunks mark
		 * the end of a chunked transfer, and thus writing an empty chunk by
		 * accident would be bad.\n
		 * Use response::end() to write the terminating chunk instead.
		 * 
		 * Named like this to avoid confusion with response::end() when
		 * chaining calls. For example:
		 * 
		 *     res->begin()
		 *         ->header("Content-Type", "text/plain")
		 *         ->begin_chunk()
		 *             ->write("Lorem ipsum dolor sit amet")
		 *             //Forgot to do ->end_chunk()!
		 *         ->end()
		 * 
		 * If this was named "end" as well, this would cause the response to
		 * never finish, as the chunk would be ended but the response would
		 * not - the user's browser would wait forever for the terminating
		 * chunk that was meant to be generated by response::end(). Instead,
		 * this now causes a compile-time error.
		 * 
		 * @throws std::runtime_error if response::on_data in #res isn't set.
		 */
		std::shared_ptr<response> end_chunk();
		
		
		
		/// Returns the chunk formatted according to the HTTP specification.
		std::vector<char> to_http();
		
		
		
		/// The response the chunk is a part of
		std::shared_ptr<response> res;
		/// The chunk body
		std::vector<char> body;
		
	protected:
		/// Whether the chunk has already ended
		bool ended;
	};
}

#endif
