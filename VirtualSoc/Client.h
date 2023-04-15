//
// Created by samuel on 20.12.2022.
//

#ifndef VIRTUALSOC1_CLIENT_H
#define VIRTUALSOC1_CLIENT_H

#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

struct GroupConv
{
    bool In;
    string groupname;
};

struct Conversation
{
    string receptor;
    bool In;
    int idConv;
};

class Client {

    string username;
    string page_type;
    string password;
    int user_id;

    GroupConv grup;
    Conversation conv;

    int clientDescriptor;

public:
    Client(string username, int fd, string password, int id, string page_type);
    string getUsername();
    int get_user_id();
    int get_fd();
    void setGroup(string groupname);
    void unsetGroup(string groupname);
    bool inGroup();
    string groupName();
    void setConv(string receptor, int convId);
    void unsetConv();
    bool inConv(int convId);
    bool isInConv();
    int getConvId();
    string getReceptor();
    int get_page_type();
};


#endif //VIRTUALSOC1_CLIENT_H
