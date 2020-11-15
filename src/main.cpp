#include <iostream>
#include "tokenize.cpp"
using namespace std;

int main(int args, char **cargs)
{

    if (args < 2)
    {
        cerr << "program takes file name";
        return EXIT_FAILURE;
    }
    
    vector<token> tokens = tokenize_file(cargs[1]);
    for(auto [i, j, col, row] : tokens)
        (i ? cout : cerr) << cargs[1] << ':' << row
                          << ':' << col << ": " << i
                          << ' ' << j << '\n';

    for(auto tok : tokens)
        if (tok.type == INVALID)
            return EXIT_FAILURE;
    
    return EXIT_SUCCESS;
}
