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
    if (check("&"))
    {
        unary_and_expression* ae = new unary_and_expression;
        ae->ce = parse_cast_expression();
        return ae;
    }
    if (check("*"))
    {
        unary_star_expression* se = new unary_star_expression;
        se->ce = parse_cast_expression();
        return se;
    }
    if (check("+"))
    {
        unary_plus_expression* pe = new unary_plus_expression;
        pe->ce = parse_cast_expression();
        return pe;
    }
    if (check("-"))
    {
        unary_minus_expression* me = new unary_minus_expression;
        me->ce = parse_cast_expression();
        return me;
    }
    if (check("~"))
    {
        unary_tilde_expression* te = new unary_tilde_expression;
        te->ce = parse_cast_expression();
        return te;
    }
    if (check("!"))
    {
        unary_not_expression* ne = new unary_not_expression;
        ne->ce = parse_cast_expression();
        return ne;
    }
    if (check("sizeof"))
    {
        if (unary_expression* ue = parse_unary_expression())
        {
            sizeof_expression* se = new sizeof_expression;
            se->ue = parse_unary_expression();
            return se;
        }
        if (check("("))
        {
            sizeof_type_expression* se = new sizeof_type_expression;
            se->tn = parse_type_name();
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
        cast_expression* ce = new cast_expression;
        ce->tn = parse_type_name();
        accept(")");
        ce->ce = parse_cast_expression();
        return ce;
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
    if (multiplicative_expression* lhs = parse_multiplicative_expression())
    {
        if (check("*"))
        {
            mul_expression* me = new mul_expression;
            me->lhs = lhs;
            me->rhs = parse_cast_expression();
            return me;
        }
        if (check("/"))
        {
            div_expression* me = new div_expression;
            me->lhs = lhs;
            me->rhs = parse_cast_expression();
            return me;
        }
        if (check("%"))
        {
            mod_expression* me = new mod_expression;
            me->lhs = lhs;
            me->rhs = parse_cast_expression();
            return me;
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
    if (additive_expression* lhs = parse_additive_expression())
    {
        if (check("+"))
        {
            add_expression* ae = new add_expression;
            ae->lhs = lhs;
            ae->rhs = parse_multiplicative_expression();
            return ae;
        }
        if (check("-"))
        {
            sub_expression* se = new sub_expression;
            se->lhs = lhs;
            se->rhs = parse_multiplicative_expression();
            return se;
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
    if (shift_expression* lhs = parse_shift_expression())
    {
        if (check("<<"))
        {
            lshift_expression* ae = new lshift_expression;
            ae->lhs = lhs;
            ae->rhs = parse_additive_expression();
            return ae;
        }
        if (check(">>"))
        {
            rshift_expression* ae = new rshift_expression;
            ae->lhs = lhs;
            ae->rhs = parse_additive_expression();
            return ae;
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
    if (relational_expression* lhs = parse_relational_expression())
    {
        if (check("<"))
        {
            less_expression* le = new less_expression;
            le->lhs = lhs;
            le->rhs = parse_shift_expression();
            return le;
        }
        if (check(">"))
        {
            greater_expression* ge = new greater_expression;
            ge->lhs = lhs;
            ge->rhs = parse_shift_expression();
            return ge;
        }
        if (check("<="))
        {
            less_equal_expression* le = new less_equal_expression;
            le->lhs = lhs;
            le->rhs = parse_shift_expression();
            return le;
        }
        if (check(">="))
        {
            greater_equal_expression* ge = new greater_equal_expression;
            ge->lhs = lhs;
            ge->rhs = parse_shift_expression();
            return ge;
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
    if (equality_expression* lhs = parse_equality_expression())
    {
        if (check("=="))
        {
            equal_expression* ee = new equal_expression;
            ee->lhs = lhs;
            ee->rhs = parse_relational_expression();
            return ee;
        }
        if (check("!="))
        {
            not_equal_expression* ne = new not_equal_expression;
            ne->lhs = lhs;
            ne->rhs = parse_relational_expression();
            return ne;
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
    if (and_expression* lhs = parse_and_expression())
    {
        and_expression* ae = new and_expression;
        ae->lhs = lhs;
        accept("&");
        ae->rhs = parse_equality_expression();
        return ae;
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
    if (exclusive_or_expression* lhs = parse_exclusive_or_expression())
    {
        exclusive_or_expression* xe = new exclusive_or_expression;
        xe->lhs = lhs;
        accept("^");
        xe->rhs = parse_and_expression();
        return xe;
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
    if (inclusive_or_expression* lhs = parse_inclusive_or_expression())
    {
        inclusive_or_expression* oe = new inclusive_or_expression;
        oe->lhs = lhs;
        accept("|");
        oe->rhs = parse_exclusive_or_expression();
        return oe;
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
    if (logical_and_expression* lhs = parse_logical_and_expression())
    {
        logical_and_expression* ae = new logical_and_expression;
        ae->lhs = lhs;
        accept("&&");
        ae->rhs = parse_inclusive_or_expression();
        return ae;
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
    if (logical_or_expression* lhs = parse_logical_or_expression())
    {
        logical_or_expression* oe = new logical_or_expression;
        oe->lhs = lhs;
        accept("||");
        oe->rhs = parse_logical_and_expression();
        return oe;
    }
    return nullptr;
}

conditional_expression* parser::parse_conditional_expression()
{
    if (logical_or_expression* oe = parse_logical_or_expression())
    {
        conditional_expression* ce = new conditional_expression;
        if (check("?"))
        {
            ce->expr1 = oe;
            ce->expr2 = parse_expression();
            accept(":");
            ce->expr3 = parse_conditional_expression();
        }
        else
        {
            ce->oe = oe;
        }
        return ce;
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
        // ...
        parse_assignment_expression();
    }
    return nullptr;
}

constant_expression* parser::parse_constant_expression()
{
    constant_expression* ce = new constant_expression;
    ce->ce = parse_conditional_expression();
    return ce;
}

expression* parser::parse_expression()
{
    expression* expr = new expression;
    do
        expr->ae.push_back(parse_assignment_expression());
    while (check(","));
    return expr;
}

declaration* parser::parse_declaration()
{
    declaration* decl = new declaration;
    decl->ds = parse_declaration_specifiers();
    if (declarator* d = parse_declarator())
        decl->d = d;
    return decl;
}

declaration_specifiers* parser::parse_declaration_specifiers()
{
    declaration_specifiers* ds = new declaration_specifiers;
    ds->ts = parse_type_specifier();
    return ds;
}

storage_class_specifier* parser::parse_storage_class_specifier()
{
    static const vector<string> specifiers = {
        "typedef", "extern", "static", "_Thread_local", "auto", "register"
    };
    if (check_any(specifiers))
    {
        storage_class_specifier* ss = new storage_class_specifier;
        ss->tok = *tokit++;
        return ss;
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
        ts->tok = *tokit++;
        return ts;
    }
    if (check_any({"struct", "union"}))
    {
        struct_or_union_specifier* ss = new struct_or_union_specifier;
        ss->sou = *tokit++;
        ss->id = *tokit++;
        return ss;
    }
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
        tq->tok = *tokit++;
        return tq;
    }
    return nullptr;
}

function_specifier* parser::parse_function_specifier()
{
    static const vector<string> specifiers = {
        "inline", "_Noreturn"
    };
    if (check_any(specifiers))
    {
        function_specifier* fs = new function_specifier;
        fs->tok = *tokit++;
        return fs;
    }
    return nullptr;
}

declarator* parser::parse_declarator()
{
    if (pointer* p = parse_pointer())
    {
        declarator* decl = new declarator;
        decl->p = p;
        decl->dd = parse_direct_declarator();
        return decl;
    }
    if (direct_declarator* dd = parse_direct_declarator())
    {
        declarator* decl = new declarator;
        decl->dd = parse_direct_declarator();
        return decl;
    }
    return nullptr;
}

direct_declarator* parser::parse_direct_declarator()
{
    if (tokit->type == IDENTIFIER)
    {
        direct_declarator* dd = new direct_declarator;
        dd->tok = *tokit++;
        return dd;
    }
    if (check("("))
    {
        parenthesized_declarator* pd = new parenthesized_declarator;
        pd->decl = parse_declarator();
        accept(")");
        return pd;
    }
    if (direct_declarator* dd = parse_direct_declarator())
    {
        function_declarator* fd = new function_declarator;
        fd->dd = dd;
        do
            fd->pl.push_back(parse_parameter_declaration());
        while (check(","));
        return fd;
    }
    return nullptr;
}

pointer* parser::parse_pointer()
{
    if (check("*"))
    {
        pointer* p = new pointer;
        while (type_qualifier* tq = parse_type_qualifier())
            p->tql.push_back(tq);
        if (pointer* nxt = parse_pointer())
            p->p = nxt;
        return p;
    }
    return nullptr;
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

direct_abstract_declarator* parser::parse_direct_abstract_declarator()
{
    // ...
    if (check("("))
    {
        parse_abstract_declarator();
        accept(")");
    }
    return nullptr;
}

abstract_declarator* parser::parse_abstract_declarator()
{
    if (pointer* p = parse_pointer())
    {
        abstract_declarator* ad = new abstract_declarator;
        if (direct_abstract_declarator* dad = parse_direct_abstract_declarator())
        {
            // ...
            ;
        }
        return ad;
    }
    return nullptr;
}

type_name* parser::parse_type_name()
{
    type_name* tn = new type_name;
    while (true)
    {
        if (type_specifier* ts = parse_type_specifier())
        {
            tn->sqs.push_back(ts);
            continue;
        }
        if (type_qualifier* tq = parse_type_qualifier())
        {
            tn->sqs.push_back(tq);
            continue;
        }
        break;
    }
    if (tn->sqs.empty())
        return delete tn, nullptr;

    tn->ad = parse_abstract_declarator();
    return tn;
}

parameter_declaration* parser::parse_parameter_declaration()
{
    if (declaration_specifiers* ds = parse_declaration_specifiers())
    {
        parameter_declaration* pd = new parameter_declaration;
        if (declarator* decl = parse_declarator())
        {
            pd->decl = decl;
        }
        else
        {
            pd->ad = parse_abstract_declarator();
        }
        return pd;
    }
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
