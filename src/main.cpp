#include <iostream>
#include "parser.h"
using namespace std;

int main(int args, char **argv)
{
    // ??? Don't know why I put this here :(
    if (args != 3)
    {
        cerr << "program takes file name";
        return EXIT_FAILURE;
    }

    vector<token> tokens = tokenize_file(argv[2]);
    bool failure = false;
    for (auto tok : tokens)
        if (tok.type == INVALID)
            failure = true;

    if (string(argv[1]) == "--tokenize")
    {
        for (auto tok : tokens)
            (tok.type ? cout : cerr) << argv[2] << ':' << tok.row
                                     << ':' << tok.col << ": " << tok.type
                                     << ' ' << tok.str << '\n';

        return failure ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    if (failure)
        return EXIT_FAILURE;

    try
    {
        translation_unit* tu = parser(tokens).parse();
        if (string(argv[1]) == "--print-ast")
        {
            // ...
        }
        delete tu;
    }
    catch (exception e)
    {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
