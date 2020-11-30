#include "parser.h"

primary_expression* parser::parse_primary_expression()
{
    if (tokit->type == IDENTIFIER)
    {
        primary_expression* pe = new primary_expression;
        pe->tok = *tokit;
        return pe;
    }
    if (tokit->type == CONSTANT)
    {
        primary_expression* pe = new primary_expression;
        pe->tok = *tokit;
        return pe;
    }
    if (tokit->type == STRING_LITERAL)
    {
        primary_expression* pe = new primary_expression;
        pe->tok = *tokit;
        return pe;
    }
    if (check("("))
    {
        primary_expression* pe = new primary_expression;
        pe->expr = parse_expression();
        accept(")");
        return pe;
    }
    return nullptr;
}

postfix_expression* parser::parse_postfix_expression()
{
    if (primary_expression* pe = parse_primary_expression())
    {
        postfix_expression* e = new postfix_expression;
        e->pe = pe;
        return e;
    }
    if (postfix_expression* pe = parse_postfix_expression())
    {
        if (check("["))
        {
            subscript_expression* se = new subscript_expression;
            se->pfe = pe;
            se->expr = parse_expression();
            accept("]");
            return se;
        }
        if (check("("))
        {
            call_expression* ce = new call_expression;
            ce->pfe = pe;
            do
                ce->args.push_back(parse_assignment_expression());
            while (check(","));
            accept(")");
            return ce;
        }
        if (check("."))
        {
            if (tokit->type == IDENTIFIER)
            {
                dot_expression* de = new dot_expression;
                de->pfe = pe;
                de->id = *tokit;
                return de;
            }
        }
        if (check("->"))
        {
            if (tokit->type == IDENTIFIER)
            {
                arrow_expression* ae = new arrow_expression;
                ae->pfe = pe;
                ae->id = *tokit;
                return ae;
            }
        }
        if (check("++"))
        {
            postfix_increment_expression* ie = new postfix_increment_expression;
            ie->pfe = pe;
            return ie;
        }
        if (check("--"))
        {
            postfix_decrement_expression* de = new postfix_decrement_expression;
            de->pfe = pe;
            return de;
        }
        reject();
    }
    // ...
    return nullptr;
}

unary_expression* parser::parse_unary_expression()
{
    if (postfix_expression* pe = parse_postfix_expression())
    {
        unary_expression* ue = new unary_expression;
        ue->pe = pe;
        return ue;
    }
    if (check("++"))
    {
        prefix_increment_expression* ie = new prefix_increment_expression;
        ie->ue = parse_unary_expression();
        return ie;
    }
    if (check("--"))
    {
        prefix_decrement_expression* de = new prefix_decrement_expression;
        de->ue = parse_unary_expression();
        return de;
    }
    if (check_any({"&", "*", "+", "-", "~", "!"}))
    {
        // ...
        parse_cast_expression();
    }
    if (check("sizeof"))
    {
        if (unary_expression* ue = parse_unary_expression())
        {
            // ...
        }
        if (check("("))
        {
            // ...
            accept(")");
        }
    }
    return nullptr;
}

cast_expression* parser::parse_cast_expression()
{
    if (unary_expression* ue = parse_unary_expression())
    {
        cast_expression* ce = new cast_expression;
        ce->ue = ue;
        return ce;
    }
    if (check("("))
    {
        // parse_type_name();
        accept(")");
        parse_cast_expression();
    }
    return nullptr;
}

multiplicative_expression* parser::parse_multiplicative_expression()
{
    if (cast_expression* ce = parse_cast_expression())
    {
        multiplicative_expression* me = new multiplicative_expression;
        me->ce = ce;
        return me;
    }
    if (multiplicative_expression* me = parse_multiplicative_expression())
    {
        if (check("*"))
        {
            parse_cast_expression();
        }
        if (check("/"))
        {
            parse_cast_expression();
        }
        if (check("%"))
        {
            parse_cast_expression();
        }
        reject();
    }
    return nullptr;
}

additive_expression* parser::parse_additive_expression()
{
    if (multiplicative_expression* me = parse_multiplicative_expression())
    {
        additive_expression* ae = new additive_expression;
        ae->me = me;
        return ae;
    }
    if (additive_expression* ae = parse_additive_expression())
    {
        if (check("+"))
        {
            parse_multiplicative_expression();
        }
        if (check("-"))
        {
            parse_multiplicative_expression();
        }
        reject();
    }
    return nullptr;
}

shift_expression* parser::parse_shift_expression()
{
    if (additive_expression* ae = parse_additive_expression())
    {
        shift_expression* se = new shift_expression;
        se->ae = ae;
        return se;
    }
    if (shift_expression* ae = parse_shift_expression())
    {
        if (check("<<"))
        {
            parse_additive_expression();
        }
        if (check(">>"))
        {
            parse_additive_expression();
        }
        reject();
    }
    return nullptr;
}

