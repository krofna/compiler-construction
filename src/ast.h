#pragma once
#include "error.h"
#include <map>
#include "llvm/IR/IRBuilder.h"
using namespace llvm;

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
    function_object(bool is_defined);

    FunctionType *type = nullptr;
    Function *function = nullptr;
    bool is_defined;
};


struct variable_object : object
{
    variable_object(Type *type);
    Value *store = nullptr;
    Type *type = nullptr;
};

struct struct_or_union_specifier;

struct tag
{
    tag(struct_or_union_specifier* sus);

    string h; // h stands for i Hate this course
    map<string, int> indices;
    StructType *type;
};

struct scope
{
    scope(bool global) : global(global)
    {
    }
    ~scope()
    {
        for (auto [str, obj] : vars)
            delete obj;
        for (auto [str, obj] : tags)
            delete obj;
    }
    bool global;
    map<string, object*> vars;
    map<string, tag*> tags;
};

struct declspec
{
    virtual void print() = 0;
};

struct specifier_qualifier : declspec
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
    virtual bool is_void() = 0;
};

struct builtin_type_specifier : type_specifier
{
    void print();
    virtual bool is_void();

    token tok;
};

struct declarator;

struct struct_declaration
{
    void print();

    Type *type;
    struct_or_union_specifier* sus = nullptr;
    vector<specifier_qualifier*> sqs;
    vector<declarator*> ds;
};

struct struct_or_union_specifier : type_specifier
{
    void print();
    virtual bool is_void();

    token sou;
    token id;
    bool has_sds = false;
    vector<struct_declaration*> sds;
};

struct type_name
{
    void print();

    Type* type;
    struct_or_union_specifier* sus = nullptr;
    vector<specifier_qualifier*> sqs;
    declarator* ad = nullptr;
};

struct declaration_specifiers
{
    void print();

    Type *type;
    struct_or_union_specifier* sus = nullptr;
    vector<declspec*> declspecs;
};

struct storage_class_specifier : declspec
{
    void print();

    token tok;
};

struct function_specifier : declspec
{
    void print();

    token tok;
};

struct direct_declarator
{
    virtual ~direct_declarator() {}
    virtual void print();
    virtual token get_identifier();
    virtual bool is_definition(); // is function pointer
    virtual bool is_identifier();
    virtual bool is_pointer();
    virtual int num_pointers();

    token tok;
};

struct parenthesized_declarator : direct_declarator
{
    void print();
    virtual token get_identifier();
    virtual bool is_definition();
    virtual bool is_identifier();
    virtual bool is_pointer();
    virtual int num_pointers();

    declarator* decl;
};

struct parameter_declaration
{
    void print();

    declaration_specifiers* ds;
    declarator* decl = nullptr;
};

struct function_declarator : direct_declarator
{
    void print();
    virtual token get_identifier();
    virtual bool is_definition();
    virtual bool is_identifier();
    virtual bool is_pointer();
    bool is_noparam();
    virtual int num_pointers();

    direct_declarator* dd;
    vector<parameter_declaration*> pl;
};

struct pointer
{
    void print();
    int num_pointers();

    vector<type_qualifier*> tql;
    pointer* p = nullptr;
};

struct declarator
{
    void print();
    token get_identifier();
    bool is_pointer();
    declarator* unparenthesize();
    int num_pointers();
    Value* codegen();

    pointer* p = nullptr;
    direct_declarator* dd;
};

struct declaration
{
    ~declaration();
    void print();
    Value* codegen();

    declaration_specifiers* ds;
    vector<declarator*> d;
};

struct expression;

struct primary_expression
{
    virtual ~primary_expression();
    virtual void print();
    virtual Value* make_lvalue();
    virtual Value* make_rvalue();

    object* var = nullptr;
    token tok;
};

struct parenthesized_expression : primary_expression
{
    ~parenthesized_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    expression* expr;
};

struct postfix_expression
{
    virtual ~postfix_expression();
    virtual void print();
    virtual Value* make_lvalue();
    virtual Value* make_rvalue();

    token op;
    postfix_expression* pfe = nullptr;
    primary_expression* pe = nullptr;
};

struct subscript_expression : postfix_expression
{
    ~subscript_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    expression* expr;
};

struct assignment_expression;

struct call_expression : postfix_expression
{
    ~call_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    vector<assignment_expression*> args;
};

struct dot_expression : postfix_expression
{
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    token id;
};

struct arrow_expression : postfix_expression
{
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    token id;
};

struct postfix_increment_expression : postfix_expression
{
    void print();
    Value* make_lvalue();
    Value* make_rvalue();
};

struct postfix_decrement_expression : postfix_expression
{
    void print();
    Value* make_lvalue();
    Value* make_rvalue();
};

struct unary_expression
{
    virtual ~unary_expression();
    virtual void print();
    virtual Value* make_lvalue();
    virtual Value* make_rvalue();

