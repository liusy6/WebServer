#pragma once
#include <sys/types.h>
#include <functional>
#include <memory>
#include <vector>
#include <string>
#include "EventLoop.h"
#include "base/noncopyable.h"
#include "base/Declare.h"

class EventLoopThreadPool : noncopyable {
	public:
		typedef std::function<void(EventLoop*)> ThreadInitCallback;
		EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
  		~EventLoopThreadPool();
  		void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  		void start(const ThreadInitCallback& cb = ThreadInitCallback());
  		EventLoop* getNextLoop();
  		EventLoop* getLoopForHash(size_t hashCode);
  		std::vector<EventLoop*> getAllLoops();
  		bool started() const { return started_; }
  		const string& name() const { return name_; }
  	
	private:
		EventLoop* baseLoop_;
  		string name_;
  		bool started_;
  		int numThreads_;
  		int next_;
  		std::vector<std::unique_ptr<EventLoopThread>> threads_;
  		std::vector<EventLoop*> loops_;  	
  		
};
