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
    PUNCTUATOR,

    END_OF_FILE
};

struct token
{
    token(token_type type = INVALID, int col = 0, int row = 0) : type(type), col(col), row(row)
    {
    }

    token(token_type type, char* p, int sz, int col, int row)
        : type(type), str(p, sz), col(col), row(row)
    {
    }

    token_type type;
    string str;
    int col, row;
};

vector<token> tokenize_file(const char* name);
ostream& operator<<(ostream& out, const token_type tokn);
const string& stringify(token_type type);
