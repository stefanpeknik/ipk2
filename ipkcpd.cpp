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
        char buf[BUFSIZE];
        int server_socket, bytestx, bytesrx;
        socklen_t clientlen;
        struct sockaddr_in client_address, server_address;
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
            clientlen = sizeof(client_address);
            bytesrx = recvfrom(server_socket, buf, BUFSIZE, 0, (struct sockaddr *)&client_address, &clientlen);
            if (bytesrx < 0)
                perror("ERROR: recvfrom:");

            hostp = gethostbyaddr((const char *)&client_address.sin_addr.s_addr,
                                  sizeof(client_address.sin_addr.s_addr), AF_INET);

            hostaddrp = inet_ntoa(client_address.sin_addr);
            printf("Message (%lu) from %s: %s\n", strlen(buf + 2), hostaddrp, buf + 2);

            string output;
            try
            {
                output = Solve(string(buf + 2, strlen(buf + 2))); // can throw runtime_error("Empty input string")
                bzero(buf, BUFSIZE);
                strcpy(buf + 3, output.c_str());
                buf[0] = '\1';
                buf[1] = '\0';
                buf[2] = (char)output.length();
                strcpy(buf + 3, output.c_str());
            }
            catch (const runtime_error &e)
            {
                output = e.what();
                bzero(buf, BUFSIZE);
                strcpy(buf + 3, output.c_str());
                buf[0] = '\1';
                buf[1] = '\1';
                buf[2] = (char)output.length();
                strcpy(buf + 3, output.c_str());
            }
            printf("out: %s\n", buf + 3);

            /* odeslani zpravy zpet klientovi  */
            bytestx = sendto(server_socket, buf, output.length() + 3, 0, (struct sockaddr *)&client_address, clientlen);
            if (bytestx < 0)
                perror("ERROR: sendto:");
        }
    }
}

void PrintTokens(vector<Token> tokens)
{
    for (size_t i = 0; i < tokens.size(); i++)
    {
        switch (tokens[i].type)
        {
        case OPERATOR:
            switch (tokens[i].op)
            {
            case ADD:
                cout << "ADD" << endl;
                break;
            case SUB:
                cout << "SUB" << endl;
                break;
            case MUL:
                cout << "MUL" << endl;
                break;
            case DIV:
                cout << "DIV" << endl;
                break;
            }
            break;

        case OPEN_PAREN:
            cout << "OPEN_PAREN" << endl;
            break;

        case CLOSE_PAREN:
            cout << "CLOSE_PAREN" << endl;
            break;

        case OPERAND:
            cout << tokens[i].value << endl;
            break;
        }
    }
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

    // print tokens
    PrintTokens(tokens);

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
