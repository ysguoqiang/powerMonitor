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
#include "threadPool.h"
#include <string.h>
#define SERV_PORT 10001
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
    int                 listenfd, connfd, sockfd;
    struct sockaddr_in  cliaddr, servaddr;
    int epfd;
    int nfds;
    ssize_t n;
    char line[100];
    socklen_t clilen = sizeof(cliaddr);

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        exit(0);
    }
    int reuse = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(SERV_PORT);

    
    if((epfd = epoll_create(20)) < 0)
    {
        perror("epoll_create");
        exit(0);
    }

    struct epoll_event ev,events[20];   
    
    ev.data.fd = listenfd;
    ev.events = EPOLLIN | EPOLLET;
    
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev) < 0)
    {
        perror("epoll_ctl");
        exit(0);
    }
    setNonblocking (listenfd);
    if(bind(listenfd,(struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind");
        exit(0);
    }
    if(listen(listenfd, LISTENQ) < 0)
    {
        perror("listen");
        exit(0);
    }
    threadPool threadpool(10);
    for ( ; ; )
    {
        if((nfds = epoll_wait(epfd, events, 20, 0)) <= 0)
        {
            continue;
        }
        for(int i = 0; i < nfds; i++)
        {
            if(events[i].data.fd == listenfd)
            {
                for( ; ; )
                {
                    if((connfd = accept(listenfd,(struct sockaddr*) &cliaddr, &clilen)) <= 0)
                        break;
                    std::cout << "Server got a connection from "<<
                    inet_ntoa(cliaddr.sin_addr) << ntohs(cliaddr.sin_port) << std::endl;          
                    setNonblocking (connfd);
                    threadpool.addWork(connfd);
                }
            }
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