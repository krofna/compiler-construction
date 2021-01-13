#pragma once
#include <stdexcept>
#include "tokenize.h"

struct error : std::runtime_error
{
    error(const token& tok) :
        std::runtime_error(to_string(tok.row)
                           + ":"
                           + to_string(tok.col)
                           + ": "
                           + to_string(tok.type)
                           + ' '
                           + tok.str)
    {
    }

    static void reject(token tok)
    {
        throw error(tok);
    }
};
