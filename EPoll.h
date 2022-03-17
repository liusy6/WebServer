#pragma once
#include <sys/epoll.h>
#include <memory>
#include <map>
#include <vector>
#include "Channel.h"
#include "Timestamp.h"

using namespace std;
class EPoll {
	public:
		EPoll();
		~EPoll();
		Timestamp poll(int timeoutMs, vector<Channel*>* channels);
		void updateChannel(Channel* channel);
		void removeChannel(Channel* channel);
		bool hasChannel(Channel* channel);
		
	protected:
		typedef std::map<int, Channel*> ChannelMap;
  		ChannelMap channels_;
	
	private:
		typedef std::vector<struct epoll_event> EventList;
		static const int kInitEventListSize = 16;
		int epollfd_;
  		EventList events_;		
	
};
