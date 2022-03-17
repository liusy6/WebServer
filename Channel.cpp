#include <sstream>
#include <poll.h>
#include <assert.h>
#include "Channel.h"
#include "EventLoop.h"

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
	:loop_(loop),
	fd_(fd),
	events_(0),
	revents_(0),
	index_(-1),
	addedToLoop_(false) { }
	
Channel::~Channel() {
	assert(!addedToLoop_);
	if (loop_->isInLoopThread()) {
		assert(!loop_->hasChannel(this));
	}
}

void Channel::update() {
	addedToLoop_ = true;
	loop_->updateChannel(this);
}

void Channel::remove() {
  	assert(isNoneEvent());
  	addedToLoop_ = false;
  	//loop_->removeChannel(this);
}



