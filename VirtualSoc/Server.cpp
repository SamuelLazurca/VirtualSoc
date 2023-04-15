//
// Created by samuel on 20.12.2022.
//

#include "Server.h"
#include <string>
#include <cstring>
#include<iterator>

using namespace std;

Server::Server() {

}

void Server::run() {

    if(db.openDataBase()) return;

    if(Socket.Bind()) return;

    if(Socket.Listen()) return;

    while(true)
    {
        if(Socket.AcceptNewClients()) break;

        for(int fd = 0; fd <= Socket.nfds; fd++)
        {
            if(fd != Socket.socketDescriptor && FD_ISSET(fd, &Socket.readFds))
            {
                if(treatClient(fd))
                {
                    disconnectClient(fd, true);
                }
            }
        }
    }

    db.closeDataBase();
}

int Server::treatClient(int clientDescriptor) {

    string command;

    if(receiveCommand(clientDescriptor, command)) return 1;

    if(Quit(clientDescriptor, command)){
        disconnectClient(clientDescriptor, false);
        return 0;
    }

    if(clients.find(clientDescriptor) == clients.end()) {
        int result = processCommandUserWithoutAccount(command, clientDescriptor);

        if (result) return errno;
    }
    else
    {
        int result = processCommand(command, clientDescriptor);

        if (result) return errno;
    }

    return 0;
}

int Server::receiveCommand(int clientDescriptor, string &command) {
    char comm[1024];
    long int length = 0;
    size_t bytes_read;

    if(-1 == (bytes_read = read(clientDescriptor, &length, sizeof(long int)))) return errno;

    if(bytes_read == 0) return 1;

    if(-1 == (bytes_read = read(clientDescriptor, comm, length))) return errno;

    if(bytes_read == 0) return 1;

    command = comm;

    return 0;
}

int Server::processCommand(string command, int clientDescriptor) {
    char command_as_char[1024];

    strcpy(command_as_char, command.c_str());

    if(!valid_command(command_as_char))  return unknown_command(clientDescriptor);

    if(clients[clientDescriptor]->isInConv())
    {
        if (strstr(command_as_char, "exit conv") == command_as_char) { clients[clientDescriptor]->unsetConv(); cout << clients[clientDescriptor]->getUsername() << " exited the conversation." << endl;}
        else return send_message(clientDescriptor, clients[clientDescriptor]->getReceptor(), clients[clientDescriptor]->getConvId(), command_as_char);
    }
    else
    if (clients[clientDescriptor]->inGroup()) {
        if (strstr(command_as_char, "exit group") == command_as_char) return exit_group(clientDescriptor);
        else if (strstr(command_as_char, "add member ") == command_as_char)
            return add_member(clientDescriptor, command_as_char);
        else return send_group_message(clientDescriptor, command_as_char);
    }
    else {
        if (strstr(command_as_char, "post ") == command_as_char) {
            return post(clientDescriptor, command_as_char);
        } else if (strstr(command_as_char, "show posts from ") == command_as_char) {
            return showposts(clientDescriptor, command_as_char, true);
        } else if (strstr(command_as_char, "enter group ") == command_as_char) {
            return enter_group(clientDescriptor, command_as_char);
        } else if (strstr(command_as_char, "create group ") == command_as_char) {
            return create_group(clientDescriptor, command_as_char);
        } else if (strstr(command_as_char, "write ") == command_as_char) {
            return enter_conversation(clientDescriptor, command_as_char);
        } else if (strstr(command_as_char, "logout") == command_as_char) {
            return logout(clientDescriptor);
        } else if (strstr(command_as_char, "my friends list") == command_as_char) {
            return get_my_friends_list(clientDescriptor);
        } else if (strstr(command_as_char, "send friend request ") == command_as_char) {
            return send_friend_request(clientDescriptor, command_as_char);
        }
        else if (strstr(command_as_char, "accept friend request from ") == command_as_char) {
            return accept_as_friend(clientDescriptor, command_as_char);
        }
        else if (strstr(command_as_char, "friend requests") == command_as_char) {
            return friend_requests_list(clientDescriptor);
        }
        else if (strstr(command_as_char, "my posts") == command_as_char) {
            return send_my_posts(clientDescriptor);
        }
        else {
            return unknown_command(clientDescriptor);
        }
    }

    return 0;
}

bool Server::Quit(int clientDescriptor, string command) {
    if(command == "quit") return true;
    return false;
}

