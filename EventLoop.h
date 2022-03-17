#pragma once
#include <atomic>
#include <functional>
#include <vector>
#include "base/noncopyable.h"
#include "EPoll.h"
#include "base/Mutex.h"
#include "base/CurrentThread.h"
#include "Timestamp.h"
#include "base/Declare.h"


class EventLoop : noncopyable{
	public:
		typedef std::function<void()> TimerCallback;
		typedef std::function<void()> Functor;
		EventLoop();
		~EventLoop();
		void loop();
		void quit();
	
		Timestamp pollReturnTime() const { return pollReturnTime_; }
		int64_t iteration() const { return iteration_; }
		void runInLoop(Functor cb);
		void queueInLoop(Functor cb);
		size_t queueSize() const;
		int runAt(Timestamp when, TimerCallback cb);
		int runAfter(double delay, TimerCallback cb);
        int runEvery(double interval, TimerCallback cb);
		//void cancel(int timerfd);
		void wakeup();
		void updateChannel(Channel* channel);
  		//void removeChannel(Channel* channel);
  		bool hasChannel(Channel* channel);
  		
  		bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
  		
  		private:
  			void handleRead();
  			void doPendingFunctors();
  			typedef std::vector<Channel*> ChannelList;
  			bool looping_;
			std::atomic<bool> quit_;
  			bool eventHandling_;
  			bool callingPendingFunctors_;
  			int64_t iteration_;
  			const pid_t threadId_;
			std::unique_ptr<EPoll> poller_;
			std::unique_ptr<TimerQueue> timerQueue_;
  			Timestamp pollReturnTime_;		
  			int eventfd_;
  			int createEventfd();
			std::unique_ptr<Channel> wakeupChannel_;
			mutable MutexLock mutex_;
			std::vector<Functor> pendingFunctors_;
		
};

