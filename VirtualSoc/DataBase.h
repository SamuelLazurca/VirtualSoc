//
// Created by samuel on 30.12.2022.
//

#ifndef VIRTUALSOC1_DATABASE_H
#define VIRTUALSOC1_DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

using namespace std;

class DataBase {
    sqlite3 *myDb;
    char* errMsg = 0;
    string dbName = "VirtualSoc.db";
    string singleResult;
    vector<string> multipleResults;
public:

    int openDataBase();
    void closeDataBase();
    sqlite3* getMyDb();
};


#endif //VIRTUALSOC1_DATABASE_H