int Server::disconnectClient(int clientDescriptor, bool forcedClose) {

    if(!forcedClose) {
        char quit[128] = "Exiting the app...";
        int len = strlen(quit);

        if(-1 == write(clientDescriptor, &len, sizeof(int))) return errno;
        if(-1 == write(clientDescriptor, quit, len)) return errno;

        strcpy(quit, "quit");
        len = strlen(quit);

        if(-1 == write(clientDescriptor, &len, sizeof(int))) return errno;
        if(-1 == write(clientDescriptor, quit, len)) return errno;
    }

    Socket.removeActFds(clientDescriptor);

    if(clients.find(clientDescriptor) != clients.end()) {
        string disconnectedClientName = clients[clientDescriptor]->getUsername();

        if(clients[clientDescriptor]->isInConv()) clients[clientDescriptor]->unsetConv();
        else if(clients[clientDescriptor]->inGroup()) exit_group(clientDescriptor);

        delete clients.find(clientDescriptor)->second;
        clients.erase(clientDescriptor);
        fdMap.erase(disconnectedClientName);
        close(clientDescriptor);
    }

    cout << "[server] S-a deconectat clientul cu descriptorul " << clientDescriptor << "." << endl;

    return 0;
}

int Server::login(int clientDescriptor, char* command) {
    char username[1024];
    char password[1024];
    strcpy(username, command + 6);

    int it = 0;

    while(username[it] != ' ' && username[it] != 0) it++;

    strcpy(password, username + it + 1);
    username[it] = 0;

    if(strlen(username) < 1 || strlen(password) < 1) sendClient(clientDescriptor, "\n\033[31m[Error] Wrong username or password\033[m\n");

    if(clients.find(clientDescriptor) == clients.end()) {
        if (fdMap.find(username) == fdMap.end()) {
            if (correct_credentials(username, password)) {
                DataBaseAction d(db.getMyDb());

                char sql[128];

                sprintf(sql,"SELECT id FROM users WHERE name = \'%s\';", username);

                if(d.execWithOneResult(sql)) return errno;

                int id = atoi(d.getResult().c_str());

                sprintf(sql,"SELECT page_type FROM users WHERE name = \'%s\';", username);

                if(d.execWithOneResult(sql)) return errno;

                Client *newClient = new Client(username, clientDescriptor, password, id, d.getResult().c_str());

                clients[clientDescriptor] = newClient;
                fdMap[username] = clientDescriptor;

                char message[1024] = "\n Welcome back ";
                strcat(message, username);
                strcat(message, "! If you need any help type the following command : \'help\'\n");

                int len = strlen(message) + 1;

                if(-1 == write(clientDescriptor, &len, sizeof(int))) return errno;
                if(-1 == write(clientDescriptor, message, len)) return errno;
            }
            else sendClient(clientDescriptor, "\n\033[31m[Error] Wrong username or password\033[m\n");
        }
    }
    return 0;
}

bool Server::correct_credentials(string username, string password) {
    DataBaseAction d(db.getMyDb());

    char sql[256];
    sprintf(sql, "SELECT count(*) FROM users WHERE name = \'%s\' and password = \'%s\';", username.c_str(), password.c_str());
    if(d.execWithOneResult(sql)) return errno;

    if(atoi(d.getResult().c_str()) == 1) return true;

    return false;
}

bool Server::user_exists(string username)
{
    DataBaseAction d(db.getMyDb());

    char sql[256];
    sprintf(sql, "SELECT count(*) FROM users WHERE name = \'%s\';", username.c_str());
    if(d.execWithOneResult(sql)) return errno;

    if(atoi(d.getResult().c_str()) == 1) return true;

    return false;

}

int Server::create_account(int clientDescriptor, char *command) {
    char username[1024];
    char password[1024];
    char page_type[32];
    char user_type[32];

    strcpy(username, command + 15);

    int it = 0;

    while(username[it] != ' ' && username[it] != 0) it++;

    strcpy(password, username + it + 1);
    username[it] = 0;

    it = 0;
    while(password[it] != ' ' && password[it] != 0) it++;

    strcpy(page_type, password + it + 1);
    password[it] = 0;

    it = 0;

    while(page_type[it] != ' ' && page_type[it] != 0) it++;

    strcpy(user_type, page_type + it + 1);
    page_type[it] = 0;

    cout << username << " " << password << " " << page_type << " " << user_type << endl;

    if(strlen(username) < 1 || strlen(password) < 1) sendClient(clientDescriptor, "\n\033[31m[Error] Too short username or password\033[m\n");

    int p_type, u_type;

    if(strcmp(page_type, "public") == 0) p_type = 0;
    else if(strcmp(page_type, "private") == 0) p_type = 1;
    else return sendClient(clientDescriptor, "\n\033[31m[Error] Page type unknown\033[m\n");

    if(strcmp(user_type, "regular") == 0) u_type = 0;
    else if(strcmp(user_type, "administrator") == 0) u_type = 1;
    else return sendClient(clientDescriptor, "\n\033[31m[Error] User type unknown\033[m\n");

    if(clients.find(clientDescriptor) == clients.end()) {
        if (fdMap.find(username) == fdMap.end()) {
            if(!user_exists(username))
            {
                DataBaseAction d(db.getMyDb());

                d.execWithOneResult("SELECT IFNULL(MAX(ID),0) FROM USERS;");

                int id = atoi(d.getResult().c_str());
                id++;

                if(-1 == insert(username, password, p_type, id, u_type)) return -1;

                sprintf(page_type, "%d", p_type);

                Client *newClient = new Client(username, clientDescriptor, password, id, page_type);

                clients[clientDescriptor] = newClient;
                fdMap[username] = clientDescriptor;

                char message[1024] = "\nHello ";
                strcat(message, username);
                strcat(message, "! If you need any help type the following command : \'help\'\n");

                int len = strlen(message) + 1;

                if(-1 == write(clientDescriptor, &len, sizeof(int))) return errno;
                if(-1 == write(clientDescriptor, message, len)) return errno;

                cout<<"[server] New account created : " << username <<endl;
            }
            else sendClient(clientDescriptor, "\n\033[31m[Error] Username already taken\033[m\n");
        }
        }

            return 0;
}

