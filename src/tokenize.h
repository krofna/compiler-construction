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

string fix_digraph(const string& str);

struct token
{
    token() : type(INVALID), col(0), row(0)
    {
    }

    token(token_type type, char* p, int sz, int col, int row)
        : type(type), str(p, sz), col(col), row(row)
    {
        if (type == PUNCTUATOR)
            str = fix_digraph(str);
    }

    token_type type;
    string str;
    int col, row;
};

vector<token> tokenize_file(const char* name);
ostream& operator<<(ostream& out, const token_type tokn);
const string& stringify(token_type type);
