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
sigaction ��ѯ�������źŴ���ʽ
������ʹ��send�������ݵ�һ���ѹرտͻ���ʱ��send()����δ����ʧ��ʱ���ں˸���SIGPIPE�źŵ��½����˳�
���Ҫ���� SIGPIPE�ź�
*/ 
void handle_for_sigpipe() {
  struct sigaction sa;
  memset(&sa, '\0', sizeof(sa));
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  if (sigaction(SIGPIPE, &sa, NULL)) return;
}

//��socket����Ϊ������
/*
int fcntl(int fd, int cmd,...)�ṩ�˶�fd�ĸ��ֿ��Ʋ�����cmd����ָ��ִ�к������͵Ĳ���
F_GETFL��ȡfd��״̬��־���ɹ�����fd��״̬��־��֮-1 
F_SETFL����fd��״̬��־ 
*/ 
int setSocketNonBlocking(int fd) {
	int flag = fcntl(fd, F_GETFL, 0);
  	if (flag == -1) return -1;

  	flag |= O_NONBLOCK;
  	if (fcntl(fd, F_SETFL, flag) == -1) return -1;
  	return 0;
}

int socket_bind_listen(int port) {
    //������Ķ˿ں��Ƿ�Ϸ� 
	if (port < 0 || port > 65535) return -1;
  	//����socket�����ؼ�����������
  	/*
	int socket(int domain, int type, int protocol)
	domain:�ײ�Э����	PF_INET��AF_INET->4	 PF_INET6��AF_INET6->6
	type:��������	SOCK_STREAM:TCP	  SOCK_UGRAM:UDP
	protocol:����Э�飬0->Ĭ��Э��	 
	*/ 
  	int listen_fd = 0;
  	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1;
	
  	int optval = 1;
  	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval,sizeof(optval)) == -1) {
    	close(listen_fd);
    	return -1;
  	}

  	//���÷�����IP�Ͷ˿ڣ��������������
  	struct sockaddr_in server_addr;
  	/*
  	socket��ַ�ṹ������IPv4
  	struct sockaddr_in {	  
		sa_family_t	sin_family; ��ַ��
		u_int16_t sin_port; �˿ںţ�Ҫ�������ֽ����ʾ
		struct in_addr sin_addr; IPv4��ַ�ṹ��
	}
	struct  n_addr {
		u_int32_t s_addr; ��ַ���������ֽ����ʾ
	}
	����socket��̽ӿ�ʹ�õĵ�ַ�������Ͷ���sockaddr 
	void bzero(void *s, int n) ���ַ���s��ǰn���ֽ���Ϊ0 
	�����ֽ���������ֽ����ת����htonl��htons��ntohl��ntohs 
	int bind(int sockfd, const struct sockaddr* my_addr, socklen_t addrlen)����socket
	��my_addr��ָ��socket��ַ�����δ������sockfd��ָ���ɹ�����0��֮-1 
	close��ֹ�����ݴ��͵��������򣬵�ֱ�����ü�����λ0ʱ�ŷ��� 
	*/  
  	bzero((char *)&server_addr, sizeof(server_addr));
  	server_addr.sin_family = AF_INET;
  	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  	server_addr.sin_port = htons((unsigned short)port);
  	if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    	close(listen_fd);
    	return -1;
  	}

  //��ʼ���������ȴ����г�ΪMAX_LISTENFD
  /*
  int listen(int sockfd,int backlog) ����һ���������д�Ŵ�����Ŀͻ�����
  sockfdָ����������socket��backlog��ʾ�ں˼������е���󳤶ȣ��������������µĿͻ����ӣ���ȫ����״̬��
  �ɹ�����1��֮-1  
  */ 
  if (listen(listen_fd, MAX_LISTENFD) == -1) {
    close(listen_fd);
    return -1;
  }

  //��Ч����������
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