int Server::insert(string username, string password, int page_type, int id, int user_type) {
    DataBaseAction d(db.getMyDb());

    char sql[128];
    sprintf(sql, "INSERT INTO USERS (ID, NAME, PASSWORD, PAGE_TYPE, USER_TYPE) VALUES(%d, \'%s\', \'%s\', %d, %d);", id, username.c_str(), password.c_str(), page_type, user_type);

    if(d.execWithOneResult(sql)) return errno;

    return 0;
}

int Server::processCommandUserWithoutAccount(string command, int clientDescriptor) {
    char command_as_char[1024];

    strcpy(command_as_char, command.c_str());

    if(!valid_command(command_as_char))  return unknown_command(clientDescriptor);

    if(strstr(command_as_char, "login") == command_as_char)
    {
        return login(clientDescriptor, command_as_char);
    }
    else
    if(strstr(command_as_char, "create account") == command_as_char)
    {
        return create_account(clientDescriptor, command_as_char);
    }
    else
    if(strstr(command_as_char, "show posts from ") == command_as_char)
    {
        return showposts(clientDescriptor, command_as_char, false);
    }
    else
    {
        return unknown_command(clientDescriptor);
    }

    return 0;
}

int Server::post(int clientDescriptor, char *command) {
    DataBaseAction d(db.getMyDb());

    int user_id = clients[clientDescriptor]->get_user_id();

    char sql[4096];

    sprintf(sql, "SELECT COUNT(*) FROM posts WHERE ID_USER = %d;", user_id);
    if(d.execWithOneResult(sql)) return errno;

    int nr = atoi(d.getResult().c_str()) + 1;

    char p_type[64];

    strcpy(p_type, command + 5);

    int it = 0;

    while(p_type[it] != 0 && p_type[it] != ' ') it++;

    char content[1024] = "";

    strcpy(content, p_type + it + 1);

    p_type[it] = 0;

    int post_type;

    if(strcmp(p_type, "public") == 0) post_type = 0;
    else if(strcmp(p_type, "friends") == 0) post_type = 7;
    else if(strcmp(p_type, "relatives") == 0) post_type = 1;
    else if(strcmp(p_type, "close_friends") == 0) post_type = 2;
    else if(strcmp(p_type, "known_friends") == 0) post_type = 3;
    else if(strcmp(p_type, "relatives&close_friends") == 0 || strcmp(p_type, "close_friends&relatives") == 0) post_type = 4;
    else if(strcmp(p_type, "relatives&known_friends") == 0 || strcmp(p_type, "known_friends&relatives") == 0) post_type = 5;
    else if(strcmp(p_type, "close_friends&known_friends") == 0 || strcmp(p_type, "known_friends&close_friends") == 0) post_type = 6;
    else {sendClient(clientDescriptor, "\n\033[31m[Error] Failed to post : lookup the correct form of this command in help\033[m\n"); return 0;}

    if(post_type == 0 && clients[clientDescriptor]->get_page_type() == 1) return sendClient(clientDescriptor, "\n\033[31m[Error] Failed to post : you can't post public because your page is private\033[m\n");

    if(strlen(content) > 0) {
        sprintf(sql, "INSERT INTO posts (ID_USER, CONTENT, NR, P_TYPE) VALUES(%d, \'%s\', %d, %d);", user_id, content, nr, post_type);
        if(d.execWithOneResult(sql)) return errno;
    }
    else
    {
        sendClient(clientDescriptor, "\n\033[31m[Error] Failed to post : lookup the correct form of this command in help\033[m\n");
    }

    return 0;
}

