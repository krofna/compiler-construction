#include "parser.h"

primary_expression* parser::parse_primary_expression()
{
    if (tokit->type == IDENTIFIER)
    {
        primary_expression* pe = new primary_expression;
        pe->tok = *tokit++;
        return pe;
    }
    if (tokit->type == CONSTANT)
    {
        primary_expression* pe = new primary_expression;
        pe->tok = *tokit++;
        return pe;
    }
    if (tokit->type == STRING_LITERAL)
    {
        primary_expression* pe = new primary_expression;
        pe->tok = *tokit++;
        return pe;
    }
    if (check("("))
    {
        parenthesized_expression* pe = new parenthesized_expression;
        pe->expr = parse_expression();
        accept(")");
        return pe;
    }
    return nullptr;
}

postfix_expression* parser::parse_postfix_expression()
{
    primary_expression* pe = parse_primary_expression();
    if (!pe)
        return nullptr;

    postfix_expression* e = new postfix_expression;
    e->pe = pe;

    while (true)
    {
        if (check("["))
        {
            subscript_expression* se = new subscript_expression;
            se->pfe = e;
            se->expr = parse_expression();
            accept("]");
            e = se;
        }
        else if (check("("))
        {
            call_expression* ce = new call_expression;
            ce->pfe = e;
            do
                ce->args.push_back(parse_assignment_expression());
            while (check(","));
            accept(")");
            e = ce;
        }
        else if (check("."))
        {
            if (tokit->type == IDENTIFIER)
            {
                dot_expression* de = new dot_expression;
                de->pfe = e;
                de->id = *tokit++;
                e = de;
            }
            else
                reject();
        }
        else if (check("->"))
        {
            if (tokit->type == IDENTIFIER)
            {
                arrow_expression* ae = new arrow_expression;
                ae->pfe = e;
                ae->id = *tokit++;
                e = ae;
            }
            else
                reject();
        }
        else if (check("++"))
        {
            postfix_increment_expression* ie = new postfix_increment_expression;
            ie->pfe = e;
            e = ie;
        }
        else if (check("--"))
        {
            postfix_decrement_expression* de = new postfix_decrement_expression;
            de->pfe = e;
            e = de;
        }
        else
            break;
    }
    return e;
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
        if (check("("))
        {
            if (type_name* tn = parse_type_name())
            {
                sizeof_type_expression* se = new sizeof_type_expression;
                se->tn = tn;
                accept(")");
                return se;
            }
            tokit--;
        }
        if (unary_expression* ue = parse_unary_expression())
        {
            sizeof_expression* se = new sizeof_expression;
            se->ue = ue;
            return se;
        }
        reject();
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
    cast_expression* ce = parse_cast_expression();
    if (!ce)
        return nullptr;

    multiplicative_expression* lhs = new multiplicative_expression;
    lhs->ce = ce;

    while (true)
    {
        if (check("*"))
        {
            mul_expression* me = new mul_expression;
            me->lhs = lhs;
            me->rhs = parse_cast_expression();
            lhs = me;
        }
        else if (check("/"))
        {
            div_expression* de = new div_expression;
            de->lhs = lhs;
            de->rhs = parse_cast_expression();
            lhs = de;
        }
        else if (check("%"))
        {
            mod_expression* me = new mod_expression;
            me->lhs = lhs;
            me->rhs = parse_cast_expression();
            lhs = me;
        }
        else
            break;
    }
    return lhs;
}

additive_expression* parser::parse_additive_expression()
{
    multiplicative_expression* me = parse_multiplicative_expression();
    if (!me)
        return nullptr;

    additive_expression* lhs = new additive_expression;
    lhs->me = me;

    while (true)
    {
        if (check("+"))
        {
            add_expression* ae = new add_expression;
            ae->lhs = lhs;
            ae->rhs = parse_multiplicative_expression();
            lhs = ae;
        }
        else if (check("-"))
        {
            sub_expression* se = new sub_expression;
            se->lhs = lhs;
            se->rhs = parse_multiplicative_expression();
            lhs = se;
        }
        else
            break;
    }
    return lhs;
}

