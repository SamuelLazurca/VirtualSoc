//
// Created by samuel on 29.12.2022.
//

#include "ClientAction.h"

extern int errno;

#define ESC "\033["
#define LIGHT_BLUE_BKG "106"
#define PURPLE_TXT "35"
#define RESET "\033[m"

void help() {
    ifstream fin("help.txt");
    char buff[4096];

    cout << endl;
    while(fin.getline(buff, 4096))
    {
        cout<<buff<<endl;
    }
    cout<<endl;

    fin.close();
}

void* SendInputFromConsoleToServer(void* ptr)
{
    char msg[100];		// mesajul trimis
    int len_msg = 0;

    int sd = ((ClientAction*) ptr)->sd;

    cout << ESC << PURPLE_TXT <<"m"<< "Welcome to VirtualSoc!" << endl << endl << RESET;

    cout << "-- Type \'login <USERNAME> <PASSWORD>\' if you have already an account created and want to have full access" << endl;
    cout << "-- Don't have an account? You can create one by typing \'create account <NEW USERNAME> <NEW PASSWORD>\'" << endl;
    cout << "-- To see others public posts type \'show posts from <USERNAME>\'" << endl;
    cout << ESC << PURPLE_TXT <<"m"<< endl << "Type here..." << endl << endl << RESET;

    while(1)
    {
        bzero (msg, 100);
        fflush (stdout);
        cin.getline(msg, 1024);

        if(strcmp(msg, "help") == 0) help();
        else {
            len_msg = strlen(msg) + 1;

            if (write(sd, &len_msg, sizeof(int)) <= 0) {
                perror("[client]Eroare la write() spre server.\n");
                return (void *) errno;
            }

            if (write(sd, msg, len_msg) <= 0) {
                perror("[client]Eroare la write() spre server.\n");
                return (void *) errno;
            }
        }
    }

    return 0;
}

int ClientAction::ReadFromServer()
{
    char rasp[100];
    int len_rasp = 0;

    while(1)
    {
        bzero(rasp, 100);

        if (read (sd, &len_rasp, sizeof(int)) <= 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
        }

        if (read (sd, rasp, len_rasp) <= 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
        }

        if(strcmp(rasp, "quit") == 0) {cout << "App closed\n"; break;}

        printf ("%s\n", rasp);
    }

    return 0;
}

int ClientAction::Run()
{
    struct sockaddr_in server;
    pthread_t thread;

    if (sd == -1)
    {
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = addr;
    server.sin_port = htons (port);

    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
        perror ("[client]Eroare la connect().\n");
        return errno;
    }

    int t1 = pthread_create( &thread, NULL, SendInputFromConsoleToServer, this);

    this->ReadFromServer();

    close (sd);

    return 0;
}

ClientAction::ClientAction(char *adr, char *p)
{
    addr = inet_addr(adr);
    port = atoi (p);

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("[client] Eroare la socket().\n");
        sd = -1;
    }
}

int ClientAction::get_sd()
{
    return sd;
}

