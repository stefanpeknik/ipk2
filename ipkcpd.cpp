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
        int rc;
        int welcome_socket;
        struct sockaddr_in sa;
        struct sockaddr_in sa_client;
        char str[INET6_ADDRSTRLEN];

        socklen_t sa_client_len = sizeof(sa_client);
        if ((welcome_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("ERROR: socket");
            exit(EXIT_FAILURE);
        }

        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_port = htons((unsigned short)port_number);
        if (inet_pton(AF_INET, server_hostname, &sa.sin_addr) <= 0)
        {
            fprintf(stderr, "Invalid address: %s\n", server_hostname);
            exit(1);
        }

        if ((rc = bind(welcome_socket, (struct sockaddr *)&sa, sizeof(sa))) < 0)
        {
            perror("ERROR: bind");
            exit(EXIT_FAILURE);
        }
        if ((listen(welcome_socket, 1)) < 0)
        {
            perror("ERROR: listen");
            exit(EXIT_FAILURE);
        }

        while (1)
        {
            int comm_socket = accept(welcome_socket, (struct sockaddr *)&sa_client, &sa_client_len);
            if (comm_socket > 0)
            {
                if (inet_ntop(AF_INET6, &sa_client.sin_addr, str, sizeof(str)))
                {
                    printf("INFO: New connection:\n");
                    printf("INFO: Client address is %s\n", str);
                    printf("INFO: Client port is %d\n", ntohs(sa_client.sin_port));
                }

                char buf[BUFSIZE];
                int res = 0;
                for (;;)
                {
                    bzero(buf, BUFSIZE);
                    res = recv(comm_socket, buf, BUFSIZE, 0);
                    if (res <= 0)
                        break;

                    printf("recieved: %s", buf);

                    string output;
                    try
                    {
                        output = Solve(string(buf, strlen(buf))); // can throw runtime_error("Empty input string")
                    }
                    catch (const runtime_error &e)
                    {
                        output = e.what();
                    }
                    printf("out: %s\n", output.c_str());

                    bzero(buf, BUFSIZE);
                    strcpy(buf, "recieved");
                    send(comm_socket, buf, strlen(buf), 0);
                }
            }
            else
            {
                printf(".");
            }
            printf("Connection to %s closed\n", str);
            close(comm_socket);
        }
    }
    else // mode == "udp"
    {
    }
}