shift_expression* parser::parse_shift_expression()
{
    additive_expression* ae = parse_additive_expression();
    if (!ae)
        return nullptr;

    shift_expression* lhs = new shift_expression;
    lhs->ae = ae;

    while (true)
    {
        if (check("<<"))
        {
            lshift_expression* ae = new lshift_expression;
            ae->lhs = lhs;
            ae->rhs = parse_additive_expression();
            lhs = ae;
        }
        else if (check(">>"))
        {
            rshift_expression* ae = new rshift_expression;
            ae->lhs = lhs;
            ae->rhs = parse_additive_expression();
            lhs = ae;
        }
        else
            break;
    }
    return lhs;
}

relational_expression* parser::parse_relational_expression()
{
    shift_expression* se = parse_shift_expression();
    if (!se)
        return nullptr;

    relational_expression* lhs = new relational_expression;
    lhs->se = se;

    while (true)
    {
        if (check("<"))
        {
            less_expression* le = new less_expression;
            le->lhs = lhs;
            le->rhs = parse_shift_expression();
            lhs = le;
        }
        else if (check(">"))
        {
            greater_expression* ge = new greater_expression;
            ge->lhs = lhs;
            ge->rhs = parse_shift_expression();
            lhs = ge;
        }
        else if (check("<="))
        {
            less_equal_expression* le = new less_equal_expression;
            le->lhs = lhs;
            le->rhs = parse_shift_expression();
            lhs = le;
        }
        else if (check(">="))
        {
            greater_equal_expression* ge = new greater_equal_expression;
            ge->lhs = lhs;
            ge->rhs = parse_shift_expression();
            lhs = ge;
        }
        else
            break;
    }
    return lhs;
}

equality_expression* parser::parse_equality_expression()
{
    relational_expression* re = parse_relational_expression();
    if (!re)
        return nullptr;

    equality_expression* lhs = new equality_expression;
    lhs->re = re;

    while (true)
    {
        if (check("=="))
        {
            equal_expression* ee = new equal_expression;
            ee->lhs = lhs;
            ee->rhs = parse_relational_expression();
            lhs = ee;
        }
        if (check("!="))
        {
            not_equal_expression* ne = new not_equal_expression;
            ne->lhs = lhs;
            ne->rhs = parse_relational_expression();
            lhs = ne;
        }
        else
            break;
    }
    return lhs;
}

and_expression* parser::parse_and_expression()
{
    equality_expression* ee = parse_equality_expression();
    if (!ee)
        return nullptr;

    and_expression* lhs = new and_expression;
    lhs->ee = ee;

    while (check("&"))
    {
        and_expression* ae = new and_expression;
        ae->lhs = lhs;
        ae->rhs = parse_equality_expression();
        lhs = ae;
    }
    return lhs;
}

exclusive_or_expression* parser::parse_exclusive_or_expression()
{
    and_expression* ae = parse_and_expression();
    if (!ae)
        return nullptr;

    exclusive_or_expression* lhs = new exclusive_or_expression;
    lhs->ae = ae;

    while (check("^"))
    {
        exclusive_or_expression* xe = new exclusive_or_expression;
        xe->lhs = lhs;
        xe->rhs = parse_and_expression();
        lhs = xe;
    }
    return lhs;
}

inclusive_or_expression* parser::parse_inclusive_or_expression()
{
    exclusive_or_expression* xe = parse_exclusive_or_expression();
    if (!xe)
        return nullptr;

    inclusive_or_expression* lhs = new inclusive_or_expression;
    lhs->xe = xe;

    while (check("|"))
    {
        inclusive_or_expression* oe = new inclusive_or_expression;
        oe->lhs = lhs;
        oe->rhs = parse_exclusive_or_expression();
        lhs = oe;
    }
    return lhs;
}

