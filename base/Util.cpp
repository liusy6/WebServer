#include "Util.h"
#include "base/Define.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

/*
sigaction 查询或设置信号处理方式
进程在使用send发送数据到一个已关闭客户端时，send()函数未返回失败时，内核给出SIGPIPE信号导致进程退出
因此要屏蔽 SIGPIPE信号
*/ 
void handle_for_sigpipe() {
  struct sigaction sa;
  memset(&sa, '\0', sizeof(sa));
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  if (sigaction(SIGPIPE, &sa, NULL)) return;
}

//将socket设置为非阻塞
/*
int fcntl(int fd, int cmd,...)提供了对fd的各种控制操作，cmd参数指定执行何种类型的操作
F_GETFL获取fd的状态标志，成功返回fd的状态标志反之-1 
F_SETFL设置fd的状态标志 
*/ 
int setSocketNonBlocking(int fd) {
	int flag = fcntl(fd, F_GETFL, 0);
  	if (flag == -1) return -1;

  	flag |= O_NONBLOCK;
  	if (fcntl(fd, F_SETFL, flag) == -1) return -1;
  	return 0;
}

int socket_bind_listen(int port) {
    //检查分配的端口号是否合法 
	if (port < 0 || port > 65535) return -1;
  	//创建socket，返回监听描述符
  	/*
	int socket(int domain, int type, int protocol)
	domain:底层协议族	PF_INET、AF_INET->4	 PF_INET6、AF_INET6->6
	type:服务类型	SOCK_STREAM:TCP	  SOCK_UGRAM:UDP
	protocol:具体协议，0->默认协议	 
	*/ 
  	int listen_fd = 0;
  	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1;
	
  	int optval = 1;
  	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval,sizeof(optval)) == -1) {
    	close(listen_fd);
    	return -1;
  	}

  	//设置服务器IP和端口，与监听描述符绑定
  	struct sockaddr_in server_addr;
  	/*
  	socket地址结构体用于IPv4
  	struct sockaddr_in {	  
		sa_family_t	sin_family; 地址族
		u_int16_t sin_port; 端口号，要用网络字节序表示
		struct in_addr sin_addr; IPv4地址结构体
	}
	struct  n_addr {
		u_int32_t s_addr; 地址，用网络字节序表示
	}
	所有socket编程接口使用的地址参数类型都是sockaddr 
	void bzero(void *s, int n) 将字符串s的前n个字节置为0 
	主机字节序和网络字节序间转换：htonl、htons、ntohl、ntohs 
	int bind(int sockfd, const struct sockaddr* my_addr, socklen_t addrlen)命名socket
	将my_addr所指的socket地址分配给未命名的sockfd，指定成功返回0反之-1 
	close终止了数据传送的两个方向，但直到引用计数减位0时才发送 
	*/  
  	bzero((char *)&server_addr, sizeof(server_addr));
  	server_addr.sin_family = AF_INET;
  	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  	server_addr.sin_port = htons((unsigned short)port);
  	if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    	close(listen_fd);
    	return -1;
  	}

  //开始监听，最大等待队列长为MAX_LISTENFD
  /*
  int listen(int sockfd,int backlog) 创建一个监听队列存放待处理的客户连接
  sockfd指定被监听的socket，backlog提示内核监听队列的最大长度，超过则不再受理新的客户连接（完全连接状态）
  成功返回1反之-1  
  */ 
  if (listen(listen_fd, MAX_LISTENFD) == -1) {
    close(listen_fd);
    return -1;
  }

  //无效监听描述符
  if (listen_fd == -1) {
    close(listen_fd);
    return -1;
  }
  return listen_fd;
}

void shutDownWR(int fd) {
  	shutdown(fd, SHUT_WR);
}

void setSocketNodelay(int fd) {
  	int enable = 1;
  	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable, sizeof(enable));
}
