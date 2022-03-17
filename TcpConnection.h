#pragma once
#include <arpa/inet.h>
#include <memory>
#include "base/noncopyable.h"
#include "Timestamp.h"
#include <string>
#include "EventLoop.h"

class EventLoop;


class TcpConnection : noncopyable,public std::enable_shared_from_this<TcpConnection>
{
	public:
		typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
		typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
		typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
		TcpConnection(EventLoop* loop, const std::string& name, int sockfd, int port);
		~TcpConnection();
		EventLoop* getLoop() const { return loop_; }
		const std::string& name() const { return name_; }
		bool connected() const { return state_ == kConnected; }
		bool disconnected() const { return state_ == kDisconnected; }
		
		void forceClose();
		void forceCloseWithDelay(double seconds);
		void setTcpNoDelay(bool on);
		void startRead();
		void stopRead();
		bool isReading() const { return reading_; };
		
		void setConnectionCallback(ConnectionCallback& cb){ connectionCallback_ = cb; }
		void setCloseCallback(const CloseCallback& cb){ closeCallback_ = cb; }
		void connectEstablished();
		void connectDestroyed();
		
	private:
		typedef enum stateE{
			kDisconnected,
			kConnecting,
			kConnected,
			kDisconnecting
		}StateE;

		EventLoop* loop_;
		const std::string name_;
		StateE state_;
		bool reading_;		
		int sockfd_;
		int port_;
		std::unique_ptr<Channel> channel_;
		ConnectionCallback connectionCallback_;
		CloseCallback closeCallback_;
		//void handleRead(Timestamp receiveTime);
		//void handleClose();
		void shutdown();
		void shutdownInLoop();
		void setState(StateE s) { state_ = s; }
		void startReadInLoop();
  		void stopReadInLoop();		
};
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
