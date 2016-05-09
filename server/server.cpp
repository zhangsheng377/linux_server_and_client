#include "utility.h"
#include<map>

map<int ,CLIENT> clients_map;

int main(int argc, char *argv[])
{
    //服务器IP + port
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    //创建监听socket
    int listener = socket(PF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
        perror("listener");
        return -1;
    }
    printf("listen socket created \n");
    //绑定地址
    if( bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("bind error");
        return -1;
    }
    //监听
    int ret = listen(listener, 5);
    if(ret < 0)
    {
        perror("listen error");
        return -1;
    }
    printf("Start to listen: %s\n", SERVER_IP);
    //在内核中创建事件表
    int epfd = epoll_create(EPOLL_SIZE);
    if(epfd < 0)
    {
        perror("epfd error");
        return -1;
    }
    printf("epoll created, epollfd = %d\n", epfd);
    static struct epoll_event events[EPOLL_SIZE];
    //往内核事件表里添加事件
    addfd(epfd, listener, true);
    //主循环
    while(1)
    {
        //epoll_events_count表示就绪事件的数目
        int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
        if(epoll_events_count < 0)
        {
            perror("epoll failure");
            break;
        }
        printf("epoll_events_count = %d\n", epoll_events_count);
        //处理这epoll_events_count个就绪事件
        for(int i = 0; i < epoll_events_count; ++i)
        {
            int sockfd = events[i].data.fd;
            //新用户连接
            if(sockfd == listener)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrLength = sizeof(struct sockaddr_in);

                int clientfd = accept( listener, ( struct sockaddr* )&client_address, &client_addrLength );
                printf("client connection from: %s : % d(IP : port), socketfd = %d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), clientfd);

                addfd(epfd, clientfd, true);

                // 服务端用list保存用户连接
                clients_list.push_back(clientfd);
                CLIENT client;
                client.socketfd=clientfd;
                clients_map[clientfd]=client;
                printf("Add new socketfd = %d to epoll\n", clientfd);
                printf("Now there are %d clients int the chat room\n", (int)clients_list.size());
            }
            //处理用户发来的消息
            else
            {
                printf("start recv\n");
                // buf[BUF_SIZE] receive new chat message
                char buf[BUF_SIZE];
                bzero(buf, BUF_SIZE);
                // receive message
                //printf("read from client(clientID = %d)\n", sockfd);
                int len = recv(sockfd, buf, BUF_SIZE, 0);
                printf("recv ok\n");
                buf[len]='\0';
                if(len == 0) // len = 0 means the client closed connection
                {
                    close(sockfd);
                    clients_list.remove(sockfd); //server remove the client
                    map<int,CLIENT>::iterator map_it;
                    map_it=clients_map.find(sockfd);
                    printf("ClientID = %d closed.\n now there are %d client in the char room\n", clients_map[sockfd].ID, (int)clients_list.size());
                    clients_map.erase(map_it);
                }
                else if(len < 0)
                {
                    perror("error");
                    return -1;
                }
                printf("start memcpy\n");
                CLIENT client;
                memcpy(&client,buf,sizeof(buf));
                clients_map[sockfd]=client;
                printf("ClientID = %d comes.\n", clients_map[sockfd].ID);
                //send back message
                char message[BUF_SIZE];
                bzero(message, BUF_SIZE);
                // format message
                sprintf(message, "Server received ClientID=%d 's message.\n",clients_map[sockfd].ID);
                if( send(sockfd, message, BUF_SIZE, 0) < 0 )
                {
                    perror("error");
                    return -1;
                }
            }
        }
    }
    close(listener); //关闭socket
    close(epfd); //关闭内核
    return 0;
}
