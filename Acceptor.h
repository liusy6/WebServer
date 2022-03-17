#pragma once
#include <functional>
#include <sys/socket.h>
#include "Channel.h"
#include "base/Declare.h"
#include "base/noncopyable.h"

using namespace std;
class Acceptor : noncopyable {
	public:
		typedef function<void (int sockfd)> NewConnectionCallback;
		Acceptor(EventLoop* loop, int port);
		~Acceptor();
		void start();
		void setNewConnectionCallback(NewConnectionCallback&& cb){
			newConnectionCallback_ = cb;
		}
		void handleRead();
		void listen();
		bool listening() const { return listening_; }
		
	private:
		EventLoop* loop_;
		int port_;
		int listenfd_;		
		
		NewConnectionCallback newConnectionCallback_;
		bool listening_;
		Channel* acceptChannel_;
		
};
