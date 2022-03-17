#include "EventLoop.h"
#include "Server.h"

int main(int args, char** argv)
{
	EventLoop loop;
    Server server(&loop, 4, 4000);
    server.start();
    loop.loop();
    return 0;
}
