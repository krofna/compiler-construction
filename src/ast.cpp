#include "ast.h"

string direct_declarator::get_identifier()
{
    return tok.str;
}

string parenthesized_declarator::get_identifier()
{
    return decl->get_identifier();
}

string function_declarator::get_identifier()
{
    return dd->get_identifier();
}

string declarator::get_identifier()
{
    return dd->get_identifier();
}

bool direct_declarator::is_identifier()
{
    return true;
}

bool parenthesized_declarator::is_identifier()
{
    return decl->dd->is_identifier();
}

bool function_declarator::is_identifier()
{
    return false;
}

bool direct_declarator::is_definition()
{
    return false;
}

bool parenthesized_declarator::is_definition()
{
    if (decl->p && decl->dd->is_identifier())
        return true;
    return decl->dd->is_definition();
}

bool function_declarator::is_definition()
{
    return dd->is_definition();
}
