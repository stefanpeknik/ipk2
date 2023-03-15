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
#include <stack>
#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

#define BUFSIZE 1024

enum TokenType
{
    OPERATOR,
    OPEN_PAREN,
    CLOSE_PAREN,
    OPERAND
};

enum OpType
{
    ADD,
    SUB,
    MUL,
    DIV
};

typedef struct
{
    TokenType type;
    OpType op;
    int value;
} Token;

int evaluatePrefix(string expr);

// global variables for signal handler
string mode;
int comm_socket;
int server_socket;

void signalHandler(int signum)
{
    if (mode == "tcp")
    {
        /* odeslani ukonceni spojeni */
        send(comm_socket, "BYE\n", 4, 0);

        /* ukonceni spojeni */
        close(comm_socket);
    }
    else if (mode == "udp")
    {
        /* ukonceni spojeni */
    }

    exit(EXIT_SUCCESS);
}

int main(int argc, const char *argv[])
{
    const char *server_hostname;
    int port_number;

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

    /* Zachyceni ctrl-C */
    signal(SIGINT, signalHandler);

    if (strcmp(server_hostname, "") == 0 || port_number == 0 || mode.empty())
    {
        fprintf(stderr, "Usage: %s -h <host> -p <port> -m <mode>\n", argv[0]);
        return 1;
    }

    if (mode == "tcp")
    {
        char buf[BUFSIZE];
        int rc;
        int welcome_socket;
        socklen_t sa_client_len;
        struct sockaddr_in server_address, sa_client;
        char str[INET_ADDRSTRLEN];

        sa_client_len = sizeof(sa_client);
        if ((welcome_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("ERROR: socket");
            exit(EXIT_FAILURE);
        }

        bzero((char *)&server_address, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons((unsigned short)port_number);
        if (inet_pton(AF_INET, server_hostname, &server_address.sin_addr) <= 0)
        {
            fprintf(stderr, "Invalid address: %s\n", server_hostname);
            exit(1);
        }

        if ((rc = bind(welcome_socket, (struct sockaddr *)&server_address, sizeof(server_address))) < 0)
        {
            perror("ERROR: bind");
            exit(EXIT_FAILURE);
        }
        if ((listen(welcome_socket, 1)) < 0)
        {
            perror("ERROR: listen");
            exit(EXIT_FAILURE);
        }

        bool greeted = false;

        while (1)
        {
            comm_socket = accept(welcome_socket, (struct sockaddr *)&sa_client, &sa_client_len);
            if (comm_socket > 0)
            {
                if (inet_ntop(AF_INET, &sa_client.sin_addr, str, sizeof(str)))
                {
                    printf("INFO: New connection:\n");
                    printf("INFO: Client address is %s\n", str);
                    printf("INFO: Client port is %d\n", ntohs(sa_client.sin_port));
                }
                else
                {
                    perror("ERROR: inet_ntop");
                    exit(EXIT_FAILURE);
                }

                int res = 0;
                for (;;)
                {
                    bzero(buf, BUFSIZE);
                    res = recv(comm_socket, buf, BUFSIZE, 0);
                    if (res <= 0)
                        break;

                    printf("recieved: %s", buf);

                    if (!greeted)
                    {
                        if (strcmp(buf, "HELLO\n") != 0)
                        {
                            printf("ERROR: HELLO expected\n");
                            send(comm_socket, "BYE\n", 4, 0);
                            close(comm_socket);
                            break;
                        }
                        else
                        {
                            printf("INFO: HELLO recieved\n");
                            send(comm_socket, "HELLO\n", 6, 0);
                            greeted = true;
                            continue;
                        }
                    }

                    if (strcmp(buf, "BYE\n") == 0)
                    {
                        printf("INFO: BYE recieved\n");
                        send(comm_socket, "BYE\n", 4, 0);
                        break;
                    }

                    string exprsn = buf;

                    int result;
                    string output;
                    try
                    {
                        result = evaluatePrefix(exprsn);
                        output = "RESULT " + to_string(result) + "\n";
                    }
                    catch (const runtime_error &e)
                    {
                        output = string(e.what()) + "\n";
                    }
                    printf("out: %s", output.c_str());

                    bzero(buf, BUFSIZE);
                    strcpy(buf, output.c_str());
                    send(comm_socket, buf, strlen(buf), 0);
                }
            }
            else
            {
                perror("ERROR: accept");
                exit(EXIT_FAILURE);
            }
            printf("Connection to %s closed\n", str);
            close(comm_socket);
            greeted = false;
        }
    }
    else // mode == "udp"
    {
        char buf[BUFSIZE];
        int bytestx, bytesrx;
        socklen_t sa_client_len;
        struct sockaddr_in server_address, sa_client;
        int optval;
        const char *hostaddrp;
        struct hostent *hostp;

        /* Vytvoreni soketu */
        if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) <= 0)
        {
            perror("ERROR: socket");
            exit(EXIT_FAILURE);
        }
        /* potlaceni defaultniho chovani rezervace portu ukonceni aplikace */
        optval = 1;
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

        /* adresa serveru, potrebuje pro prirazeni pozadovaneho portu */
        bzero((char *)&server_address, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons((unsigned short)port_number);
        if (inet_pton(AF_INET, server_hostname, &server_address.sin_addr) <= 0)
        {
            fprintf(stderr, "Invalid address: %s\n", server_hostname);
            exit(1);
        }

        if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
        {
            perror("ERROR: binding");
            exit(EXIT_FAILURE);
        }

        while (1)
        {
            printf("INFO: Ready.\n");
            /* prijeti odpovedi a jeji vypsani */
            bzero(buf, BUFSIZE);
            sa_client_len = sizeof(sa_client);
            bytesrx = recvfrom(server_socket, buf, BUFSIZE, 0, (struct sockaddr *)&sa_client, &sa_client_len);
            if (bytesrx < 0)
            {
                perror("ERROR: recvfrom");
                exit(EXIT_FAILURE);
            }

            hostp = gethostbyaddr((const char *)&sa_client.sin_addr.s_addr,
                                  sizeof(sa_client.sin_addr.s_addr), AF_INET);

            hostaddrp = inet_ntoa(sa_client.sin_addr);
            printf("Message (%lu) from %s: %s\n", strlen(buf + 2), hostaddrp, buf + 2);

            string exprsn = string(buf + 2, (size_t)(buf + 1));
            int result;
            string output;
            try
            {
                result = evaluatePrefix(exprsn);
                output = to_string(result);
            }
            catch (const runtime_error &e)
            {
                output = e.what();
            }
            strcpy(buf + 3, output.c_str());

            printf("out: %s\n", buf + 3);

            /* odeslani zpravy zpet klientovi  */
            bytestx = sendto(server_socket, buf, output.length() + 3, 0, (struct sockaddr *)&sa_client, sa_client_len);
            if (bytestx < 0)
                perror("ERROR: sendto:");
        }
    }
    return 0;
}

int evaluatePrefix(string expr)
{
    std::stack<int> s;

    // Tokenize the expr
    std::vector<Token> tokens;
    for (size_t i = 0; i < expr.length(); i++)
    {
        // parenthesis
        if (expr[i] == '(')
            tokens.push_back((Token){.type = OPEN_PAREN});
        else if (expr[i] == ')')
            tokens.push_back((Token){.type = CLOSE_PAREN});
        // operators
        else if (expr[i] == '+')
        {
            tokens.push_back((Token){.type = OPERATOR, .op = ADD});
        }
        else if (expr[i] == '-')
        {
            tokens.push_back((Token){.type = OPERATOR, .op = SUB});
        }
        else if (expr[i] == '*')
        {
            tokens.push_back((Token){.type = OPERATOR, .op = MUL});
        }
        else if (expr[i] == '/')
        {
            tokens.push_back((Token){.type = OPERATOR, .op = DIV});
        }
        // operands
        else if (isdigit(expr[i]))
        {
            string operand = "";
            int j = i;
            while (isdigit(expr[j]))
            {
                i = j;
                operand.append(1, expr[j++]);
            }
            tokens.push_back((Token){.type = OPERAND, .value = stoi(operand)});
        }
        else if (isspace(expr[i]))
        {
        }
        else
        {
            stringstream ss;
            ss << "Invalid character: " << expr[i] << " at index " << i;
            throw runtime_error(ss.str());
        }
    }

    // evaluate the expression
    stack<Token> st;
    for (size_t i = 0; i < tokens.size(); i++)
    {
        if (tokens[i].type == OPEN_PAREN)
        {
            st.push(tokens[i]);
        }
        else if (tokens[i].type == OPERAND)
        {
            st.push(tokens[i]);
        }
        else if (tokens[i].type == OPERATOR)
        {
            st.push(tokens[i]);
        }
        else if (tokens[i].type == CLOSE_PAREN)
        {
            vector<Token> operands;
            while (st.top().type != OPERATOR) // load operands
            {
                if (st.top().type != OPERAND) // cannot accept anything other than operands
                {
                    throw runtime_error("Invalid expression");
                }
                operands.push_back(st.top());
                st.pop();
            }

            if (operands.size() < 2) // cannot accept less than 2 operands
            {
                throw runtime_error("Expression in parenthesis must have at least 2 operands");
            }

            Token op = st.top(); // load operator
            st.pop();

            if (st.empty() || st.top().type != OPEN_PAREN) // check if there is a matching open parenthesis after the operator
            {
                throw runtime_error("Missing open parenthesis");
            }
            st.pop(); // pop open parenthesis

            // evaluate the expression
            reverse(operands.begin(), operands.end()); // reverse the operands so that the first operand is at the top of the stack
            int result;
            switch (op.op)
            {
            case ADD:
                result = operands[0].value + operands[1].value;
                for (size_t i = 2; i < operands.size(); i++)
                {
                    result += operands[i].value;
                }
                st.push((Token){.type = OPERAND, .value = result});
                break;

            case SUB:
                result = operands[0].value - operands[1].value;
                for (size_t i = 2; i < operands.size(); i++)
                {
                    result -= operands[i].value;
                }
                st.push((Token){.type = OPERAND, .value = result});
                break;

            case MUL:
                result = operands[0].value * operands[1].value;
                for (size_t i = 2; i < operands.size(); i++)
                {
                    result *= operands[i].value;
                }
                st.push((Token){.type = OPERAND, .value = result});
                break;

            case DIV:
                result = operands[0].value / operands[1].value;
                for (size_t i = 2; i < operands.size(); i++)
                {
                    result /= operands[i].value;
                }
                st.push((Token){.type = OPERAND, .value = result});
                break;
            }
        }
        else
        {
            throw runtime_error("Unexpected error");
        }
    }

    if (st.size() > 1 || st.top().type != OPERAND) // check if the stack has only one operand left
    {
        throw runtime_error("Invalid expression");
    }

    return st.top().value; // return expression result;
}