    postfix_expression* pe = nullptr;
};

struct prefix_increment_expression : unary_expression
{
    ~prefix_increment_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    unary_expression* ue;
};

struct prefix_decrement_expression : unary_expression
{
    ~prefix_decrement_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    unary_expression* ue;
};

struct cast_expression;

struct unary_and_expression : unary_expression
{
    ~unary_and_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    cast_expression* ce;
};

struct unary_star_expression : unary_expression
{
    ~unary_star_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    cast_expression* ce;
};

struct unary_plus_expression : unary_expression
{
    ~unary_plus_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    cast_expression* ce;
};

struct unary_minus_expression : unary_expression
{
    ~unary_minus_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    cast_expression* ce;
};

struct unary_tilde_expression : unary_expression
{
    ~unary_tilde_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    cast_expression* ce;
};

struct unary_not_expression : unary_expression
{
    ~unary_not_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    cast_expression* ce;
};

struct sizeof_expression : unary_expression
{
    ~sizeof_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    unary_expression* ue;
};

struct sizeof_type_expression : unary_expression
{
    ~sizeof_type_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    type_name* tn;
};

struct cast_expression
{
    ~cast_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    unary_expression* ue = nullptr;
    type_name* tn = nullptr;
    cast_expression* ce = nullptr;
};

struct multiplicative_expression
{
    virtual ~multiplicative_expression();
    virtual void print();
    virtual Value* make_lvalue();
    virtual Value* make_rvalue();

    cast_expression* ce = nullptr;
};

struct mul_expression : multiplicative_expression
{
    ~mul_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    multiplicative_expression* lhs;
    cast_expression* rhs;
};

struct div_expression : multiplicative_expression
{
    ~div_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    multiplicative_expression* lhs;
    cast_expression* rhs;
};

struct mod_expression : multiplicative_expression
{
    ~mod_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    multiplicative_expression* lhs;
    cast_expression* rhs;
};

struct additive_expression
{
    virtual ~additive_expression();
    virtual void print();
    virtual Value* make_lvalue();
    virtual Value* make_rvalue();

    multiplicative_expression* me = nullptr;
};

struct add_expression : additive_expression
{
    ~add_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    additive_expression* lhs;
    multiplicative_expression* rhs;
};

struct sub_expression : additive_expression
{
    ~sub_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    additive_expression* lhs;
    multiplicative_expression* rhs;
};

struct shift_expression
{
    virtual ~shift_expression();
    virtual void print();
    virtual Value* make_lvalue();
    virtual Value* make_rvalue();

    additive_expression* ae = nullptr;
};

struct rshift_expression : shift_expression
{
    ~rshift_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    shift_expression* lhs;
    additive_expression* rhs;
};

struct lshift_expression : shift_expression
{
    ~lshift_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    shift_expression* lhs;
    additive_expression* rhs;
};

struct relational_expression
{
    virtual ~relational_expression();
    virtual void print();
    virtual Value* make_lvalue();
    virtual Value* make_rvalue();

    shift_expression* se = nullptr;
};

struct less_expression : relational_expression
{
    ~less_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    relational_expression* lhs;
    shift_expression* rhs;
};

struct greater_expression : relational_expression
{
    ~greater_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    relational_expression* lhs;
    shift_expression* rhs;
};

struct less_equal_expression : relational_expression
{
    ~less_equal_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    relational_expression* lhs;
    shift_expression* rhs;
};

struct greater_equal_expression : relational_expression
{
    ~greater_equal_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    relational_expression* lhs;
    shift_expression* rhs;
};

struct equality_expression
{
    virtual ~equality_expression();
    virtual void print();
    virtual Value* make_lvalue();
    virtual Value* make_rvalue();

    relational_expression* re = nullptr;
};

struct equal_expression : equality_expression
{
    ~equal_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    equality_expression* lhs;
    relational_expression* rhs;
};

struct not_equal_expression : equality_expression
{
    ~not_equal_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    equality_expression* lhs;
    relational_expression* rhs;
};

struct and_expression
{
    ~and_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    equality_expression* ee = nullptr;
    and_expression* lhs = nullptr;
    equality_expression* rhs = nullptr;
};

struct exclusive_or_expression
{
    ~exclusive_or_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    and_expression* ae = nullptr;
    exclusive_or_expression* lhs = nullptr;
    and_expression* rhs = nullptr;
};

struct inclusive_or_expression
{
    ~inclusive_or_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    exclusive_or_expression* xe = nullptr;
    inclusive_or_expression* lhs = nullptr;
    exclusive_or_expression* rhs = nullptr;
};

struct logical_and_expression
{
    ~logical_and_expression();
    void print();
    Value* make_rvalue();
    Value* make_lvalue();

    inclusive_or_expression* oe = nullptr;
    logical_and_expression* lhs = nullptr;
    inclusive_or_expression* rhs = nullptr;
};

