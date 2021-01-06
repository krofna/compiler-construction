#pragma once
#include "tokenize.h"

using token_iter = vector<token>::iterator;

struct node
{
};

struct specifier_qualifier
{
};

struct type_qualifier : specifier_qualifier
{
    token tok;
};

struct type_specifier : specifier_qualifier
{
};

struct builtin_type_specifier : type_specifier
{
    token tok;
};

struct declarator;

struct struct_declaration
{
    type_specifier* ts;
    vector<declarator*> ds;
};

struct struct_or_union_specifier : type_specifier
{
    token sou;
    token id;
    vector<struct_declaration*> sds;
};

struct abstract_declarator;
struct parameter_declaration;

struct direct_abstract_declarator
{
    abstract_declarator* ad = nullptr;
    direct_abstract_declarator* dad = nullptr;
    vector<parameter_declaration*> pl;
};

struct pointer;

struct abstract_declarator
{
    pointer* p = nullptr;
    direct_abstract_declarator* dad = nullptr;
};

struct type_name
{
    vector<specifier_qualifier*> sqs;
    abstract_declarator* ad = nullptr;
};

struct declaration_specifiers
{
    type_specifier* ts;
};

struct storage_class_specifier
{
    token tok;
};

struct function_specifier
{
    token tok;
};

struct declarator;

struct direct_declarator
{
    token tok;
};

struct parenthesized_declarator : direct_declarator
{
    declarator* decl;
};

struct parameter_declaration
{
    declaration_specifiers* ds;
    declarator* decl = nullptr;
    abstract_declarator* ad = nullptr;
};

struct function_declarator : direct_declarator
{
    direct_declarator* dd;
    vector<parameter_declaration*> pl;
};

struct pointer
{
    vector<type_qualifier*> tql;
    pointer* p = nullptr;
};

struct declarator
{
    pointer* p = nullptr;
    direct_declarator* dd;
};

struct declaration
{
    ~declaration()
    {
        delete ds;
        delete d;
    }

    declaration_specifiers* ds;
    declarator* d = nullptr;
};

struct expression;

struct primary_expression
{
    token tok;
    expression* expr = nullptr;
};

struct postfix_expression
{
    postfix_expression* pfe = nullptr;
    primary_expression* pe = nullptr;
};

struct subscript_expression : postfix_expression
{
    expression* expr;
};

struct assignment_expression;

struct call_expression : postfix_expression
{
    vector<assignment_expression*> args;
};

struct dot_expression : postfix_expression
{
    token id;
};

struct arrow_expression : postfix_expression
{
    token id;
};

struct postfix_increment_expression : postfix_expression
{
};

struct postfix_decrement_expression : postfix_expression
{
};

struct unary_expression
{
    postfix_expression* pe = nullptr;
};

struct prefix_increment_expression : unary_expression
{
    unary_expression* ue;
};

struct prefix_decrement_expression : unary_expression
{
    unary_expression* ue;
};

struct cast_expression;

struct unary_and_expression : unary_expression
{
    cast_expression* ce;
};

struct unary_star_expression : unary_expression
{
    cast_expression* ce;
};

struct unary_plus_expression : unary_expression
{
    cast_expression* ce;
};

struct unary_minus_expression : unary_expression
{
    cast_expression* ce;
};

struct unary_tilde_expression : unary_expression
{
    cast_expression* ce;
};

struct unary_not_expression : unary_expression
{
    cast_expression* ce;
};

struct sizeof_expression : unary_expression
{
    unary_expression* ue;
};

struct sizeof_type_expression : unary_expression
{
    type_name* tn;
};

struct cast_expression
{
    unary_expression* ue = nullptr;
    type_name* tn = nullptr;
    cast_expression* ce = nullptr;
};

struct multiplicative_expression
{
    cast_expression* ce = nullptr;
};

struct mul_expression : multiplicative_expression
{
    multiplicative_expression* lhs;
    cast_expression* rhs;
};

struct div_expression : multiplicative_expression
{
    multiplicative_expression* lhs;
    cast_expression* rhs;
};

