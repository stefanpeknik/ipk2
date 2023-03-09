#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
using namespace std;

#define BUFSIZE 1024

string Solve(string recieved);
int main(int argc, const char *argv[])
{
    const char *server_hostname;
    int port_number;
    string mode;

    /* 1. test vstupnich parametru: */

    for (int i = 1; i < argc; i++)
    {
        string arg = argv[i];

        if (arg == "-h" && i + 1 < argc)
        {
            server_hostname = argv[++i];
        }
        else if (arg == "-p" && i + 1 < argc)
        {
            string port_str = argv[++i];
            char *p;
            port_number = strtol(port_str.c_str(), &p, 10);
            if (*p)
            {
                cerr << "Invalid port number: " << port_str << endl;
                return EXIT_FAILURE;
            }
        }
        else if (arg == "-m" && i + 1 < argc)
        {
            mode = argv[++i];
            if (mode != "tcp" && mode != "udp")
            {
                cerr << "Invalid mode: " << mode << endl;
                return EXIT_FAILURE;
            }
        }
        else
        {
            cerr << "Unknown argument: " << arg << endl;
            return EXIT_FAILURE;
        }
    }

    if (strcmp(server_hostname, "") == 0 || port_number == 0 || mode.empty())
    {
        fprintf(stderr, "Usage: %s -h <host> -p <port> -m <mode>\n", argv[0]);
        return 1;
    }

    if (mode == "tcp")
    {
    }
    else // mode == "udp"
    {
    }
}
