//
// Created by samuel on 20.12.2022.
//

#ifndef VIRTUALSOC1_SOCKETACTION_H
#define VIRTUALSOC1_SOCKETACTION_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sqlite3.h>

extern int errno;

class SocketAction {

public:
    int PORT;
    struct sockaddr_in server;
    struct sockaddr_in from;
    socklen_t len;
    struct timeval tv;
    int optval;
    int nfds;
    fd_set readFds;
    fd_set actFds;
    fd_set writeFds;
    int socketDescriptor;

    SocketAction();

    int Bind();
    int Listen();
    int AcceptNewClients();
    int removeActFds(int DisconnectedClient);

};


#endif //VIRTUALSOC1_SOCKETACTION_H