int Server::showposts(int clientDescriptor, char *command, bool connected) {
    DataBaseAction d(db.getMyDb());

    char sql[1024];

    char username[64];
    int id1, id2;

    strcpy(username, command + 16);

    if(!connected)
    {
        if(user_exists(username)) {
            sprintf(sql, "SELECT id FROM users WHERE name = \'%s\';", username);

            if(d.execWithOneResult(sql)) return errno;

            id2 = atoi(d.getResult().c_str());

            char message[128];
            sprintf(message, "\n\033[35m%s posts:\n\033[m", username);
            sendClient(clientDescriptor, message);

            show_public_posts(clientDescriptor, id2);

            sendClient(clientDescriptor, " ");
        }
        else sendClient(clientDescriptor, "\n\033[31m[Error] User doesn't exist\033[m\n");

        cout << "Not connected user with descriptor " << clientDescriptor << " requested to see posts from " << username << "." << endl;

        return 0;
    }

    if(user_exists(username)) {
        id1 = clients[clientDescriptor]->get_user_id();

        sprintf(sql, "SELECT id FROM users WHERE name = \'%s\';", username);

        if(d.execWithOneResult(sql)) return errno;

        id2 = atoi(d.getResult().c_str());

        sprintf(sql, "SELECT FRIENDSHIP_TYPE FROM friends WHERE (ID1 = %d and ID2 = %d) or (ID1 = %d and ID2 = %d);",
                id1, id2, id2, id1);

        if(d.execWithOneResult(sql)) return errno;

        int ftype = atoi(d.getResult().c_str());

        char message[128];
        sprintf(message, "\n\033[35m%s posts:\n\033[m", username);
        sendClient(clientDescriptor, message);

        switch (ftype) {
            case 1 :
                show_relatives_posts(clientDescriptor, id2);
                break;
            case 2 :
                show_close_friends_posts(clientDescriptor, id2);
                break;
            case 3 :
                show_known_ones_posts(clientDescriptor, id2);
                break;
            default :
                show_public_posts(clientDescriptor, id2);
        }

        sendClient(clientDescriptor, " ");
    }
    else sendClient(clientDescriptor, "\n\033[31m[Error] User doesn't exist\033[m\n");

    cout << "User with descriptor " << clientDescriptor << " requested to see posts from " << username << "." << endl;

    return 0;
}

int Server::unknown_command(int clientDescriptor) {
    char message[24] = "Unknown command";

    int len = strlen(message)+1;

    if(-1 == write(clientDescriptor, &len, sizeof(int))) return errno;
    if(-1 == write(clientDescriptor, message, len)) return errno;

    return 0;
}

int Server::show_public_posts(int clientDescriptor, int id) {
    DataBaseAction d(db.getMyDb());

    char sql[1024];

    sprintf(sql, "SELECT CONTENT FROM posts WHERE id_user = \'%d\' and p_type = %d ORDER BY nr;", id, 0);

    d.execWithMultipleResults(sql);

    d.getResults(clientDescriptor);
    return 0;
}

int Server::show_relatives_posts(int clientDescriptor, int id) {
    DataBaseAction d(db.getMyDb());

    char sql[1024];

    sprintf(sql, "SELECT CONTENT FROM posts WHERE id_user = \'%d\' and p_type IN (%d, %d, %d, %d. %d) ORDER BY nr;", id, 0, 1, 4, 5, 7);

    d.execWithMultipleResults(sql);

    d.getResults(clientDescriptor);
    return 0;
}

int Server::show_close_friends_posts(int clientDescriptor, int id) {
    DataBaseAction d(db.getMyDb());

    char sql[1024];

    sprintf(sql, "SELECT CONTENT FROM posts WHERE id_user = \'%d\' and p_type IN (%d, %d, %d, %d, %d) ORDER BY nr;", id, 0, 2, 4, 6, 7);

    d.execWithMultipleResults(sql);

    d.getResults(clientDescriptor);
    return 0;
}

int Server::show_known_ones_posts(int clientDescriptor, int id) {
    DataBaseAction d(db.getMyDb());

    char sql[1024];

    sprintf(sql, "SELECT CONTENT FROM posts WHERE id_user = \'%d\' and p_type IN (%d, %d, %d, %d, %d) ORDER BY nr;", id, 0, 3, 6, 5, 7);

    d.execWithMultipleResults(sql);

    d.getResults(clientDescriptor);
    return 0;
}

int Server::send_message(int clientDescriptor, string receptor, int convId, char* message) {
    DataBaseAction d(db.getMyDb());

    /*char receiver[1024];
    char message[1024];
    int receiverId;
    int senderId = clients[clientDescriptor]->get_user_id();

    int k = 0, i = 5;
    bool next = false;

    while(command[i] != 0)
    {
        if(next == false) {
            if (command[i] == ' ') { next = true; receiver[k] = 0; k = -1;}
            else receiver[k] = command[i];
        }
        else message[k] = command[i];

        i++;
        k++;
    }
    message[k] = 0;

    receiverId = getId(receiver, &d);

    if(theyAreFriends(senderId, receiverId)) {
        if (fdMap.find(receiver) != fdMap.end()) {

            int receptorDescriptor = fdMap[receiver];

            int len = strlen(message) + 1;

            write(receptorDescriptor, &len, sizeof(int));
            write(receptorDescriptor, message, len);
        }
    }*/

    if(fdMap.find(receptor) != fdMap.end())
    {
        int receptorDescriptor = fdMap[receptor];

        int len = strlen(message) + 1;

        if(clients[receptorDescriptor]->inConv(convId))
        {
            if(-1 == write(receptorDescriptor, &len, sizeof(int))) return errno;
            if(-1 == write(receptorDescriptor, message, len)) return errno;
        }
    }

    char sql[128];

    sprintf(sql, "SELECT IFNULL(MAX(nr), 0) FROM messages WHERE conv_id = %d;", convId);
    if(d.execWithOneResult(sql)) return errno;

    int nr = atoi(d.getResult().c_str()) + 1;

    sprintf(sql, "INSERT INTO messages (CONV_ID, ID_SENDER, MESSAGE, NR) VALUES(%d, %d, \'%s\', %d);", convId, clients[clientDescriptor]->get_user_id(), message, nr);
    if(d.execWithOneResult(sql)) return errno;

    return 0;
}

