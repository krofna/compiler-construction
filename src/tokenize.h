#pragma once
#include <vector>
#include <string>
using namespace std;

enum token_type : int
{
    INVALID = 0,

    KEYWORD,
    IDENTIFIER,
    CONSTANT,
    STRING_LITERAL,
    PUNCTUATOR
};

struct token
{
    token(token_type type, char* p, int sz, int col, int row)
        : type(type), str(p, sz), col(col), row(row)
    {
    }

    token_type type;
    string str;
    int col, row;
};

vector<token> tokenize_file(const char* name);
