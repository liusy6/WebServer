#pragma once
#include <string>
#include <assert.h>
#include <pthread.h>
#include "base/Condition.h"
#include "EventLoop.h"
#include "base/Mutex.h"
#include "base/Thread.h"
#include "base/noncopyable.h"
#include "base/Declare.h"

using namespace std;

class EventLoopThread : noncopyable{
	public:
		typedef std::function<void(EventLoop*)> ThreadInitCallback;
		EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(), const std::string& name = std::string());
		~EventLoopThread();
		EventLoop* startLoop();
		
	private:
		void threadFunc();
		
		//EventLoop* loop_  GUARDED_BY(mutex_);
		EventLoop* loop_;		
		bool exiting_;
  		Thread thread_;
		MutexLock mutex_;
		Condition cond_;
  		//Condition cond_ GUARDED_BY(mutex_);
  		ThreadInitCallback callback_;
		
		
};
