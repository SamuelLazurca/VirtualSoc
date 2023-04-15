//
// Created by samuel on 20.12.2022.
//

#include "Client.h"
#include "DataBaseAction.h"

Client::Client(string username, int fd, string password, int id, string page_type) {

    this->username = username;
    this->clientDescriptor = fd;
    this->password = password;
    this->user_id = id;
    this->page_type = page_type;
    this->grup.In = false;
    this->grup.groupname = "";
    this->conv.In = false;
    this->conv.receptor = "";
    this->conv.idConv = 0;
}

string Client::getUsername() {
    return username;
}

int Client::get_user_id() {
    return user_id;
}

void Client::setGroup(string groupname) {
    grup.In = true;
    grup.groupname = groupname;
}

void Client::unsetGroup(string groupname) {
    grup.In = false;
    grup.groupname = "";
}

bool Client::inGroup() {
    return grup.In;
}

string Client::groupName() {
    return grup.groupname;
}

int Client::get_fd() {
    return clientDescriptor;
}

void Client::setConv(string receptor, int convId) {
    conv.In = true;
    conv.receptor = receptor;
    conv.idConv = convId;
}

void Client::unsetConv() {
    conv.In = false;
    conv.receptor = "";
    conv.idConv = 0;
}

bool Client::inConv(int convId) {
    if(conv.In && conv.idConv == convId) return true;
    return false;
}

int Client::getConvId() {
    return conv.idConv;
}

string Client::getReceptor() {
    return conv.receptor;
}

bool Client::isInConv() {
    return conv.In;
}

int Client::get_page_type() {
    return atoi(page_type.c_str());
}

