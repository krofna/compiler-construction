#include "ast.h"

token direct_declarator::get_identifier()
{
    return tok;
}

token parenthesized_declarator::get_identifier()
{
    return decl->get_identifier();
}

token function_declarator::get_identifier()
{
    return dd->get_identifier();
}

token declarator::get_identifier()
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

bool direct_declarator::is_pointer()
{
    return false;
}

bool parenthesized_declarator::is_pointer()
{
    return decl->is_pointer();
}

bool function_declarator::is_pointer()
{
    return dd->is_pointer();
}

bool declarator::is_pointer()
{
    if (p)
        return true;
    return dd->is_pointer();
}

declarator* declarator::unparenthesize()
{
    if (parenthesized_declarator* pd = dynamic_cast<parenthesized_declarator*>(dd))
        return pd->decl->unparenthesize();
    return this;
}

bool builtin_type_specifier::is_void()
{
    return tok.str == "void";
}

bool struct_or_union_specifier::is_void()
{
    return false;
}
