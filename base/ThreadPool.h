#pragma once 
#include <sys/types.h>
#include <vector>
#include <deque>
#include <string.h>
#include <functional>
#include "Condition.h"
#include "base/Mutex.h"
#include "base/Thread.h"
#include "base/noncopyable.h"

class ThreadPool : noncopyable{
	public:
		typedef std::function<void ()> Task;
		explicit ThreadPool(const std::string& name = std::string("ThreadPool"));
		~ThreadPool();
		
		void setMaxQueueSize(int maxsize) { maxQueueSize_ = maxsize; }
		void setThreadInitCallback(const Task& cb){
			threadInitCallback_ = cb;
		}
		void start(int numThreads);
		void stop();
		//size_t queueSize();
		const std::string& name() const { return name_; }
		void run(Task f);
		
	private:
		//bool isFull() const REQUIRES(mutex_);
		bool isFull();
		void runInThread();
		Task take();
		mutable MutexLock mutex_;
		Condition notEmpty_;
		Condition notFull_;
  		//Condition notEmpty_ GUARDED_BY(mutex_);
  		//Condition notFull_ GUARDED_BY(mutex_);
  		std::string name_;
  		Task threadInitCallback_; 
		std::vector<std::unique_ptr<Thread>> threads_;
  		std::deque<Task> queue_;
		//std::deque<Task> queue_ GUARDED_BY(mutex_);
  		size_t maxQueueSize_;
  		bool running_;
		
};
