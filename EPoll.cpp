#include "EPoll.h"
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include "Channel.h"
#include "base/Util.h"
#include "base/Define.h"

using namespace std;

const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;

void memZero(void* p, size_t n) { memset(p, 0, n); }

EPoll::EPoll()
	:epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
	events_(kInitEventListSize)
	{
		if (epollfd_ < 0) {
			cout << "epoll_create error, errno:" << epollfd_ << endl;
		}
	}
	
EPoll::~EPoll() { close(epollfd_); }

Timestamp EPoll::poll(int timeoutMs, vector<Channel*>* channels) {
	int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), events_.size(),timeoutMs);
	Timestamp now(Timestamp::now());
	if (numEvents > 0) {
		for (int i = 0; i < numEvents;  ++i) {
			Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
			channel->set_revents(events_[i].events);
			channels->push_back(channel);
		}
	}
	return now;
}

bool EPoll::hasChannel(Channel* channel) {
	ChannelMap::const_iterator it = channels_.find(channel->fd());
  	return it != channels_.end() && it->second == channel;
}

void EPoll::updateChannel(Channel* channel) {
	const int index = channel->index();
	int fd = channel->fd();
	struct epoll_event ev;
	memZero(&events_, sizeof events_);
    ev.data.ptr = channel;
    ev.events = channel->events();
	if (index == kNew || index == kDeleted) {
		if (index == kNew) {
        	assert(channels_.find(fd) == channels_.end());
      		channels_[fd] = channel;			
		}
		else {
			assert(channels_.find(fd) != channels_.end());
      		assert(channels_[fd] == channel);
		}
		channel->set_index(kAdded);
        epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev);
	}
	else {
		(void)fd;
		assert(channels_.find(fd) != channels_.end());
    	assert(channels_[fd] == channel);
    	assert(index == kAdded);
    	if (channel->isNoneEvent()) {
    		epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ev);
    		channel->set_index(kDeleted);
		}
		else {
			epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev);
		}
	}
}

void EPoll::removeChannel(Channel* channel) {
	int fd = channel->fd();
	assert(channels_.find(fd) != channels_.end());
  	assert(channels_[fd] == channel);
  	assert(channel->isNoneEvent());
  	int index = channel->index();
  	assert(index == kAdded || index == kDeleted);
  	size_t n = channels_.erase(fd);
  	(void)n;
  	assert(n == 1);

  	if (index == kAdded) {
		struct epoll_event ev;
		memZero(&events_, sizeof events_);
    	ev.data.ptr = channel;
    	ev.events = channel->events();
    	epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ev);
  	}
  	channel->set_index(kNew);
}
	

