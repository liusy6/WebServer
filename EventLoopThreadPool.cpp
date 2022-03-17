#include "EventLoopThreadPool.h"
#include <stdio.h>
#include <string>
#include "EventLoop.h"
#include "EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg)
  	:baseLoop_(baseLoop),
    name_(nameArg),
    started_(false),
    numThreads_(0),
    next_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool() { }

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
  	assert(!started_);
  	assert(baseLoop_->isInLoopThread()); 
  	started_ = true;
  	for (int i = 0; i < numThreads_; ++i){
    	char buf[name_.size() + 32];
    	EventLoopThread* t = new EventLoopThread(cb, buf);
    	threads_.push_back(std::unique_ptr<EventLoopThread>(t));
    	loops_.push_back(t->startLoop());
  	}
  	if (numThreads_ == 0 && cb) cb(baseLoop_);
}

EventLoop* EventLoopThreadPool::getNextLoop() {
  	assert(baseLoop_->isInLoopThread()); 
  	assert(started_);
  	EventLoop* loop = baseLoop_;

  	if (!loops_.empty()) {
    	loop = loops_[next_];
    	++next_;
  	}
  	return loop;
}

EventLoop* EventLoopThreadPool::getLoopForHash(size_t hashCode) {
  	assert(baseLoop_->isInLoopThread()); 
  	EventLoop* loop = baseLoop_;

  	if (!loops_.empty()) {
    	loop = loops_[hashCode % loops_.size()];
  	}
  	return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
  	assert(baseLoop_->isInLoopThread()); 
  	assert(started_);
  	if (loops_.empty()) {
    	return std::vector<EventLoop*>(1, baseLoop_);
  	}
  	else {
    	return loops_;
  	}
}