struct mod_expression : multiplicative_expression
{
    multiplicative_expression* lhs;
    cast_expression* rhs;
};

struct additive_expression
{
    multiplicative_expression* me = nullptr;
};

struct add_expression : additive_expression
{
    additive_expression* lhs;
    multiplicative_expression* rhs;
};

struct sub_expression : additive_expression
{
    additive_expression* lhs;
    multiplicative_expression* rhs;
};

struct shift_expression
{
    additive_expression* ae = nullptr;
};

struct rshift_expression : shift_expression
{
    shift_expression* lhs;
    additive_expression* rhs;
};

struct lshift_expression : shift_expression
{
    shift_expression* lhs;
    additive_expression* rhs;
};

struct relational_expression
{
    shift_expression* se = nullptr;
};

struct less_expression : relational_expression
{
    relational_expression* lhs;
    shift_expression* rhs;
};

struct greater_expression : relational_expression
{
    relational_expression* lhs;
    shift_expression* rhs;
};

struct less_equal_expression : relational_expression
{
    relational_expression* lhs;
    shift_expression* rhs;
};

struct greater_equal_expression : relational_expression
{
    relational_expression* lhs;
    shift_expression* rhs;
};

struct equality_expression
{
    relational_expression* re = nullptr;
};

struct equal_expression : equality_expression
{
    equality_expression* lhs;
    relational_expression* rhs;
};

struct not_equal_expression : equality_expression
{
    equality_expression* lhs;
    relational_expression* rhs;
};

struct and_expression
{
    equality_expression* ee = nullptr;
    and_expression* lhs = nullptr;
    equality_expression* rhs = nullptr;
};

struct exclusive_or_expression
{
    and_expression* ae = nullptr;
    exclusive_or_expression* lhs = nullptr;
    and_expression* rhs = nullptr;
};

struct inclusive_or_expression
{
    exclusive_or_expression* xe = nullptr;
    inclusive_or_expression* lhs = nullptr;
    exclusive_or_expression* rhs = nullptr;
};

struct logical_and_expression
{
    inclusive_or_expression* oe = nullptr;
    logical_and_expression* lhs = nullptr;
    inclusive_or_expression* rhs = nullptr;
};

struct logical_or_expression
{
    logical_and_expression* ae = nullptr;
    logical_or_expression* lhs = nullptr;
    logical_and_expression* rhs = nullptr;
};

struct conditional_expression
{
    logical_or_expression* oe = nullptr;
    logical_or_expression* expr1 = nullptr;
    expression* expr2 = nullptr;
    conditional_expression* expr3 = nullptr;
};

struct assignment_expression
{
    conditional_expression* lhs = nullptr;
    assignment_expression* rhs = nullptr;
};

struct constant_expression
{
    conditional_expression* ce;
};

struct expression
{
    vector<assignment_expression*> ae;
};

struct statement
{
};

struct labeled_statement : statement
{
    statement* stat;
};

struct goto_label : labeled_statement
{
    token id;
};

struct case_label : labeled_statement
{
    constant_expression* ce;
};

struct default_label : labeled_statement
{
};

struct expression_statement : statement
{
    expression* expr;
};

struct selection_statement : statement
{
    expression* expr;
    statement* stat;
};

struct if_statement : selection_statement
{
    statement* estat = nullptr;
};

struct switch_statement : selection_statement
{
};

struct iteration_statement : statement
{
};

struct while_statement : iteration_statement
{
    expression* expr;
    statement* stat;
};

struct do_while_statement : iteration_statement
{
    statement* stat;
    expression* expr;
};

struct for_statement : iteration_statement
{
    expression* expr1;
    expression* expr2;
    expression* expr3;
    statement* stat;
};

struct jump_statement : statement
{
};

struct goto_statement : jump_statement
{
    token id;
};

struct continue_statement : jump_statement
{
};

struct break_statement : jump_statement
{
};

struct return_statement : jump_statement
{
    expression* expr;
};

struct block_item
{
};

