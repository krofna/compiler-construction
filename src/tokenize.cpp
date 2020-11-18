#include <fstream>
#include <algorithm>
#include <cstring>
#include "tokenize.h"
using namespace std;

const string token_names[] =
{
    "error:", "keyword", "identifier",
    "constant", "string-literal", "punctuator"
};

ostream& operator<<(ostream& out, const token_type tokn)
{
    return out << token_names[tokn];
}

vector<string> punctuators =
{
    "[", "]", "(", ")", "{", "}", ".", "->", "++", "--", "&", "*", "+", "-",
    "~", "!", "/", "%", "<<", ">>", "<", ">", "<=", ">=", "==", "!=", "^",
    "|", "&&", "||", "?", ":", ";", "...", "=", "*=", "/=", "%=", "+=", "-=",
    "<<=", ">>=", "&=", "^=", "|=", ",", "#", "##"
};

vector<string> keywords =
{
    "auto", "break", "case", "char", "const", "continue", "default",
    "double", "do",  "else", "enum", "extern", "float", "for", "goto", "if",
    "inline", "int", "long", "register", "restrict", "return", "short",
    "signed", "sizeof", "static", "struct", "switch", "typedef", "union",
    "unsigned", "void", "volatile", "while", "_Alignas", "_Alignof",
    "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary",
    "_Noreturn", "_Static_assert", "_Thread_local"
};

int match_str(const string& s, char* p)
{
    int match = 0;
    for (int j = 0; j < s.size() && *p && s[j] == *p; ++j, ++p)
        ++match;

    return match;
}

int read_keyword(char* p)
{
    for (const string& s : keywords)
        if (match_str(s, p) == s.size())
            return s.size();

    return 0;
}

int read_punctuator(char* p)
{
    sort(begin(punctuators), end(punctuators), [](const string& x, const string& y)
    {
        return x.size() > y.size();
    });

    for (const string& s : punctuators)
        if (match_str(s, p) == s.size())
            return s.size();

    return 0;
}

int read_string(char* p)
{
    if (*p != '"')
        return 0;

    int match = 1;
    while (*p)
    {
        ++p;
        ++match;
        if (*p == '"')
            return match;

        if (*p == '\\')
        {
            ++match;
            ++p;
        }
    }

    return 0;
}

int read_number(char* p)
{
    int match = 0;
    while (isdigit(*p))
    {
        ++match;
        ++p;
    }

    return match;
}

int read_char(char* p)
{
    int match = 0;
    if (*p != '\'') return 0;
    ++match;
    ++p;

    if (*p == '\\')
    {
        ++match;
        ++p;
        if (!*p) return 0;
    }

    ++p;
    ++match;

    if (*p == '\'')
        return ++match;

    return 0;
}

int read_identifier(char* p)
{
    int match = 0;
    if (!isalpha(*p) && *p != '_') return 0;
    ++match;
    ++p;
    while (*p && (isalpha(*p) || isdigit(*p) || *p == '_'))
    {
        ++p;
        ++match;
    }

    return match;
}

vector<token> tokenize(char* p)
{
    vector<token> tokens;
    int row = 1, col = 1;
    while (*p)
    {
        if (*p == '/' && *(p + 1) == '/')
        {
            p += 2;
            while (*p != '\n')
                p++;
            continue;
        }

        if (*p == '/' && *(p + 1) == '*')
        {
            p = strstr(p + 2, "*/") + 2;
            continue;
        }

        if (*p == '\n')
        {
            col = 1;
            ++row;
            ++p;
            continue;
        }

        if (isspace(*p))
        {
            ++col;
            ++p;
            continue;
        }

        if (int numt = read_number(p))
        {
            tokens.emplace_back(CONSTANT, p, numt, col, row);
            p += numt;
            col += numt;
            continue;
        }

        if (int punct = read_punctuator(p))
        {
            tokens.emplace_back(PUNCTUATOR, p, punct, col, row);
            p += punct;
            col += punct;
            continue;
        }

        if (int ident = read_identifier(p))
        {
            int keyt = read_keyword(p);
            if (keyt == ident) tokens.emplace_back(KEYWORD, p, keyt, col, row);
            else tokens.emplace_back(IDENTIFIER, p, ident, col, row);
            p += ident;
            col += ident;
            continue;
        }

        if (int chrt = read_char(p))
        {
            tokens.emplace_back(CONSTANT, p, chrt, col, row);
            p += chrt;
            col += chrt;
            continue;
        }

        if (int strt = read_string(p))
        {
            tokens.emplace_back(STRING_LITERAL, p, strt, col, row);
            p += strt;
            col += strt;
            continue;
        }

        if (tokens.size() && tokens.back().type == INVALID) tokens.back().str += *p;
        else tokens.emplace_back(INVALID, p, 1, col, row);
        ++p;
    }

    return tokens;
}

vector<token> tokenize_file(const char* name)
{
    ifstream f(name);
    if (!f)
        return {};

    f.seekg(0, ios::end);
    int sz = f.tellg();
    f.seekg(0, ios::beg);
    string data(sz, 0);
    f.read(&data[0], sz);
    return tokenize(&data[0]);
}
