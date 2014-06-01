#ifndef EHTTP_RESPONSE_H
#define EHTTP_RESPONSE_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <ehttp/private/_util.h>

namespace ehttp
{
	class request;
	class response : public std::enable_shared_from_this<response>
	{
	public:
		class chunk;
		
		response(std::shared_ptr<request> req = 0);
		virtual ~response();
		
		void begin(uint16_t code = 200, std::string custom_reason = "");
		std::shared_ptr<chunk> begin_chunk();
		
		void header(std::string name, std::string value);
		
		void write(const std::vector<char> &data);
		void write(const std::string &data);
		
		void end(bool chunked = false);
		
		
		
		std::shared_ptr<request> req;
		
		uint16_t code;
		std::string reason;
		std::map<std::string,std::string,ci_less> headers;
		std::vector<char> body;
		
		std::vector<char> to_http();
		
		
		
		std::function<void(std::shared_ptr<response> res)> on_end;
		std::function<void(std::shared_ptr<response> res, std::shared_ptr<chunk> chunk)> on_chunk;
		
	protected:
		struct impl;
		impl *p;
	};
	
	
	
	class response::chunk : public std::enable_shared_from_this<response::chunk>
	{
	public:
		chunk(std::weak_ptr<response> res);
		virtual ~chunk();
		
		void write(const std::vector<char> &data);
		void write(const std::string &data);
		
		void end();
		
		
		
		std::weak_ptr<response> res;
		std::vector<char> body;
		
		std::vector<char> to_http();
	};
}

#endif
