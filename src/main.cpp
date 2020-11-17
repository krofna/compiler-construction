#include <iostream>
#include "tokenize.h"
using namespace std;

int main(int args, char **cargs)
{

    // ??? Don't know why I put this here :(
    if (args < 2)
    {
        cerr << "program takes file name";
        return EXIT_FAILURE;
    }
    
    vector<token> tokens = tokenize_file(cargs[2]);
    for(auto tok : tokens)
        (tok.type ? cout : cerr) << cargs[2] << ':' << tok.row
                          << ':' << tok.col << ": " << tok.type
                          << ' ' << tok.str << '\n';

    for(auto tok : tokens)
        if (tok.type == INVALID)
            return EXIT_FAILURE;
    
    return EXIT_SUCCESS;
}
