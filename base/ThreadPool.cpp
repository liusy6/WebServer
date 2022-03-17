#include "ThreadPool.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <base/Mutex.h>
#include "base/Thread.h"
#include "base/Declare.h"

ThreadPool::ThreadPool(const std::string& name)
	:mutex_(),
	notEmpty_(mutex_),
	notFull_(mutex_),
	name_(name),
	maxQueueSize_(0),
	running_(false)
	{}
	
ThreadPool::~ThreadPool() { }

void ThreadPool::start(int numThreads) {
	assert(threads_.empty());
	running_ = true;
	threads_.reserve(numThreads);
	for (int i= 0; i < numThreads; ++i) {
		char id[32];
		std::cout<<"id is "<<id<<std::endl;
		threads_.emplace_back(new Thread(std::bind(&ThreadPool::runInThread,this),name_+id));
		threads_[i]->start();		
	}
	if (numThreads == 0 && threadInitCallback_) { threadInitCallback_(); }
}

void ThreadPool::stop() {
  	{
  		MutexLockGuard lock(mutex_);
  		running_ = false;
  		notEmpty_.notifyAll();
  		notFull_.notifyAll();
  	}
  	for (auto& thr : threads_) {
    	thr->join();
  	}
}

/*size_t ThreadPool::queueSize() const{
 	MutexLockGuard lock(mutex_);
 	return queue_.size();
}*/

void ThreadPool::run(Task task) {
  	if (threads_.empty()) { task(); }
  	else {
    	MutexLockGuard lock(mutex_);
    	while (isFull() && running_) { notFull_.wait(); }
    	if (!running_) return;
    	assert(!isFull());
    	queue_.push_back(std::move(task));
    	notEmpty_.notify();
  	}
}

ThreadPool::Task ThreadPool::take() {
  	MutexLockGuard lock(mutex_);
  	while (queue_.empty() && running_) { notEmpty_.wait(); }
  	Task task;
  	if (!queue_.empty()) {
    	task = queue_.front();
    	queue_.pop_front();
    	if (maxQueueSize_ > 0) { notFull_.notify(); }
  	}
  	return task;
}

bool ThreadPool::isFull() { return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_; }

void ThreadPool::runInThread() {
	
}

