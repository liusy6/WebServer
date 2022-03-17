#include <assert.h>
#include <memory>
#include <sys/socket.h>
#include <stdio.h>
#include <string>
#include "Server.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"


const std::string name = "myWebServer";

template<typename T>
inline T* get_pointer(const std::shared_ptr<T>& ptr) { return ptr.get(); }

Server::Server(EventLoop* loop, int threadnum, int port)
	:loop_(loop),
	port_(port),
	threadnum_(threadnum),
	acceptor_(NULL),
	started_(false),
	threadPool_(new EventLoopThreadPool(loop, name)) {
		nextConnId_ = 1;
		threadPool_->setThreadNum(threadnum_);
	}
	
Server::~Server() {
	assert(loop_->isInLoopThread());
	
	for (auto& item : connections_) {
		TcpConnectionPtr conn(item.second);
		item.second.reset();
		conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
	}
}

void Server::start() {
	shared_ptr<Acceptor> acceptor(new Acceptor(loop_,port_));
	acceptor_ = acceptor;
	acceptor_->setNewConnectionCallback(std::bind(&Server::newConnection,this,std::placeholders::_1));
	acceptor_->start();
	started_ = true;
	if (!started_) {
		threadPool_->start(threadInitCallback_);
		assert(!acceptor_->listening());
		loop_->runInLoop(bind(&Acceptor::listen, get_pointer(acceptor_)));
	}
}

void Server::newConnection(int sockfd) {
	assert(loop_->isInLoopThread());
	EventLoop* ioLoop = threadPool_->getNextLoop();
	char buf[64];
	snprintf(buf, sizeof buf, "-%d#%d",port_,nextConnId_);
	string connName = buf;
	++nextConnId_;
	TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, port_));
	connections_[connName] = conn;
	conn->setConnectionCallback(connectionCallback_);
	conn->setCloseCallback(std::bind(&Server::removeConnection, this,std::placeholders::_1));
	ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void Server::removeConnection(const TcpConnectionPtr& conn) {
	loop_->runInLoop(std::bind(&Server::removeConnectionInLoop, this, conn));
}

void Server::removeConnectionInLoop(const TcpConnectionPtr& conn) {
	assert(loop_->isInLoopThread());
	EventLoop* ioLoop = conn->getLoop();
	ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}