relational_expression* parser::parse_relational_expression()
{
    if (shift_expression* se = parse_shift_expression())
    {
        relational_expression* re = new relational_expression;
        re->se = se;
        return re;
    }
    if (relational_expression* re = parse_relational_expression())
    {
        if (check("<"))
        {
            parse_shift_expression();
        }
        if (check(">"))
        {
            parse_shift_expression();
        }
        if (check("<="))
        {
            parse_shift_expression();
        }
        if (check(">="))
        {
            parse_shift_expression();
        }
        reject();
    }
    return nullptr;
}

equality_expression* parser::parse_equality_expression()
{
    if (relational_expression* re = parse_relational_expression())
    {
        equality_expression* ee = new equality_expression;
        ee->re = re;
        return ee;
    }
    if (equality_expression* re = parse_equality_expression())
    {
        if (check("=="))
        {
            parse_relational_expression();
        }
        if (check("!="))
        {
            parse_relational_expression();
        }
        reject();
    }
    return nullptr;
}

and_expression* parser::parse_and_expression()
{
    if (equality_expression* ee = parse_equality_expression())
    {
        and_expression* ae = new and_expression;
        ae->ee = ee;
        return ae;
    }
    if (and_expression* ae = parse_and_expression())
    {
        accept("&");
        parse_equality_expression();
    }
    return nullptr;
}

exclusive_or_expression* parser::parse_exclusive_or_expression()
{
    if (and_expression* ae = parse_and_expression())
    {
        exclusive_or_expression* xe = new exclusive_or_expression;
        xe->ae = ae;
        return xe;
    }
    if (exclusive_or_expression* xe = parse_exclusive_or_expression())
    {
        accept("^");
        parse_and_expression();
    }
    return nullptr;
}

inclusive_or_expression* parser::parse_inclusive_or_expression()
{
    if (exclusive_or_expression* xe = parse_exclusive_or_expression())
    {
        inclusive_or_expression* ie;
        ie->xe = xe;
        return ie;
    }
    if (inclusive_or_expression* oe = parse_inclusive_or_expression())
    {
        accept("|");
        parse_exclusive_or_expression();
    }
    return nullptr;
}

logical_and_expression* parser::parse_logical_and_expression()
{
    if (inclusive_or_expression* oe = parse_inclusive_or_expression())
    {
        logical_and_expression* ae;
        ae->oe = oe;
        return ae;
    }
    if (logical_and_expression* ae = parse_logical_and_expression())
    {
        accept("&&");
        parse_inclusive_or_expression();
    }
    return nullptr;
}

logical_or_expression* parser::parse_logical_or_expression()
{
    if (logical_and_expression* ae = parse_logical_and_expression())
    {
        logical_or_expression* oe = new logical_or_expression;
        oe->ae = ae;
        return oe;
    }
    if (logical_or_expression* oe = parse_logical_or_expression())
    {
        accept("||");
        parse_and_expression();
    }
    return nullptr;
}

conditional_expression* parser::parse_conditional_expression()
{
    if (logical_or_expression* oe = parse_logical_or_expression())
    {
        if (check("?"))
        {
            parse_expression();
            accept(":");
            parse_conditional_expression();
        }
    }
    return nullptr;
}

assignment_expression* parser::parse_assignment_expression()
{
    if (conditional_expression* ce = parse_conditional_expression())
    {
        assignment_expression* ae = new assignment_expression;
        ae->ce = ce;
        return ae;
    }
    if (unary_expression* ue = parse_unary_expression())
    {
        accept_any({"=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|="});
        parse_assignment_expression();
    }
    return nullptr;
}

constant_expression* parser::parse_constant_expression()
{
    parse_conditional_expression();
    return nullptr;
}

expression* parser::parse_expression()
{
    expression* expr = new expression;
    do
        expr->ae.push_back(parse_assignment_expression());
    while (check(","));
    return expr;
}

node* parser::parse_direct_declarator()
{
    return nullptr;
}

declaration* parser::parse_declaration()
{
    declaration* decl = new declaration;
    // ...
    return decl;
}

labeled_statement* parser::parse_labeled_statement()
{
    if (tokit->type == IDENTIFIER)
    {
        goto_label* gl = new goto_label;
        gl->id = *tokit;
        gl->stat = parse_statement();
        return gl;
    }
    if (check("case"))
    {
        case_label* cl = new case_label;
        cl->ce = parse_constant_expression();
        cl->stat = parse_statement();
        return cl;
    }
    if (check("default"))
    {
        default_label* dl = new default_label;
        dl->stat = parse_statement();
        return dl;
    }
    return nullptr;
}

expression_statement* parser::parse_expression_statement()
{
    expression_statement* es = new expression_statement;
    es->expr = parse_expression();
    accept(";");
    return es;
}