int Server::getId(char *username, DataBaseAction *d) {
    char sql[1024];

    sprintf(sql, "SELECT id FROM users WHERE name = \'%s\';", username);

    if(d->execWithOneResult(sql)) return errno;

    return atoi(d->getResult().c_str());
}

bool Server::theyAreFriends(int id1, int id2) {
    DataBaseAction d(db.getMyDb());

    char sql[1024];

    sprintf(sql, "SELECT COUNT(*) FROM friends WHERE (id1 = %d and id2 = %d) or (id1 = %d and id2 = %d);", id1, id2, id2, id1);

    if(d.execWithOneResult(sql)) return false;

    if(atoi(d.getResult().c_str()) == 1) return true;
    else return false;
}

int Server::enter_group(int clientDescriptor, char *command) {
    int userId = clients[clientDescriptor]->get_user_id();
    char groupname[100];
    int groupId;

    strcpy(groupname, command + 12);

    DataBaseAction d(db.getMyDb());

    char sql[1024];

    sprintf(sql, "SELECT COUNT(*) FROM groups WHERE groupname = \'%s\';", groupname);

    if(d.execWithOneResult(sql)) return errno;

    if(atoi(d.getResult().c_str()) == 1)
    {
        sprintf(sql, "SELECT group_id FROM groups WHERE groupname = \'%s\';", groupname);

        if(d.execWithOneResult(sql)) return errno;

        groupId = atoi(d.getResult().c_str());

        sprintf(sql, "SELECT COUNT(*) FROM group_members WHERE group_id = %d and member_id = %d;", groupId, userId);

        if(d.execWithOneResult(sql)) return errno;
cout << groupId << " " <<userId << endl;
        if(atoi(d.getResult().c_str()) == 1)
        {
            if(groups.find(groupname)!=groups.end())
            {
                groups[groupname]->membersFd.push_back(clientDescriptor);
                clients[clientDescriptor]->setGroup(groupname);

                cout << "User " << clients[clientDescriptor]->getUsername() << " entered the group chat " << groupname << ". The group had already active members." << endl;
            }
            else
            {
                sprintf(sql, "SELECT admin_id FROM groups WHERE group_id = %d;", groupId);
                if(d.execWithOneResult(sql)) return errno;

                Group *newGroup = new Group;
                newGroup->groupId = groupId;
                newGroup->groupname = groupname;
                newGroup->adminId = atoi(d.getResult().c_str());
                newGroup->membersFd.push_back(clientDescriptor);

                groups[groupname] = newGroup;

                clients[clientDescriptor]->setGroup(groupname);

                cout << "User " << clients[clientDescriptor]->getUsername() << " entered the group chat " << groupname << ". The group didn't have active members." << endl;
            }
            char message[128];
            sprintf(message, "\n\033[35mWelcome to %s!\nIf you want to exit this group conversation type \'exit group\'.\n\033[m", groupname);
            sendClient(clientDescriptor, message);

            latestGroupMessages(clientDescriptor, groupId, 5, 0);
        }
    }

    return 0;
}

int Server::exit_group(int clientDescriptor) {
    string groupname = clients[clientDescriptor]->groupName();
    vector<int>::iterator it;

    for (it = groups[groupname]->membersFd.begin(); it < groups[groupname]->membersFd.end(); it++)
    {
        if(*it == clientDescriptor)
        {
            groups[groupname]->membersFd.erase(it);
            break;
        }
    }

    cout << "User " << clients[clientDescriptor]->getUsername() << " exited the group chat " << groupname << "." << endl;

    if(groups[groupname]->membersFd.size() == 0)
    {
        delete groups[groupname];
        groups.erase(groupname);

        cout << "Group " << groupname << " didn't have any remaining active members so it was eliminated from the active groups list." << endl;
    }

    clients[clientDescriptor]->unsetGroup(groupname);

    return 0;
}

