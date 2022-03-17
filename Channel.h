#pragma once
#include <functional>
#include <memory>
#include "base/noncopyable.h"
#include "base/Declare.h"
#include "Timestamp.h"


class Channel : noncopyable {
	public:
		typedef std::function<void()> EventCallback;
		typedef std::function<void(Timestamp)> ReadEventCallback;
		Channel(EventLoop* loop, int fd);
		~Channel();
		
		void handleEvent() {};
		void setReadCallback(ReadEventCallback cb){ 
			readCallback_ = std::move(cb); 
		}
		int fd() const { return fd_; }
		int events() const { return events_; };
		void set_revents(int revt) { revents_ = revt; }
		bool isNoneEvent() const { return events_ == kNoneEvent; }
		void enableReading() { events_ |= kReadEvent; update(); }
		void disableReading() { events_ &= ~kReadEvent; update(); }
		void enableWriting() { events_ |= kWriteEvent; update(); }
  		void disableWriting() { events_ &= ~kWriteEvent; update(); } 
		void disableAll() { events_ = kNoneEvent; update(); }
  		bool isWriting() const { return events_ & kWriteEvent; }
  		bool isReading() const { return events_ & kReadEvent; }
  		
  		int index() { return index_; }
  		void set_index(int idx) { index_ = idx; }
  		
  		EventLoop* ownerLoop() { return loop_; }
  		void remove();
  		
  	private:
  		void update();
		
		static const int kNoneEvent;
  		static const int kReadEvent;
  		static const int kWriteEvent;	
  		EventLoop* loop_;
  		const int  fd_;
  		int events_;
  		int revents_;
  		int index_;
  		bool addedToLoop_;
  		ReadEventCallback readCallback_;
  		
  		
};
typedef std::shared_ptr<Channel> SP_Channel;
