#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

static int callback(void *data, int argc, char **argv, char **azColName){
    int i;
    //fprintf(stderr, "%s: ", (const char*)data);

    for(i = 0; i<argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }

    printf("\n");
    return 0;
}

int main(int argc, char* argv[]) {
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    char *sql;
    const char* data = "Callback function called";

    /* Open database */
    rc = sqlite3_open("VirtualSoc.db", &db);

    if( rc ) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return(0);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    /* Create SQL statement */
    //sql = "INSERT INTO group_messages (GROUP_ID, MEMBER_ID, CONTENT, NR) VALUES (1,1,\'salut!\',2);";
    //sql = "UPDATE users SET page_type = 0;";
    //sql = "CREATE TABLE GROUP_MESSAGES(GROUP_ID INT, MEMBER_ID INT, CONTENT TEXT, NR INT);";
    //sql =  CREATE TABLE MESSAGES(CONV_ID INT, ID_SENDER INT, MESSAGE TEXT, NR INT);";
    //sql = "DELETE FROM groups WHERE group_id >7;";
    //sql = "INSERT INTO CONVERSATIONS VALUES(1, 1, 3);";
    //sql = "SELECT * FROM users where name = \'Mike\';";
    //sql = "SELECT m.message FROM (SELECT message, nr FROM messages WHERE conv_id = 1 ORDER BY nr DESC LIMIT 3 OFFSET 0) m ORDER BY m.nr;";
    //sql = "SELECT u.name FROM friends f join users u on f.id2 = u.id where f.id1 = 2 UNION SELECT u.name FROM friends f join users u on f.id1 = u.id where f.id2 = 2;";
    //sql = "DELETE FROM group_members WHERE group_id = 1 and member_id = 3;";
    sql = "SELECT * FROM groups";
    //sql = "SELECT m.name ||' : '|| m.message FROM (SELECT u.name, n.message, n.nr FROM messages n JOIN users u ON u.id = n.id_sender WHERE n.conv_id = 2 ORDER BY nr DESC LIMIT 5 OFFSET 0) m ORDER BY m.nr;";
    //sql = "DELETE FROM friend_requests WHERE id_sender = 1 and id_friend = 4 and friend_type = 2;";
    //sql = "CREATE TABLE FRIEND_REQUESTS (ID_SENDER INT, ID_FRIEND INT, FRIEND_TYPE INT);";
    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);

    if( rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        fprintf(stdout, "Operation done successfully\n");
    }
    sqlite3_close(db);
    return 0;
}