#include <unordered_map>
#include <stdexcept>
#include <sstream>
#include <ehttp/response.h>

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
struct response::impl
{
	bool chunked, head_sent, body_sent, ended;
	
	impl():
		chunked(false), head_sent(false), body_sent(false), ended(false)
	{}
};

response::response(std::shared_ptr<request> req):
	req(req),
	code(0),
	p(new impl)
{
	
}

response::~response()
{
	delete p;
}

std::shared_ptr<response> response::begin(uint16_t code, std::string custom_reason)
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
	else
	{
		if(p->ended)
			throw std::logic_error("Can't reuse an already sent response");
		if(p->head_sent || p->body_sent)
			throw std::logic_error("Can't reuse a partially sent response");
		if(p->chunked)
			throw std::logic_error("Can't reuse a chunked response");
		
		headers.clear();
		body.clear();
	}
	
	return shared_from_this();
}

std::shared_ptr<response> response::header(std::string name, std::string value)
{
	if(p->head_sent)
		throw std::logic_error("Attempted to modify already sent headers");
	
	headers.insert(std::pair<std::string,std::string>(name, value));
	
	return shared_from_this();
}

std::shared_ptr<response> response::write(const std::vector<char> &data)
{
	if(!p->body_sent)
	{
		if(!p->chunked)
		{
			 body.insert(body.end(), data.begin(), data.end());
		}
		else
		{
			this->begin_chunk()
				->write(data)
				->end_chunk();
		}
	}
	else
	{
		throw std::logic_error("Attempted to write to an already sent response");
	}
	
	return shared_from_this();
}

std::shared_ptr<response> response::write(const std::string &data)
{
	this->write(std::vector<char>(data.begin(), data.end()));
	return shared_from_this();
}

void response::end()
{
	// Ignore attempts to end an already ended response
	if(p->ended)
		return;
	
	if(!p->chunked)
	{
		if(!on_head)
			throw std::runtime_error("response::end() for non-chunked responses requires an on_head handler");
		if(!on_body)
			throw std::runtime_error("response::end() for non-chunked responses requires an on_body handler");
		
		this->header("Content-Length", std::to_string(body.size()));
		
		event_head(shared_from_this(), this->to_http(true));
		p->head_sent = true;
		
		event_body(shared_from_this(), body);
		p->body_sent = true;
		
		event_end(shared_from_this());
	}
	else
	{
		if(!on_chunk)
			throw std::runtime_error("response::end() for chunked responses requires an on_chunk handler");
		
		// Chunked connections are terminated by an empty chunk
		event_chunk(shared_from_this(), this->begin_chunk(), std::vector<char>());
		
		event_end(shared_from_this());
	}
	
	p->ended = true;
}

std::shared_ptr<response> response::make_chunked()
{
	// Ignore attempts to make an already chunked response chunked
	if(p->chunked)
		return shared_from_this();
	
	if(!on_head)
		throw std::runtime_error("response::make_chunked() requires an on_head handler");
	
	p->chunked = true;
	this->header("Transfer-Encoding", "chunked");
	
	event_head(shared_from_this(), this->to_http(true));
	p->head_sent = true;
	
	if(this->body.size() > 0)
	{
		std::shared_ptr<chunk> chk = this->begin_chunk();
		chk->write(this->body);
		this->body.clear();
		chk->end_chunk();
	}
	
	return shared_from_this();
}

std::shared_ptr<response::chunk> response::begin_chunk()
{
	return std::make_shared<chunk>(shared_from_this());
}

std::vector<char> response::to_http(bool headers_only)
{
	// @todo Skip the whole stringstream step and all the copying
	std::stringstream ss;
	
	ss << "HTTP/1.1 " << code << " " << reason << "\r\n";
	for(auto it = headers.begin(); it != headers.end(); it++)
		ss << it->first << ": " << it->second << "\r\n";
	ss << "\r\n";
	
	if(!headers_only)
		ss << std::string(body.begin(), body.end());
	
	std::string str = ss.str();
	return std::vector<char>(str.begin(), str.end());
}



response::chunk::chunk(std::shared_ptr<response> res):
	res(res)
{
	
}

response::chunk::~chunk()
{
	
}

std::shared_ptr<response::chunk> response::chunk::write(const std::vector<char> &data)
{
	body.insert(body.end(), data.begin(), data.end());
	return shared_from_this();
}

std::shared_ptr<response::chunk> response::chunk::write(const std::string &data)
{
	this->write(std::vector<char>(data.begin(), data.end()));
	return shared_from_this();
}

std::shared_ptr<response> response::chunk::end_chunk()
{
	if(!res->on_chunk)
		throw std::runtime_error("response::chunk::end() requires an on_chunk handler");
	
	res->make_chunked();
	res->event_chunk(res, shared_from_this(), body);
	return res;
}

std::vector<char> response::chunk::to_http()
{
	/// @todo Skip the whole stringstream step and all the copying
	std::stringstream ss;
	
	ss << std::hex << body.size() << "\r\n";
	ss << std::string(body.begin(), body.end()) << "\r\n";
	
	std::string str = ss.str();
	return std::vector<char>(str.begin(), str.end());
}
