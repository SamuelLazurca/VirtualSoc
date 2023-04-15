//
// Created by samuel on 20.12.2022.
//

#include "SocketAction.h"

char *conv_addr(struct sockaddr_in address)
{
    static char str[25];
    char port[7];

    /* adresa IP a clientului */
    strcpy(str, inet_ntoa(address.sin_addr));
    /* portul utilizat de client */
    bzero(port, 7);
    sprintf(port, ":%d", ntohs(address.sin_port));
    strcat(str, port);
    return (str);
}

SocketAction::SocketAction() {

    PORT = 2728;
    optval = 1;

    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    if ((socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server] Eroare la socket().\n");
        return;
    }

    setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    FD_ZERO(&actFds);
    FD_SET(socketDescriptor, &actFds);

    nfds = socketDescriptor;

    bcopy((char *)&actFds, (char *)&readFds, sizeof(readFds));

    FD_ZERO(&writeFds);

}

int SocketAction::Bind() {

    if (bind(socketDescriptor, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server] Eroare la bind().\n");
        return errno;
    }

    return 0;
}

int SocketAction::Listen() {

    if (listen(socketDescriptor, 5) == -1)
    {
        perror("[server] Eroare la listen().\n");
        return errno;
    }

    printf("[server] Asteptam la portul %d...\n", PORT);
    fflush(stdout);

    return 0;
}

int SocketAction::AcceptNewClients() {

    int client;

    bcopy((char *) &actFds, (char *) &readFds, sizeof(readFds));

    if (select(nfds + 1, &readFds, NULL, NULL, &tv) < 0) {
        perror("[server] Eroare la select().\n");
        return errno;
    }

    if (FD_ISSET(socketDescriptor, &readFds)) {
        len = sizeof(from);
        bzero(&from, sizeof(from));

        client = accept(socketDescriptor, (struct sockaddr *) &from, &len);

        if (client < 0) {
            perror("[server] Eroare la accept().\n");
            //continue;
        }

        if (nfds < client)
            nfds = client;

        FD_SET(client, &actFds);

        printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n", client, conv_addr(from));
        fflush(stdout);
    }

    return 0;
}

int SocketAction::removeActFds(int DisconnectedClient) {
    FD_CLR(DisconnectedClient, &actFds);
    return 0;
}
