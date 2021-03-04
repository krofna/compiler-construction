#include <string>
using namespace std;

extern string escapable_chars;
extern string escaped_chars;

string unescape(const string& s)
{
    string z;
    for (int i = 1; i < s.size() - 1; ++i)
    {
        size_t idx = escapable_chars.find(s[i + 1]);
        if (s[i] == '\\' && idx != escapable_chars.npos)
            z.push_back(escaped_chars[idx]), ++i;
        else
            z.push_back(s[i]);
    }
    return z;
}
