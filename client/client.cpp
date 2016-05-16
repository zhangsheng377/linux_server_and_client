#include "utility.h"
#include<map>

const int CLIENTNUM=3001;//从1开始
map<int,int> map_ID_sockets;//从1开始
map<int,CLIENT> map_socket_clients;

int main(int argc, char *argv[])
{
    //用户连接的服务器 IP + port
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    struct rlimit rt;//资源限制符
    //设置每个进程允许打开的最大文件数
    rt.rlim_max=rt.rlim_cur=EPOLL_SIZE;
    if(setrlimit(RLIMIT_NOFILE,&rt)==-1)
    {
        perror("setrlimt error.\n");
        return -1;
    }
    // 创建epoll
    int epfd = epoll_create(EPOLL_SIZE);
    if(epfd < 0)
    {
        perror("epfd error");
        return -1;
    }
    static struct epoll_event events[EPOLL_SIZE];

    for(int c_i=1; c_i<CLIENTNUM+1; c_i++)//从1开始
    {
        // 创建socket
        map_ID_sockets[c_i]= socket(PF_INET, SOCK_STREAM, 0);
        if(map_ID_sockets[c_i]< 0)
        {
            perror("sock error");
            return -1;
        }
        // 连接服务端
        if(connect(map_ID_sockets[c_i], (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        {
            perror("connect error");
            return -1;
        }

        //将sock添加到内核事件表中
        addfd(epfd, map_ID_sockets[c_i], true);
    }

    // 创建管道，其中fd[0]用于父进程读，fd[1]用于子进程写
    int pipe_fd[2];
    if(pipe(pipe_fd) < 0)
    {
        perror("pipe error");
        return -1;
    }

    //将管道读端描述符添加到内核事件表中
    addfd(epfd, pipe_fd[0], true);

    // Fork
    int pid = fork();
    if(pid < 0)
    {
        perror("fork error");
        return -1;
    }
    else if(pid == 0)   // 子进程
    {
        //子进程负责写入管道，因此关闭读端
        close(pipe_fd[0]);

        bool isClientwork=true;
        while(isClientwork)
        {
            sleep(1);
            //cout<<"end sleep"<<endl;
            printf("Please input the sockets' ID: ");

            int ID;
            cin>>ID;
            if(cin.fail())
            {
                cout<<"input is wrong, please try again."<<endl;
                cin.clear();
                cin.ignore(4096,'\n');
                continue;
            }

            //判别ID是否存在的工作交给父进程

            printf("Please input the order: ");
            // 聊天信息缓冲区
            char order[BUF_SIZE];
            bzero(order, BUF_SIZE);
            cin>>order;
            char message[BUF_SIZE];
            bzero(message, BUF_SIZE);
            if(strcmp(order,"00")==0)
            {
                sprintf(message,"%s %d %s",order,ID," ");
            }
            else if(strcmp(order,"-1")==0)//关闭当前socket
            {
                sprintf(message,"%s %d %s",order,ID," ");
            }
            else
            {
                printf("Please input the message: ");
                // 聊天信息缓冲区
                char msg[BUF_SIZE];
                bzero(msg, BUF_SIZE);
                cin>>msg;
                sprintf(message,"%s %d %s",order,ID,msg);
            }
            // 子进程将信息写入管道
            if( write(pipe_fd[1], message, strlen(message)  ) < 0 )//zsd
            {
                perror("fork error");
                return -1;
            }
        }
    }
    else   //pid > 0 父进程
    {
        //父进程负责读管道数据，因此先关闭写端
        close(pipe_fd[1]);
        bool isClientwork=true;
        while(isClientwork)
        {
            int epoll_events_count = epoll_wait( epfd, events, EPOLL_SIZE, -1 );
            //处理就绪事件
            for(int i = 0; i < epoll_events_count ; ++i)
            {
                // 聊天信息缓冲区
                char message[BUF_SIZE];
                bzero(message, BUF_SIZE);
                if(events[i].data.fd == pipe_fd[0])
                {
                    char order[BUF_SIZE];
                    bzero(order, BUF_SIZE);
                    int ID;
                    char msg[BUF_SIZE];
                    bzero(msg, BUF_SIZE);
                    //从管道读子进程键盘输入的指令
                    int ret = read(events[i].data.fd, message, BUF_SIZE);
                    if(ret == 0)
                    {
                        //isClientwork = 0;
                    }
                    else
                    {
                        sscanf(message,"%s %d %s",order,&ID,msg);
                        //printf("msg = %s\n",msg);
                        map<int,int>::iterator map_int_int_it;
                        map_int_int_it=map_ID_sockets.find(ID);
                        if(map_int_int_it==map_ID_sockets.end() ) //没找到
                        {
                            printf("This ID's socket can't find, you must enroll(00) it. ID = %d\n",ID);
                            // 创建socket
                            map_ID_sockets[ID]= socket(PF_INET, SOCK_STREAM, 0);
                            // 连接服务端
                            if(connect(map_ID_sockets[ID], (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
                            {
                                perror("connect error");
                                return -1;
                            }
                            //将sock添加到内核事件表中
                            addfd(epfd, map_ID_sockets[ID], true);
                        }
                        else
                        {
                            if(strcmp(order,"00")==0)
                            {
                                char send_message[BUF_SIZE];
                                bzero(send_message, BUF_SIZE);
                                CLIENT client;
                                client.ID=ID;
                                client.live_sec=10;
                                client.socketfd=map_ID_sockets[ID];
                                map_socket_clients[map_ID_sockets[ID]]=client;
                                // 将信息发送给服务端
                                char client_info[BUF_SIZE];
                                bzero(client_info, BUF_SIZE);
                                memcpy(client_info,&client,sizeof(CLIENT));
                                strcat(send_message,order);
                                strcat(&send_message[ORDER_LEN],client_info);
                                send(map_ID_sockets[ID],send_message, BUF_SIZE, 0);
                                printf("ID = %d, socket = %d, send message: %s\n",ID,map_ID_sockets[ID],send_message);
                            }
                            else if(strcmp(order,"-1")==0)//关闭当前socket
                            {
                                map<int,CLIENT>::iterator map_int_client_it;
                                map_int_client_it=map_socket_clients.find(map_ID_sockets[ID]);
                                char send_message[BUF_SIZE];
                                bzero(send_message, BUF_SIZE);
                                strcat(send_message,order);
                                strcat(&send_message[ORDER_LEN],msg);
                                send(map_ID_sockets[ID],send_message, BUF_SIZE, 0);
                                //printf("send message: %s\n",send_message);
                                close(map_ID_sockets[ID]);
                                delfd(epfd, map_ID_sockets[ID], true);/////////////////////
                                printf("ClientID = %d closed.\n", ID);//zsd
                                map_ID_sockets.erase(map_int_int_it);
                                map_socket_clients.erase(map_int_client_it);
                            }
                            else
                            {
                                char send_message[BUF_SIZE];
                                bzero(send_message, BUF_SIZE);
                                strcat(send_message,order);
                                strcat(&send_message[ORDER_LEN],msg);
                                send(map_ID_sockets[ID],send_message, BUF_SIZE, 0);
                                printf("send message: %s\n",send_message);
                            }
                        }
                    }
                }
                else//从服务器接收的
                {
                    //服务端发来消息
                    int sock=events[i].data.fd;
                    //接受服务端消息
                    int ret = recv(sock, message, BUF_SIZE, 0);

                    // ret= 0 服务端将该socket关闭
                    if(ret == 0)
                    {
                        map<int,int>::iterator map_int_int_it;
                        for(map_int_int_it=map_ID_sockets.begin(); map_int_int_it!=map_ID_sockets.end(); ++map_int_int_it)
                        {
                            if(map_int_int_it->second==sock) break;
                        }
                        if(map_int_int_it==map_ID_sockets.end()) printf("Can't find the socket to close.\n");
                        else
                        {
                            map_ID_sockets.erase(map_int_int_it);
                        }
                        map<int,CLIENT>::iterator map_int_client_it;
                        map_int_client_it=map_socket_clients.find(sock);
                        map_socket_clients.erase(map_int_client_it);
                        close(sock);//不知要不要这样重复关闭
                        delfd(epfd, sock, true);/////////////////////
                        printf("Server closed connection: %d\n", sock);
                    }
                    else printf("%s\n", message);
                }
            }//for
        }//while
    }

    if(pid)
    {
        //关闭父进程和sock
        printf("close the father thread.\n");
        close(pipe_fd[0]);
        //close(sock);
    }
    else
    {
        //关闭子进程
        printf("close the son thread.\n");
        close(pipe_fd[1]);
    }

    return 0;
}
