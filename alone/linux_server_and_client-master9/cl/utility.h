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

#include <algorithm>
#include <sys/shm.h>
#include <math.h>
#include <time.h>
#include <sys/resource.h> //setrlimit


#define MYPORT  8887



using namespace std;

// clients_list save all the clients's socket
//list<int> clients_list;

/********************** macro defintion **************************/
// server ip
#define SERVER_IP "127.0.0.1"
// server port
#define SERVER_PORT 8888
//epoll size
#define EPOLL_SIZE 10001
//message buffer size
#define BUF_SIZE 0xFFFF

#define MYPORT  8887

const int ORDER_LEN=2;
/*
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
*/
struct CLIENT
{
    int id;
    int pwd;
    int hdf_type; //切换类型
    int bss_type; //业务类型
    double life_time;
    int degree;
    //int sockfd;
    struct sockaddr_in client_address;
    int state;//0是timeout，1是带宽被踢，2是带宽被拒，3是数据库被拒
    bool isalive;
};




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
    //if( enable_et ) ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    setnonblocking(fd);
    //printf("fd added to epoll!\n\n");
}

void delfd( int epollfd, int fd, bool enable_et )
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    //if( enable_et ) ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}




string IntToString ( int a)
{
    string s;
    while(a!=0)
    {
        s.append(1, a%10+'0');
        a = a/10;
    }
    reverse (s.begin(), s.end());
    return s;
}
int StringToInt(string s)
{
    int sum=0;
    for (int i=0; i<(int)s.size(); i++)
    {
        sum += sum*10 + (s[i] - '0');
    }
    return sum;
}

int GenKey(int x)
{
    string s = "9211";
    string temp = IntToString(x);
    s.append(temp);
    int xx = StringToInt(s);
    return xx;
}




#endif // UTILITY_H_INCLUDED


