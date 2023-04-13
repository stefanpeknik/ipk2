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

/*
Grammar:
1. S -> ( OPERAND SP E SPACE E )
2. E -> 
*/

enum TokenType
{
    OPERATOR,    // +, -, *, /
    OPERAND,     // 1*DIGIT / DIGIT = 0-9
    OPEN_PAREN,  // (
    CLOSE_PAREN, // )
    SPACE        // ' ' == \032
};

enum OpType
{
    ADD, // +
    SUB, // -
    MUL, // *
    DIV  // /
};

enum TerminalAction
{
    SHIFT,  // <
    REDUCE, // >
    EQ      // =
};

typedef struct Token
{
    TokenType type;
    union Value
    {
        OpType op;
        int operand;
    };
} Token;

typedef struct Terminal
{
    Token token;
    TerminalAction action;
} Terminal;

int Main()
{
    string input;
    cout << "Enter an expression: ";
    getline(cin, input);
    cout << "You entered: " << input << endl;
    return 0;
}

int EvaluateExpression(string expr)
{
    stack<Terminal> parserStack;
}