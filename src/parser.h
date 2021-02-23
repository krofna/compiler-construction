#pragma once
#include "ast.h"

using token_iter = vector<token>::iterator;

class parser
{
public:
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

    vector<goto_statement*> gotos;
    map<string, goto_label*> labels;

    iteration_statement* current_loop = nullptr;
    switch_statement* current_switch = nullptr;

    void resolve_gotos()
    {
        for (goto_statement* gs : gotos)
        {
            auto glp = labels.find(gs->id.str);
            if (glp == labels.end())
                error::reject(gs->id);

            gs->gl = glp->second;
        }
        gotos.clear();
        labels.clear();
    }

    vector<scope*> scopes;

    string get_digraph(const string& str)
    {
        static const map<string, string> digraphs =
            {
                {"[", "<:"}, {"]", ":>"}, {"{", "<%"},
                {"}", "%>"}, {"#", "%:"}, {"##", "%:%:"}
            };
        auto it = digraphs.find(str);
        if (it != digraphs.end())
            return it->second;
        return str;
    }

    bool check(const string& what)
    {
        if (tokit->str != what && tokit->str != get_digraph(what))
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

    void accepts(const string& what)
    {
        if (!check(what))
            reject();
    }

    template <class T> T* accept(T* ptr)
    {
        if (!ptr)
            reject();
        return ptr;
    }

    void accept_any(const vector<string>& what)
    {
        for (const string& s : what)
            if (check(s))
                return;
        reject();
    }

    void reject(int rollback = 0)
    {
        while (rollback--)
            --tokit;
        error::reject(*tokit);
    }

    token parse_token()
    {
        if (tokit->type == END_OF_FILE)
            reject();
        return *tokit++;
    }

    bool check_identifier()
    {
        if (tokit->type != IDENTIFIER)
            return false;
        return true;
    }

    token parse_identifier()
    {
        if (!check_identifier())
            reject();
        return *tokit++;
    }

    object* find_var(const string& id)
    {
        for (auto i = scopes.rbegin(); i != scopes.rend(); ++i)
        {
            scope* s = *i;
            auto it = s->vars.find(id);
            if (it != s->vars.end())
                return it->second;
        }
        return nullptr;
    }

    tag* find_tag(const string& id)
    {
        for (auto i = scopes.rbegin(); i != scopes.rend(); ++i)
        {
            scope* s = *i;
            auto it = s->tags.find(id);
            if (it != s->tags.end())
                return it->second;
        }
        return nullptr;
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
    compound_statement* parse_compound_statement(bool open_scope);
    function_definition* parse_function_definition();
    external_declaration* parse_external_declaration();
    translation_unit* parse_translation_unit();
};
