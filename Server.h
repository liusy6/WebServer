#pragma once
#include "TcpConnection.h"
#include <sys/epoll.h>
#include <string>
#include "base/noncopyable.h"
#include "EventLoopThreadPool.h"
#include "base/Define.h"
#include <map>
#include <memory>

class EventLoop;
class Acceptor;
class EventLoopThreadPool;

class Server : noncopyable {
 	public:
 		typedef std::function<void(EventLoop*)> ThreadInitCallback;
  		Server(EventLoop* loop, int threadnum, int port);
  		~Server();
  		int port() const { return port_; }
  		EventLoop* getLoop() const { return loop_; }
  		void start();
  		void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
		void setConnectionCallback(const TcpConnection::ConnectionCallback& cb) { connectionCallback_ = cb; }
		std::shared_ptr<EventLoopThreadPool> threadPool(){ return threadPool_; }
  		 
  	
  	private:
	typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
  	typedef std::map<string, TcpConnectionPtr> ConnectionMap;
	EventLoop* loop_;  
	int port_;     
	int threadnum_;
	std::shared_ptr<Acceptor> acceptor_;
	bool started_;
        std::shared_ptr<EventLoopThreadPool> threadPool_;	              
        ThreadInitCallback threadInitCallback_;
        TcpConnection::ConnectionCallback connectionCallback_;
        void newConnection(int sockfd);
        void removeConnection(const TcpConnectionPtr& conn);
        void removeConnectionInLoop(const TcpConnectionPtr& conn);
        int nextConnId_;
        ConnectionMap connections_;

};
