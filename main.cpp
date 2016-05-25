#include <sys/epoll.h>
#include <iostream>
#include <vector>
#include <string>

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h> 
#include "ThreadPool.h"
#include <string.h>
#define POWER_LISTEN_PORT 10028
#define PHONE_LISTEN_PORT 10030
#define LISTENQ 10
static int setNonblocking (int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1) 
    {
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    int                 powerlistenfd, connfd, phonelistenfd;
    struct sockaddr_in  cliaddr, servaddr;
    int epfd;
    int nfds;
    ssize_t n;
    char line[100];
    socklen_t clilen = sizeof(cliaddr);

    if((powerlistenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        exit(0);
    }
    int reuse = 1;
    setsockopt(powerlistenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(POWER_LISTEN_PORT);

    setNonblocking (powerlistenfd);
    if(bind(powerlistenfd,(struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind");
        exit(0);
    }
    if(listen(powerlistenfd, LISTENQ) < 0)
    {
        perror("listen");
        exit(0);
    }

    if((phonelistenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        exit(0);
    }
    setsockopt(phonelistenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(PHONE_LISTEN_PORT);

    setNonblocking (phonelistenfd);
    if(bind(phonelistenfd,(struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind");
        exit(0);
    }
    if(listen(phonelistenfd, LISTENQ) < 0)
    {
        perror("listen");
        exit(0);
    }

    if((epfd = epoll_create(20)) < 0)
    {
        perror("epoll_create");
        exit(0);
    }

    struct epoll_event ev,events[20];   
    
    ev.data.fd = powerlistenfd;
    ev.events = EPOLLIN | EPOLLET;
    
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, powerlistenfd, &ev) < 0)
    {
        perror("epoll_ctl");
        exit(0);
    }

    ev.data.fd = phonelistenfd;
    ev.events = EPOLLIN | EPOLLET;
    
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, phonelistenfd, &ev) < 0)
    {
        perror("epoll_ctl");
        exit(0);
    }


    ThreadPool threadpool(10);
    for ( ; ; )
    {
        if((nfds = epoll_wait(epfd, events, 20, 0)) <= 0)
        {
            continue;
        }
        for(int i = 0; i < nfds; i++)
        {
            if(events[i].data.fd == powerlistenfd)
            {
                for( ; ; )
                {
                    if((connfd = accept(powerlistenfd,(struct sockaddr*) &cliaddr, &clilen)) <= 0)
                        break;
                    std::cout << "Server got a power connection from "<<
                    inet_ntoa(cliaddr.sin_addr) << ntohs(cliaddr.sin_port) << std::endl;          
                    threadpool.addWriteWork(connfd);
                }
            }
            /*if(events[i].data.fd == phonelistenfd)
            {
                for( ; ; )
                {
                    if((connfd = accept(phonelistenfd,(struct sockaddr*) &cliaddr, &clilen)) <= 0)
                        break;
                    std::cout << "Server got a phone connection from "<<
                    inet_ntoa(cliaddr.sin_addr) << ntohs(cliaddr.sin_port) << std::endl;          
                    setNonblocking (connfd);
                    threadpool.addReadWork(connfd);
                }
            }*/
            /*else if(events[i].events&EPOLLIN)
            {
                for( ; ; )
                {
                    int ret;
                    ret = read(events[i].data.fd, line, 100);
                    if(ret < 0 && ret != EAGAIN)
                        break;
                    else if(ret == EAGAIN)
                        continue;
                    n += ret;
                }
            //      line[n] = '\0';
                    ev.data.fd = events[i].data.fd;
                    ev.events = EPOLLOUT | EPOLLET;
                    if(epoll_ctl(epfd, EPOLL_CTL_MOD, events[i].data.fd, &ev) < 0)
                        perror("epoll_ctl2");
            }*/
           /* else if(events[i].events&EPOLLOUT)
            {
                for( ; ; )
                {
                    int ret;
                    ret = write(events[i].data.fd, line, n);
                    if((n -= ret) <= 0)
                        break;
                    write(STDOUT_FILENO, line, n);
                }
                    //memset(line, 0, 100);
                    ev.data.fd = events[i].data.fd;
                    ev.events = EPOLLIN | EPOLLET;  
                    if(epoll_ctl(epfd, EPOLL_CTL_MOD, events[i].data.fd, &ev) < 0)
                        perror("epoll_ctl3");
            }*/
        }
    }
}