struct declaration_item : block_item
{
    declaration* decl;
};

struct statement_item : block_item
{
    statement* stat;
};

struct compound_statement : statement
{
    ~compound_statement()
    {
        for (block_item* i : bi)
            delete i;
    }
    vector<block_item*> bi;
};

struct function_definition
{
    ~function_definition()
    {
        delete ds;
        delete dec;
        delete cs;
    }
    declaration_specifiers* ds;
    declarator* dec;
    compound_statement* cs;
};

struct external_declaration
{
    function_definition* fd = nullptr;
    declaration* decl = nullptr;
};

struct translation_unit
{
    ~translation_unit()
    {
        for (external_declaration* d : ed)
            delete d;
    }
    void print();
    vector<external_declaration*> ed;
};

class parser
{
public:
    struct error : exception
    {
        token_iter tokit;

        error(token_iter tokit) : tokit(tokit)
        {
        }

        const char* what() const noexcept
        {
            return stringify(tokit->type).c_str();
        }
    };

    parser(vector<token>& tokens) : tokens(tokens), tokit(tokens.begin())
    {
    }

    translation_unit* parse()
    {
        return parse_translation_unit();
    }

private:
    vector<token>& tokens;
    token_iter tokit;

    bool check(const string& what)
    {
        if (tokit == tokens.end() || tokit->str != what)
            return false;
        return ++tokit, true;
    }

    bool check_any(const vector<string>& what)
    {
        for (const string& s : what)
            if (check(s))
                return --tokit, true;
        return false;
    }

    void accept(const string& what)
    {
        if (!check(what))
            reject();
    }

    void accept_any(const vector<string>& what)
    {
        for (const string& s : what)
            if (check(s))
                return;
        reject();
    }

    void reject()
    {
        throw error(tokit);
    }

    expression* parse_expression();
    primary_expression* parse_primary_expression();
    postfix_expression* parse_postfix_expression();
    unary_expression* parse_unary_expression();
    cast_expression* parse_cast_expression();
    multiplicative_expression* parse_multiplicative_expression();
    additive_expression* parse_additive_expression();
    shift_expression* parse_shift_expression();
    relational_expression* parse_relational_expression();
    equality_expression* parse_equality_expression();
    and_expression* parse_and_expression();
    exclusive_or_expression* parse_exclusive_or_expression();
    inclusive_or_expression* parse_inclusive_or_expression();
    logical_and_expression* parse_logical_and_expression();
    logical_or_expression* parse_logical_or_expression();
    conditional_expression* parse_conditional_expression();
    assignment_expression* parse_assignment_expression();
    constant_expression* parse_constant_expression();

    function_specifier* parse_function_specifier();
    storage_class_specifier* parse_storage_class_specifier();
    declaration* parse_declaration();
    labeled_statement* parse_labeled_statement();
    expression_statement* parse_expression_statement();
    selection_statement* parse_selection_statement();
    iteration_statement* parse_iteration_statement();
    jump_statement* parse_jump_statement();
    statement* parse_statement();
    type_qualifier* parse_type_qualifier();
    type_specifier* parse_type_specifier();
    struct_or_union_specifier* parse_struct_or_union_specifier();
    struct_declaration* parse_struct_declaration();
    vector<declarator*> parse_struct_declarator_list();
    vector<struct_declaration*> parse_struct_declaration_list();
    direct_abstract_declarator* parse_direct_abstract_declarator();
    abstract_declarator* parse_abstract_declarator();
    type_name* parse_type_name();
    declaration_specifiers* parse_declaration_specifiers();
    vector<parameter_declaration*> parse_parameter_type_list();
    pointer* parse_pointer();
    parameter_declaration* parse_parameter_declaration();
    direct_declarator* parse_nof_direct_declarator();
    direct_declarator* parse_direct_declarator();
    declarator* parse_declarator();
    block_item* parse_block_item();
    compound_statement* parse_compound_statement();
    function_definition* parse_function_definition();
    external_declaration* parse_external_declaration();
    translation_unit* parse_translation_unit();
};
