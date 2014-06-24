#include <map>
#include <regex>
#include <ehttp/router.h>

using namespace ehttp;

struct router::impl
{
	std::map<std::regex,handler_func> handlers;
};

router::router():
	p(new impl)
{
	
}