int Server::send_group_message(int clientDescriptor, char *message) {
    DataBaseAction d(db.getMyDb());
    char sql[128];
    char full_message[1024];

    string groupname = clients[clientDescriptor]->groupName();

    int len = groups[groupname]->membersFd.size();
    int nr = 0;

    strcpy(full_message, clients[clientDescriptor]->getUsername().c_str());
    strcat(full_message, " : ");
    strcat(full_message, message);

    int len_msg = strlen(full_message);

    for (int i = 0; i < len; i++)
    {
        int fd = groups[groupname]->membersFd[i];

        if(fd != clientDescriptor)
        {
            if(-1 == write(fd, &len_msg, sizeof(int))) return errno;
            if(-1 == write(fd, full_message, len_msg)) return errno;
        }
    }

    sprintf(sql, "SELECT IFNULL(MAX(nr),0) FROM group_messages WHERE group_id = %d;", groups[groupname]->groupId);
    if(d.execWithOneResult(sql)) return errno;
    nr = atoi(d.getResult().c_str()) + 1;

    sprintf(sql, "INSERT INTO group_messages (GROUP_ID, MEMBER_ID, CONTENT, NR) VALUES(%d, %d, \'%s\', %d);", groups[groupname]->groupId, clients[clientDescriptor]->get_user_id(), message, nr);
    d.execWithOneResult(sql);

    return 0;
}

int Server::create_group(int clientDescriptor, char *command) {
    DataBaseAction d(db.getMyDb());

    int userId = clients[clientDescriptor]->get_user_id();
    char groupname[100];
    int groupId;
    int adminId;

    char sql[512];

    strcpy(groupname, command + 13);

    sprintf(sql, "SELECT COUNT(*) FROM groups WHERE groupname = \'%s\';", groupname);

    d.execWithOneResult(sql);

    if(atoi(d.getResult().c_str()) == 1) return sendClient(clientDescriptor, "\n\033[31m Group with same name already exists!\033[m\n");

    d.execWithOneResult("SELECT IFNULL(MAX(GROUP_ID), 0) FROM groups;");

    groupId = atoi(d.getResult().c_str()) + 1;

    adminId = clients[clientDescriptor]->get_user_id();

    sprintf(sql, "INSERT INTO groups (GROUP_ID, GROUPNAME, ADMIN_ID) VALUES(%d, \'%s\', %d);", groupId, groupname, adminId);

    if(d.execWithOneResult(sql)) return errno;
    cout << groupId <<endl << adminId << endl;

    sprintf(sql, "INSERT INTO group_members (GROUP_ID, MEMBER_ID) VALUES(%d, %d);", groupId, adminId);

    if(d.execWithOneResult(sql)) return errno;

    Group *newGroup = new Group;
    newGroup->groupId = groupId;
    newGroup->groupname = groupname;
    newGroup->adminId = adminId;
    newGroup->membersFd.push_back(clientDescriptor);

    groups[groupname] = newGroup;

    clients[clientDescriptor]->setGroup(groupname);

    cout << "Group " << groupname << " was created by " << clients[clientDescriptor]->getUsername() << endl;

    clients[clientDescriptor]->setGroup(groupname);

    sendClient(clientDescriptor, "\n\033[35m Group created succesfully!\033[m\n");

    cout << "User " << clients[clientDescriptor]->getUsername() << " entered the group chat " << groupname << "." << endl;

    return 0;
}

int Server::add_member(int clientDescriptor, char *command) {
    DataBaseAction d(db.getMyDb());

    if(clients[clientDescriptor]->get_user_id() == groups[clients[clientDescriptor]->groupName()]->adminId)
    {
        char username[64];

        strcpy(username, command + 11);

        if(user_exists(username)) {
            char sql[128];

            sprintf(sql, "SELECT ID FROM users WHERE name = \'%s\';", username);
            if(d.execWithOneResult(sql)) return errno;

            int id_friend = atoi(d.getResult().c_str()); cout<<id_friend;

            if (theyAreFriends(clients[clientDescriptor]->get_user_id(), id_friend)) {
                sprintf(sql, "INSERT INTO group_members (GROUP_ID, MEMBER_ID) VALUES(%d, %d);", groups[clients[clientDescriptor]->groupName()]->groupId, id_friend);
                if(d.execWithOneResult(sql)) return errno;

                cout << clients[clientDescriptor]->getUsername() << " added a new member " << username << " to group " << clients[clientDescriptor]->groupName() << "." << endl;
            }
            else sendClient(clientDescriptor, "\n\033[31m[Error] You can't add this person to this group because you're not friends\033[m\n");
        }
        else sendClient(clientDescriptor, "\n\033[31m[Error] You can't add this person to this group because this account doesn't exist\033[m\n");
    }
    else sendClient(clientDescriptor, "\n\033[31m[Error] You can't add new members to this group because you're not the administrator of it\033[m\n");
    return 0;
}

