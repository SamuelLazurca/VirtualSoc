#include "DataBaseAction.h"
#include <unistd.h>

static int one_callback(void *result, int argc, char **argv, char **azColName) {
    string* r = (string*) result;
    *r = argv[0];

    return 0;
}

static int multiple_callbacks(void *list, int argc, char **argv, char **azColName) {
    int i;

    vector<string>* v = (vector<string>*) list;
    v->push_back(argv[0]);

    return 0;
}

string DataBaseAction::getResult() {
    return oneResult;
}

void DataBaseAction::getResults(int clientDescriptor) {
    int len = 0;

    for(int i = 0; i < multipleResults.size(); i++)
    {
        len = multipleResults[i].length();

        write(clientDescriptor, &len, sizeof(int));
        write(clientDescriptor, multipleResults[i].c_str(), len);
    }
}

DataBaseAction::DataBaseAction(sqlite3 *Db) {
    myDb = Db;
}

int DataBaseAction::execWithOneResult(char *sql_command) {

    int r = sqlite3_exec(myDb, sql_command, one_callback, &this->oneResult, &errMsg);

    if (r != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return r;
    }

    return 0;
}

void DataBaseAction::execWithMultipleResults(char *sql_command) {
    int r = sqlite3_exec(myDb, sql_command, multiple_callbacks, &this->multipleResults, &errMsg);

    if (r != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        //fprintf(stdout, "Succes\n");
    }
}

void DataBaseAction::getAndSentMessages(int clientDescriptor) {
    int len = multipleResults.size();
    int len_msg = 0;

    for(int i = 0; i < len; i++)
    {
        len_msg = multipleResults[i].length();

        if(-1 == write(clientDescriptor, &len_msg, sizeof(int))) return;
        if(-1 == write(clientDescriptor, multipleResults[i].c_str(), len_msg)) return;
    }
}

int DataBaseAction::sendFriendsList(int clientDescriptor) {
    int len = 0;

    for(int i = 0; i < multipleResults.size(); i++)
    {
        len = multipleResults[i].length();

        write(clientDescriptor, &len, sizeof(int));
        write(clientDescriptor, multipleResults[i].c_str(), len);
    }
    return 0;
}

void DataBaseAction::getAndSendGroupMessages(int clientDescriptor) {
    int len = 0;

    for(int i = 0; i < multipleResults.size(); i++)
    {
        len = multipleResults[i].length();
        if(-1 == write(clientDescriptor, &len, sizeof(int))) return;
        if(-1 == write(clientDescriptor, multipleResults[i].c_str(), len)) return;
    }
}
