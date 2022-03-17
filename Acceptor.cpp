#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "base/Util.h"
#include "Acceptor.h"
#include "EventLoop.h"

Acceptor::Acceptor(EventLoop* loop, int port)
	:loop_(loop),
	port_(port),
	listenfd_(NULL)
	{
	    listening_ = false;
	    listenfd_ = socket_bind_listen(port_);
	    handle_for_sigpipe();
	    if (setSocketNonBlocking(listenfd_) < 0) {
    		perror("set socket non block failed");
    		abort();
  	     }	
	    acceptChannel_ = new Channel(loop_, listenfd_);
	}
	
Acceptor::~Acceptor() {
	acceptChannel_->disableAll();
	acceptChannel_->remove();
}

void Acceptor::start() {
	acceptChannel_->setReadCallback(std::bind(&Acceptor::handleRead, this));   
}

void Acceptor::listen() {
	assert(loop_->isInLoopThread());
	listening_ = true;
	acceptChannel_->enableReading();
}	

void Acceptor::handleRead() {
	assert(loop_->isInLoopThread());
	int connfd;
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(struct sockaddr_in);
    connfd = accept(listenfd_, (sockaddr*)&cliaddr, (socklen_t*)&clilen);
    if (connfd >= 0){
    	if (newConnectionCallback_)
    	{
    		setSocketNonBlocking(connfd);
    		newConnectionCallback_(connfd);
		}
		else
		{
			close(connfd);
		}
	}
	else{
		
	}
}
