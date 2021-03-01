#include "ast.h"

statement::~statement()
{
}

iteration_statement::~iteration_statement()
{
}

block_item::~block_item()
{
}

translation_unit::~translation_unit()
{
    delete sc;
    for (external_declaration* d : ed)
        delete d;
}

external_declaration::~external_declaration()
{
    delete fd;
    delete decl;
}

function_definition::~function_definition()
{
    delete ds;
    delete dec;
    delete cs;
    delete sc;
}

compound_statement::~compound_statement()
{
    delete sc;
    for (block_item* i : bi)
        delete i;
}

statement_item::~statement_item()
{
    delete stat;
}

declaration_item::~declaration_item()
{
    delete decl;
}

return_statement::~return_statement()
{
    delete expr;
}

for_statement::~for_statement()
{
    delete expr1;
    delete expr2;
    delete expr3;
    delete stat;
}

do_while_statement::~do_while_statement()
{
    delete stat;
    delete expr;
}

while_statement::~while_statement()
{
    delete stat;
    delete expr;
}

if_statement::~if_statement()
{
    delete estat;
}

selection_statement::~selection_statement()
{
}

expression_statement::~expression_statement()
{
    delete expr;
}

case_label::~case_label()
{
    delete ce;
}

labeled_statement::~labeled_statement()
{
    delete stat;
}

expression::~expression()
{
    for (assignment_expression* e : ae)
        delete e;
}

constant_expression::~constant_expression()
{
    delete ce;
}

assignment_expression::~assignment_expression()
{
    delete lhs;
    delete rhs;
}

conditional_expression::~conditional_expression()
{
    delete oe;
    delete expr1;
    delete expr2;
    delete expr3;
}

logical_or_expression::~logical_or_expression()
{
    delete ae;
    delete lhs;
    delete rhs;
}

logical_and_expression::~logical_and_expression()
{
    delete oe;
    delete lhs;
    delete rhs;
}

inclusive_or_expression::~inclusive_or_expression()
{
    delete xe;
    delete lhs;
    delete rhs;
}

exclusive_or_expression::~exclusive_or_expression()
{
    delete ae;
    delete lhs;
    delete rhs;
}

and_expression::~and_expression()
{
    delete ee;
    delete lhs;
    delete rhs;
}

not_equal_expression::~not_equal_expression()
{
    delete lhs;
    delete rhs;
}

equal_expression::~equal_expression()
{
    delete lhs;
    delete rhs;
}

equality_expression::~equality_expression()
{
    delete re;
}

greater_equal_expression::~greater_equal_expression()
{
    delete lhs;
    delete rhs;
}

less_equal_expression::~less_equal_expression()
{
    delete lhs;
    delete rhs;
}

greater_expression::~greater_expression()
{
    delete lhs;
    delete rhs;
}

less_expression::~less_expression()
{
    delete lhs;
    delete rhs;
}

relational_expression::~relational_expression()
{
    delete se;
}

lshift_expression::~lshift_expression()
{
    delete lhs;
    delete rhs;
}

rshift_expression::~rshift_expression()
{
    delete lhs;
    delete rhs;
}

shift_expression::~shift_expression()
{
    delete ae;
}

sub_expression::~sub_expression()
{
    delete lhs;
    delete rhs;
}

add_expression::~add_expression()
{
    delete lhs;
    delete rhs;
}

additive_expression::~additive_expression()
{
    delete me;
}

mod_expression::~mod_expression()
{
    delete lhs;
    delete rhs;
}

div_expression::~div_expression()
{
    delete lhs;
    delete rhs;
}

mul_expression::~mul_expression()
{
    delete lhs;
    delete rhs;
}

multiplicative_expression::~multiplicative_expression()
{
    delete ce;
}

cast_expression::~cast_expression()
{
    delete ue;
    delete tn;
    delete ce;
}

sizeof_type_expression::~sizeof_type_expression()
{
    delete tn;
}

sizeof_expression::~sizeof_expression()
{
    delete ue;
}

unary_not_expression::~unary_not_expression()
{
    delete ce;
}

unary_tilde_expression::~unary_tilde_expression()
{
    delete ce;
}

unary_minus_expression::~unary_minus_expression()
{
    delete ce;
}

unary_plus_expression::~unary_plus_expression()
{
    delete ce;
}

unary_star_expression::~unary_star_expression()
{
    delete ce;
}

unary_and_expression::~unary_and_expression()
{
    delete ce;
}

prefix_decrement_expression::~prefix_decrement_expression()
{
    delete ue;
}

prefix_increment_expression::~prefix_increment_expression()
{
    delete ue;
}

unary_expression::~unary_expression()
{
    delete pe;
}

call_expression::~call_expression()
{
    for (assignment_expression* ae : args)
        delete ae;
}

subscript_expression::~subscript_expression()
{
    delete expr;
}

postfix_expression::~postfix_expression()
{
    delete pfe;
    delete pe;
}

primary_expression::~primary_expression()
{
}

parenthesized_expression::~parenthesized_expression()
{
    delete expr;
}

declaration::~declaration()
{
    delete ds;
    for (declarator* d : d)
        delete d;
}

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

token function_definition::get_identifier()
{
    return dec->get_identifier();
}

variable_object::variable_object(Type *type) : type(type)
{
}
