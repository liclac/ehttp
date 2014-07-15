#include <unordered_map>
#include <stdexcept>
#include <ehttp/HTTPResponse.h>

using namespace ehttp;

static const std::unordered_map<uint16_t,std::string> standard_statuses = {
	{100, "Continue"},
	{101, "Switching Protocols"},
	{102, "Processing"},
	
	{200, "OK"},
	{201, "Created"},
	{202, "Accepted"},
	{203, "Non-Authoritative Information"},
	{204, "No Content"},
	{205, "Reset Content"},
	{206, "Partial Content"},
	{207, "Multi-Status"},							// WebDAV
	{208, "Already Reported"},						// WebDAV
	{226, "IM Used"},								// WebDAV
	
	{300, "Multiple Choices"},
	{301, "Moved Permanently"},
	{302, "Found"},
	{303, "See Other"},
	{304, "Not Modified"},
	{305, "Use Proxy"},
	{306, "Switch Proxy"},
	{307, "Temporary Redirect"},
	{308, "Permanent Redirect"},
	
	{400, "Bad Request"},
	{401, "Unauthorized"},
	{402, "Payment Required"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{406, "Not Acceptable"},
	{407, "Proxy Authentication Required"},
	{408, "Request Timeout"},
	{409, "Conflict"},
	{410, "Gone"},
	{411, "Length Required"},
	{412, "Precondition Failed"},
	{413, "Request Entity Too Large"},
	{414, "Request-URI Too Long"},
	{415, "Unsupported Media Type"},
	{416, "Requested Range Not Satisfiable"},
	{417, "Expectation Failed"},
	{418, "I'm a teapot"},							// Best response code
	{419, "Authentication Timeout"},
	{420, "Enhance Your Calm"},						// Nonstandard, used by Twitter
	{422, "Unprocessable Entity"},					// WebDAV
	{423, "Locked"},								// WebDAV
	{424, "Failed Dependency"},						// WebDAV
	{425, "Unordered Collection"},					// WebDAV
	{426, "Upgrade Required"},
	{428, "Precondition Required"},
	{429, "Too Many Requests"},
	{431, "Request Header Fields Too Large"},
	{440, "Login Timeout"},							// Microsoft
	{444, "No Response"},							// Nginx, to ward off malicious request
	{449, "Retry With"},							// Microsoft
	{450, "Blocked by Windows Parental Controls"},	// Microsoft, why
	{451, "Unavailable for Legal Reasons"},
	{494, "Request Header Too Large"},				// Nginx, old status from pre-431
	
	{500, "Internal Server Error"},
	{501, "Not Implemented"},
	{502, "Bad Gateway"},
	{503, "Service Unavailable"},
	{504, "Gateway Timeout"},
	{505, "HTTP Version Not Supported"},
	{506, "Variant Also Negotiates"},
	{507, "Insufficient Storage"},					// WebDAV
	{508, "Loop Detected"},							// WebDAV
	{510, "Not Extended"},
	{511, "Network Authentication Required"}
};



/// \private
struct HTTPResponse::impl
{
	bool chunked, head_sent, body_sent, ended;
	
	impl():
		chunked(false), head_sent(false), body_sent(false), ended(false)
	{}
};

HTTPResponse::HTTPResponse(std::shared_ptr<HTTPRequest> req, std::function<void(std::shared_ptr<HTTPResponse> res, std::vector<char> data)> on_data, std::function<void(std::shared_ptr<HTTPResponse> res)> on_end):
	req(req),
	code(0),
	on_data(on_data), on_end(on_end),
	p(new impl)
{
	
}

HTTPResponse::~HTTPResponse()
{
	delete p;
}

std::shared_ptr<HTTPResponse> HTTPResponse::begin(uint16_t code, std::string custom_reason)
{
	if(code != 0)
	{
		this->code = code;
		if(custom_reason.empty())
		{
			auto it = standard_statuses.find(code);
			if(it != standard_statuses.end()) reason = it->second;
			else reason = "???";
		} else reason = custom_reason;
	}
	
	p->chunked = false;
	p->head_sent = false;
	p->body_sent = false;
	p->ended = false;
	
	headers.clear();
	body.clear();
	
	return shared_from_this();
}

std::shared_ptr<HTTPResponse> HTTPResponse::header(std::string name, std::string value)
{
	if(p->head_sent)
		throw std::logic_error("Attempted to modify already sent headers");
	
	headers.insert(std::pair<std::string,std::string>(name, value));
	
	return shared_from_this();
}

std::shared_ptr<HTTPResponse> HTTPResponse::write(const std::vector<char> &data)
{
	if(p->body_sent)
		throw std::logic_error("Attempted to write to an already sent response");
	
	if(!p->chunked)
		 body.insert(body.end(), data.begin(), data.end());
	else
		this->begin_chunk()
			->write(data)
			->end_chunk();
	
	return shared_from_this();
}

std::shared_ptr<HTTPResponse> HTTPResponse::write(const std::string &data)
{
	this->write(std::vector<char>(data.begin(), data.end()));
	return shared_from_this();
}

void HTTPResponse::end()
{
	// Ignore attempts to end an already ended response
	if(p->ended)
		return;
	
	if(!on_data)
		throw std::runtime_error("HTTPResponse::end() for non-chunked responses requires an on_data handler");
	
	p->ended = true;
	
	if(!p->chunked)
	{
		this->header("Content-Length", std::to_string(body.size()));
		
		p->head_sent = true;
		p->body_sent = true;
		
		event_data(shared_from_this(), this->to_http());
		event_end(shared_from_this());
	}
	else
	{
		// Chunked connections are terminated by an empty chunk
		event_data(shared_from_this(), chunk(shared_from_this()).to_http());
		event_end(shared_from_this());
	}
}

std::shared_ptr<HTTPResponse> HTTPResponse::make_chunked()
{
	// Ignore attempts to make an already chunked response chunked
	if(p->chunked)
		return shared_from_this();
	
	if(!on_data)
		throw std::runtime_error("HTTPResponse::make_chunked() requires an on_data handler");
	
	p->chunked = true;
	this->header("Transfer-Encoding", "chunked");
	
	p->head_sent = true;
	event_data(shared_from_this(), this->to_http(true));
	
	if(body.size() > 0)
	{
		std::shared_ptr<chunk> chk = this->begin_chunk();
		chk->write(body);
		body.clear();
		chk->end_chunk();
	}
	
	return shared_from_this();
}

std::shared_ptr<HTTPResponse::chunk> HTTPResponse::begin_chunk()
{
	// This will be documented if it ever becomes possible to make it happen
	if(p->head_sent && !p->chunked)
		throw std::logic_error("Attempted to begin a chunk on a nonchunked response");
	if(p->ended)
		throw std::logic_error("Attempted to begin a chunk on an ended response");
	
	return std::make_shared<chunk>(shared_from_this());
}

bool HTTPResponse::is_chunked() const
{
	return p->chunked;
}

std::vector<char> HTTPResponse::to_http(bool headers_only)
{
	std::stringstream ss;
	
	// Status line
	ss << "HTTP/1.1 " << code << " " << reason << "\r\n";
	
	// Generate the Date header on the fly
	ss << "Date: " << util::http_date() << "\r\n";
	
	// Headers
	for(auto it = headers.begin(); it != headers.end(); it++)
		ss << it->first << ": " << it->second << "\r\n";
	
	// A blank line terminates the header section
	ss << "\r\n";
	
	
	
	// Build a vector!
	std::string headers_str = ss.str();
	std::vector<char> data(headers_str.begin(), headers_str.end());
	
	// Append the body if we're not going for only the headers
	if(!headers_only)
		data.insert(data.end(), body.begin(), body.end());
	
	return data;
}



HTTPResponse::chunk::chunk(std::shared_ptr<HTTPResponse> res):
	res(res),
	ended(false)
{
	
}

HTTPResponse::chunk::~chunk()
{
	
}

std::shared_ptr<HTTPResponse::chunk> HTTPResponse::chunk::write(const std::vector<char> &data)
{
	if(ended)
		throw std::logic_error("Attempted to write to an already ended chunk");
	
	body.insert(body.end(), data.begin(), data.end());
	return shared_from_this();
}

std::shared_ptr<HTTPResponse::chunk> HTTPResponse::chunk::write(const std::string &data)
{
	this->write(std::vector<char>(data.begin(), data.end()));
	return shared_from_this();
}

std::shared_ptr<HTTPResponse> HTTPResponse::chunk::end_chunk()
{
	// Ignore attempts to end an already ended chunk
	if(ended)
		return res;
	
	// Ignore attempts to end an empty chunk, to prevent premature termination
	if(body.size() == 0)
		return res;
	
	if(!res->on_data)
		throw std::runtime_error("HTTPResponse::chunk::end() requires an on_data handler");
	
	ended = true;
	
	res->make_chunked();
	res->event_data(res, this->to_http());
	
	return res;
}

std::vector<char> HTTPResponse::chunk::to_http()
{
	/*
	 * Chunks are in the format:
	 * <size, hex without 0x>\r\n
	 * <data>\r\n
	 */
	
	std::string hex_size = util::to_hex(body.size());
	std::string crlf = "\r\n";
	
	std::vector<char> data;
	data.reserve(hex_size.size() + body.size() + crlf.size() * 2);
	data.insert(data.end(), hex_size.begin(), hex_size.end());
	data.insert(data.end(), crlf.begin(), crlf.end());
	data.insert(data.end(), body.begin(), body.end());
	data.insert(data.end(), crlf.begin(), crlf.end());
	
	return data;
}
