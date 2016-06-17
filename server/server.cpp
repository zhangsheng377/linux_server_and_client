#define NDEBUG
#define NDEBUGMYCOUNT
#define NDEBUGBAND

#include "utility.h"
#include <map>
#include "vertify.h"
#include "zhangxiaofei.hpp"

//map<int ,CLIENT> clients_map;
map<int ,CLIENT> map_id_clients;
//map<int,int> map_timerfd_sockets;
map<int,int> map_timerfd_ids;

int main(int argc, char *argv[])
{
    //服务器IP + port
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    //serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    struct rlimit rt;//资源限制符
    //设置每个进程允许打开的最大文件数
    rt.rlim_max=rt.rlim_cur=EPOLL_SIZE;
    if(setrlimit(RLIMIT_NOFILE,&rt)==-1)
    {
        perror("setrlimt error.\n");
        return -1;
    }
    //创建监听socket
    int listener = socket(PF_INET, SOCK_DGRAM, 0);
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
    /*int ret = listen(listener, 5);
    if(ret < 0)
    {
        perror("listen error");
        return -1;
    }
    printf("Start to listen: %s\n", SERVER_IP);*/
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


    if(CreateDb()==false)
    {
        perror("creat sqlite3 error");
        return -1;
    }


    int pipe_fd = -1;
    int res = 0;
    char buffer[PIPE_BUF + 1];
    if(access(fifo_name, F_OK) == -1)
    {
        //管道文件不存在
        //创建命名管道
        res = mkfifo(fifo_name, 0777);
        if(res != 0)
        {
            fprintf(stderr, "Could not create fifo %s\n", fifo_name);
            exit(EXIT_FAILURE);
        }
    }
    printf("Process %d opening FIFO O_WRONLY | O_NONBLOCK\n", getpid());
    //以只写非阻塞方式打开FIFO文件，以只读方式打开数据文件
    pipe_fd = open(fifo_name, open_mode);
    printf("Process %d result %d\n", getpid(), pipe_fd);

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
        //处理这epoll_events_count个就绪事件
        for(int i = 0; i < epoll_events_count; ++i)
        {
            int fd = events[i].data.fd;
            char buf[BUF_SIZE];
            bzero(buf, BUF_SIZE);
            int ret_len;
            //处理用户发来的消息
            if(fd == listener)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrLength = sizeof(struct sockaddr_in);

                int recvfrom_len=recvfrom(listener,buf,BUF_SIZE,0,(struct sockaddr*)&client_address,&client_addrLength);

                //if(recvfrom_len==-1)
                if(recvfrom_len<=0)
                {
                    printf("recvfrom errno = %d\n",errno);
                    perror("Receive Data Failed: ");
                    continue;
                }

                /*cout<<"client_address.sin_addr = "<<inet_ntoa(client_address.sin_addr)<<endl;
                cout<<"client_address.sin_port = "<<client_address.sin_port<<endl;
                ret_len=sendto(listener,buf,strlen(buf),0,(struct sockaddr*)&client_address,sizeof(client_address));
                printf("ret of sendto = %d\n",ret_len);
                if(ret_len<=0)
                {
                    printf("sendto errno = %d\n",errno);
                    perror("Send Data Failed: ");
                    continue;
                }*/

                if(buf[0]=='\0'){
                    continue;
                }

                char order[ORDER_LEN+1],message[BUF_SIZE];
                bzero(order, ORDER_LEN+1);
                bzero(message, BUF_SIZE);
                strncat(order,buf,ORDER_LEN);
                //strcat(message,&buf[ORDER_LEN]);

                struct CLIENT client;
                memcpy(&client,&buf[ORDER_LEN],sizeof(CLIENT));
                client.client_address=client_address;

                strcat(message,&buf[ORDER_LEN+sizeof(CLIENT)]);

                if(strcmp(order,"00")==0)//接收结构体
                {
                    if(search(client.id,client.pwd)==false)
                    {
                        printf("===========search DB = false================\n");
                        printf("Reject USER ID = %d .\n\n",client.id);

                        bzero(buffer,sizeof(buffer));
                        sprintf(buffer,"rejectdb %d %d %s",client.id, (int)map_id_clients.size(),"");
                        //向FIFO文件写数据
                        ret_len=write(pipe_fd, buffer, sizeof(buffer));
                        if(ret_len==-1) printf("write error on fifo1\n");

                        char message_send[BUF_SIZE];
                        bzero(message_send, BUF_SIZE);
                        sprintf(message_send, "-5");
                        sendto(listener,message_send,strlen(message_send),0,(struct sockaddr*)&client_address,sizeof(client_address));

                        continue;
                    }

                    bzero(buffer,sizeof(buffer));
                    sprintf(buffer,"accessdb %d %d %s",client.id, (int)map_id_clients.size(),"");
                    //向FIFO文件写数据
                    ret_len=write(pipe_fd, buffer, sizeof(buffer));
                    if(ret_len==-1) printf("write error on fifo2\n");

                    client.degree=searchDegree(client.id);
                    switchcasein(makelevel(client.degree,client.hdf_type,client.bss_type));

                    bzero(buffer,sizeof(buffer));
                    sprintf(buffer,"qos %d %d %s",(int)Umax, 0,"");
                    //向FIFO文件写数据
                    ret_len=write(pipe_fd, buffer, sizeof(buffer));
                    if(ret_len==-1) printf("write error on fifo0\n");

                    int band=returnmyband(makelevel(client.degree,client.hdf_type,client.bss_type),returnband);

                    if(returnband[6]==0)//接入不成功
                    {
                        printf("================ switchcasein = false ================\n" );
                        printf("Reject USER ID= %d .\n\n",client.id);

                        char message_send[BUF_SIZE];
                        bzero(message_send, BUF_SIZE);
                        sprintf(message_send, "-4");
                        sendto(listener,message_send,strlen(message_send),0,(struct sockaddr*)&client_address,sizeof(client_address));

                        bzero(buffer,sizeof(buffer));
                        sprintf(buffer,"rejecthdf %d %d %s",client.id, (int)map_id_clients.size(),"");
                        //向FIFO文件写数据
                        ret_len=write(pipe_fd, buffer, sizeof(buffer));
                        if(ret_len==-1) printf("write error on fifo3\n");

                        map<int,CLIENT>::iterator map_int_CLIENT_itt;
                        for(int c_i=0; c_i<18; c_i++) my_count[c_i]=0;
                        for(map_int_CLIENT_itt=map_id_clients.begin(); map_int_CLIENT_itt!=map_id_clients.end(); ++map_int_CLIENT_itt)
                        {
                            ++my_count[makelevel(map_int_CLIENT_itt->second.degree,map_int_CLIENT_itt->second.hdf_type,map_int_CLIENT_itt->second.bss_type)];
                        }

                        continue;
                    }
                    else//接入成功
                    {
                        bzero(buffer,sizeof(buffer));
                        sprintf(buffer,"accesshdf %d %d %s",client.id, band,"");
                        //向FIFO文件写数据
                        ret_len=write(pipe_fd, buffer, sizeof(buffer));
                        if(ret_len==-1) printf("write error on fifo4\n");

                        bzero(buffer,sizeof(buffer));
                        sprintf(buffer,"qos %d %d %s",(int)Umax, 0,"");
                        //向FIFO文件写数据
                        ret_len=write(pipe_fd, buffer, sizeof(buffer));
                        if(ret_len==-1) printf("write error on fifo0\n");

                        printf("================ switchcasein = true ================\n" );

                        if(returnband[8]>0)//需要踢人
                        {
                            bzero(buffer,sizeof(buffer));
                            sprintf(buffer,"throw1 %d %d %s",client.id, 0,"");
                            //向FIFO文件写数据

                            map<int,CLIENT>::iterator map_int_CLIENT_it;
                            for(map_int_CLIENT_it=map_id_clients.begin(); map_int_CLIENT_it!=map_id_clients.end(); ++map_int_CLIENT_it)
                            {
                                if(map_int_CLIENT_it->second.degree==returnband[7])
                                {
                                    if(returnband[8]>0)
                                    {
                                        printf("===========need to throw the user ==========\n");

                                        char message_send[BUF_SIZE];
                                        bzero(message_send, BUF_SIZE);
                                        sprintf(message_send, "-3");
                                        sendto(listener,message_send,strlen(message_send),0,(struct sockaddr*)&(map_int_CLIENT_it->second.client_address),sizeof((map_int_CLIENT_it->second.client_address)));
                                        printf("throw out the USER ID = %d\n",map_int_CLIENT_it->second.id);

                                        char temp_cs[10];
                                        bzero(temp_cs,sizeof(temp_cs));
                                        sprintf(temp_cs,"%d,",map_int_CLIENT_it->second.id);
                                        strcat(buffer,temp_cs);

                                        map_id_clients.erase(map_int_CLIENT_it);
                                        map_int_CLIENT_it=map_id_clients.begin();
                                        --returnband[8];
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                            }//for

                            buffer[strlen(buffer)-1]='\0';
                            //向FIFO文件写数据
                            ret_len=write(pipe_fd, buffer, sizeof(buffer));
                            if(ret_len==-1) printf("write error on fifo5\n");
                        }
                        if(returnband[10]>0)//需要踢人
                        {
                            bzero(buffer,sizeof(buffer));
                            sprintf(buffer,"throw2 %d %d %s",client.id, 0,"");

                            map<int,CLIENT>::iterator map_int_CLIENT_it;
                            for(map_int_CLIENT_it=map_id_clients.begin(); map_int_CLIENT_it!=map_id_clients.end(); ++map_int_CLIENT_it)
                            {
                                if(map_int_CLIENT_it->second.degree==returnband[9])
                                {
                                    if(returnband[10]>0)
                                    {
                                        printf("===========need to throw the user ==========\n");

                                        char message_send[BUF_SIZE];
                                        bzero(message_send, BUF_SIZE);
                                        sprintf(message_send, "-3");
                                        sendto(listener,message_send,strlen(message_send),0,(struct sockaddr*)&(map_int_CLIENT_it->second.client_address),sizeof((map_int_CLIENT_it->second.client_address)));
                                        printf("throw out USER ID = %d\n",map_int_CLIENT_it->second.id);

                                        char temp_cs[10];
                                        bzero(temp_cs,sizeof(temp_cs));
                                        sprintf(temp_cs,"%d,",map_int_CLIENT_it->second.id);
                                        strcat(buffer,temp_cs);

                                        map_id_clients.erase(map_int_CLIENT_it);
                                        map_int_CLIENT_it=map_id_clients.begin();
                                        --returnband[10];
                                    }
                                    else
                                    {
                                        break;
                                    }
                                }
                            }//for

                            buffer[strlen(buffer)-1]='\0';
                            //向FIFO文件写数据
                            ret_len=write(pipe_fd, buffer, sizeof(buffer));
                            if(ret_len==-1) printf("write error on fifo6\n");
                        }
                    }//end else(接入成功)

                    map_id_clients[client.id]=client;
                    map<int,CLIENT>::iterator map_int_CLIENT_itt;
                    for(int c_i=0; c_i<18; c_i++) my_count[c_i]=0;
                    for(map_int_CLIENT_itt=map_id_clients.begin(); map_int_CLIENT_itt!=map_id_clients.end(); ++map_int_CLIENT_itt)
                    {
                        ++my_count[makelevel(map_int_CLIENT_itt->second.degree,map_int_CLIENT_itt->second.hdf_type,map_int_CLIENT_itt->second.bss_type)];
                    }

                    char message_send_tmp[BUF_SIZE];
                    bzero(message_send_tmp, BUF_SIZE);
                    sprintf(message_send_tmp, "02");
                    sendto(listener,message_send_tmp,strlen(message_send_tmp),0,(struct sockaddr*)&client_address,sizeof(client_address));

                    printf("USER ID = %d connected.\n", client.id);
                    printf("User level is %d\n",client.degree);
                    printf("The bandwidth for this user is %d\n",band);
                    printf("Now there are %d users in the satellite.\n\n", (int)map_id_clients.size());

                    bzero(buffer,sizeof(buffer));
                    sprintf(buffer,"come %d %d %s",client.id, band,"");
                    //向FIFO文件写数据
                    ret_len=write(pipe_fd, buffer, sizeof(buffer));
                    if(ret_len==-1) printf("write error on fifo7\n");

                    bzero(buffer,sizeof(buffer));
                    sprintf(buffer,"qos %d %d %s",(int)Umax, 0,"");
                    //向FIFO文件写数据
                    ret_len=write(pipe_fd, buffer, sizeof(buffer));
                    if(ret_len==-1) printf("write error on fifo0\n");

                    struct timespec now;
                    if(clock_gettime(CLOCK_REALTIME,&now)==-1)
                    {
                        printf("clock_gettime error.\n");
                        return -1;
                    }
                    struct itimerspec new_value;
                    new_value.it_value.tv_sec=now.tv_sec+(int)client.life_time;
                    new_value.it_value.tv_nsec=(client.life_time-(int)client.life_time*1.0)*1000000000;
                    new_value.it_interval.tv_sec=0;
                    new_value.it_interval.tv_nsec=0;
                    int timerfd=timerfd_create(CLOCK_REALTIME,0);
                    while(timerfd==-1)
                    {
                        timerfd=timerfd_create(CLOCK_REALTIME,0);
                    }
                    map_timerfd_ids[timerfd]=client.id;
                    addfd(epfd, timerfd, true);
                    timerfd_settime(timerfd,TFD_TIMER_ABSTIME,&new_value,NULL);

                    //send back message
                    char message_send[BUF_SIZE];
                    bzero(message_send, BUF_SIZE);
                    // format message
                    sprintf(message_send, "Server received ClientID=%d 's message.\n",client.id);

                    if(map_id_clients.find(client.id)!=map_id_clients.end())
                    {
                        sendto(listener,message_send,strlen(message_send),0,(struct sockaddr*)&(client.client_address),sizeof(client.client_address));
                    }
                }//end strcmp 00
                else if(strcmp(order,"-1")==0)//此socket退出
                {
                    map<int,CLIENT>::iterator map_int_client_it;
                    map_int_client_it=map_id_clients.find(client.id);

                    if(map_int_client_it!=map_id_clients.end())
                    {
                        printf("USER ID = %d closed. now there are %d users in the satellite.\n", client.id, (int)map_id_clients.size()-1);//zsd

                        switchcaseout(makelevel(map_int_client_it->second.degree,map_int_client_it->second.hdf_type,map_int_client_it->second.bss_type));

                        bzero(buffer,sizeof(buffer));
                        sprintf(buffer,"qos %d %d %s",(int)Umax, 0,"");
                        //向FIFO文件写数据
                        ret_len=write(pipe_fd, buffer, sizeof(buffer));
                        if(ret_len==-1) printf("write error on fifo0\n");

                        map_id_clients.erase(map_int_client_it);

                        map<int,CLIENT>::iterator map_int_CLIENT_itt;
                        for(int c_i=0; c_i<18; c_i++) my_count[c_i]=0;
                        for(map_int_CLIENT_itt=map_id_clients.begin(); map_int_CLIENT_itt!=map_id_clients.end(); ++map_int_CLIENT_itt)
                        {
                            ++my_count[makelevel(map_int_CLIENT_itt->second.degree,map_int_CLIENT_itt->second.hdf_type,map_int_CLIENT_itt->second.bss_type)];
                        }
                    }
                }
                else if(strcmp(order,"01")==0)//发送数据
                {
                    map<int,CLIENT>::iterator map_int_client_it;
                    map_int_client_it=map_id_clients.find(client.id);

                    if(map_int_client_it!=map_id_clients.end()){
                        printf("USER ID = %d says: %s\n", client.id,message);
                        char message_send[BUF_SIZE];
                        bzero(message_send, BUF_SIZE);
                        sprintf(message_send, "Server received User ID=%d 's message.\n",client.id);
                        sendto(listener,message_send,strlen(message_send),0,(struct sockaddr*)&(client.client_address),sizeof(client.client_address));
                    }
                    else{
                        char message_send[BUF_SIZE];
                        bzero(message_send, BUF_SIZE);
                        sprintf(message_send, "Server says: Please send the user's info first.\n");
                        sendto(listener,message_send,strlen(message_send),0,(struct sockaddr*)&(client.client_address),sizeof(client.client_address));
                    }
                }
            }
            //定时器
            else if(map_timerfd_ids.find(fd)!=map_timerfd_ids.end())//删除timefd时，不能删map_timefd，要不然进不来，回到socket那
            {
                int timerfd=fd;
                int id=map_timerfd_ids[timerfd];

                if(map_id_clients.find(id)==map_id_clients.end())//找不到此timefd对应的id对应的client
                {
                    map<int,int>::iterator map_int_int_it;
                    map_int_int_it=map_timerfd_ids.find(timerfd);
                    if(map_int_int_it!=map_timerfd_ids.end()) map_timerfd_ids.erase(map_int_int_it);
                    close(timerfd);
                    delfd(epfd, timerfd, true);
                    continue;
                }

                char message_send[BUF_SIZE];
                bzero(message_send, BUF_SIZE);
                sprintf(message_send, "-2");
                sendto(listener,message_send,strlen(message_send),0,(struct sockaddr*)&(map_id_clients[id].client_address),sizeof(map_id_clients[id].client_address));

                map<int,CLIENT>::iterator map_int_client_it;
                map_int_client_it=map_id_clients.find(id);

                if(map_int_client_it!=map_id_clients.end())
                {
                    printf("\n====================Duration over !!!===================\nUser ID = %d closed. now there are %d users in the satellite.\n\n", id, (int)map_id_clients.size()-1);//zsd

                    bzero(buffer,sizeof(buffer));
                    sprintf(buffer,"timeout %d %d %s",id, (int)map_id_clients.size()-1,"");
                    //向FIFO文件写数据
                    ret_len=write(pipe_fd, buffer, sizeof(buffer));
                    if(ret_len==-1) printf("write error on fifo0\n");

                    switchcaseout(makelevel(map_id_clients[id].degree,map_id_clients[id].hdf_type,map_id_clients[id].bss_type));

                    bzero(buffer,sizeof(buffer));
                    sprintf(buffer,"qos %d %d %s",(int)Umax, 0,"");
                    //向FIFO文件写数据
                    ret_len=write(pipe_fd, buffer, sizeof(buffer));
                    if(ret_len==-1) printf("write error on fifo0\n");

                    map_id_clients.erase(map_int_client_it);
                    map<int,CLIENT>::iterator map_int_CLIENT_itt;
                    for(int c_i=0; c_i<18; c_i++) my_count[c_i]=0;
                    for(map_int_CLIENT_itt=map_id_clients.begin(); map_int_CLIENT_itt!=map_id_clients.end(); ++map_int_CLIENT_itt)
                    {
                        ++my_count[makelevel(map_int_CLIENT_itt->second.degree,map_int_CLIENT_itt->second.hdf_type,map_int_CLIENT_itt->second.bss_type)];
                    }
                }

                close(timerfd);

                delfd(epfd, timerfd, true);

                map<int,int>::iterator map_int_int_it;
                map_int_int_it=map_timerfd_ids.find(timerfd);
                if(map_int_int_it!=map_timerfd_ids.end()) map_timerfd_ids.erase(map_int_int_it);
            }
        }
    }
    close(listener); //关闭socket
    close(epfd); //关闭内核
    close(pipe_fd);
    return 0;
}
