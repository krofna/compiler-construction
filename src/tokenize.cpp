#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
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

int match(const string& s, char* p)
{
    int match = 0;
    for (int j = 0; j < s.size() && *p && s[j] == *p; ++j, ++p)
        ++match;
    
    return match;
}

int read_keyword(char* p)
{
    for (const string& s : keywords)
        if (match(s, p) == s.size())
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
        if (match(s, p) == s.size())
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

struct token
{
    token(token_type type, char* p, int sz) : type(type), str(p, sz)
    {
    }

    token_type type;
    string str;
};

vector<token> tokenize(char* p)
{
    vector<token> tokens;
    int row = 1, col = 1;
    while (*p)
    {
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

        if (int punct = read_punctuator(p))
        {
            tokens.emplace_back(PUNCTUATOR, p, punct);
            p += punct;
            col += punct;
            continue;
        }

        if (int keyt = read_keyword(p))
        {
            tokens.emplace_back(KEYWORD, p, keyt);
            p += keyt;
            col += keyt;
            continue;
        }

        if (int strt = read_string(p))
        {
            tokens.emplace_back(STRING_LITERAL, p, strt);
            p += strt;
            col += strt;
            continue;
        }
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
