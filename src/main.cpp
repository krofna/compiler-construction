#include <iostream>
#include "parser.h"
using namespace std;

int task_b(const char* filename)
{
    vector<token> tokens = tokenize_file(filename);
    bool failure = false;
    for (auto tok : tokens)
        if (tok.type == INVALID)
            failure = true;

    for (auto tok : tokens)
        (tok.type ? cout : cerr) << filename << ':' << tok.row
                                 << ':' << tok.col << ": " << tok.type
                                 << ' ' << tok.str << '\n';

    return failure ? EXIT_FAILURE : EXIT_SUCCESS;
}

int task_cde(const char* filename, bool print)
{
    // todo: print tokenizer error
    vector<token> tokens = tokenize_file(filename);
    for (auto tok : tokens)
        if (tok.type == INVALID)
            return EXIT_FAILURE;

    try
    {
        translation_unit* tu = parser(tokens).parse();
        if (print)
            tu->print();
        delete tu;
    }
    catch (const error& e)
    {
        cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(int args, char **argv)
{
    if (args != 3)
    {
        cerr << "program takes file name";
        return EXIT_FAILURE;
    }

    string opt = argv[1];
    if (opt == "--tokenize")
        return task_b(argv[2]);
    if (opt == "--parse")
        return task_cde(argv[2], false);
    if (opt == "--print-ast")
        return task_cde(argv[2], true);

    return EXIT_FAILURE;
}
