#include <iostream>
#include "parser.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/PrettyStackTrace.h"
using namespace std;

int task_b(const char* filename)
{
    vector<token> tokens = tokenize_file(filename);
    bool failure = false;
    for (auto tok : tokens)
        if (tok.type == INVALID)
            failure = true;

    for (auto tok : tokens)
        if (tok.type != END_OF_FILE)
            (tok.type ? cout : cerr) << filename << ':' << tok.row
                                     << ':' << tok.col << ": " << tok.type
                                     << ' ' << tok.str << '\n';

    return failure ? EXIT_FAILURE : EXIT_SUCCESS;
}

int task_cdef(const char* filename, bool print, bool compile)
{
    vector<token> tokens = tokenize_file(filename);
    for (auto tok : tokens)
    {
        if (tok.type == INVALID)
        {
            cerr << filename << ':' << tok.row
                 << ':' << tok.col << ": " << tok.type
                 << ' ' << tok.str << '\n';
            return EXIT_FAILURE;
        }
    }

    try
    {
        translation_unit* tu = parser(tokens).parse();
        if (print)
            tu->print();
        if (compile)
            tu->codegen(filename);
        delete tu;
    }
    catch (const error& e)
    {
        cerr << filename << ":" << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        cerr << "program takes file name";
        return EXIT_FAILURE;
    }

    sys::PrintStackTraceOnErrorSignal(argv[0]);
    PrettyStackTraceProgram X(argc, argv);

    string opt = argv[1];
    if (opt == "--tokenize")
        return task_b(argv[2]);
    if (opt == "--parse")
        return task_cdef(argv[2], false, false);
    if (opt == "--print-ast")
        return task_cdef(argv[2], true, false);
    if (opt == "--compile")
        return task_cdef(argv[2], false, true);
    return EXIT_FAILURE;
}