logical_and_expression* parser::parse_logical_and_expression()
{
    inclusive_or_expression* oe = parse_inclusive_or_expression();
    if (!oe)
        return nullptr;

    logical_and_expression* lhs = new logical_and_expression;
    lhs->oe = oe;

    while (check("&&"))
    {
        logical_and_expression* ae = new logical_and_expression;
        ae->lhs = lhs;
        ae->rhs = parse_inclusive_or_expression();
        lhs = ae;
    }
    return lhs;
}

logical_or_expression* parser::parse_logical_or_expression()
{
    logical_and_expression* ae = parse_logical_and_expression();
    if (!ae)
        return nullptr;

    logical_or_expression* lhs = new logical_or_expression;
    lhs->ae = ae;

    while (check("||"))
    {
        logical_or_expression* oe = new logical_or_expression;
        oe->lhs = lhs;
        oe->rhs = parse_logical_and_expression();
        lhs = oe;
    }
    return lhs;
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
            ce->oe = oe;

        return ce;
    }
    return nullptr;
}

assignment_expression* parser::parse_assignment_expression()
{
    if (conditional_expression* ce = parse_conditional_expression())
    {
        assignment_expression* ae = new assignment_expression;
        ae->lhs = ce;
        if (check_any({"=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|="}))
        {
            ae->op = *tokit++;
            ae->rhs = parse_assignment_expression();
        }
        return ae;
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
    if (assignment_expression* ae = parse_assignment_expression())
    {
        expression* expr = new expression;
        expr->ae.push_back(ae);
        while (check(","))
            expr->ae.push_back(parse_assignment_expression());
        return expr;
    }
    return nullptr;
}

declaration* parser::parse_declaration()
{
    token_iter old = tokit;
    if (declaration_specifiers* ds = parse_declaration_specifiers())
    {
        declaration* decl = new declaration;
        decl->ds = ds;
        if (declarator* d = parse_declarator())
            decl->d = d;

        if (!check(";"))
        {
            tokit = old;
            delete decl;
            return nullptr;
        }
        return decl;
    }
    return nullptr;
}

declaration_specifiers* parser::parse_declaration_specifiers()
{
    if (type_specifier* ts = parse_type_specifier())
    {
        declaration_specifiers* ds = new declaration_specifiers;
        ds->ts = ts;
        return ds;
    }
    return nullptr;
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
    if (struct_or_union_specifier* ss = parse_struct_or_union_specifier())
        return ss;
    return nullptr;
}

struct_or_union_specifier* parser::parse_struct_or_union_specifier()
{
    if (check_any({"struct", "union"}))
    {
        struct_or_union_specifier* ss = new struct_or_union_specifier;
        ss->sou = *tokit++;
        if (check("{"))
        {
            ss->sds = parse_struct_declaration_list();
            accept("}");
        }
        else
        {
            if (tokit->type != IDENTIFIER)
                reject();

            ss->id = *tokit++;
            if (check("{"))
            {
                ss->sds = parse_struct_declaration_list();
                accept("}");
            }
        }
        return ss;
    }
    return nullptr;
}

struct_declaration* parser::parse_struct_declaration()
{
    if (type_specifier* ts = parse_type_specifier())
    {
        struct_declaration* sd = new struct_declaration;
        sd->ts = ts;
        sd->ds = parse_struct_declarator_list();
        accept(";");
        return sd;
    }
    return nullptr;
}

vector<declarator*> parser::parse_struct_declarator_list()
{
    vector<declarator*> ds;
    if (declarator* d = parse_declarator())
    {
        ds.push_back(d);
        while (check(","))
        {
            ds.push_back(parse_declarator());
            if (!ds.back())
                reject();
        }
    }
    return ds;
}

vector<struct_declaration*> parser::parse_struct_declaration_list()
{
    vector<struct_declaration*> sds;
    while (struct_declaration* sd = parse_struct_declaration())
        sds.push_back(sd);
    return sds;
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
        decl->dd = dd;
        return decl;
    }
    return nullptr;
}

