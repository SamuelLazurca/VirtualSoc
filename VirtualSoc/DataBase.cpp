//
// Created by samuel on 30.12.2022.
//

#include "DataBase.h"

int DataBase::openDataBase() {
    int r = sqlite3_open(dbName.c_str(), &myDb);

    if( r ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(myDb));
        return errno;
    } else {
        fprintf(stderr, "Opened database successfully\n");
        return 0;
    }
}

void DataBase::closeDataBase() {
    sqlite3_close(myDb);
    fprintf(stderr, "Closed database successfully\n");
}

sqlite3 *DataBase::getMyDb() {
    return myDb;
}
