#pragma once
#include <functional>
#include "Timestamp.h"
#include "base/noncopyable.h"

class Timer : noncopyable {
	public:
		typedef std::function<void()> TimerCallback;
		Timer(TimerCallback cb, Timestamp when, double interval)
			:callback_(std::move(cb)),
			expiration_(when),
			interval_(interval),
			repeat_(interval > 0.0){}
		
		void run() const { callback_(); }
		Timestamp expiration() const { return expiration_; }
		bool repeat() const { return repeat_; }		
		void restart(Timestamp now) {
			if (repeat_) {
				expiration_ = now.nowAfter(interval_);
			}
			else {
				expiration_ = Timestamp(0);
			}
		}
		
	private:
		const TimerCallback callback_;
		Timestamp expiration_;
		const double interval_;
		const bool repeat_;
			
};
