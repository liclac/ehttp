#include <unordered_map>
#include <stdexcept>
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



struct response::impl
{
	bool chunked, ended;
	
	impl():
		chunked(false), ended(false)
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

void response::begin(uint16_t code, std::string custom_reason)
{
	this->code = code;
	if(custom_reason.empty())
	{
		auto it = standard_statuses.find(code);
		if(it != standard_statuses.end()) reason = it->second;
		else reason = "???";
	} else reason = custom_reason;
}

void response::header(std::string name, std::string value)
{
	if(p->ended)
		throw new std::runtime_error("Attempted to modify already sent headers");
	
	headers.insert(std::pair<std::string,std::string>(name, value));
}

void response::write(const std::vector<char> &data)
{
	if(p->ended)
	{
		if(!p->chunked)
			throw new std::runtime_error("Attempted to write to an already written response");
		else
		{
			std::shared_ptr<chunk> chk = this->begin_chunk();
			chk->write(data);
			chk->end();
		}
	}
	else body.insert(body.end(), data.begin(), data.end());
}

void response::write(const std::string &data)
{
	this->write(std::vector<char>(data.begin(), data.end()));
}

void response::end(bool chunked)
{
	// Ignore attempts to end an already ended response
	if(p->ended)
		return;
	
	if(!on_end)
		throw new std::runtime_error("Response ended without an on_end handler");
	
	if(!p->chunked)
	{
		if(chunked)
		{
			this->header("Transfer-Encoding", "chunked");
			p->chunked = true;
			
			if(this->body.size() > 0)
			{
				std::shared_ptr<chunk> chk = this->begin_chunk();
				chk->write(this->body);
				this->body.clear();
				
				on_end(shared_from_this());
				chk->end();
			}
			else
				on_end(shared_from_this());
		}
		else
			on_end(shared_from_this());
	}
	else if(!chunked)
		on_end(shared_from_this());
	
	p->ended = true;
}

std::shared_ptr<response::chunk> response::begin_chunk()
{
	if(!p->chunked)
		throw new std::runtime_error("Attempted to begin a chunk on a non-chunked connection; call ehttp::response::end(true) first");
	
	return std::make_shared<chunk>(shared_from_this());
}



response::chunk::chunk(std::weak_ptr<response> res):
	res(res)
{
	
}

response::chunk::~chunk()
{
	
}

void response::chunk::write(const std::vector<char> &data)
{
	body.insert(body.end(), data.begin(), data.end());
}

void response::chunk::write(const std::string &data)
{
	this->write(std::vector<char>(data.begin(), data.end()));
}

void response::chunk::end()
{
	auto res_p = res.lock();
	if(!res_p->on_chunk)
		throw new std::runtime_error("Chunk ended without an on_chunk handler");
	
	res_p->on_chunk(res_p, shared_from_this());
}