selection_statement* parser::parse_selection_statement()
{
    if (check("if"))
    {
        if_statement* is = new if_statement;
        accept("(");
        is->expr = parse_expression();
        accept(")");
        is->stat = parse_statement();
        if (check("else"))
            is->estat = parse_statement();
        return is;
    }
    if (check("switch"))
    {
        switch_statement* ss = new switch_statement;
        accept("(");
        ss->expr = parse_expression();
        accept(")");
        ss->stat = parse_statement();
        return ss;
    }
    return nullptr;
}

iteration_statement* parser::parse_iteration_statement()
{
    if (check("while"))
    {
        while_statement* ws = new while_statement;
        accept("(");
        ws->expr = parse_expression();
        accept(")");
        ws->stat = parse_statement();
        return ws;
    }
    if (check("do"))
    {
        do_while_statement* dws = new do_while_statement;
        dws->stat = parse_statement();
        accept("while");
        accept("(");
        dws->expr = parse_expression();
        accept(")");
        accept(";");
        return dws;
    }
    if (check("for"))
    {
        for_statement* fs = new for_statement;
        accept("(");
        fs->expr1 = parse_expression();
        accept(";");
        fs->expr2 = parse_expression();
        accept(";");
        fs->expr3 = parse_expression();
        accept(")");
        fs->stat = parse_statement();
        return fs;
    }
    return nullptr;
}

jump_statement* parser::parse_jump_statement()
{
    if (check("goto"))
    {
        goto_statement* gs = new goto_statement;
        gs->id = *tokit;
        accept(";");
        return gs;
    }
    if (check("continue"))
    {
        continue_statement* cs = new continue_statement;
        accept(";");
        return cs;
    }
    if (check("break"))
    {
        break_statement* bs = new break_statement;
        accept(";");
        return bs;
    }
    if (check("return"))
    {
        return_statement* rs = new return_statement;
        rs->expr = parse_expression();
        accept(";");
        return rs;
    }
    return nullptr;
}

statement* parser::parse_statement()
{
    if (labeled_statement* ls = parse_labeled_statement())
        return ls;
    if (compound_statement* cs = parse_compound_statement())
        return cs;
    if (expression_statement* es = parse_expression_statement())
        return es;
    if (selection_statement* ss = parse_selection_statement())
        return ss;
    if (iteration_statement* is = parse_iteration_statement())
        return is;
    if (jump_statement* js = parse_jump_statement())
        return js;
    return nullptr;
}

type_qualifier* parser::parse_type_qualifier()
{
    static const vector<string> qualifiers = {
        "const", "restrict", "volatile", "_Atomic"
    };
    if (check_any(qualifiers))
    {
        type_qualifier* tq = new type_qualifier;
        // tq->tok = *tokit++;
        return tq;
    }
    return nullptr;
}

type_specifier* parser::parse_type_specifier()
{
    static const vector<string> builtin_types = {
        "void", "char", "short", "int", "long", "float", "double",
        "signed", "unsigned", "_Bool", "_Complex"
    };
    if (check_any(builtin_types))
    {
        builtin_type_specifier* ts = new builtin_type_specifier;
        // ts->tok = *tokit++;
        return ts;
    }
    if (check_any({"struct", "union"}))
    {
        struct_or_union_specifier* ss = new struct_or_union_specifier;
        // ts->sou = *tokit++;
        ss->id = *tokit++;
        return ss;
    }
    return nullptr;
}

type_name* parser::parse_type_name()
{
    return nullptr;
}

declaration_specifiers* parser::parse_declaration_specifiers()
{
    declaration_specifiers* ds = new declaration_specifiers;
    ds->ts = parse_type_specifier();
    return ds;
}

declarator* parser::parse_declarator()
{
    return nullptr;
}

block_item* parser::parse_block_item()
{
    if (declaration* decl = parse_declaration())
    {
        declaration_item* di = new declaration_item;
        di->decl = decl;
        return di;
    }
    if (statement* stat = parse_statement())
    {
        statement_item* si = new statement_item;
        si->stat = stat;
        return si;
    }
    return nullptr;
}

compound_statement* parser::parse_compound_statement()
{
    compound_statement* cs = new compound_statement;
    accept("{");
    while (!check("}"))
        cs->bi.push_back(parse_block_item());
    return cs;
}

function_definition* parser::parse_function_definition()
{
    function_definition* fd = new function_definition;
    fd->ds = parse_declaration_specifiers();
    fd->dec = parse_declarator();
    fd->cs = parse_compound_statement();
    return fd;
}

external_declaration* parser::parse_external_declaration()
{
    external_declaration* ed = new external_declaration;
    while (tokit != tokens.end())
        ed->fd.push_back(parse_function_definition());
    return ed;
}

translation_unit* parser::parse_translation_unit()
{
    translation_unit* root = new translation_unit;
    while (tokit != tokens.end())
        root->ed.push_back(parse_external_declaration());
    return root;
}