int Server::enter_conversation(int clientDescriptor, char *command) {
    DataBaseAction d(db.getMyDb());
    char receiver[128];
    int convId;

    int id1, id2;

    strcpy(receiver, command + 6);

    if(user_exists(receiver))
    {
        id1 = clients[clientDescriptor]->get_user_id();
        id2 = getId(receiver, &d);

        if(theyAreFriends(id1, id2))
        {
            cout << "User " << clients[clientDescriptor]->getUsername() << " entered the conversation with " << receiver << "." << endl;
            char message[128];
            sprintf(message, "\n\033[35mPrivate conversation with %s\nIf you want to exit this conversation type \'exit conv\'.\n\033[m", receiver);
            sendClient(clientDescriptor, message);

            if (conv_exists(id1, id2)) {
                convId = getConvId(id1, id2);
                cout << "The conversation already existed. (conv_id = " << convId << ")." << endl;
                latestMessages(convId, clientDescriptor, 5, 0);
            } else {
                convId = createConv(id1, id2);
                cout << "The conversation didn't exist, so it was added to the database. (conv_id = " << convId << ")." << endl;
            }
            clients[clientDescriptor]->setConv(receiver, convId);
        }
        else sendClient(clientDescriptor, "\n\033[31m\n[Error] You can't chat with this person because you're not friends\n\033[m\n");

    }
    else sendClient(clientDescriptor, "\n\033[31m\n[Error] You can't chat with this person because this account doesn't exist\n\033[m\n");

    return 0;
}

bool Server::conv_exists(int id1, int id2) {
    DataBaseAction d(db.getMyDb());
    char sql[1024];

    sprintf(sql, "SELECT COUNT(*) FROM conversations WHERE (id1 = %d and id2 = %d) or (id1 = %d and id2 = %d);", id1, id2, id2, id1);

    if(d.execWithOneResult(sql)) return errno;

    if(atoi(d.getResult().c_str()) == 1) return true;
    else return false;
}

int Server::getConvId(int id1, int id2) {
    DataBaseAction d(db.getMyDb());
    char sql[1024];

    sprintf(sql, "SELECT conv_id FROM conversations WHERE (id1 = %d and id2 = %d) or (id1 = %d and id2 = %d);", id1, id2, id2, id1);

    if(d.execWithOneResult(sql)) return errno;

    return atoi(d.getResult().c_str());
}

int Server::createConv(int id1, int id2) {
    DataBaseAction d(db.getMyDb());
    char sql[1024];
    int newConvId;

    sprintf(sql, "SELECT IFNULL(MAX(CONV_ID), 0) FROM conversations;");

    if(d.execWithOneResult(sql)) return errno;

    newConvId = atoi(d.getResult().c_str()) + 1;

    sprintf(sql, "INSERT INTO conversations (CONV_ID, ID1, ID2) VALUES(%d, %d, %d);", newConvId, id1, id2);

    if(d.execWithOneResult(sql)) return errno;

    return newConvId;
}

int Server::latestMessages(int convId, int clientDescriptor, int limit, int offset) {
    DataBaseAction d(db.getMyDb());

    char sql[256];

    sprintf(sql, "SELECT m.name || ' : ' || m.message FROM (SELECT name, message, nr FROM messages JOIN users ON id = id_sender WHERE conv_id = %d ORDER BY nr DESC LIMIT %d OFFSET %d) m ORDER BY m.nr;", convId, limit, offset);
    d.execWithMultipleResults(sql);

    d.getAndSentMessages(clientDescriptor);

    return 0;
}

int Server::logout(int clientDescriptor) {
    if(clients.find(clientDescriptor) != clients.end()) {
        string disconnectedClientName = clients[clientDescriptor]->getUsername();

        delete clients.find(clientDescriptor)->second;
        clients.erase(clientDescriptor);
        fdMap.erase(disconnectedClientName);

        sendClient(clientDescriptor, "\n\033[35mLogout succesfully!\033[m\n");
    }

    cout << "[server] Clientul cu descriptorul " << clientDescriptor << " a facut logout." << endl;

    return 0;
}

int Server::sendClient(int clientDescriptor, char* message) {
    int bytes_written = 0;
    int len = strlen(message);

    if(-1 == (bytes_written = write(clientDescriptor, &len, sizeof(len)))) return errno;
    if(-1 == (bytes_written = write(clientDescriptor, message, len))) return errno;

    return 0;
}

int Server::get_my_friends_list(int clientDescriptor) {
    DataBaseAction d(db.getMyDb());
    char sql[256];

    sprintf(sql, "SELECT u.name FROM friends f join users u on f.id2 = u.id where f.id1 = %d UNION SELECT u.name FROM friends f join users u on f.id1 = u.id where f.id2 = %d;", clients[clientDescriptor]->get_user_id(), clients[clientDescriptor]->get_user_id());
    d.execWithMultipleResults(sql);
    sendClient(clientDescriptor, "\n List of friends:\n");
    d.sendFriendsList(clientDescriptor);
    sendClient(clientDescriptor, " ");

    return 0;
}

int Server::latestGroupMessages(int clientDescriptor, int groupId, int limit, int offset) {
    DataBaseAction d(db.getMyDb());

    char sql[256];

    sprintf(sql, "SELECT m.name || ' : ' || m.content FROM (SELECT name, content, nr FROM group_messages JOIN users ON id = member_id WHERE group_id = %d ORDER BY nr DESC LIMIT %d OFFSET %d) m ORDER BY m.nr;", groupId, limit, offset);

    d.execWithMultipleResults(sql);

    d.getAndSendGroupMessages(clientDescriptor);

    return 0;
}