direct_declarator* parser::parse_nof_direct_declarator()
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
    return nullptr;
}

direct_declarator* parser::parse_direct_declarator()
{
    if (direct_declarator* dd = parse_nof_direct_declarator())
    {
        if (check("("))
        {
            function_declarator* fd = new function_declarator;
            fd->dd = dd;
            fd->pl = parse_parameter_type_list();
            if (fd->pl.empty())
                reject();
            accept(")");
            return fd;
        }
        return dd;
    }
    return nullptr;
}

vector<parameter_declaration*> parser::parse_parameter_type_list()
{
    vector<parameter_declaration*> pl;
    if (parameter_declaration* pd = parse_parameter_declaration())
    {
        pl.push_back(pd);
        while (check(","))
        {
            pl.push_back(parse_parameter_declaration());
            if (!pl.back())
                reject();
        }
    }
    return pl;
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

direct_abstract_declarator* parser::parse_direct_abstract_declarator()
{
    if (check("("))
    {
        direct_abstract_declarator* dad = new direct_abstract_declarator;
        if (abstract_declarator* ad = parse_abstract_declarator())
        {
            dad->ad = ad;
            accept(")");
            if (check("("))
            {
                dad->pl = parse_parameter_type_list();
                accept(")");
            }
        }
        else
        {
            dad->pl = parse_parameter_type_list();
            accept(")");
        }
        return dad;
    }
    return nullptr;
}

abstract_declarator* parser::parse_abstract_declarator()
{
    if (pointer* p = parse_pointer())
    {
        abstract_declarator* ad = new abstract_declarator;
        ad->p = p;
        if (direct_abstract_declarator* dad = parse_direct_abstract_declarator())
            ad->dad = dad;
        return ad;
    }
    else if (direct_abstract_declarator* dad = parse_direct_abstract_declarator())
    {
        abstract_declarator* ad = new abstract_declarator;
        ad->dad = dad;
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
        pd->ds = ds;
        if (declarator* decl = parse_declarator())
            pd->decl = decl;
        else
            pd->ad = parse_abstract_declarator();
        return pd;
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

labeled_statement* parser::parse_labeled_statement()
{
    if (tokit->type == IDENTIFIER)
    {
        token id = *tokit++;
        if (!check(":"))
        {
            tokit--;
            return nullptr;
        }
        goto_label* gl = new goto_label;
        gl->id = id;
        gl->stat = parse_statement();
        return gl;
    }
    if (check("case"))
    {
        case_label* cl = new case_label;
        cl->ce = parse_constant_expression();
        accept(":");
        cl->stat = parse_statement();
        return cl;
    }
    if (check("default"))
    {
        default_label* dl = new default_label;
        accept(":");
        dl->stat = parse_statement();
        return dl;
    }
    return nullptr;
}

compound_statement* parser::parse_compound_statement()
{
    if (check("{"))
    {
        compound_statement* cs = new compound_statement;
        while (!check("}"))
        {
            cs->bi.push_back(parse_block_item());
            if (!cs->bi.back())
                reject();
        }
        return cs;
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

expression_statement* parser::parse_expression_statement()
{
    if (expression* expr = parse_expression())
    {
        expression_statement* es = new expression_statement;
        es->expr = expr;
        accept(";");
        return es;
    }
    return nullptr;
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
        gs->id = *tokit++;
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
    if (declaration* decl = parse_declaration())
    {
        external_declaration* ed = new external_declaration;
        ed->decl = decl;
        return ed;
    }
    if (function_definition* fd = parse_function_definition())
    {
        external_declaration* ed = new external_declaration;
        ed->fd = fd;
        return ed;
    }
    return nullptr;
}

translation_unit* parser::parse_translation_unit()
{
    translation_unit* root = new translation_unit;
    while (tokit != tokens.end())
        root->ed.push_back(parse_external_declaration());
    return root;
}