struct logical_or_expression
{
    ~logical_or_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    logical_and_expression* ae = nullptr;
    logical_or_expression* lhs = nullptr;
    logical_and_expression* rhs = nullptr;
};

struct conditional_expression
{
    ~conditional_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    logical_or_expression* oe = nullptr;
    logical_or_expression* expr1 = nullptr;
    expression* expr2 = nullptr;
    conditional_expression* expr3 = nullptr;
};

struct assignment_expression
{
    ~assignment_expression();
    void print();
    Value* make_lvalue();
    Value* make_rvalue();

    conditional_expression* lhs = nullptr;
    token op;
    assignment_expression* rhs = nullptr;
};

struct constant_expression
{
    ~constant_expression();
    void print();
    Value* make_rvalue();
    Value* make_lvalue();

    conditional_expression* ce;
};

struct expression
{
    ~expression();
    void print();
    Value* make_rvalue();
    Value* make_lvalue();

    vector<assignment_expression*> ae;
};

struct statement
{
    virtual ~statement() = 0;
    virtual Value* codegen() = 0;
    virtual void print() = 0;
};

struct labeled_statement : statement
{
    virtual ~labeled_statement();
    virtual void print() = 0;
    virtual Value* codegen() = 0;

    statement* stat;
};

struct goto_label : labeled_statement
{
    void print();
    virtual Value* codegen();

    token id;
};

struct case_label : labeled_statement
{
    ~case_label();
    void print();
    virtual Value* codegen();

    constant_expression* ce;
};

struct default_label : labeled_statement
{
    void print();
    virtual Value* codegen();
};

struct expression_statement : statement
{
    ~expression_statement();
    void print();
    virtual Value* codegen();

    expression* expr = nullptr;
};

struct selection_statement : statement
{
    virtual ~selection_statement() = 0;
    virtual void print() = 0;
    virtual Value* codegen() = 0;
};

struct if_statement : selection_statement
{
    ~if_statement();
    void print();
    virtual Value* codegen();

    expression* expr;
    statement* stat;
    statement* estat = nullptr;
};

struct switch_statement : selection_statement
{
    void print();
    virtual Value* codegen();

    expression* expr;
    statement* stat;
};

struct iteration_statement : statement
{
    virtual ~iteration_statement() = 0;
    virtual void print() = 0;
    virtual Value* codegen() = 0;
};

struct while_statement : iteration_statement
{
    ~while_statement();
    void print();
    virtual Value* codegen();

    expression* expr;
    statement* stat;
};

struct do_while_statement : iteration_statement
{
    ~do_while_statement();
    void print();
    virtual Value* codegen();

    statement* stat;
    expression* expr;
};

struct for_statement : iteration_statement
{
    ~for_statement();
    void print();
    virtual Value* codegen();

    expression* expr1;
    expression* expr2;
    expression* expr3;
    statement* stat;
};

struct jump_statement : statement
{
    virtual void print() = 0;
    virtual Value* codegen() = 0;
};

struct goto_statement : jump_statement
{
    void print();
    virtual Value* codegen();

    goto_label* gl;
    token id;
};

struct continue_statement : jump_statement
{
    void print();
    virtual Value* codegen();
};

struct break_statement : jump_statement
{
    void print();
    virtual Value* codegen();
};

struct return_statement : jump_statement
{
    ~return_statement();
    void print();
    virtual Value* codegen();

    expression* expr;
};

struct block_item
{
    virtual ~block_item() = 0;
    virtual void print() = 0;
    virtual Value* codegen() = 0;
};

struct declaration_item : block_item
{
    ~declaration_item();
    void print();
    Value* codegen();

    declaration* decl;
};

struct statement_item : block_item
{
    ~statement_item();
    void print();
    Value* codegen();

    statement* stat;
};

struct compound_statement : statement
{
    ~compound_statement();
    void print();
    virtual Value* codegen();

    scope* sc = nullptr;
    vector<block_item*> bi;
};

struct function_definition
{
    ~function_definition();
    void print();
    Value* codegen();

    token get_identifier();

    scope* sc;
    declaration_specifiers* ds;
    declarator* dec;
    compound_statement* cs;
};

struct external_declaration
{
    ~external_declaration();
    void print();
    Value* codegen();

    function_definition* fd = nullptr;
    declaration* decl = nullptr;
};

struct translation_unit
{
    ~translation_unit();
    void print();
    Value* codegen(const char* filename);

    scope* sc;
    vector<external_declaration*> ed;
};

object* find_var(const string& id);
variable_object* find_variable(const string& id);
function_object* find_function(const string& id);
tag* find_tag(const string& id);
Type* register_type(struct_or_union_specifier* ss);
void resolve_gotos();

extern vector<scope*> scopes;
extern vector<goto_statement*> gotos;
extern map<string, goto_label*> labels;
