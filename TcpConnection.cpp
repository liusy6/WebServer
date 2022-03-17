#include <sys/socket.h>
#include <errno.h>
#include <assert.h>
#include "TcpConnection.h"
#include "Channel.h"
#include "EventLoop.h"
#include "base/Util.h"

TcpConnection::TcpConnection(EventLoop* loop, const string& name,int sockfd, int port)
	:loop_(loop),
	name_(name),
	state_(kConnecting),
	reading_(true),
	sockfd_(sockfd),
	port_(port),
	channel_(new Channel(loop, sockfd))
{}
	
TcpConnection::~TcpConnection() {
	assert(state_ == kDisconnected);
}

void TcpConnection::shutdown() {
  if (state_ == kConnected) {
    setState(kDisconnecting);
    loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
  }
}

void TcpConnection::shutdownInLoop() {
  assert(loop_->isInLoopThread());
  if (!channel_->isWriting()) shutDownWR(sockfd_);
}

void TcpConnection::setTcpNoDelay(bool on) { setSocketNodelay(sockfd_); }

void TcpConnection::startRead() {
  	loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop() {
  	assert(loop_->isInLoopThread());
  	if (!reading_ || !channel_->isReading()) {
    	channel_->enableReading();
    	reading_ = true;
  	}
}

void TcpConnection::stopRead() { loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this)); }

void TcpConnection::stopReadInLoop() {
	assert(loop_->isInLoopThread());
  	if (reading_ || channel_->isReading()) {
    	channel_->disableReading();
    	reading_ = false;
  	}
}

void TcpConnection::connectEstablished() {
	assert(loop_->isInLoopThread());
  	assert(state_ == kConnecting);
  	setState(kConnected);
  	channel_->enableReading();
  	connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
	assert(loop_->isInLoopThread());
  	if (state_ == kConnected) {
    	setState(kDisconnected);
    	channel_->disableAll();
    	connectionCallback_(shared_from_this());
  	}
  	channel_->remove();
}

