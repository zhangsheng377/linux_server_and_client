#include <sqlite3.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <algorithm>
#include <iostream>
using namespace std;

bool createTable();//´´½¨±í
bool insert();
bool search(int , int);
//bool StartVer(int , int);
string IntToString ( int );
int StringToInt(string );
int GenKey(int );

sqlite3 *DB = NULL;	
char * errMsg = NULL;
int rc = 0;
int nrow = 0, ncolumn = 0;
char **azResult; 