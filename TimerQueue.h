#pragma once
#include <functional>
#include <set>
#include <vector>
#include "base/Mutex.h"
#include "base/noncopyable.h"
#include "base/Declare.h"
#include "Timestamp.h"
#include "Channel.h"

class TimerQueue : noncopyable{
	public:
		typedef std::function<void()> TimerCallback;
		TimerQueue(EventLoop* loop);
		~TimerQueue();
		long addTimer(TimerCallback cb, Timestamp when, double interval);
		//void cancel(long timerId);
		
	private:
		typedef std::pair<Timestamp, Timer*> Entry;
  		typedef std::set<Entry> TimerList;
		
		void addTimerInLoop(Timer* timer);
  		//void cancelInLoop(long timerId); 
		void handleRead();
		std::vector<Entry> getExpired(Timestamp now);
  		void reset(const std::vector<Entry>& expired, Timestamp now);
   		bool insert(Timer* timer);
   		
   		EventLoop* loop_;
   		const int timerfd_;
   		Channel timerfdChannel_;
   		TimerList timers_;
   		bool callingExpiredTimers_; 
		  	
};
