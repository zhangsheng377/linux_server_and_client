#include <vertify.h>

void CreateDb()//1， 建立数据库
{
	rc = sqlite3_open("my.db", &DB);

	    if(rc)
	    {
	        cout<<"打开数据库 失败"<< endl;
        	return 0;
    	    }

        createTable();
        insert(); 
	sqlite3_close(DB);
}

bool search(int ID, int RcvKey) //2, 查询认证 
{
    RcvKey = RcvKey^8888888;
    char value[500];
    char skey[100];
    int x = ID;
    sprintf(value,"SELECT * FROM MY WHERE ID=%d", x);

sqlite3_get_table( DB , value , &azResult , &nrow , &ncolumn , &errMsg );
for( int i=0 ; i<( nrow + 1 ) * ncolumn ; i++ )
printf( "azResult[%d] = %s\n", i , azResult[i] );
     int temp = (nrow + 1) * ncolumn-2;
     sprintf(skey, "%d" , RcvKey);
	//printf("%s", azResult[temp]);//hou mian chu xian 0??
     char ResultTmp[10]; int k=0;int len =strlen(azResult[temp]); 
	for(int j=0; j<len; j++)
		ResultTmp[k++] = azResult[temp][j];
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
    for (int i=0; i<s.size(); i++)
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
       cout<<"建表失败："<<errMsg<< endl;
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



