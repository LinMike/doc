#include<sys/socket.h>
#include<sys/epoll.h>
#include<iostream>
#include<sys/un.h>
#include<errno.h>
#include<string.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<fcntl.h>
#include<signal.h>
#include<unistd.h>

bool is_running = true;
void handler(int sig)
{
	is_running = false;
}

int main()
{
	signal(SIGINT, handler);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(2232);
	memset(addr.sin_zero, '0', sizeof(addr.sin_zero));
	int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if(sockfd == -1)
	{
		std::cerr<<"create socket failed"<<std::endl;
		return -1;
	}
	int con = connect(sockfd, (sockaddr*)&addr, sizeof(addr));
	fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
	if(con == -1)
	{
		std::cerr<<"connect socket failed"<<std::endl;
		return -1;
	}
//	char buffer[1024];
	std::string line;
	while(is_running){
//		std::cin>>buffer;
		std::getline(std::cin, line);
		if(line.size() == 0)
		{
			shutdown(sockfd, SHUT_RDWR);
			close(sockfd);
			sockfd = -1;
			break;
		}
		while(is_running)
		{
			int nsend = send(sockfd, line.data(), line.size(), 0);// non blocked
			std::cout<<"send data size = "<<nsend<<std::endl;
		}
	}
	if(sockfd != -1)
	{
		shutdown(sockfd, SHUT_RDWR);
		close(sockfd);
	}
	return 0;
}
