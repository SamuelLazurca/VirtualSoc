
#ifndef SQLITETEST_DATABASE_H
#define SQLITETEST_DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

using namespace std;

class DataBaseAction {
    sqlite3 *myDb;
    char* errMsg = 0;
    string oneResult;
    vector<string> multipleResults;
public:
    DataBaseAction(sqlite3 *Db);
    int execWithOneResult(char* sql_command);
    void execWithMultipleResults(char* sql_command);
    string getResult();
    void getResults(int clientDescriptor);
    int sendFriendsList(int clientDescriptor);
    void getAndSentMessages(int clientDescriptor);
    void getAndSendGroupMessages(int clientDescriptor);
};


#endif //SQLITETEST_DATABASE_H
