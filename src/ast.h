#pragma once
#include "error.h"
#include <map>

struct node
{
};

struct object
{
    virtual ~object()
    {
    }
};

struct function_object : object
{
    bool is_defined;
    function_object(bool is_defined): is_defined(is_defined) {}
};

struct variable_object : object
{
};

struct tag
{
    bool is_defined;
    tag(bool is_defined): is_defined(is_defined) {}
};

struct scope
{
    ~scope()
    {
        for (auto [str, obj] : vars)
            delete obj;
        for (auto [str, obj] : tags)
            delete obj;
    }
    map<string, object*> vars;
    map<string, tag*> tags;
};

struct specifier_qualifier
{
    virtual void print() = 0;
};

struct type_qualifier : specifier_qualifier
{
    void print();

    token tok;
};

struct type_specifier : specifier_qualifier
{
    virtual void print() = 0;
};

struct builtin_type_specifier : type_specifier
{
    void print();

    token tok;
};

struct declarator;

struct struct_declaration
{
    void print();

    type_specifier* ts;
    vector<declarator*> ds;
};

struct struct_or_union_specifier : type_specifier
{
    void print();

    token sou;
    token id;
    bool has_sds = false;
    vector<struct_declaration*> sds;
};

struct abstract_declarator;
struct parameter_declaration;

struct direct_abstract_declarator
{
    void print();

    abstract_declarator* ad = nullptr;
    vector<parameter_declaration*> pl;
};

struct pointer;

struct abstract_declarator
{
    void print();

    pointer* p = nullptr;
    direct_abstract_declarator* dad = nullptr;
};

struct type_name
{
    void print();

    vector<specifier_qualifier*> sqs;
    abstract_declarator* ad = nullptr;
};

struct declaration_specifiers
{
    void print();

    type_specifier* ts;
};

struct storage_class_specifier
{
    void print();

    token tok;
};

struct function_specifier
{
    void print();

    token tok;
};

struct direct_declarator
{
    virtual void print();
    virtual token get_identifier();
    virtual bool is_definition(); // is function pointer
    virtual bool is_identifier();

    token tok;
};

struct parenthesized_declarator : direct_declarator
{
    void print();
    virtual token get_identifier();
    virtual bool is_definition();
    virtual bool is_identifier();

    declarator* decl;
};

struct parameter_declaration
{
    void print();

    declaration_specifiers* ds;
    declarator* decl = nullptr;
    abstract_declarator* ad = nullptr;
};

struct function_declarator : direct_declarator
{
    void print();
    virtual token get_identifier();
    virtual bool is_definition();
    virtual bool is_identifier();

    direct_declarator* dd;
    vector<parameter_declaration*> pl;
};

struct pointer
{
    void print();

    vector<type_qualifier*> tql;
    pointer* p = nullptr;
};

struct declarator
{
    void print();
    token get_identifier();

    pointer* p = nullptr;
    direct_declarator* dd;
};

struct declaration
{
    void print();

    ~declaration()
    {
        delete ds;
        for (declarator* d : d)
            delete d;
    }

    declaration_specifiers* ds;
    vector<declarator*> d;
};

struct expression;

struct primary_expression
{
    virtual void print();

    object* var = nullptr;
    token tok;
};

struct parenthesized_expression : primary_expression
{
    void print();

    expression* expr;
};

struct postfix_expression
{
    virtual void print();

    postfix_expression* pfe = nullptr;
    primary_expression* pe = nullptr;
};

struct subscript_expression : postfix_expression
{
    void print();

    expression* expr;
};

struct assignment_expression;

struct call_expression : postfix_expression
{
    void print();

    vector<assignment_expression*> args;
};

struct dot_expression : postfix_expression
{
    void print();

    token id;
};

struct arrow_expression : postfix_expression
{
    void print();

    token id;
};

struct postfix_increment_expression : postfix_expression
{
    void print();
};

struct postfix_decrement_expression : postfix_expression
{
    void print();
};

struct unary_expression
{
    virtual void print();

    postfix_expression* pe = nullptr;
};

struct prefix_increment_expression : unary_expression
{
    void print();

    unary_expression* ue;
};

struct prefix_decrement_expression : unary_expression
{
    void print();

    unary_expression* ue;
};

struct cast_expression;

struct unary_and_expression : unary_expression
{
    void print();

    cast_expression* ce;
};

struct unary_star_expression : unary_expression
{
    void print();

    cast_expression* ce;
};

struct unary_plus_expression : unary_expression
{
    void print();

    cast_expression* ce;
};

struct unary_minus_expression : unary_expression
{
    void print();

    cast_expression* ce;
};

struct unary_tilde_expression : unary_expression
{
    void print();

    cast_expression* ce;
};

struct unary_not_expression : unary_expression
{
    void print();

    cast_expression* ce;
};

struct sizeof_expression : unary_expression
{
    void print();

    unary_expression* ue;
};

struct sizeof_type_expression : unary_expression
{
    void print();

    type_name* tn;
};

struct cast_expression
{
    void print();

    unary_expression* ue = nullptr;
    type_name* tn = nullptr;
    cast_expression* ce = nullptr;
};

struct multiplicative_expression
{
    virtual void print();

    cast_expression* ce = nullptr;
};

struct mul_expression : multiplicative_expression
{
    void print();

    multiplicative_expression* lhs;
    cast_expression* rhs;
};

struct div_expression : multiplicative_expression
{
    void print();

    multiplicative_expression* lhs;
    cast_expression* rhs;
};

struct mod_expression : multiplicative_expression
{
    void print();

