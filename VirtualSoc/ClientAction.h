//
// Created by samuel on 29.12.2022.
//

#ifndef VIRTUALSOC1_CLIENTACTION_H
#define VIRTUALSOC1_CLIENTACTION_H

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>

using namespace std;

class ClientAction {
    in_addr_t addr;
    int port;
    int sd;
public:
    ClientAction(char adr[], char p[]);
    int Run();
    friend void* SendInputFromConsoleToServer(void* ptr);
    int ReadFromServer();
    int get_sd();
};


#endif //VIRTUALSOC1_CLIENTACTION_H
