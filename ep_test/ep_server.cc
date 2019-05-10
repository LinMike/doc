#include<sys/socket.h>
#include<sys/epoll.h>
#include<iostream>
#include<sys/un.h>
#include<errno.h>
#include<string.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<signal.h>
#include<unistd.h>
#include<fcntl.h>

const int MAXConnection = 5;

bool is_running = true;
void signalHandler(int signal){
	is_running = false;
}

int main()
{
	signal(SIGINT, signalHandler);
	int listenfd = -1;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd == -1)
	{
		std::cerr<<"create socket failed"<<std::endl;
		return -1;
	}

	int reuse = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));//set address could be reuse immediately, no need to wait for two mints

	sockaddr_in s_addr;
	s_addr.sin_family = AF_INET;
	s_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	s_addr.sin_port = htons(2232);
	memset(s_addr.sin_zero, '0', sizeof(s_addr.sin_zero));
	if(bind(listenfd, (sockaddr*)&s_addr, sizeof(s_addr)) == -1)
	{
		std::cerr<<"bind local server address failed"<<std::endl;
		return -1;
	}

	if(listen(listenfd, MAXConnection) == -1)
	{
		std::cerr<<"listen port error"<<std::endl;
		return -1;
	}

	int epfd = epoll_create(MAXConnection);
	epoll_event evts[MAXConnection];
	int sockfds[MAXConnection];
	std::fill(sockfds, &sockfds[MAXConnection-1], -1);
	memset(evts, '0', sizeof(evts));
	evts[0].data.fd = listenfd;
	evts[0].events = EPOLLIN | EPOLLET;
	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &evts[0]);
	sockfds[0] = listenfd;

	int evt_index = 1;
	while(is_running)
	{
		int nevs = epoll_wait(epfd, evts, MAXConnection, 10);
		if(nevs < 0)
		{
			std::cerr<<"epoll wait failed"<<std::endl;
			return -1;
		}
		else if(nevs == 0)
			std::cout<<"nothing happend"<<std::endl;
		std::cout<<nevs<<" events occured"<<std::endl;
		sockaddr_in client_addr;

		for(uint i=0;i<nevs;i++)
		{
			if(evts[i].events & EPOLLIN)
			{
				std::cout<<"event index "<<i<<" is readable"<<std::endl;
				if(evts[i].data.fd == listenfd)
				{
					socklen_t len = sizeof(sockaddr);
//					int sockfd = accept(listenfd, (sockaddr*)&client_addr, &len);
//					fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);//set new socket connection is non block mode(for send() and recv())
					int sockfd = accept4(listenfd, (sockaddr*)&client_addr, &len, SOCK_NONBLOCK);//return non blocked socket
					std::cout<<"has new client connected to server, sockfd "<<sockfd<<std::endl;
					if(sockfd == -1)
					{
						std::cerr<<"accept client sock failed"<<std::endl;
						continue;
					}

					sockfds[evt_index++] = sockfd;
					epoll_event ev;
					ev.data.fd = sockfd;
					ev.events = EPOLLIN | EPOLLET;
					epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

					epoll_event ev2;
					ev2.data.fd = listenfd;
					ev2.events = EPOLLIN | EPOLLET;
					epoll_ctl(epfd, EPOLL_CTL_MOD, listenfd, &ev2);
				}
				else
				{
					std::cout<<"has readable message come in"<<std::endl;
					int sockfd = evts[i].data.fd;
					char buffer[1024];
					memset(buffer, 0, sizeof(buffer));
					int nread = recv(sockfd, buffer, sizeof(buffer), MSG_DONTWAIT);
					if(nread < 0)
					{
						std::cerr<<"receive message from client failed, "<<nread<<", fd "<<sockfd<<std::endl;
						continue;
					}
					else if(nread == 0)
					{
						shutdown(sockfd, SHUT_RDWR);
						close(sockfd);
						epoll_event ev;
						epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, &ev);
						std::cerr<<"client "<<inet_ntoa(client_addr.sin_addr)<<":"<<ntohs(client_addr.sin_port)<<" is disconnect to server"<<std::endl;
						continue;
					}
					std::cout<<"receive mgs size = "<<nread<<std::endl;
					std::cout<<"msg = "<<buffer<<std::endl;

					epoll_event ev;
					ev.data.fd = sockfd;
					ev.events = EPOLLIN | EPOLLET;
					epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
				}
			}
		}
	}
	for(uint i=0;i<MAXConnection;i++)
	{
		shutdown(sockfds[i], SHUT_RDWR);
		close(sockfds[i]);
	}

	return 0;
}
