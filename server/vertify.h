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
static char **azResult;

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
    char value[500];
    char skey[100];
    int x = ID;
    sprintf(value,"SELECT * FROM MY WHERE ID=%d", x);

//printf("start search db 1.\n" );

    sqlite3_get_table( DB , value , &azResult , &nrow , &ncolumn , &errMsg );

//printf("nrow=%d    ncolumn=%d\n",nrow,ncolumn);//zsd

    for( int i=0 ; i<( nrow + 1 ) * ncolumn ; i++ )
        printf( "azResult[%d] = %s\n", i , azResult[i] );
    int temp = (nrow + 1) * ncolumn-2;
    sprintf(skey, "%d" , RcvKey);
    //printf("%s", azResult[temp]);//hou mian chu xian 0??

    printf("start search db 2.\n" );

    char ResultTmp[10];
    int k=0;
    int len =strlen(azResult[temp]);

//printf("start search db 3.\n" );

    for(int j=0; j<len; j++)
        ResultTmp[k++] = azResult[temp][j];

//printf("start search db 4.\n" );

    if (strcmp(ResultTmp, skey) == 0)
        return true;
    else
        return false;

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
    sqlite3_get_table( DB , value , &azResult , &nrow , &ncolumn , &errMsg );
    //for( int i=0 ; i<( nrow + 1 ) * ncolumn ; i++ )
    //printf( "azResult[%d] = %s\n", i , azResult[i] );
    int tempdegree = (nrow + 1) * ncolumn-1;
    int RcvDegree  = azResult[tempdegree][0] - '0';
    return RcvDegree;
}

#endif // VERTIFY_H_INCLUDED