int Server::send_friend_request(int clientDescriptor, char* command_as_char) {
    DataBaseAction d(db.getMyDb());

    int friend_type;

    char s_friend_type[32];
    char username[64];

    strcpy(s_friend_type, command_as_char + 20);

    int i = 0;

    while(s_friend_type[i] != 0 && s_friend_type[i] != ' ') i++;

    strcpy(username, s_friend_type + i + 1);

    s_friend_type[i] = 0; cout << username << endl;

    if(strcmp(s_friend_type, "relative") == 0)  friend_type = 1;
    else if(strcmp(s_friend_type, "close_friend") == 0)  friend_type = 2;
    else if(strcmp(s_friend_type, "known_one") == 0)  friend_type = 3;
    else return 0;

    if(user_exists(username))
    {
        char sql[1028];

        int myId = clients[clientDescriptor]->get_user_id();
        int friendId = getId(username, &d);

        if(!theyAreFriends(myId, friendId)) {
            sprintf(sql,
                    "SELECT COUNT(*) FROM friend_requests WHERE (id_sender = %d and id_friend = %d) or (id_sender = %d and id_friend = %d);",
                    myId, friendId, friendId, myId);

            if(d.execWithOneResult(sql)) return errno;

            if (atoi(d.getResult().c_str()) == 0) {
                sprintf(sql, "INSERT INTO friend_requests (ID_SENDER, ID_FRIEND, FRIEND_TYPE) VALUES(%d, %d, %d);",
                        myId,
                        friendId, friend_type);

                if(d.execWithOneResult(sql)) return errno;

                sendClient(clientDescriptor, "\n\033[35m Friend request sent!\033[m\n");
            }
        }
        else sendClient(clientDescriptor, "\n\033[33m[Warning] You are already friends\033[m\n");
    }
    else sendClient(clientDescriptor, "\n\033[31m[Error] User doesn't exist\033[m\n");

    return 0;
}

int Server::accept_as_friend(int clientDescriptor, char *command_as_char) {
    DataBaseAction d(db.getMyDb());

    char username[64];
    strcpy(username, command_as_char + 27);

    if(user_exists(username)) {
        int friendId = getId(username, &d);
        int myId = clients[clientDescriptor]->get_user_id();

        if (!theyAreFriends(friendId, myId)) {
            char sql[128];

            sprintf(sql, "SELECT COUNT(*) FROM friend_requests WHERE id_sender = %d", friendId);

            if(d.execWithOneResult(sql)) return errno;

            if (atoi(d.getResult().c_str()) == 1) {
                sprintf(sql, "SELECT friend_type FROM friend_requests WHERE id_sender = %d and id_friend = %d;",
                        friendId, myId);

                d.execWithOneResult(sql);

                int friendship_type = atoi(d.getResult().c_str());

                sprintf(sql, "INSERT INTO friends (ID1, ID2, FRIENDSHIP_TYPE) VALUES(%d, %d, %d);", friendId, myId,
                        friendship_type);

                if(d.execWithOneResult(sql)) return errno;

                sprintf(sql,
                        "DELETE FROM friend_requests WHERE id_sender = %d and id_friend = %d and friend_type = %d;",
                        friendId, myId, friendship_type);

                if(d.execWithOneResult(sql)) return errno;

                sendClient(clientDescriptor, "\n\033[35m You are now friends!\033[m\n");
            }
        } else sendClient(clientDescriptor, "\n\033[33m[Warning] You are already friends\033[m\n");
    }
    else sendClient(clientDescriptor, "\n\033[31m[Error] User doesn't exist\033[m\n");


    return 0;
}

int Server::friend_requests_list(int clientDescriptor) {
    DataBaseAction d(db.getMyDb());

    sendClient(clientDescriptor, "\n\033[35m List of friend requests:\033[m\n");

    char sql[128];

    sprintf(sql, "SELECT u.name FROM users u JOIN friend_requests f ON u.id = f.id_sender and id_friend = %d;", clients[clientDescriptor]->get_user_id());

    d.execWithMultipleResults(sql);

    d.getResults(clientDescriptor);

    sendClient(clientDescriptor, " ");

    sendClient(clientDescriptor, " ");

    return 0;
}

int Server::send_my_posts(int clientDescriptor) {
    DataBaseAction d(db.getMyDb());

    char sql[128];

    sprintf(sql, "SELECT content FROM posts WHERE id_user = %d ORDER BY nr;", clients[clientDescriptor]->get_user_id());

   d.execWithMultipleResults(sql);

    d.getResults(clientDescriptor);

    return 0;
}

bool Server::valid_command(char *command) {
    int len = strlen(command);
    for(int i = 0; i < len; i++)
        if(!((command[i] >= 'a' && command[i] <= 'z') || (command[i] >= 'A' && command[i] <= 'Z') || (command[i] >= '0' && command[i] <= '9') || strchr("?!.,_:;() ", command[i]) != NULL))
            return false;

    return true;
}
