#include "Socket.h"

Socket::Socket(int port, std::string ip)
	:port_(port), ip_(ip)
{
	servAddr_.sin_family = AF_INET;
	servAddr_.sin_port = port_;
	
}
int Socket::CreateAndListen()
{
	sockFd_ = socket(AF_INET, SOCK_STREAM, 0);
	if(sockFd_ <= 0)
		throw std::runtime_error("Failed to create socket"); 
}

int Socket::GetFd()
{
	return sockFd_;
}