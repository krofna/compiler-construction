#include <iostream>
#include "parser.h"
using namespace std;

int main(int args, char **cargs)
{
    // ??? Don't know why I put this here :(
    if (args < 2)
    {
        cerr << "program takes file name";
        return EXIT_FAILURE;
    }

    if (string(cargs[1]) == "--tokenize")
    {
        vector<token> tokens = tokenize_file(cargs[2]);
        for(auto tok : tokens)
            (tok.type ? cout : cerr) << cargs[2] << ':' << tok.row
                                     << ':' << tok.col << ": " << tok.type
                                     << ' ' << tok.str << '\n';

        for(auto tok : tokens)
            if (tok.type == INVALID)
                return EXIT_FAILURE;
        
        
        if (string(cargs[1]) == "--parse")
            {
                try
                    {
                        translation_unit* tu = parser(tokens).parse();
                    }
                catch (exception e)
                    {
                        cerr << e.what() << endl;
                    }
            }
        
    }
    
    return EXIT_SUCCESS;
}
