//
// Created by samuel on 20.12.2022.
//

#ifndef VIRTUALSOC1_SERVER_H
#define VIRTUALSOC1_SERVER_H

#include "SocketAction.h"
#include "Client.h"
#include "DataBaseAction.h"
#include "DataBase.h"

#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

struct Group
{
    string groupname;
    int groupId;
    int adminId;

    vector<int> membersFd;
};

class Server {
    DataBase db;
    SocketAction Socket;
    unordered_map<string, Group*> groups;
    unordered_map<int, Client*> clients;
    unordered_map<string, int> fdMap;

public:
    Server();
    void run();
    int treatClient(int clientDescriptor);
    int receiveCommand(int clientDescriptor, string &command);
    int processCommandUserWithoutAccount(string command, int clientDescriptor);
    int processCommand(string command, int clientDescriptor);
    int login(int clientDescriptor, char* command);
    int create_account(int clientDescriptor, char* command);
    bool Quit(int clientDescriptor, string command);
    bool correct_credentials(string username, string password);
    bool user_exists(string username);
    int disconnectClient(int clientDescriptor, bool forcedClose);
    int insert(string username, string password, int page_type, int user_id, int user_type);
    int post(int clientDescriptor, char* command);
    int showposts(int clientDescriptor, char* command, bool connected);
    int unknown_command(int clientDescriptor);
    int show_public_posts(int clientDescriptor, int id);
    int show_relatives_posts(int clientDescriptor, int id);
    int show_close_friends_posts(int clientDescriptor, int id);
    int show_known_ones_posts(int clientDescriptor, int id);
    int send_message(int clientDescriptor, string receptor, int convId, char* message);
    int getId(char* username, DataBaseAction *d);
    bool theyAreFriends(int id1, int id2);
    int enter_group(int clientDescriptor, char* command);
    int create_group(int clientDescriptor, char* command);
    int exit_group(int clientDescriptor);
    int send_group_message(int clientDescriptor, char* message);
    int add_member(int clientDescriptor, char* command);
    int enter_conversation(int clientDescriptor, char* command);
    bool conv_exists(int id1, int id2);
    int getConvId(int id1, int id2);
    int createConv(int id1, int id2);
    int latestMessages(int convId, int clientDescriptor, int min, int max);
    int logout(int clientDescriptor);
    int sendClient(int clientDescriptor, char* message);
    int get_my_friends_list(int clientDescriptor);
    int latestGroupMessages(int clientDescriptor, int groupId, int limit, int offset);
    int send_friend_request(int clientDescriptor, char* command_as_char);
    int accept_as_friend(int clientDescriptor, char* command_as_char);
    int friend_requests_list(int clientDescriptor);
    int send_my_posts(int clientDescriptor);
    bool valid_command(char* command);
};


#endif //VIRTUALSOC1_SERVER_H
