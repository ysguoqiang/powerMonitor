#include <stdexcept>
#include <sys/socket.h>
#include <string>
#include <arpa/inet.h>
class Socket
{
private:
	int sockFd_;
	int port_;
	std::string ip_;
	struct sockaddr_in servAddr_, cliAddr_;
public:
	Socket(int port, std::string ip_);
	int BindndListen();
	int GetFd();
};