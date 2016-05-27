#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>

int ID=0;
int total_clients=0;
int band=0;
int timeoutID=0;
int rejectdbID=0;
int accessdbID=0;
int rejecthdfID=0;
int accesshdfID=0;
int comeIDthrow=0;
char throwID1[4096];
char throwID2[4096];

void showdetail()
{
    //system("clear");
    printf("\033[2J");
    printf("\033[0;0H");//把光标定位在0,0
    printf("==================== Refreshing area =================== \n");
    printf("The coming client's id : %4d\n",ID);
    printf("The bandwidth allocated to the client is  : %3d\n",band);
    printf("Now there are %4d cliens in the satellite. \n",total_clients);
    printf("\n");
    printf("==================== Emergency area ==================== \n");
    printf("NO. %4d client is timeout. \n",timeoutID);
    printf("-------------------------------------------------------- \n");
    printf("Reject NO. %4d client, because his authentication failed. \n",rejectdbID);
    printf("-------------------------------------------------------- \n");
    printf("Reject NO. %4d client, because the bandwidth isn't enough. \n",rejecthdfID);
    printf("-------------------------------------------------------- \n");
    char throwID[4096*2+1];
    strcpy(throwID,throwID1);
    strcat(throwID,throwID2);
    printf("Beacuse of the NO. %4d client coming, we need to throw out clients NO. %s \n",comeIDthrow,throwID);
}

int main()
{
    const char *fifo_name = "/tmp/my_fifo";
    int pipe_fd = -1;
    //int data_fd = -1;
    int res = 0;
    int open_mode = O_RDONLY;
    char buffer[PIPE_BUF + 1];
    int bytes_read = 0;
    //int bytes_write = 0;
    //清空缓冲数组
    memset(buffer, '\0', sizeof(buffer));

    printf("Process %d opening FIFO O_RDONLY\n", getpid());
    //以只读阻塞方式打开管道文件，注意与fifowrite.c文件中的FIFO同名
    pipe_fd = open(fifo_name, open_mode);
    //以只写方式创建保存数据的文件
    //data_fd = open("DataFormFIFO.txt", O_WRONLY|O_CREAT, 0644);
    printf("Process %d result %d\n",getpid(), pipe_fd);

    if(pipe_fd != -1)
    {
        /*do
        {
            //读取FIFO中的数据，并把它保存在文件DataFormFIFO.txt文件中
            res = read(pipe_fd, buffer, PIPE_BUF);
            bytes_write = write(data_fd, buffer, res);
            bytes_read += res;
        }while(res > 0);*/
        while(true)
        {
            bzero(buffer,sizeof(buffer));
            res = read(pipe_fd, buffer, PIPE_BUF);
            if(res>0 && buffer[0]!='\0')
            {
                //printf("%s\n",buffer);
                char cs1[4096],cs2[4096];
                int i1=0,i2=0;
                sscanf(buffer,"%s %d %d %s",cs1,&i1,&i2,cs2);
                if(strcmp(cs1,"timeout")==0){
                    timeoutID=i1;
                    total_clients=i2;
                }
                else if(strcmp(cs1,"rejectdb")==0){
                    rejectdbID=i1;
                    total_clients=i2;
                }
                else if(strcmp(cs1,"accessdb")==0){
                    accessdbID=i1;
                    //ID=i1;
                    total_clients=i2;
                }
                else if(strcmp(cs1,"rejecthdf")==0){
                    rejecthdfID=i1;
                    total_clients=i2;
                }
                else if(strcmp(cs1,"accesshdf")==0){
                    accesshdfID=i1;
                    ID=i1;
                    band=i2;
                }
                else if(strcmp(cs1,"throw1")==0){
                    comeIDthrow=i1;
                    ID=i1;
                    strcpy(throwID1,cs2);
                }
                else if(strcmp(cs1,"throw2")==0){
                    if(comeIDthrow!=i1){
                        comeIDthrow=i1;
                        strcpy(throwID1,"");
                    }
                    ID=i1;
                    strcpy(throwID1,cs2);
                }
                else if(strcmp(cs1,"come")==0){
                    ID=i1;
                    band=i2;
                }
            }
            showdetail();
        }
        close(pipe_fd);
        //close(data_fd);
    }
    else
        exit(EXIT_FAILURE);

    printf("Process %d finished, %d bytes read\n", getpid(), bytes_read);
    exit(EXIT_SUCCESS);
}
