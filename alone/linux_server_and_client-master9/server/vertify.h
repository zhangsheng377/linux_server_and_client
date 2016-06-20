#ifndef VERTIFY_H_INCLUDED
#define VERTIFY_H_INCLUDED

#include "sqlite3.h"
#include <stdio.h>
#include <string>
#include <string.h>
#include <algorithm>
#include <iostream>
using namespace std;

bool CreateDb();
bool createTable();//´´½¨±í
bool insert();
bool search(int , int);
//bool StartVer(int , int);
string IntToString ( int );
int StringToInt(string );
int GenKey(int );

static sqlite3 *DB = NULL;
//static sqlite3 *myDB1=NULL;//zsd
static char * errMsg = NULL;
static int rc = 0;
static int nrow = 0, ncolumn = 0;
static char** azResult;
const int RESULTLEN=10;

bool CreateDb()//1， 建立数据库
{
    rc = sqlite3_open("my.db", &DB);

    if(rc)
    {
        cout<<"打开数据库 失败"<< endl;
        return false;
    }

    createTable();
    insert();
    //sqlite3_close(DB);
    return true;
}

bool search(int ID, int RcvKey) //2, 查询认证
{
    //printf("start search db.\n" );

    RcvKey = RcvKey^8888888;
    //printf("RcvKey = %d\n",RcvKey);//zsd
    char value[500];
    char skey[100];
    int x = ID;

    sprintf(value,"SELECT COUNT(ID) FROM MY WHERE ID=%d", x);
    if(sqlite3_get_table( DB , value , &azResult , &nrow , &ncolumn , &errMsg )!=SQLITE_OK)
    {
        printf("sqlite3_get_table is error !!!! errMsg = %s\n",errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    if((azResult[1][0]-'0')>0)
    {

        //sprintf(value,"SELECT * FROM MY WHERE ID=%d", x);
        sprintf(value,"SELECT MYKEY FROM MY WHERE ID=%d", x);//只查找一个字段

        //printf("start search db 1.\n" );
        //printf("nrow=%d    ncolumn=%d\n",nrow,ncolumn);//zsd
        if(sqlite3_get_table( DB , value , &azResult , &nrow , &ncolumn , &errMsg )!=SQLITE_OK)
        {
            printf("sqlite3_get_table is error !!!! errMsg = %s\n",errMsg);
            sqlite3_free(errMsg);
            return false;
        }
        //printf("end sqlite3_get_table\n");
//

        /*for( int i=0 ; i<( nrow + 1 ) * ncolumn ; i++ )
            printf( "azResult[%d] = %s\n", i , azResult[i] );*/
        //int temp = (nrow + 1) * ncolumn-2;
        int temp=1;//因为只查一个字段，所以就一列，第一行是字段名，所以第二行第一个编号就是1,从0开始
        sprintf(skey, "%d" , RcvKey);
        //printf("%s", azResult[temp]);//hou mian chu xian 0??



        char ResultTmp[RESULTLEN+1];
        //printf("start search db 2.\n" );
        //int k=0;
        int len =strlen(azResult[temp]);
        //printf("len = %d\n",len);
        //printf("start search db 3.\n" );
        //printf("temp = %d\n",temp);
        //printf("ID = %d\n",ID);
        for(int j=0; j<len; j++)
            //int j;
            //for(j=0; azResult[temp][j]!='\0' || j<RESULTLEN; j++)
        {
            //printf("J =%d\n",j);
            ResultTmp[j] = azResult[temp][j];
        }
        //ResultTmp[j]='\0';
        ResultTmp[len]='\0';

//printf("start search db 4.\n" );

        //printf("ResultTmp = %s  , skey = %s \n",  ResultTmp, skey);

        //char ccc[4096];cin>>ccc;

        if (strcmp(ResultTmp, skey) == 0)
            return true;
        else
            return false;
    }
    else
    {
        return false;
    }
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



bool createTable()
{

    int rc = sqlite3_exec(DB, " CREATE TABLE MY(ID INTEGER PRIMARY KEY,MYKEY INTEGER,DEGREE INTEGER);", 0, 0, &errMsg);

    if(rc == SQLITE_OK)
        cout<< "创建表 tblTest 成功!" << endl;
    else
        //cout<<"建表失败："<<errMsg<< endl;
        cout<<" "<<errMsg<< endl;
    return true;
}

bool insert()
{
    int degree;
    for (int i=1; i<2100; i++)
    {
        char value[500];

        int keytemp = GenKey(i);
        int key = keytemp ^ 8888888;
        if (i<700)
            degree = 0;
        else if(i>=700 && i<1400)
            degree = 1;
        else
            degree = 2;
        sprintf(value, "INSERT INTO MY(ID, MYKEY, DEGREE)values(%d, %d,%d)", i, key, degree);
        rc = sqlite3_exec(DB,value,0,0,&errMsg);
        //bzero(value, 500);

    }

    return 1;
}

int searchDegree(int ID) //根据ID查找返回用户等级degree
{
    char value[500];
    int x = ID;
    sprintf(value,"SELECT * FROM MY WHERE ID=%d", x);
    //sprintf(value,"SELECT DEGREE FROM MY WHERE ID=%d", x);
    sqlite3_get_table( DB , value , &azResult , &nrow , &ncolumn , &errMsg );
    //for( int i=0 ; i<( nrow + 1 ) * ncolumn ; i++ )
    //printf( "azResult[%d] = %s\n", i , azResult[i] );
    int tempdegree = (nrow + 1) * ncolumn-1;
    //int tempdegree = 1;
    int RcvDegree  = azResult[tempdegree][0] - '0';
    return RcvDegree;
}
int makelevel(int degree,int handoff,int bss)
{
    if(handoff==0 && bss!=0)
     {
     return 11+bss+2*degree;
     }
    else if(handoff==0)
    {
        return 4*degree+3;
    }
    else
        return bss+degree*4;
}
int returnmyband(int level,int band[])
{
    switch (level)
    {
    case 0:
    case 3:
    case 4:
    case 7:
    case 8:
    case 11:
    {
        return 32;
        break;
    }
    case 1:
    {
        return band[0];
        break;
    }
    case 2:
    {
        return band[1];
        break;
    }
    case 5:
    {
        return band[2];
        break;
    }
    case 6:
    {
        return band[3];
        break;
    }
    case 9:
    {
        return band[4];
        break;
    }
    case 10:
    {
        return band[5];
        break;
    }
    default:
    {
        return band[level];
    }
    }
    return 0;
}
#endif // VERTIFY_H_INCLUDED
