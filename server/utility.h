#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

class CLIENT
{
public:
    int ID;
    char code[128];
    int action;
    int type;
    int socketfd;
    CLIENT();
};
CLIENT::CLIENT()
{
    ID=-1;
    action=1;
    type=2;
}
// clients_list save all the clients's socket
//list<int> clients_list;

/********************** macro defintion **************************/
// server ip
#define SERVER_IP "127.0.0.1"
// server port
#define SERVER_PORT 8888
//epoll size
#define EPOLL_SIZE 5000
//message buffer size
#define BUF_SIZE 0xFFFF
const int ORDER_LEN=2;



/********************** some function **************************/
/**
* @param sockfd: socket descriptor
* @return 0
**/
int setnonblocking(int sockfd)
{
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)| O_NONBLOCK);
    return 0;
}

/**
* @param epollfd: epoll handle
* @param fd: socket descriptor
* @param enable_et : enable_et = true, epoll use ET; otherwise LT
**/
void addfd( int epollfd, int fd, bool enable_et )
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if( enable_et ) ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    setnonblocking(fd);
    //printf("fd added to epoll!\n");
}

void delfd( int epollfd, int fd, bool enable_et )
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if( enable_et ) ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}

#endif // UTILITY_H_INCLUDED

