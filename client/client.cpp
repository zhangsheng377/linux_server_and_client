#include "utility.h"
#include<vector>

const int CLIENTNUM=101;
vector<int> sockets(CLIENTNUM);


int main(int argc, char *argv[])
{
    //用户连接的服务器 IP + port
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    // 创建epoll
    int epfd = epoll_create(EPOLL_SIZE);
    if(epfd < 0)
    {
        perror("epfd error");
        return -1;
    }
    static struct epoll_event events[EPOLL_SIZE];

    for(int c_i=1; c_i<CLIENTNUM+1; c_i++)//最后4个会创建不了，查查服务器代码
    {
        // 创建socket
        sockets[c_i]= socket(PF_INET, SOCK_STREAM, 0);
        if(sockets[c_i] < 0)
        {
            perror("sock error");
            return -1;
        }
        // 连接服务端
        if(connect(sockets[c_i], (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        {
            perror("connect error");
            return -1;
        }

        //将sock和管道读端描述符都添加到内核事件表中
        addfd(epfd, sockets[c_i], true);
    }

    // Fork
    int pid = fork();
    if(pid < 0)
    {
        perror("fork error");
        return -1;
    }
    else if(pid == 0)   // 子进程
    {
        sleep(1);
        while(true)
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
            cout<<"ID = "<<ID<<endl;

            if(ID<1 || ID>CLIENTNUM)
            {
                cout<<"ID is error"<<endl;
                continue;
            }

            printf("Please input the code: ");
            // 聊天信息缓冲区
            char message[BUF_SIZE];
            cin>>message;
            cout<<"code = "<<message<<endl;
            CLIENT client;
            client.ID=ID;
            client.socketfd=sockets[ID];
            strcpy(client.code,message);
            // 将信息发送给服务端
            char client_msg[BUF_SIZE];
            memcpy(client_msg,&client,sizeof(CLIENT));
            send(sockets[ID],client_msg, BUF_SIZE, 0);
        }
    }
    else   //pid > 0 父进程
    {
        bool isClientwork=true;
        while(isClientwork)
        {
            int epoll_events_count = epoll_wait( epfd, events, EPOLL_SIZE, -1 );
            //处理就绪事件
            for(int i = 0; i < epoll_events_count ; ++i)
            {
                // 聊天信息缓冲区
                char message[BUF_SIZE];
                bzero(&message, BUF_SIZE);

                //服务端发来消息
                int sock=events[i].data.fd;
                //接受服务端消息
                int ret = recv(sock, message, BUF_SIZE, 0);

                // ret= 0 服务端关闭
                if(ret == 0)
                {
                    printf("Server closed connection: %d\n", sock);
                    //进程退出后自动关闭所有socket
                    isClientwork = false;
                }
                else printf("%s\n", message);

            }//for
        }//while
    }


    return 0;
}
