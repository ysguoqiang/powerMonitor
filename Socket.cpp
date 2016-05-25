#include "Socket.h"


Socket::Socket(int port = 0, std::string ip = "")
	:port_(port), ip_(ip)
{
	bzero(&servAddr_, sizeof(servAddr_));
	if(ip.empty())
		servAddr_.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		inet_pton(AF_INET, ip_.c_str(), &servAddr_.sin_addr);
	servAddr_.sin_family = AF_INET;
	servAddr_.sin_port = port_;
}

void Socket::CreateAndListen()
{
	sockFd_ = socket(AF_INET, SOCK_STREAM, 0);
	if(sockFd_ <= 0)
		throw std::runtime_error("Failed to create socket"); 
	int reuse = 1;
    setsockopt(sockFd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if(bind(sockFd_,(struct sockaddr*) &servAddr_, sizeof(servAddr_)) < 0)
    {
    	throw std::runtime_error("Failed to bind socket"); 
    }
    if(listen(sockFd_, 10) < 0)
    {
    	throw std::runtime_error("Failed to listen socket"); 
    }
}

int Socket::GetFd()
{

	return sockFd_;
}