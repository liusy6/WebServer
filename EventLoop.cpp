#include <algorithm>
#include <sys/eventfd.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <vector>
#include "EventLoop.h"
#include "base/Mutex.h"
#include "Channel.h"
#include "EPoll.h"
#include "TimerQueue.h"
#include "base/Util.h"
#include "base/Define.h"

__thread EventLoop* t_loopInThisThread = 0;

EventLoop::EventLoop()
	:looping_(false),
	quit_(false),
	eventHandling_(false),
	callingPendingFunctors_(false),
	iteration_(0),
	threadId_(CurrentThread::tid()),
	poller_(new EPoll()),
	timerQueue_(new TimerQueue(this)),
	eventfd_(createEventfd()),
	wakeupChannel_(new Channel(this, eventfd_))
	{
		if (t_loopInThisThread) {
		}
		else{
			t_loopInThisThread = this;
		}
		wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead,this));
		wakeupChannel_->enableReading();
	}

EventLoop::~EventLoop() {
  	wakeupChannel_->disableAll();
  	wakeupChannel_->remove();
  	close(eventfd_);
  	t_loopInThisThread = NULL;
}

void EventLoop::loop() {
  	assert(!looping_);
  	assert(isInLoopThread());
  	looping_ = true;
  	quit_ = false;
  	std::vector<Channel*> channels;
  	while (!quit_) {
    	channels.clear();
    	pollReturnTime_ = poller_->poll(kPollTimeMs, &channels);
    	iteration_ = iteration_ + 1;
    	eventHandling_ = true;
    	for (auto& it : channels) it->handleEvent();
    	eventHandling_ = false;
    	doPendingFunctors();
  }
  	looping_ = false;
}

void EventLoop::quit() {
  	quit_ = true;
  	if (!isInLoopThread()) {
    	wakeup();
  	}
}

void EventLoop::runInLoop(Functor cb) {
  	if (isInLoopThread()) {
    	cb();
  	}
  	else {
    	queueInLoop(std::move(cb));
  	}
}

void EventLoop::queueInLoop(Functor cb) {
  	{
  		MutexLockGuard lock(mutex_);
  		pendingFunctors_.push_back(std::move(cb));
  	}

  	if (!isInLoopThread() || callingPendingFunctors_) {	wakeup(); }
}

size_t EventLoop::queueSize() const { 
  	MutexLockGuard lock(mutex_);
  	return pendingFunctors_.size();
}

/*void EventLoop::cancel(int timerfd){
	return timerQueue_->cancel(timerfd);
}*/

int EventLoop::runAt(Timestamp when, TimerCallback cb) {
    return timerQueue_->addTimer(std::move(cb), when, 0.0);
}

int EventLoop::runAfter(double delay, TimerCallback cb) {
	return timerQueue_->addTimer(std::move(cb), Timestamp::nowAfter(delay), 0.0);
}

int EventLoop::runEvery(double interval, TimerCallback cb) {
  	return timerQueue_->addTimer(std::move(cb), Timestamp::nowAfter(interval), interval);
}

void EventLoop::updateChannel(Channel* channel) {
  	assert(channel->ownerLoop() == this);
  	assert(isInLoopThread());
  	poller_->updateChannel(channel);
}

/*void EventLoop::removeChannel(Channel* channel)
{
  	assert(channel->ownerLoop() == this);
  	assert(isInLoopThread());
  	std::vector<SP_Channel> channels;
  	channels_.clear();
  	if (eventHandling_)
  	{
    	assert( channel == NULL ||
        	std::find(channels_.begin(), channels_.end(), channel) == channels_.end());
  	}
  	poller_->removeChannel(channel);
}*/

bool EventLoop::hasChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assert(isInLoopThread());
  return poller_->hasChannel(channel);
}

void EventLoop::wakeup() {
  	uint64_t one = 1;
  	ssize_t n = write(eventfd_, (char*)(&one), sizeof one);
  	if (n != sizeof one) {
  		
  	}
}

void EventLoop::handleRead() {
  	uint64_t one = 1;
  	ssize_t n = read(eventfd_, &one, sizeof one);
  	if (n != sizeof one)
  	{
    	
  	}
}

void EventLoop::doPendingFunctors() {
  	std::vector<Functor> functors;
  	callingPendingFunctors_ = true;
  	{
  		MutexLockGuard lock(mutex_);
  		functors.swap(pendingFunctors_);
  	}

  	for (const Functor& functor : functors) {
    	functor();
  	}
  	callingPendingFunctors_ = false;
}

int EventLoop::createEventfd() {
  	int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  	if (evtfd < 0){
    	abort();
  	}
  	return evtfd;
}


