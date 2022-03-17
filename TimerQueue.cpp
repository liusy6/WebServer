#include "TimerQueue.h"
#include <assert.h>
#include <sys/timerfd.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include "EventLoop.h"
#include "Timer.h"

inline void memZero(void* p, size_t n) { memset(p, 0, n); }

int createTimerfd() {
	int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                 TFD_NONBLOCK | TFD_CLOEXEC);
  	if (timerfd < 0){
   		std::cout << "failed in timerfd_create" << endl;
  	}
  	return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when) {
  	int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
  	if (microseconds < 100)	microseconds = 100;
  	struct timespec ts;
  	ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
  	ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
  	return ts;
}

void resetTimerfd(int timerfd, Timestamp expiration) {
  	struct itimerspec newValue;
  	struct itimerspec oldValue;
  	memZero(&newValue, sizeof newValue);
  	memZero(&oldValue, sizeof oldValue);
  	newValue.it_value = howMuchTimeFromNow(expiration);
  	int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
  	if (ret) {
  		std::cout << "timerfd_settime() failed" << endl;
  	}
}

TimerQueue::TimerQueue(EventLoop* loop)
	:loop_(loop),
	timerfd_(createTimerfd()),
	timerfdChannel_(loop, timerfd_),
	timers_(),
	callingExpiredTimers_(false)
	{
		timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this));
		timerfdChannel_.enableReading();
	}

TimerQueue::~TimerQueue() {
	timerfdChannel_.disableAll();
	timerfdChannel_.remove();
	close(timerfd_);
	for (const Entry& timer : timers_)  delete timer.second;
}

long TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval) {
	Timer* timer = new Timer(std::move(cb), when, interval);
	loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop,this,timer));
	return (long)(timer);
}

void TimerQueue::addTimerInLoop(Timer* timer) {
  	assert(loop_->isInLoopThread());
  	bool earliestChanged = insert(timer);
  	if (earliestChanged) {
    	resetTimerfd(timerfd_, timer->expiration());
  	}
}

void TimerQueue::handleRead() {
  	assert(loop_->isInLoopThread());
  	Timestamp now(Timestamp::now());
  	//readTimerfd(timerfd_, now);

  	std::vector<Entry> expired = getExpired(now);

  	callingExpiredTimers_ = true;
  	//cancelingTimers_.clear();
  	for (const Entry& it : expired) {
    	it.second->run();
  	}
  	callingExpiredTimers_ = false;
  	reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
  	std::vector<Entry> expired;
  	Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
  	TimerList::iterator end = timers_.lower_bound(sentry);
  	assert(end == timers_.end() || now < end->first);
  	std::copy(timers_.begin(), end, back_inserter(expired));
  	timers_.erase(timers_.begin(), end);
  	return expired;
  }

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now) {
  	Timestamp nextExpire;

  	for (const Entry& it : expired) {
    	//ActiveTimer timer(it.second, it.second->sequence());
    	if (it.second->repeat())
        	//&& cancelingTimers_.find(timer) == cancelingTimers_.end())
    	{
      		it.second->restart(now);
      		insert(it.second);
    	}
    	else {
      		delete it.second; 
    	}
  	}

  	if (!timers_.empty()) {
    	nextExpire = timers_.begin()->second->expiration();
  	}

  	if (nextExpire.valid()) {
    	resetTimerfd(timerfd_, nextExpire);
  	}
}

bool TimerQueue::insert(Timer* timer) {
  	assert(loop_->isInLoopThread());
  	bool earliestChanged = false;
  	Timestamp when = timer->expiration();
  	TimerList::iterator it = timers_.begin();
  	if (it == timers_.end() || when < it->first) earliestChanged = true;
  	{
    	std::pair<TimerList::iterator, bool> result = timers_.insert(Entry(when, timer));
    	assert(result.second); (void)result;
  	}
  	return earliestChanged;
}
