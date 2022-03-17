#pragma once 
#include <sys/types.h>
#include <string>

class Timestamp{
	public:
		Timestamp()
			:microSecondsSinceEpoch_(0)
		{
		}
		explicit Timestamp(int64_t microSecondsSinceEpochArg)
    : microSecondsSinceEpoch_(microSecondsSinceEpochArg)
  		{
  		}
		~Timestamp() { }
		void swap(Timestamp& that) { std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_); }
		std::string toString() const;
		bool valid() const { return microSecondsSinceEpoch_ >0; } 
		int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
		time_t secondsSinceEpoch() const { return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond); }
		
		friend bool operator <(Timestamp l, Timestamp r);
		friend bool operator ==(Timestamp l, Timestamp r);
		
		static Timestamp now();  
		static Timestamp nowAfter(double seconds);
		static const int kMicroSecondsPerSecond = 1000 * 1000;
		
	private:
		int64_t microSecondsSinceEpoch_;
};

