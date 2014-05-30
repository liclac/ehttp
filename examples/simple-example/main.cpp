#include <ehttp/server.h>

int main(int argc, const char **argv)
{
	ehttp::server srv;
	srv.listen(8080);
	srv.run();
	
	return 0;
}