    multiplicative_expression* lhs;
    cast_expression* rhs;
};

struct additive_expression
{
    virtual void print();

    multiplicative_expression* me;
};

struct add_expression : additive_expression
{
    void print();

    additive_expression* lhs;
    multiplicative_expression* rhs;
};

struct sub_expression : additive_expression
{
    void print();

    additive_expression* lhs;
    multiplicative_expression* rhs;
};

struct shift_expression
{
    virtual void print();

    additive_expression* ae;
};

struct rshift_expression : shift_expression
{
    void print();

    shift_expression* lhs;
    additive_expression* rhs;
};

struct lshift_expression : shift_expression
{
    void print();

    shift_expression* lhs;
    additive_expression* rhs;
};

struct relational_expression
{
    virtual void print();

    shift_expression* se;
};

struct less_expression : relational_expression
{
    void print();

    relational_expression* lhs;
    shift_expression* rhs;
};

struct greater_expression : relational_expression
{
    void print();

    relational_expression* lhs;
    shift_expression* rhs;
};

struct less_equal_expression : relational_expression
{
    void print();

    relational_expression* lhs;
    shift_expression* rhs;
};

struct greater_equal_expression : relational_expression
{
    void print();

    relational_expression* lhs;
    shift_expression* rhs;
};

struct equality_expression
{
    virtual void print();

    relational_expression* re;
};

struct equal_expression : equality_expression
{
    void print();

    equality_expression* lhs;
    relational_expression* rhs;
};

struct not_equal_expression : equality_expression
{
    void print();

    equality_expression* lhs;
    relational_expression* rhs;
};

struct and_expression
{
    void print();

    equality_expression* ee = nullptr;
    and_expression* lhs = nullptr;
    equality_expression* rhs = nullptr;
};

struct exclusive_or_expression
{
    void print();

    and_expression* ae = nullptr;
    exclusive_or_expression* lhs = nullptr;
    and_expression* rhs = nullptr;
};

struct inclusive_or_expression
{
    void print();

    exclusive_or_expression* xe = nullptr;
    inclusive_or_expression* lhs = nullptr;
    exclusive_or_expression* rhs = nullptr;
};

struct logical_and_expression
{
    void print();

    inclusive_or_expression* oe = nullptr;
    logical_and_expression* lhs = nullptr;
    inclusive_or_expression* rhs = nullptr;
};

struct logical_or_expression
{
    void print();

    logical_and_expression* ae = nullptr;
    logical_or_expression* lhs = nullptr;
    logical_and_expression* rhs = nullptr;
};

struct conditional_expression
{
    void print();

    logical_or_expression* oe = nullptr;
    logical_or_expression* expr1 = nullptr;
    expression* expr2 = nullptr;
    conditional_expression* expr3 = nullptr;
};

struct assignment_expression
{
    void print();

    conditional_expression* lhs = nullptr;
    token op;
    assignment_expression* rhs = nullptr;
};

struct constant_expression
{
    void print();

    conditional_expression* ce;
};

struct expression
{
    void print();

    vector<assignment_expression*> ae;
};

struct statement
{
    virtual ~statement()
    {
    }

    virtual void print() = 0;
};

struct labeled_statement : statement
{
    virtual void print() = 0;

    statement* stat;
};

struct goto_label : labeled_statement
{
    void print();

    token id;
};

struct case_label : labeled_statement
{
    void print();

    constant_expression* ce;
};

struct default_label : labeled_statement
{
    void print();
};

struct expression_statement : statement
{
    void print();

    expression* expr = nullptr;
};

struct selection_statement : statement
{
    virtual void print() = 0;

    expression* expr;
    statement* stat;
};

struct if_statement : selection_statement
{
    void print();

    statement* estat = nullptr;
};

struct switch_statement : selection_statement
{
    void print();
};

struct iteration_statement : statement
{
    virtual void print() = 0;
};

struct while_statement : iteration_statement
{
    void print();

    expression* expr;
    statement* stat;
};

struct do_while_statement : iteration_statement
{
    void print();

    statement* stat;
    expression* expr;
};

struct for_statement : iteration_statement
{
    void print();

    expression* expr1;
    expression* expr2;
    expression* expr3;
    statement* stat;
};

struct jump_statement : statement
{
    virtual void print() = 0;
};

struct goto_statement : jump_statement
{
    void print();

    goto_label* gl;
    token id;
};

struct continue_statement : jump_statement
{
    void print();
};

struct break_statement : jump_statement
{
    void print();
};

struct return_statement : jump_statement
{
    ~return_statement()
    {
        delete expr;
    }
    void print();

    expression* expr;
};

struct block_item
{
    virtual ~block_item()
    {
    }
    virtual void print() = 0;
};

struct declaration_item : block_item
{
    ~declaration_item()
    {
        delete decl;
    }
    void print();

    declaration* decl;
};

struct statement_item : block_item
{
    ~statement_item()
    {
        delete stat;
    }
    void print();

    statement* stat;
};

struct compound_statement : statement
{
    ~compound_statement()
    {
        delete sc;
        for (block_item* i : bi)
            delete i;
    }
    void print();

    scope* sc;
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
    void print();

    declaration_specifiers* ds;
    declarator* dec;
    compound_statement* cs;
};

struct external_declaration
{
    void print();

    function_definition* fd = nullptr;
    declaration* decl = nullptr;
};

struct translation_unit
{
    ~translation_unit()
    {
        delete sc;
        for (external_declaration* d : ed)
            delete d;
    }
    void print();

    scope* sc;
    vector<external_declaration*> ed;
};
